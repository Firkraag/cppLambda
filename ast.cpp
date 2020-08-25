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
using std::shared_ptr;
using std::to_string;

bool OptimizeChange = true;

static bool is_constant_var(const VarAst &var_ast) {
    auto define = var_ast.define.lock();
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
static shared_ptr<Ast> make_continuation(function<shared_ptr<Ast>(shared_ptr<Ast>)> callback)
{
    auto continuation = generate_unique_symbol("R");
    return make_shared<LambdaAst>(
        "",
        vector<string>{continuation},
        callback(make_shared<VarAst>(continuation)));
}
static auto make_lambda_callback(Environment &env, LambdaAst &lambda_ast)
{
    auto lambda_function = [&env, &lambda_ast](function<void(shared_ptr<Object>)> callback, const vector<shared_ptr<Object>> &args) {
        int index = 0;
        auto scope = env.extend();
        assert(lambda_ast.get_params().size() >= args.size());
        for (auto arg : args)
        {
            scope->define(lambda_ast.get_params()[index++], arg);
        }
        for (size_t i = index; i < lambda_ast.get_params().size(); i++)
        {
            scope->define(lambda_ast.get_params()[i], FALSE);
        }

        return lambda_ast.get_body()->evaluate_callback(scope, callback);
    };
    // std::function<shared_ptr<Object>(auto... args)> a;
    // function<shared_ptr<Object>(initializer_list<shared_ptr<Object>>)> a;
    // a = lambda_function;
    auto lambda = make_shared<FunctionCallback>(lambda_function);
    if (!lambda_ast.get_name().empty())
    {
        env = *(env.extend());
        env.define(lambda_ast.get_name(), lambda);
    }
    return lambda;
}
static auto make_lambda(Environment &env, const LambdaAst &lambda_ast)
{
    auto lambda_function = [&env, &lambda_ast](const vector<shared_ptr<Object>> &args) {
        int index = 0;
        auto scope = env.extend();
        // (cout << ... << args) << endl;
        // scope->define(lambda_ast.get_params()[index])
        assert(lambda_ast.get_params().size() >= args.size());
        for (auto arg : args)
        {
            scope->define(lambda_ast.get_params()[index++], arg);
        }
        for (size_t i = index; i < lambda_ast.get_params().size(); i++)
        {
            scope->define(lambda_ast.get_params()[i], FALSE);
        }

        return lambda_ast.get_body()->evaluate(scope);
    };
    // std::function<shared_ptr<Object>(auto... args)> a;
    // function<shared_ptr<Object>(initializer_list<shared_ptr<Object>>)> a;
    // a = lambda_function;
    auto lambda = make_shared<Function>(lambda_function);
    if (!lambda_ast.get_name().empty())
    {
        env = *(env.extend());
        env.define(lambda_ast.get_name(), lambda);
    }
    return lambda;
}
static string make_var(const string &name)
{
    return name;
}
static bool is_bool(const Ast &ast)
{
    const BinaryAst *binary_ast = dynamic_cast<const BinaryAst *>(&ast);

    if (binary_ast != nullptr)
    {
        auto op = binary_ast->get_operator();
        if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=")
        {
            return true;
        }
        if (op == "&&" || op == "||")
        {
            return is_bool(*binary_ast->get_left()) && is_bool(*binary_ast->get_right());
        }
    }
    return false;
}

template <typename T>
static bool vector_pointer_equality(const vector<T> &v1, const vector<T> &v2)
{
    return std::equal(v1.begin(), v1.end(), v2.begin(), v2.end(), [](const T &p1, const T &p2) {
        return *p1 == *p2;
        });
}
// bool Ast::operator==(const Ast &ast) const
// {
//     // cout << "Ast comparasion equality" << endl;
//     return false;
// }
// ostream &Ast::operator<<(ostream &output)
// {
//     output << "Ast()";
//     return output;
// }

shared_ptr<Ast> NumberAst::to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)> callback)
{
    return callback(shared_from_this());
}
void NumberAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback)
{
    callback(value);
}
shared_ptr<Object> NumberAst::evaluate(shared_ptr<Environment> env)
{
    return value;
}
bool NumberAst::operator==(const Ast &ast) const
{
    const NumberAst *number_ast = dynamic_cast<const NumberAst *>(&ast);
    return number_ast != NULL && *(number_ast->value) == *value;
}

