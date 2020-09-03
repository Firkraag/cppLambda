#include "ast.hpp"
#include "utils.hpp"
#include <assert.h>

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
        return lambda_ast.get_body().evaluate(scope);
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

shared_ptr<Object> NumberAst::evaluate(shared_ptr<Environment> env) const
{
    return value;
}

shared_ptr<Object> BooleanAst::evaluate(shared_ptr<Environment> env) const
{
    return value;
}

shared_ptr<Object> StringAst::evaluate(shared_ptr<Environment> env) const
{
    return value;
}
shared_ptr<Object> VarAst::evaluate(shared_ptr<Environment> environment) const
{
    return environment->get(name);
}
shared_ptr<Object> LambdaAst::evaluate(shared_ptr<Environment> environment) const
{
    return make_lambda(*environment, *this);
}
shared_ptr<Object> LetAst::evaluate(shared_ptr<Environment> environment) const
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
shared_ptr<Object> CallAst::evaluate(shared_ptr<Environment> environment) const
{
    auto function = func->evaluate(environment);
    vector<shared_ptr<Object>> function_args;
    for (auto &arg : args)
    {
        function_args.push_back(arg->evaluate(environment));
    }
    return (*function)(std::move(function_args));
}
shared_ptr<Object> ProgAst::evaluate(shared_ptr<Environment> environment) const
{
    shared_ptr<Object> result = FALSE;
    for (auto &expr : prog)
    {
        result = expr->evaluate(environment);
    }
    return result;
}
shared_ptr<Object> IfAst::evaluate(shared_ptr<Environment> environment) const
{
    auto condition = cond->evaluate(environment);
    if (*condition)
    {
        return then->evaluate(environment);
    }
    return else_->evaluate(environment);
}
shared_ptr<Object> BinaryAst::evaluate(shared_ptr<Environment> environment) const
{
    return apply_op(operator_, left->evaluate(environment), right->evaluate(environment));
}
shared_ptr<Object> AssignAst::evaluate(shared_ptr<Environment> environment) const
{
    const VarAst *left_node = dynamic_cast<const VarAst *>(left.get());
    if (left_node == nullptr)
    {
        throw "cannot assign to ";
    }

    return environment->set(left_node->get_name(), right->evaluate(environment));
}