#include "utils.hpp"
#include "ast.hpp"
#include "object.hpp"
#include <assert.h>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <initializer_list>
#include <functional>
#include <algorithm>
using std::function;
using std::initializer_list;
using std::make_shared;
using std::make_unique;
using std::shared_ptr;
using std::to_string;
using std::unique_ptr;

bool OptimizeChange = true;

static bool is_constant_var(const VarAst &var_ast)
{
    auto& define = var_ast.define;
    switch (define->var_kind)
    {
    case VarKind::IifeParam:
    case VarKind::GlobalVar:
        return define->assigned == 1;
        break;
    case VarKind::LambdaParam:
        return define->assigned == 0;
        break;
    default:
        abort();
        break;
    }
}

unique_ptr<Ast> NumberAst::optimize(LambdaAst *closure)
{
    return make_unique<NumberAst>(value);
}

unique_ptr<Ast> BooleanAst::optimize(LambdaAst *closure)
{
    return make_unique<BooleanAst>(value);
}

unique_ptr<Ast> StringAst::optimize(LambdaAst *closure)
{
    return make_unique<StringAst>(value);
}
unique_ptr<Ast> VarAst::optimize(LambdaAst *closure)
{
    return make_unique<VarAst>(name);
}

unique_ptr<Ast> LambdaAst::optimize(LambdaAst *closure)
{
    CallAst *lambda_body = dynamic_cast<CallAst *>(body.get());
    if (lambda_body != nullptr)
    {
        auto &args = lambda_body->get_args();
        int index = 0;
        if (args.size() == params.size() && all_of(args.begin(), args.end(), [&index, this](const unique_ptr<Ast>& arg) {
                const VarAst *lambda_body_arg = dynamic_cast<const VarAst *>(arg.get());
                return lambda_body_arg != nullptr && lambda_body_arg->get_name() == params[index++];
            }))
        {
            auto& func = lambda_body->get_func();
            const VarAst *lambda_body_func = dynamic_cast<const VarAst *>(&func);
            if (lambda_body_func != nullptr && std::find(params.begin(), params.end(), lambda_body_func->get_name()) == params.end())
            {
                if (::is_constant_var(*lambda_body_func))
                {
                    ::OptimizeChange = true;
                    return func.optimize(closure);
                }
            }
        }
    }
    auto optimized_body = body->optimize(this);
    vector<string> optimized_iife_params;
    for (auto &&iife_param : iife_params)
    {
        if (!env->get(iife_param)->refs.empty())
        {
            optimized_iife_params.push_back(iife_param);
        }
    }
    return make_unique<LambdaAst>(name, params, std::move(optimized_body), optimized_iife_params);
}

unique_ptr<Ast> LetAst::optimize(LambdaAst *closure)
{
    return make_unique<LetAst>(std::move(vardefs), std::move(body));
}

static unique_ptr<Ast> unwrap_iife(CallAst &iife_ast, LambdaAst *closure)
{
    vector<unique_ptr<Ast>> prog;
    auto& func = iife_ast.get_func();
    LambdaAst *iife_func = dynamic_cast<LambdaAst *>(&func);
    auto iife_func_params = iife_func->get_params();
    auto& iife_args = iife_ast.get_args();
    assert(iife_func_params.size() >= iife_args.size());
    auto rename_iife_param = [&closure, &iife_func](const string &param) {
        auto closure_env = closure->get_env();
        auto renamed_param = closure_env->contains(param) ? generate_unique_symbol(param) : param;
        auto var_define = iife_func->get_env()->get(param);
        closure_env->define(renamed_param, var_define);
        vector<string> &closure_iife_params = closure->get_iife_params();
        closure_iife_params.push_back(renamed_param);
        for (auto &&ref : var_define->refs)
        {
            ref->set_name(renamed_param);
        }
        return renamed_param;
    };
    size_t index = 0;
    for (; index < iife_args.size(); index++)
    {
        prog.push_back(make_unique<AssignAst>(
            make_unique<VarAst>(rename_iife_param(iife_func_params[index])),
            std::move(iife_args[index])));
    }
    while (index < iife_func_params.size())
    {
        prog.push_back(make_unique<AssignAst>(
            make_unique<VarAst>(rename_iife_param(iife_func_params[index++])),
            make_unique<BooleanAst>(false)));
    }
    prog.push_back(iife_func->get_body().optimize(closure));
    return make_unique<ProgAst>(std::move(prog));
}
unique_ptr<Ast> CallAst::optimize(LambdaAst *closure)
{
    const LambdaAst *lambda_ast = dynamic_cast<const LambdaAst *>(func.get());
    if (lambda_ast != nullptr && lambda_ast->get_name().empty())
    {
        if (closure != nullptr)
        {
            ::OptimizeChange = true;
            // return unwrap_iife(std::dynamic_pointer_cast<CallAst>(this->shared_from_this()), closure);
            return unwrap_iife(*this, closure);
        }
    }
    vector<unique_ptr<Ast>> args;
    for (auto &&arg : this->args)
    {
        args.push_back(arg->optimize(closure));
    }
    return make_unique<CallAst>(func->optimize(closure), std::move(args));
}