string NumberAst::to_js()
{
    return value->to_string();
}

shared_ptr<Ast> BooleanAst::to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)> callback)
{
    return callback(shared_from_this());
}
void BooleanAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback)
{
    callback(value);
}
shared_ptr<Object> BooleanAst::evaluate(shared_ptr<Environment> env)
{
    return value;
}
bool BooleanAst::operator==(const Ast &ast) const
{
    const BooleanAst *boolean_ast = dynamic_cast<const BooleanAst *>(&ast);
    return boolean_ast != NULL && *(boolean_ast->value) == *value;
}
string BooleanAst::to_js()
{
    return value ? "true" : "false";
}

// ostream &BooleanAst::operator<<(ostream &output)
// {
//     output << "BooleanAst(value=" << value << ")";
//     return output;
// }
shared_ptr<Ast> StringAst::to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)> callback)
{
    return callback(shared_from_this());
}
void StringAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback)
{
    callback(value);
}
shared_ptr<Object> StringAst::evaluate(shared_ptr<Environment> env)
{
    return value;
}
bool StringAst::operator==(const Ast &ast) const
{
    const StringAst *string_ast = dynamic_cast<const StringAst *>(&ast);
    return string_ast != NULL && *(string_ast->value) == *value;
}
string StringAst::to_js()
{
    return "\"" + value->to_string() + "\"";
}

// ostream &StringAst::operator<<(ostream &output)
// {
//     output << "String(value=" << value << ")";
//     return output;
// }

void VarAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    shared_ptr<VarDefine> var_define;
    auto scope = closure_environment->lookup(name);
    if (scope == nullptr)
    {
        var_define = make_shared<VarDefine>(VarKind::GlobalVar);
        global_environment->define(name, var_define);
    }
    else
    {
        var_define = scope->get(name);
    }
    var_define->refs.push_back(std::dynamic_pointer_cast<VarAst>(shared_from_this()));
    this->define = var_define;
}
shared_ptr<Ast> VarAst::to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)> callback)
{
    return callback(shared_from_this());
}
void VarAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback)
{
    callback(environment->get(name));
}
shared_ptr<Object> VarAst::evaluate(shared_ptr<Environment> environment)
{
    return environment->get(name);
}
bool VarAst::operator==(const Ast &ast) const
{
    const VarAst *var_ast = dynamic_cast<const VarAst *>(&ast);
    return var_ast != NULL && var_ast->name == name;
}

string VarAst::to_js()
{
    return make_var(name);
}
// ostream &VarAst::operator<<(ostream &output)
// {
//     output << "VarAst(name=" << name << ")";
//     return output;
// }

bool VarDef::operator==(const VarDef &vardef_ast) const
{
    // const VarDefAst *vardef_ast = dynamic_cast<const VarDefAst *>(&ast);
    if (name != vardef_ast.name)
    {
        return false;
    }
    if (define == nullptr)
    {
        if (vardef_ast.define == nullptr)
        {
            return true;
        }
        return false;
    }
    else if (vardef_ast.define == nullptr)
    {
        return false;
    }
    else
    {
        return *define == *(vardef_ast.define);
    }
}

void LambdaAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    // this->closure = closure;
    // auto closure_env = closure == nullptr ? global_environment : closure->get_env();
    auto current_environment = env = closure_environment->extend();
    if (!name.empty())
    {
        current_environment->define(name, make_shared<VarDefine>(VarKind::LambdaParam));
    }
    for (auto &&param : params)
    {
        current_environment->define(param, make_shared<VarDefine>(VarKind::LambdaParam));
    }
    for (auto &&param : iife_params)
    {
        current_environment->define(param, make_shared<VarDefine>(VarKind::IifeParam));
    }

    body->make_scope(current_environment, global_environment);
}
shared_ptr<Ast> LambdaAst::optimize(shared_ptr<LambdaAst> closure)
{
    const CallAst* lambda_body = dynamic_cast<const CallAst *>(body.get());
    if (lambda_body != nullptr)
    {
        auto& args = lambda_body->get_args();
        int index = 0;
        if (args.size() == params.size() && all_of(args.begin(), args.end(), [&index, this](shared_ptr<Ast> arg) {

            const VarAst * lambda_body_arg = dynamic_cast<const VarAst *>(arg.get());
            return lambda_body_arg != nullptr && lambda_body_arg->get_name() == params[index++];
            })) {
            auto func = lambda_body->get_func();
            const VarAst * lambda_body_func = dynamic_cast<const VarAst *>(func.get());
            if (lambda_body_func != nullptr && std::find(params.begin(), params.end(), lambda_body_func->get_name()) == params.end())
            {
                if (is_constant_var(*lambda_body_func))
                {
                    OptimizeChange = true;
                    return func->optimize(closure);
                }
            }
        }
    }
    auto optimized_body = body->optimize(std::dynamic_pointer_cast<LambdaAst>(shared_from_this()));
    vector<string> optimized_iife_params;
    for (auto &&iife_param : iife_params)
    {
        if (!env->get(iife_param)->refs.empty())
        {
            optimized_iife_params.push_back(iife_param);
        }
    }
    return make_shared<LambdaAst>(name, params, optimized_body, optimized_iife_params);
}
shared_ptr<Ast> LambdaAst::to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)> callback)
{
    auto continuation = generate_unique_symbol("K");
    auto lambda_body = body->to_cps([&continuation](shared_ptr<Ast> lambda_body) {
        return make_shared<CallAst>(make_shared<VarAst>(continuation), vector<shared_ptr<Ast>>{lambda_body});
        });
    vector<string> lambda_params{ continuation };
    for (auto &&param : params)
    {
        lambda_params.push_back(param);
    }

    return callback(make_shared<LambdaAst>(name, lambda_params, lambda_body));
}
void LambdaAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback)
{
    callback(make_lambda_callback(*environment, *this));
}
shared_ptr<Object> LambdaAst::evaluate(shared_ptr<Environment> environment)
{
    return make_lambda(*environment, *this);
}
bool LambdaAst::operator==(const Ast &ast) const
{
    const LambdaAst *lambda_ast = dynamic_cast<const LambdaAst *>(&ast);
    return lambda_ast != NULL && name == lambda_ast->get_name() && params == lambda_ast->params && *body == *(lambda_ast->body);
}

string LambdaAst::to_js()
{
    string code = "(function ";
    string func_name = name.empty() ? generate_unique_symbol("anonymous") : name;
    code += make_var(func_name);
    vector<string> js_param_list;
    for (auto &&var : params)
    {
        js_param_list.push_back(make_var(var));
    }

    code += "(" + join(js_param_list.begin(), js_param_list.end(), ", ") + ") {";
    if (!iife_params.empty()) {
        code += "let " + join(iife_params.begin(), iife_params.end(), ", ") + ";";
    }
    code += "GUARD(arguments, " + func_name + ");";
    code += "return " + body->to_js() + " })";
    return code;
}

// ostream &LambdaAst::operator<<(ostream &output)
// {
//     output << "LambdaAst(name=" << name << ", params=[";
//     for (auto &&param : params)
//     {
//         output << param << ", ";
//     }
//     output << "], body=" << body << ")";
//     return output;
// }

