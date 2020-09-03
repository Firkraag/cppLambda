#include <assert.h>
#include "ast.hpp"
#include "utils.hpp"
static auto make_lambda_callback(Environment &env, const LambdaAst &lambda_ast)
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

        return lambda_ast.get_body().evaluate_callback(scope, callback);
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
void NumberAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const
{
    callback(value);
}
void BooleanAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const
{
    callback(value);
}
void StringAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const
{
    callback(value);
}
void VarAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const
{
    callback(environment->get(name));
}
void LambdaAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const
{
    callback(make_lambda_callback(*environment, *this));
}
void LetAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const
{
    function<void(shared_ptr<Environment>, int)> loop = [this, &loop, &callback](shared_ptr<Environment> environment, int i) {
        if (i < vardefs.size())
        {
            const VarDef &vardef = *vardefs[i];
            auto scope = environment->extend();
            if (vardef.define)
            {
                vardef.define->evaluate_callback(environment, [&i, &scope, &loop, &vardef](shared_ptr<Object> value) {
                    scope->define(vardef.name, value);
                    loop(scope, i + 1);
                });
            }
            else
            {
                scope->define(vardef.name, FALSE);
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
void CallAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const
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
                (*func)(callback, std::move(function_args));
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
void ProgAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const
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
void IfAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const
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
void BinaryAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const
{
    left->evaluate_callback(environment, [this, &environment, &callback](shared_ptr<Object> left_value) {
        right->evaluate_callback(environment, [this, &callback, &left_value](shared_ptr<Object> right_value) {
            callback(apply_op(operator_, left_value, right_value));
        });
    });
}
void AssignAst::evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const
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