static unique_ptr<Ast> optimize_aux(vector<unique_ptr<Ast>>::iterator begin, vector<unique_ptr<Ast>>::iterator end, LambdaAst *closure)
{
    if (begin == end)
    {
        ::OptimizeChange = true;
        return make_unique<BooleanAst>(false);
    }
    if (end - begin == 1)
    {
        return (*begin)->optimize(closure);
    }
    if ((*begin)->has_side_effect())
    {
        vector<unique_ptr<Ast>> prog;
        prog.push_back((*begin)->optimize(closure));
        prog.push_back(optimize_aux(begin + 1, end, closure));
        return make_unique<ProgAst>(std::move(prog));
    }
    ::OptimizeChange = true;
    return optimize_aux(begin + 1, end, closure);
}
unique_ptr<Ast> ProgAst::optimize(LambdaAst *closure)
{
    return optimize_aux(prog.begin(), prog.end(), closure);
}

unique_ptr<Ast> IfAst::optimize(LambdaAst *closure)
{
    auto optimized_cond = cond->optimize(closure);
    auto optimized_then = then->optimize(closure);
    auto optimized_else = else_->optimize(closure);
    const LiteralAst *condition = dynamic_cast<const LiteralAst *>(optimized_cond.get());
    if (condition != nullptr)
    {
        ::OptimizeChange = true;
        return *(condition->get_value()) ? std::move(optimized_then) : std::move(optimized_else);
    }
    const VarAst *cond_var = dynamic_cast<const VarAst *>(optimized_cond.get());
    if (cond_var != nullptr && is_constant_var(*cond_var))
    {
        const Ast* current_value = cond_var->define->current_value;
        if (current_value != nullptr)
        {
            if (*current_value == BooleanAst(false))
            {
                ::OptimizeChange = true;
                return optimized_else;
            }
            if (dynamic_cast<const LiteralAst *>(current_value) != nullptr)
            {
                ::OptimizeChange = true;
                return optimized_then;
            }
            if (dynamic_cast<const LambdaAst *>(current_value) != nullptr)
            {
                ::OptimizeChange = true;
                return optimized_then;
            }
        }
    }
    return make_unique<IfAst>(std::move(optimized_cond), std::move(optimized_then), std::move(optimized_else));
}

unique_ptr<Ast> BinaryAst::optimize(LambdaAst *closure)
{
    auto optimized_left = left->optimize(closure);
    auto optimized_right = right->optimize(closure);
    const LiteralAst *left = dynamic_cast<const LiteralAst *>(optimized_left.get());
    const LiteralAst *right = dynamic_cast<const LiteralAst *>(optimized_right.get());
    if (left != nullptr && right != nullptr)
    {
        auto result = apply_op(operator_, left->get_value(), right->get_value());
        const Double *number = dynamic_cast<const Double *>(result.get());
        if (number != nullptr)
        {
            ::OptimizeChange = true;
            return make_unique<NumberAst>(number->get_value());
        }
        const Boolean *boolean = dynamic_cast<const Boolean *>(result.get());
        if (boolean != nullptr)
        {
            ::OptimizeChange = true;
            return make_unique<BooleanAst>(boolean->get_value());
        }
        const String *string = dynamic_cast<const String *>(result.get());
        if (string != nullptr)
        {
            ::OptimizeChange = true;
            return make_unique<StringAst>(string->get_value());
        }
    }
    return make_unique<BinaryAst>(operator_, std::move(optimized_left), std::move(optimized_right));
}

unique_ptr<Ast> AssignAst::optimize(LambdaAst *closure)
{
    const VarAst *left_ast = dynamic_cast<const VarAst *>(left.get());
    if (left_ast != nullptr)
    {
        if (left_ast->define->refs.size() == left_ast->define->assigned)
        {
            ::OptimizeChange = true;
            return right->optimize(closure);
        }
        if (is_constant_var(*left_ast))
        {
            const VarAst *right_ast = dynamic_cast<const VarAst *>(right.get());
            if (right_ast != nullptr && is_constant_var(*right_ast))
            {
                ::OptimizeChange = true;
                for (auto &&ref : left_ast->define->refs)
                {
                    ref->set_name(right_ast->get_name());
                }
                return right->optimize(closure);
            }
        }
    }
    auto optimized_left = left->optimize(closure);
    auto optimized_right = right->optimize(closure);
    return make_unique<AssignAst>(std::move(optimized_left), std::move(optimized_right));
}