shared_ptr<Ast> LetAst::to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)> callback)
{
    if (vardefs.empty())
    {
        return body->to_cps(callback);
    }
    shared_ptr<LambdaAst> lambda_ast = make_shared<LambdaAst>(
        "",
        vector<string>{vardefs[0]->name},
        make_shared<LetAst>(vector<shared_ptr<VarDef>>(vardefs.begin() + 1, vardefs.end()), body));

    shared_ptr<Ast> arg = (vardefs[0]->define == nullptr) ? make_shared<BooleanAst>(false) : vardefs[0]->define;
    shared_ptr<Ast> call_ast = make_shared<CallAst>(lambda_ast, vector<shared_ptr<Ast>>{arg});
    return call_ast->to_cps(callback);
}

void LetAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback)
{
    function<void(shared_ptr<Environment>, int)> loop = [this, &loop, &callback](shared_ptr<Environment> environment, int i) {
        if (i < vardefs.size())
        {
            auto vardef = vardefs[i];
            auto scope = environment->extend();
            if (vardef->define)
            {
                vardef->define->evaluate_callback(environment, [&i, &scope, &loop, &vardef](shared_ptr<Object> value) {
                    scope->define(vardef->name, value);
                    loop(scope, i + 1);
                    });
            }
            else
            {
                scope->define(vardef->name, FALSE);
                loop(scope, i + 1);
            }
        }
        else
        {
            body->evaluate_callback(environment, callback);
        }
    };
    loop(environment, 0);
}
shared_ptr<Object> LetAst::evaluate(shared_ptr<Environment> environment)
{
    for (auto &&vardef : vardefs)
    {
        auto scope = environment->extend();
        if (vardef->define)
        {
            scope->define(vardef->name, vardef->define->evaluate(scope));
        }
        else
        {
            scope->define(vardef->name, FALSE);
        }
        environment = scope;
    }
    return body->evaluate(environment);
}
bool LetAst::operator==(const Ast &ast) const
{
    const LetAst *let_ast = dynamic_cast<const LetAst *>(&ast);
    return let_ast != NULL and *(let_ast->body) == *body and vector_pointer_equality(vardefs, let_ast->vardefs);
}
string LetAst::to_js()
{
    if (vardefs.empty())
    {
        return body->to_js();
    }
    vector<string> lambda_params ={ vardefs[0]->name };
    shared_ptr<LetAst> lambda_body = make_shared<LetAst>(vector<shared_ptr<VarDef>>(vardefs.begin() + 1, vardefs.end()), body);
    auto lambda_ast = make_shared<LambdaAst>("", lambda_params, lambda_body);
    vector<shared_ptr<Ast>> iife_args ={ vardefs[0]->define == nullptr ? make_shared<BooleanAst>(false) : vardefs[0]->define };
    // auto iife = make_shared<CallAst>(lambda_ast, iife_args);
    return "(" + CallAst(lambda_ast, iife_args).to_js() + ")";
}

void CallAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    func->make_scope(closure_environment, global_environment);
    for (auto &&arg : args)
    {
        arg->make_scope(closure_environment, global_environment);
    }
}
static shared_ptr<Ast> unwrap_iife(shared_ptr<CallAst> iife_ast, shared_ptr<LambdaAst> closure)
{
    vector<shared_ptr<Ast>> prog;
    const LambdaAst *iife_func = dynamic_cast<const LambdaAst *>(iife_ast->get_func().get());
    auto iife_func_params = iife_func->get_params();
    auto iife_args = iife_ast->get_args();
    assert(iife_func_params.size() >= iife_args.size());
    auto rename_iife_param = [&closure, &iife_func](const string &param) {
        auto closure_env = closure->get_env();
        auto renamed_param = closure_env->contains(param) ? generate_unique_symbol(param) : param;
        auto var_define = iife_func->get_env()->get(param);
        closure_env->define(renamed_param, var_define);
        vector<string>& closure_iife_params = closure->get_iife_params();
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
        prog.push_back(make_shared<AssignAst>(
            make_shared<VarAst>(rename_iife_param(iife_func_params[index])),
            iife_args[index]));
    }
    while (index < iife_func_params.size())
    {
        prog.push_back(make_shared<AssignAst>(
            make_shared<VarAst>(rename_iife_param(iife_func_params[index++])),
            make_shared<BooleanAst>(false)));
    }
    prog.push_back(iife_func->get_body()->optimize(closure));
    shared_ptr<Ast> result= make_shared<ProgAst>(prog);
    // cout << "\niife_unwrap\n" << result->to_js() << "\niife_unwrap\n"<< endl;
    return result;
}
shared_ptr<Ast> CallAst::optimize(shared_ptr<LambdaAst> closure)
{
    const LambdaAst *lambda_ast = dynamic_cast<const LambdaAst *>(func.get());
    if (lambda_ast != nullptr && lambda_ast->get_name().empty())
    {
        if (closure != nullptr)
        {
            OptimizeChange = true;
            return unwrap_iife(std::dynamic_pointer_cast<CallAst>(this->shared_from_this()), closure);
        }
    }
    vector<shared_ptr<Ast>> args;
    for (auto &&arg : this->args)
    {
        args.push_back(arg->optimize(closure));
    }
    return make_shared<CallAst>(func->optimize(closure), args);
}
shared_ptr<Ast> CallAst::to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)> callback)
{
    return func->to_cps([this, &callback](shared_ptr<Ast> func) {
        vector<shared_ptr<Ast>> args{ make_continuation(callback) };
        function<shared_ptr<Ast>(int)> loop = [&loop, &func, this, &args](int i) -> shared_ptr<Ast> {
            function<shared_ptr<Ast>(shared_ptr<Ast>)> arg_callback = [&i, &loop, &args](shared_ptr<Ast> arg) {
                args.push_back(arg);
                return loop(i + 1);
            };
            if (i == this->args.size())
            {
                return make_shared<CallAst>(func, args);
            }
            return this->args[i]->to_cps(arg_callback);
        };
        return loop(0);
        });
}
void CallAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback)
{
    func->evaluate_callback(environment, [this, &callback, &environment](shared_ptr<Object> func) {
        vector<shared_ptr<Object>> function_args;
        function<void(int)> loop = [this, &environment, &function_args, &loop, &func, &callback](int i) {
            auto arg_callback = [&i, &function_args, &loop](shared_ptr<Object> arg) {
                function_args.push_back(arg);
                loop(i + 1);
            };
            if (i < args.size())
            {
                args[i]->evaluate_callback(environment, arg_callback);
            }
            else
            {
                (*func)(callback, function_args);
            }
        };
        loop(0);
        });
    // func->evaluate_callback(environment, [this](function<) {
    //     auto loop = [this](int i) {
    //         if (i < args.size())
    //         {
    //             args[i]->evaluate_callback(environment, [&i, ]() {

    //             });
    //         }
    //         else {

    //         }

    //     };

    //     loop(0);
    // });
}
shared_ptr<Object> CallAst::evaluate(shared_ptr<Environment> environment)
{
    auto function = func->evaluate(environment);
    vector<shared_ptr<Object>> function_args;
    for (auto arg : args)
    {
        function_args.push_back(arg->evaluate(environment));
    }
    return (*function)(function_args);
}
bool CallAst::operator==(const Ast &ast) const
{
    const CallAst *call_ast = dynamic_cast<const CallAst *>(&ast);
    return call_ast != NULL && *(call_ast->func) == *func && vector_pointer_equality(args, call_ast->args);
}
string CallAst::to_js()
{
    string func_code = func->to_js();
    vector<string> args_js_list;
    for (auto &&arg : args)
    {
        args_js_list.push_back(arg->to_js());
    }
    return func_code + "(" + join(args_js_list.begin(), args_js_list.end(), ", ") + ")";
}

