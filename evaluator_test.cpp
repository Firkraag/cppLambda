#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include "evaluator.hpp"
#include <memory>
#include <any>
using std::any_cast;
using std::make_shared;
using std::shared_ptr;
// TEST_CASE("test evaluator")
// {
//     SECTION("test literal ast")
//     {
//         NumberAst number_ast(1.0);
//         REQUIRE(1.0 == evaluate(number_ast, make_shared<Environment>()));
//         StringAst string_ast("abc");
//         REQUIRE("abc" == evaluate(string_ast, make_shared<Environment>()));
//         BooleanAst boolean_ast1(false);
//         REQUIRE(false == evaluate(boolean_ast1, make_shared<Environment>()));
//         BooleanAst boolean_ast2(true);
//         REQUIRE(true == evaluate(boolean_ast2, make_shared<Environment>()));
//     }
//     SECTION("test binary ast")
//     {
//         BinaryAst binary_ast("+", std::make_shared<NumberAst>(1.0), std::make_shared<NumberAst>(2.0));
//         REQUIRE(3.0 == std::any_cast<double>(evaluate(binary_ast, make_shared<Environment>())));
//     }
    // SECTION("test prog ast")
    // {
    //     vector<shared_ptr<Ast>> ast_list1;
    //     ProgAst prog_ast1(ast_list1);
    //     REQUIRE(false == std::any_cast<bool>(evaluate(prog_ast1, make_shared<Environment>())));
    //     vector<shared_ptr<Ast>> ast_list2 = {std::make_shared<NumberAst>(1.0)};
    //     ProgAst prog_ast2(ast_list2);
    //     REQUIRE(1.0 == std::any_cast<double>(evaluate(prog_ast2, make_shared<Environment>())));
    //     vector<shared_ptr<Ast>> ast_list3 = {std::make_shared<NumberAst>(1.0), std::make_shared<NumberAst>(2.0)};
    //     ProgAst prog_ast3(ast_list3);
    //     REQUIRE(2.0 == std::any_cast<double>(evaluate(prog_ast3, make_shared<Environment>())));
    // }
    // SECTION("test assign ast and var ast")
    // {
    //     AssignAst assign_ast1(std::make_shared<NumberAst>(1.0), std::make_shared<StringAst>("a"));
    //     REQUIRE_THROWS(evaluate(assign_ast1, make_shared<Environment>()));
    //     vector<shared_ptr<Ast>> ast_list1 = {std::make_shared<AssignAst>(make_shared<VarAst>("x"), make_shared<NumberAst>(1.0)), make_shared<VarAst>("x")};
    //     ProgAst prog_ast1(ast_list1);
    //     REQUIRE(1.0 == any_cast<double>(evaluate(prog_ast1, make_shared<Environment>())));
    //     REQUIRE_THROWS(evaluate(AssignAst(make_shared<VarAst>("x"), make_shared<NumberAst>(1.0)), make_shared<Environment>(make_shared<Environment>())));
    // }
    // SECTION("test if ast")
    // {
    //     IfAst if_ast1(std::make_shared<StringAst>(""), std::make_shared<NumberAst>(1.0), nullptr);
    //     REQUIRE(1.0 == std::any_cast<double>(evaluate(if_ast1, make_shared<Environment>())));
    //     IfAst if_ast2(std::make_shared<BooleanAst>(false), std::make_shared<NumberAst>(1.0), std::make_shared<NumberAst>(2.0));
    //     REQUIRE(2.0 == std::any_cast<double>(evaluate(if_ast2, make_shared<Environment>())));
    //     IfAst if_ast3(std::make_shared<BooleanAst>(true), std::make_shared<NumberAst>(1.0), std::make_shared<NumberAst>(2.0));
    //     REQUIRE(1.0 == std::any_cast<double>(evaluate(if_ast3, make_shared<Environment>())));
    // }
    // // SECTION("test call ast")
    // // {
    // // }
    // SECTION("test let ast")
    // {
    //     vector<shared_ptr<VarDefAst>> vardefs1 = {
    //         make_shared<VarDefAst>("x", nullptr),
    //     };
    //     LetAst let_ast1(vardefs1, make_shared<VarAst>("x"));
    //     REQUIRE(false == any_cast<bool>(evaluate(let_ast1, make_shared<Environment>())));
    //     // let (x = 2, y = x + 1, z = x + y) x + y + z
    //     vector<shared_ptr<VarDefAst>> vardefs2 = {
    //         make_shared<VarDefAst>("x", make_shared<NumberAst>(2.0)),
    //         make_shared<VarDefAst>("y", make_shared<BinaryAst>("+", make_shared<VarAst>("x"), make_shared<NumberAst>(1.0))),
    //         make_shared<VarDefAst>("z", make_shared<BinaryAst>("+", make_shared<VarAst>("x"), make_shared<VarAst>("y"))),
    //     };

    //     shared_ptr<BinaryAst> body = make_shared<BinaryAst>(
    //         "+",
    //         make_shared<BinaryAst>("+", make_shared<VarAst>("x"), make_shared<VarAst>("y")), 
    //         make_shared<VarAst>("z"));
    //     LetAst let_ast2(vardefs2, body);
    //     REQUIRE(10.0 == any_cast<double>(evaluate(let_ast2, make_shared<Environment>())));
    // }
// }
