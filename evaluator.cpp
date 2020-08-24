// #include <any>
// #include "ast.hpp"
// #include "environment.hpp"
// #include "utils.hpp"
// using std::any;
// static any make_lambda(shared_ptr<Environment> environment, const LambdaAst &lambda_ast);

// double evaluate(const NumberAst &number_ast, shared_ptr<Environment> environment)
// {
//     return number_ast.value;
// }
// bool evaluate(const BooleanAst &boolean_ast, shared_ptr<Environment> environment)
// {
//     return boolean_ast.value;
// }
// string evaluate(const StringAst &string_ast, shared_ptr<Environment> environment)
// {
//     return string_ast.value;
// }
// any evaluate(const VarAst &var_ast, shared_ptr<Environment> environment)
// {
//     return environment->get(var_ast.name);
// }
// any evaluate(const BinaryAst &binary_ast, shared_ptr<Environment> environment)
// {
//     return apply_op(binary_ast.operator_, evaluate(*binary_ast.left, environment), evaluate(*binary_ast.right, environment));
// }
// any evaluate(const AssignAst &assign_ast, shared_ptr<Environment> environment)
// {
//     const VarAst *left = dynamic_cast<const VarAst *>(assign_ast.left.get());
//     if (left == nullptr)
//     {
//         throw "cannot assign to ";
//     }

//     return environment->set(left->name, evaluate(*assign_ast.right, environment));
// }
// any evaluate(const IfAst &if_ast, shared_ptr<Environment> environment)
// {
//     auto cond = evaluate(*if_ast.cond, environment);
//     try
//     {
//         if (std::any_cast<bool>(cond))
//         {
//             return evaluate(*if_ast.then, environment);
//         }
//         return evaluate(*if_ast.else_, environment);
//     }
//     catch (const std::bad_any_cast &e)
//     {
//         return evaluate(*if_ast.then, environment);
//     }
// }
// any evaluate(const ProgAst &prog_ast, shared_ptr<Environment> environment)
// {
//     any result = false;
//     for (auto expr : prog_ast.prog)
//     {
//         result = evaluate(*expr, environment);
//     }
//     return result;
// }
// any evaluate(const LetAst &let_ast, shared_ptr<Environment> environment)
// {
//     for (auto &&vardef : let_ast.vardefs)
//     {
//         auto scope = environment->extend(environment);
//         if (vardef->define)
//         {
//             scope->define(vardef->name, evaluate(*vardef->define, scope));
//         }
//         else
//         {
//             scope->define(vardef->name, false);
//         }
//         environment = scope;
//     }
//     return evaluate(*let_ast.body, environment);
// }
// any evaluate(Ast &ast, shared_ptr<Environment> environment)
// {
//     return ast.evaluate(*environment);
    // const LambdaAst *lambda_ast = dynamic_cast<const LambdaAst *>(&ast);
    // if (lambda_ast != nullptr)
    // {
    //     return make_lambda(environment, *lambda_ast);
    // }
    // const ProgAst *prog_ast = dynamic_cast<const ProgAst *>(&ast);
    // if (prog_ast != nullptr)
    // {
    //     any result = false;
    //     for (auto expr : prog_ast->prog)
    //     {
    //         result = evaluate(*expr, environment);
    //     }
    //     return result;
    // }
    // const LetAst *let_ast = dynamic_cast<const LetAst *>(&ast);
    // if (let_ast != nullptr)
    // {
    //     for (auto &&vardef : let_ast->vardefs)
    //     {
    //         auto scope = environment->extend(environment);
    //         if (vardef->define)
    //         {
    //             scope->define(vardef->name, evaluate(*vardef->define, scope));
    //         }
    //         else
    //         {
    //             scope->define(vardef->name, false);
    //         }
    //         environment = scope;
    //     }
    //     return evaluate(*let_ast->body, environment);
    // }

    // throw "I don't know how to evaluate";
// }

// static any make_lambda(shared_ptr<Environment> environment, const LambdaAst &lambda_ast)
// {
//     auto lambda_function = [&](any arg) {
//         auto scope = environment->extend(environment);
//         scope->define(lambda_ast.params[0], arg);
//         return evaluate(*lambda_ast.body, scope);
//     };

//     return lambda_function;
// }