static shared_ptr<Ast> optimize_aux(vector<shared_ptr<Ast>>::iterator begin, vector<shared_ptr<Ast>>::iterator end, shared_ptr<LambdaAst> closure)
{
    if (begin == end)
    {
        OptimizeChange = true;
        return make_shared<BooleanAst>(false);
    }
    if (end - begin == 1)
    {
        return (*begin)->optimize(closure);
    }
    if ((*begin)->has_side_effect())
    {
        return make_shared<ProgAst>(vector<shared_ptr<Ast>>{(*begin)->optimize(closure), optimize_aux(begin + 1, end, closure)});
    }
    OptimizeChange = true;
    return optimize_aux(begin + 1, end, closure);
}
void ProgAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    for (auto &&item : prog)
    {
        item->make_scope(closure_environment, global_environment);
    }
}
shared_ptr<Ast> ProgAst::optimize(shared_ptr<LambdaAst> closure)
{
    return optimize_aux(prog.begin(), prog.end(), closure);
}
shared_ptr<Ast> ProgAst::to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)> callback)
{
    function<shared_ptr<Ast>(vector<shared_ptr<Ast>>::iterator)> cps_body = [this, &cps_body, &callback](vector<shared_ptr<Ast>>::iterator start) {
        if (this->prog.end() == start)
        {
            return callback(make_shared<BooleanAst>(false));
        }
        if (this->prog.end() - start == 1)
        {
            return (*start)->to_cps(callback);
        }
        return (*start)->to_cps([&cps_body, &start](shared_ptr<Ast> first) {
            return make_shared<ProgAst>(vector<shared_ptr<Ast>>{first, cps_body(start + 1)});
            });
    };
    return cps_body(prog.begin());
}
void ProgAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback)
{
    function<void(shared_ptr<Object> last, int i)> loop = [this, &loop, &environment, &callback](shared_ptr<Object> last, int i) {
        if (i < prog.size())
        {
            prog[i]->evaluate_callback(environment, [&loop, &i](shared_ptr<Object> value) {
                loop(value, i + 1);
                });
        }
        else
        {
            callback(last);
        }
    };
    loop(FALSE, 0);
}
shared_ptr<Object> ProgAst::evaluate(shared_ptr<Environment> environment)
{
    shared_ptr<Object> result = FALSE;
    for (auto expr : prog)
    {
        result = expr->evaluate(environment);
    }
    return result;
}
bool ProgAst::operator==(const Ast &ast) const
{
    const ProgAst *prog_ast = dynamic_cast<const ProgAst *>(&ast);
    return prog_ast != NULL && vector_pointer_equality(prog, prog_ast->prog);
}
string ProgAst::to_js()
{
    if (prog.empty())
    {
        return "(false)";
    }
    vector<string> prog_js_list;
    for (auto &&exp : prog)
    {
        prog_js_list.push_back(exp->to_js());
    }
    return "(" + join(prog_js_list.begin(), prog_js_list.end(), ", ") + ")";
}

// def cps_cond(cond_ast: Ast) -> Ast:
//     def cps_then_and_else(result: Ast) -> Ast:
//         return CallAst(VarAst(if_continuation), [result])

//     return IfAst(
//         cond_ast,
//         to_cps(if_ast.then, cps_then_and_else),
//         to_cps(if_ast.else_, cps_then_and_else))

