#include <memory>
#include "ast.hpp"
#include "utils.hpp"
using std::make_unique;
using std::unique_ptr;
static unique_ptr<Ast> make_continuation(function<unique_ptr<Ast>(unique_ptr<Ast>)> callback)
{
    auto continuation = generate_unique_symbol("R");
    return make_unique<LambdaAst>(
        "",
        vector<string>{continuation},
        callback(make_unique<VarAst>(continuation)));
}
unique_ptr<Ast> NumberAst::to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)> callback)
{
    return callback(make_unique<NumberAst>(value));
}
unique_ptr<Ast> BooleanAst::to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)> callback)
{
    return callback(make_unique<BooleanAst>(value));
}
unique_ptr<Ast> StringAst::to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)> callback)
{
    return callback(make_unique<StringAst>(value));
}
unique_ptr<Ast> VarAst::to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)> callback)
{
    return callback(make_unique<VarAst>(name));
}
unique_ptr<Ast> LambdaAst::to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)> callback)
{
    auto continuation = generate_unique_symbol("K");
    auto lambda_body = body->to_cps([&continuation](unique_ptr<Ast> lambda_body) {
        vector<unique_ptr<Ast>> args;
        args.push_back(std::move(lambda_body));
        return make_unique<CallAst>(make_unique<VarAst>(continuation), std::move(args));
    });
    vector<string> lambda_params{continuation};
    for (auto &&param : params)
    {
        lambda_params.push_back(param);
    }

    return callback(make_unique<LambdaAst>(name, lambda_params, std::move(lambda_body)));
}
unique_ptr<Ast> LetAst::to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)> callback)
{
    if (vardefs.empty())
    {
        return body->to_cps(callback);
    }
    vector<unique_ptr<VarDef>> lambda_ast_vardefs;
    for (auto vardef = vardefs.begin() + 1; vardef < vardefs.end(); vardef++)
    {
        lambda_ast_vardefs.push_back(std::move(*vardef));
    }
    unique_ptr<LambdaAst>
        lambda_ast = make_unique<LambdaAst>(
            "",
            vector<string>{vardefs[0]->name},
            make_unique<LetAst>(std::move(lambda_ast_vardefs), std::move(body)));

    unique_ptr<Ast> arg = (vardefs[0]->define == nullptr) ? make_unique<BooleanAst>(false) : std::move(vardefs[0]->define);
    vector<unique_ptr<Ast>> args;
    args.push_back(std::move(arg));
    return CallAst(std::move(lambda_ast), std::move(args)).to_cps(callback);
}
unique_ptr<Ast> CallAst::to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)> callback)
{
    return func->to_cps([this, &callback](unique_ptr<Ast> func) {
        vector<unique_ptr<Ast>> args;
        args.push_back(make_continuation(callback));
        function<unique_ptr<Ast>(int)> loop = [&loop, &func, this, &args](int i) -> unique_ptr<Ast> {
            function<unique_ptr<Ast>(unique_ptr<Ast>)> arg_callback = [&i, &loop, &args](unique_ptr<Ast> arg) {
                args.push_back(std::move(arg));
                return loop(i + 1);
            };
            if (i == this->args.size())
            {
                return make_unique<CallAst>(std::move(func), std::move(args));
            }
            return this->args[i]->to_cps(arg_callback);
        };
        return loop(0);
    });
}
unique_ptr<Ast> ProgAst::to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)> callback)
{
    function<unique_ptr<Ast>(vector<unique_ptr<Ast>>::iterator)> cps_body = [this, &cps_body, &callback](vector<unique_ptr<Ast>>::iterator start) {
        if (this->prog.end() == start)
        {
            return callback(make_unique<BooleanAst>(false));
        }
        if (this->prog.end() - start == 1)
        {
            return (*start)->to_cps(callback);
        }
        return (*start)->to_cps([&cps_body, &start](unique_ptr<Ast> first) {
            vector<unique_ptr<Ast>> prog;
            prog.push_back(std::move(first));
            prog.push_back(cps_body(start + 1));
            return make_unique<ProgAst>(std::move(prog));
        });
    };
    return cps_body(prog.begin());
}
unique_ptr<Ast> IfAst::to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)> callback)
{
    auto if_continuation = generate_unique_symbol("I");
    auto cps_cond = [&if_continuation, this](unique_ptr<Ast> cond_ast) {
        auto cps_then_and_else = [&if_continuation](unique_ptr<Ast> result) {
            vector<unique_ptr<Ast>> args;
            args.push_back(std::move(result));
            return make_unique<CallAst>(make_unique<VarAst>(if_continuation), std::move(args));
        };
        return make_unique<IfAst>(std::move(cond_ast), then->to_cps(cps_then_and_else), else_->to_cps(cps_then_and_else));
    };
    vector<unique_ptr<Ast>> call_ast_args;
    call_ast_args.push_back(make_continuation(callback));
    return make_unique<CallAst>(make_unique<LambdaAst>("", vector<string>{if_continuation}, cond->to_cps(cps_cond)), std::move(call_ast_args));
}
unique_ptr<Ast> BinaryAst::to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)> callback)
{
    return left->to_cps([this, &callback](unique_ptr<Ast> left_ast) {
        return right->to_cps([this, &left_ast, &callback](unique_ptr<Ast> right_ast) {
            return callback(make_unique<BinaryAst>(operator_, std::move(left_ast), std::move(right_ast)));
        });
    });
}
unique_ptr<Ast> AssignAst::to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)> callback)
{
    return left->to_cps([this, &callback](unique_ptr<Ast> left_ast) {
        return right->to_cps([&left_ast, &callback](unique_ptr<Ast> right_ast) {
            return callback(make_unique<AssignAst>(std::move(left_ast), std::move(right_ast)));
        });
    });
}