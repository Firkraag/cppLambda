#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include "ast.hpp"
#include <memory>
#include <functional>
#include <any>
using std::any_cast;
using std::make_shared;
using std::make_unique;
using std::shared_ptr;
using std::unique_ptr;
TEST_CASE("test ast")
{
    SECTION("test equality")
    {
        NumberAst ast1(1.0);
        NumberAst ast2(1.0);
        REQUIRE(ast1 == ast2);
        BooleanAst ast3(false);
        BooleanAst ast4(false);
        REQUIRE(ast3 == ast4);
        StringAst ast5("abc");
        StringAst ast6("abc");
        REQUIRE(ast5 == ast6);
        REQUIRE_FALSE(ast1 == ast3);
        VarAst ast7("abc");
        VarAst ast8("abc");
        REQUIRE(ast7 == ast8);
        LambdaAst lambda_ast1("foo", vector<string>{"a", "b"}, unique_ptr<Ast>(new NumberAst(1.0)));
        LambdaAst lambda_ast2("foo", vector<string>{"a", "b"}, unique_ptr<Ast>(new NumberAst(1.0)));
        LambdaAst lambda_ast3("foo", vector<string>{"a", "b"}, unique_ptr<Ast>(new NumberAst(2.0)));
        REQUIRE(lambda_ast1 == lambda_ast2);
        REQUIRE_FALSE(lambda_ast1 == lambda_ast3);
        VarDef varast_def1("foo", nullptr);
        VarDef varast_def2("foo", nullptr);
        VarDef varast_def3("foo", unique_ptr<Ast>(new NumberAst(1.0)));
        VarDef varast_def4("foo", unique_ptr<Ast>(new NumberAst(1.0)));
        VarDef varast_def5("foo", unique_ptr<Ast>(new NumberAst(2.0)));
        REQUIRE(varast_def1 == varast_def2);
        REQUIRE_FALSE(varast_def1 == varast_def3);
        REQUIRE(varast_def3 == varast_def4);
        REQUIRE_FALSE(varast_def3 == varast_def5);
        vector<unique_ptr<VarDef>> vardefs1;
        vardefs1.push_back(unique_ptr<VarDef>(new VarDef("foo", nullptr)));
        LetAst let_ast1(std::move(vardefs1), unique_ptr<Ast>(new NumberAst(1.0)));
        vector<unique_ptr<VarDef>> vardefs2;
        vardefs2.push_back(unique_ptr<VarDef>(new VarDef("foo", nullptr)));
        LetAst let_ast2(std::move(vardefs2), unique_ptr<Ast>(new NumberAst(1.0)));
        vector<unique_ptr<VarDef>> vardefs3;
        vardefs3.push_back(unique_ptr<VarDef>(new VarDef("bar", nullptr)));
        LetAst let_ast3(std::move(vardefs3), unique_ptr<Ast>(new NumberAst(1.0)));
        vector<unique_ptr<VarDef>> vardefs4;
        vardefs4.push_back(unique_ptr<VarDef>(new VarDef("foo", nullptr)));
        LetAst let_ast4(std::move(vardefs4), unique_ptr<Ast>(new NumberAst(2.0)));
        REQUIRE(let_ast1 == let_ast2);
        REQUIRE_FALSE(let_ast1 == let_ast3);
        REQUIRE_FALSE(let_ast1 == let_ast4);
        vector<unique_ptr<Ast>> call_ast_args1;
        call_ast_args1.push_back(unique_ptr<Ast>(new NumberAst(1.0)));
        CallAst call_ast1(unique_ptr<Ast>(new NumberAst(1.0)), std::move(call_ast_args1));
        vector<unique_ptr<Ast>> call_ast_args2;
        call_ast_args2.push_back(unique_ptr<Ast>(new NumberAst(1.0)));
        CallAst call_ast2(unique_ptr<Ast>(new NumberAst(1.0)), std::move(call_ast_args2));
        vector<unique_ptr<Ast>> call_ast_args3;
        call_ast_args3.push_back(unique_ptr<Ast>(new NumberAst(2.0)));
        CallAst call_ast3(unique_ptr<Ast>(new NumberAst(1.0)), std::move(call_ast_args3));
        vector<unique_ptr<Ast>> call_ast_args4;
        call_ast_args4.push_back(unique_ptr<Ast>(new NumberAst(1.0)));
        CallAst call_ast4(unique_ptr<Ast>(new NumberAst(2.0)), std::move(call_ast_args4));
        REQUIRE(call_ast1 == call_ast2);
        REQUIRE_FALSE(call_ast1 == call_ast3);
        REQUIRE_FALSE(call_ast1 == call_ast4);
        NumberAst &ref3 = ast1;
        NumberAst &ref4 = ast2;
        REQUIRE(ref3 == ref4);
        Ast &ref1 = ast1;
        Ast &ref2 = ast2;
        REQUIRE(ref1 == ref2);
    }
    SECTION("test prog_ast equality")
    {
        vector<unique_ptr<Ast>> prog1;
        prog1.push_back(unique_ptr<Ast>(new NumberAst(1.0)));
        vector<unique_ptr<Ast>> prog2;
        prog2.push_back(unique_ptr<Ast>(new NumberAst(1.0)));
        vector<unique_ptr<Ast>> prog3;
        prog3.push_back(unique_ptr<Ast>(new NumberAst(2.0)));
        ProgAst prog_ast1(std::move(prog1));
        ProgAst prog_ast2(std::move(prog2));
        ProgAst prog_ast3(std::move(prog3));
        REQUIRE(prog_ast1 == prog_ast2);
        REQUIRE_FALSE(prog_ast1 == prog_ast3);
    }
    SECTION("test if_ast equality")
    {
        IfAst if_ast1(unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(1.0)));
        IfAst if_ast2(unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(1.0)));
        IfAst if_ast3(unique_ptr<Ast>(new NumberAst(2.0)), unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(1.0)));
        IfAst if_ast4(unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(2.0)), unique_ptr<Ast>(new NumberAst(1.0)));
        REQUIRE(if_ast1 == if_ast2);
        REQUIRE_FALSE(if_ast1 == if_ast3);
        REQUIRE_FALSE(if_ast1 == if_ast4);
    }
    SECTION("test binary_ast equality")
    {
        BinaryAst binary_ast1("+", unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(2.0)));
        BinaryAst binary_ast2("+", unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(2.0)));
        BinaryAst binary_ast3("-", unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(2.0)));
        BinaryAst binary_ast4("+", unique_ptr<Ast>(new NumberAst(2.0)), unique_ptr<Ast>(new NumberAst(2.0)));
        REQUIRE(binary_ast1 == binary_ast2);
        REQUIRE_FALSE(binary_ast1 == binary_ast3);
        REQUIRE_FALSE(binary_ast1 == binary_ast4);
    }
    SECTION("test assign_ast equality")
    {
        AssignAst assign_ast1(unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(2.0)));
        AssignAst assign_ast2(unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(2.0)));
        AssignAst assign_ast3(unique_ptr<Ast>(new NumberAst(2.0)), unique_ptr<Ast>(new NumberAst(2.0)));
        AssignAst assign_ast4(unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(1.0)));
        REQUIRE(assign_ast1 == assign_ast2);
        REQUIRE_FALSE(assign_ast1 == assign_ast3);
        REQUIRE_FALSE(assign_ast1 == assign_ast4);
    }
    SECTION("test literal ast evaluate")
    {
        NumberAst number_ast(1.0);
        REQUIRE(Double(1.0) == *number_ast.evaluate(make_unique<Environment>()));
        StringAst string_ast("abc");
        REQUIRE(String("abc") == *string_ast.evaluate(make_unique<Environment>()));
        BooleanAst boolean_ast1(false);
        REQUIRE(*FALSE == *boolean_ast1.evaluate(make_unique<Environment>()));
        BooleanAst boolean_ast2(true);
        REQUIRE(*TRUE == *boolean_ast2.evaluate(make_unique<Environment>()));
    }
    SECTION("test binary ast evaluate")
    {
        BinaryAst binary_ast("+", std::make_unique<NumberAst>(1.0), std::make_unique<NumberAst>(2.0));
        REQUIRE(Double(3.0) == *binary_ast.evaluate(make_unique<Environment>()));
    }
    SECTION("test prog ast evaluate")
    {
        vector<unique_ptr<Ast>> ast_list1;
        ProgAst prog_ast1(std::move(ast_list1));
        REQUIRE(*FALSE == *prog_ast1.evaluate(make_unique<Environment>()));
        vector<unique_ptr<Ast>> ast_list2;
        ast_list2.push_back(std::make_unique<NumberAst>(1.0));
        ProgAst prog_ast2(std::move(ast_list2));
        REQUIRE(Double(1.0) == *prog_ast2.evaluate(make_unique<Environment>()));
        vector<unique_ptr<Ast>> ast_list3;
        ast_list3.push_back(std::make_unique<NumberAst>(1.0));
        ast_list3.push_back(std::make_unique<NumberAst>(2.0));
        ProgAst prog_ast3(std::move(ast_list3));
        REQUIRE(Double(2.0) == *prog_ast3.evaluate(make_unique<Environment>()));
    }

    SECTION("test assign ast and var ast evaluate")
    {
        AssignAst assign_ast1(std::make_unique<NumberAst>(1.0), std::make_unique<StringAst>("a"));
        REQUIRE_THROWS(assign_ast1.evaluate(make_unique<Environment>()));
        vector<unique_ptr<Ast>> ast_list1;
        ast_list1.push_back(std::make_unique<AssignAst>(make_unique<VarAst>("x"), make_unique<NumberAst>(1.0)));
        ast_list1.push_back(make_unique<VarAst>("x"));
        ProgAst prog_ast1(std::move(ast_list1));
        REQUIRE(Double(1.0) == *prog_ast1.evaluate(make_unique<Environment>()));
        REQUIRE_THROWS(AssignAst(make_unique<VarAst>("x"), make_unique<NumberAst>(1.0)).evaluate(make_unique<Environment>(make_unique<Environment>())));
    }
    SECTION("test if ast")
    {
        IfAst if_ast1(std::make_unique<StringAst>(""), std::make_unique<NumberAst>(1.0), nullptr);
        REQUIRE(Double(1.0) == *if_ast1.evaluate(make_unique<Environment>()));
        IfAst if_ast2(std::make_unique<BooleanAst>(false), std::make_unique<NumberAst>(1.0), std::make_unique<NumberAst>(2.0));
        REQUIRE(Double(2.0) == *if_ast2.evaluate(make_unique<Environment>()));
        IfAst if_ast3(std::make_unique<BooleanAst>(true), std::make_unique<NumberAst>(1.0), std::make_unique<NumberAst>(2.0));
        REQUIRE(Double(1.0) == *if_ast3.evaluate(make_unique<Environment>()));
    }
    SECTION("test let ast")
    {
        vector<unique_ptr<VarDef>> vardefs1;
        vardefs1.push_back(make_unique<VarDef>("x", nullptr));
        LetAst let_ast1(std::move(vardefs1), make_unique<VarAst>("x"));
        REQUIRE(*FALSE == *let_ast1.evaluate(make_unique<Environment>()));
        // let (x = 2, y = x + 1, z = x + y) x + y + z
        vector<unique_ptr<VarDef>> vardefs2;
        vardefs2.push_back(make_unique<VarDef>("x", make_unique<NumberAst>(2.0)));
        vardefs2.push_back(make_unique<VarDef>("y", make_unique<BinaryAst>("+", make_unique<VarAst>("x"), make_unique<NumberAst>(1.0))));
        vardefs2.push_back(make_unique<VarDef>("z", make_unique<BinaryAst>("+", make_unique<VarAst>("x"), make_unique<VarAst>("y"))));

        unique_ptr<BinaryAst> body = make_unique<BinaryAst>(
            "+",
            make_unique<BinaryAst>("+", make_unique<VarAst>("x"), make_unique<VarAst>("y")),
            make_unique<VarAst>("z"));
        LetAst let_ast2(std::move(vardefs2), std::move(body));
        REQUIRE(Double(10.0) == *let_ast2.evaluate(make_unique<Environment>()));
    }
    SECTION("test lambda ast")
    {
        make_unique<VarAst>("aa");
        // unique_ptr<Environment> env = make_unique<Environment>();
        // unique_ptr<LambdaAst> lambda_ast1 = make_unique<LambdaAst>("foo", vector<string>{}, unique_ptr<Ast>(new NumberAst(3.3)));

        // auto lambda = make_lambda(*env, *lambda_ast1);
        // // Function function(lambda);
        // unique_ptr<Object> result = lambda(vector<unique_ptr<Object>>());
        // cout << *result << endl;
    }
}