// if_continuation = gensym("I")
// return CallAst(
//     LambdaAst('', [if_continuation], to_cps(if_ast.cond, cps_cond)),
//     [_make_continuation(k)])
void IfAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    cond->make_scope(closure_environment, global_environment);
    then->make_scope(closure_environment, global_environment);
    else_->make_scope(closure_environment, global_environment);
}
shared_ptr<Ast> IfAst::optimize(shared_ptr<LambdaAst> closure)
{
    auto optimized_cond = cond->optimize(closure);
    auto optimized_then = then->optimize(closure);
    auto optimized_else = else_->optimize(closure);
    const LiteralAst *condition = dynamic_cast<const LiteralAst *>(optimized_cond.get());
    if (condition != nullptr)
    {
        OptimizeChange = true;
        return *(condition->get_value()) ? optimized_then : optimized_else;
    }
    const VarAst * cond_var = dynamic_cast<const VarAst *>(optimized_cond.get());
    if (cond_var != nullptr && is_constant_var(*cond_var))
    {
        shared_ptr<Ast> current_value = cond_var->define.lock()->current_value.lock();
        if (current_value != nullptr) {
            if (*current_value == BooleanAst(false))
            {
                OptimizeChange = true;
                return optimized_else;
            }
            if (dynamic_cast<const LiteralAst *>(current_value.get()) != nullptr)
            {
                OptimizeChange = true;
                return optimized_then;
            }
            if (dynamic_cast<const LambdaAst *>(current_value.get()) != nullptr)
            {
                OptimizeChange = true;
                return optimized_then;
            }
        }
    }
    return make_shared<IfAst>(optimized_cond, optimized_then, optimized_else);
}
shared_ptr<Ast> IfAst::to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)> callback)
{
    auto if_continuation = generate_unique_symbol("I");
    auto cps_cond = [&if_continuation, this](shared_ptr<Ast> cond_ast) {
        auto cps_then_and_else = [&if_continuation](shared_ptr<Ast> result) {
            return make_shared<CallAst>(
                make_shared<VarAst>(if_continuation),
                vector<shared_ptr<Ast>>{result});
        };
        return make_shared<IfAst>(cond_ast, then->to_cps(cps_then_and_else), else_->to_cps(cps_then_and_else));
    };
    return make_shared<CallAst>(make_shared<LambdaAst>("", vector<string>{if_continuation}, cond->to_cps(cps_cond)), vector<shared_ptr<Ast>>{make_continuation(callback)});
}
void IfAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback)
{
    cond->evaluate_callback(environment, [this, &environment, &callback](shared_ptr<Object> cond) {
        if (*cond == *FALSE)
        {
            else_->evaluate_callback(environment, callback);
        }
        else
        {
            then->evaluate_callback(environment, callback);
        }
        });
}
shared_ptr<Object> IfAst::evaluate(shared_ptr<Environment> environment)
{
    auto condition = cond->evaluate(environment);
    if (*condition)
    {
        return then->evaluate(environment);
    }
    return else_->evaluate(environment);
}
bool IfAst::operator==(const Ast &ast) const
{
    const IfAst *if_ast = dynamic_cast<const IfAst *>(&ast);
    return if_ast != NULL && *(if_ast->cond) == *cond && *(if_ast->then) == *then && *(if_ast->else_) == *else_;
}
string IfAst::to_js()
{
    string cond_code = cond->to_js();
    string then_code = then->to_js();
    string else_code = else_->to_js();

    if (!is_bool(*cond))
    {
        cond_code += " !== false";
    }
    return "(" + cond_code + " ? " + then_code + " : " + else_code + ")";
}

void BinaryAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    left->make_scope(closure_environment, global_environment);
    right->make_scope(closure_environment, global_environment);
}
shared_ptr<Ast> BinaryAst::optimize(shared_ptr<LambdaAst> closure)
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
            OptimizeChange = true;
            return make_shared<NumberAst>(number->get_value());
        }
        const Boolean *boolean = dynamic_cast<const Boolean *>(result.get());
        if (boolean != nullptr)
        {
            OptimizeChange = true;
            return make_shared<BooleanAst>(boolean->get_value());
        }
        const String *string = dynamic_cast<const String *>(result.get());
        if (string != nullptr)
        {
            OptimizeChange = true;
            return make_shared<StringAst>(string->get_value());
        }
    }
    return make_shared<BinaryAst>(operator_, optimized_left, optimized_right);
}
shared_ptr<Ast> BinaryAst::to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)> callback)
{
    return left->to_cps([this, &callback](shared_ptr<Ast> left_ast) {
        return right->to_cps([this, &left_ast, &callback](shared_ptr<Ast> right_ast) {
            return callback(make_shared<BinaryAst>(operator_, left_ast, right_ast));
            });
        });
}
void BinaryAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback)
{
    left->evaluate_callback(environment, [this, &environment, &callback](shared_ptr<Object> left_value) {
        right->evaluate_callback(environment, [this, &callback, &left_value](shared_ptr<Object> right_value) {
            callback(apply_op(operator_, left_value, right_value));
            });
        });
}
shared_ptr<Object> BinaryAst::evaluate(shared_ptr<Environment> environment)
{
    return apply_op(operator_, left->evaluate(environment), right->evaluate(environment));
}
bool BinaryAst::operator==(const Ast &ast) const
{
    const BinaryAst *binary_ast = dynamic_cast<const BinaryAst *>(&ast);
    return binary_ast != NULL && *(binary_ast->left) == *left && *(binary_ast->right) == *right && binary_ast->operator_ == operator_;
}
string BinaryAst::to_js()
{
    return left->to_js() + " " + operator_ + " " + right->to_js();
}

void AssignAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    left->make_scope(closure_environment, global_environment);
    right->make_scope(closure_environment, global_environment);
    const VarAst *left_ast = dynamic_cast<const VarAst *>(left.get());
    if (left_ast != nullptr)
    {
        auto define = left_ast->define.lock();
        define->assigned++;
        define->current_value = right;
    }

}
shared_ptr<Ast> AssignAst::optimize(shared_ptr<LambdaAst> closure)
{
    const VarAst *left_ast = dynamic_cast<const VarAst *>(left.get());
    if (left_ast != nullptr)
    {
        auto define = left_ast->define.lock();
        if (define->refs.size() == define->assigned)
        {
            OptimizeChange = true;
            return right->optimize(closure);
        }
        if (is_constant_var(*left_ast)) {
            const VarAst * right_ast = dynamic_cast<const VarAst *>(right.get());
            if (right_ast != nullptr && is_constant_var(*right_ast))
            {
                OptimizeChange = true;
                for (auto &&ref : left_ast->define.lock()->refs)
                {
                    ref->set_name(right_ast->get_name());
                }
                return right->optimize(closure);
            }
        }
    }
    auto optimized_left = left->optimize(closure);
    auto optimized_right = right->optimize(closure);
    return make_shared<AssignAst>(optimized_left, optimized_right);
}
shared_ptr<Ast> AssignAst::to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)> callback)
{
    return left->to_cps([this, &callback](shared_ptr<Ast> left_ast) {
        return right->to_cps([&left_ast, &callback](shared_ptr<Ast> right_ast) {
            return callback(make_shared<AssignAst>(left_ast, right_ast));
            });
        });
}
void AssignAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback)
{
    const VarAst *left_node = dynamic_cast<const VarAst *>(left.get());
    if (left_node == nullptr)
    {
        throw "cannot assign to ";
    }
    right->evaluate_callback(environment, [&environment, &left_node, &callback](shared_ptr<Object> right) {
        callback(environment->set(left_node->get_name(), right));
        });
}
shared_ptr<Object> AssignAst::evaluate(shared_ptr<Environment> environment)
{
    const VarAst *left_node = dynamic_cast<const VarAst *>(left.get());
    if (left_node == nullptr)
    {
        throw "cannot assign to ";
    }

    return environment->set(left_node->get_name(), right->evaluate(environment));
}
bool AssignAst::operator==(const Ast &ast) const
{
    const AssignAst *assign_ast = dynamic_cast<const AssignAst *>(&ast);
    return assign_ast != NULL && *(assign_ast->left) == *left && *(assign_ast->right) == *right;
}
string AssignAst::to_js()
{
    return left->to_js() + " = " + right->to_js();
}
