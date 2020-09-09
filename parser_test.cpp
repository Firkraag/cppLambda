#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include "parse.hpp"
#include <typeinfo>
TEST_CASE("test parse")
{
    SECTION("test skip punc")
    {
        Parser parser1 = Parser(new TokenStream(new InputStreamStr(";")));
        parser1.skip_punc(';');
        REQUIRE(parser1.token_stream->eof());
        Parser parser2 = Parser(new TokenStream(new InputStreamStr(";")));
        REQUIRE_THROWS(parser2.skip_punc('e'));
    }
    SECTION("test is punc")
    {
        Parser parser1 = Parser(new TokenStream(new InputStreamStr(";")));
        REQUIRE(parser1.is_punc(';'));
    }
    SECTION("test skip keyword")
    {
        Parser parser1 = Parser(new TokenStream(new InputStreamStr("else")));
        parser1.skip_keyword("else");
        REQUIRE(parser1.token_stream->eof());
        Parser parser2 = Parser(new TokenStream(new InputStreamStr("e")));
        REQUIRE_THROWS(parser2.skip_keyword("e"));
    }
    SECTION("test is keyword")
    {
        Parser parser1 = Parser(new TokenStream(new InputStreamStr("else")));
        REQUIRE(parser1.is_keyword("else"));
    }
    SECTION("test skip operator")
    {
        Parser parser1 = Parser(new TokenStream(new InputStreamStr(">=")));
        parser1.skip_operator(">=");
        REQUIRE(parser1.token_stream->eof());
        Parser parser2 = Parser(new TokenStream(new InputStreamStr(">=")));
        REQUIRE_THROWS(parser2.skip_operator("<"));
    }
    SECTION("test is operator")
    {
        Parser parser1 = Parser(new TokenStream(new InputStreamStr(">=")));
        REQUIRE(parser1.is_operator(">="));
    }
    SECTION("test parse varname")
    {
        Parser parser1 = Parser(new TokenStream(new InputStreamStr("foo")));
        REQUIRE(parser1.parse_varname() == "foo");
        Parser parser2 = Parser(new TokenStream(new InputStreamStr("1")));
        REQUIRE_THROWS(parser2.parse_varname());
    }
    SECTION("test delimited")
    {
        auto parser = [](Parser &parser) -> string {
            return parser.parse_varname();
        };
        Parser parser1 = Parser(new TokenStream(new InputStreamStr("{a, b, c}")));
        REQUIRE(parser1.delimited('{', '}', ',', parser) == vector<string>{"a", "b", "c"});
        Parser parser2 = Parser(new TokenStream(new InputStreamStr("{}")));
        REQUIRE(parser2.delimited('{', '}', ',', parser).empty());
        Parser parser3 = Parser(new TokenStream(new InputStreamStr("{a,}")));
        REQUIRE(parser3.delimited('{', '}', ',', parser) == vector<string>{"a"});
        Parser parser4 = Parser(new TokenStream(new InputStreamStr("{a,b,}")));
        REQUIRE(parser4.delimited('{', '}', ',', parser) == vector<string>{"a", "b"});
        Parser parser5(new TokenStream(new InputStreamStr("(a, b)")));
        REQUIRE(parser5.delimited('(', ')', ',', parser) == vector<string>{"a", "b"});
    }
    SECTION("test parse lambda")
    {
        Parser parser1(new TokenStream(new InputStreamStr("lambda (a, b) 1")));
        vector<string> params{"a", "b"};
        REQUIRE(*parser1.parse_lambda("lambda") == LambdaAst("", std::move(params), make_unique<NumberAst>(1.0)));
        Parser parser2(new TokenStream(new InputStreamStr("lambda foo() \"abc\"")));
        REQUIRE(*parser2.parse_lambda("lambda") == LambdaAst("foo", vector<string>(), make_unique<StringAst>("abc")));
    }
    SECTION("test parse let")
    {
        Parser parser1(new TokenStream(new InputStreamStr("let (a = 1) 1")));
        vector<unique_ptr<VarDef>> vardefs1;
        vardefs1.push_back(make_unique<VarDef>("a", make_unique<NumberAst>(1.0)));
        LetAst let_ast1(std::move(vardefs1), make_unique<NumberAst>(1.0));
        REQUIRE(*parser1.parse_let() == let_ast1);
        Parser parser2(new TokenStream(new InputStreamStr("let (a) 1")));
        vector<unique_ptr<VarDef>> vardefs2;
        vardefs2.push_back(make_unique<VarDef>("a", nullptr));
        LetAst let_ast2(std::move(vardefs2), make_unique<NumberAst>(1.0));
        REQUIRE(*parser2.parse_let() == let_ast2);
        Parser parser3(new TokenStream(new InputStreamStr("let foo (a) foo")));
        unique_ptr<LambdaAst> lambda_ast = make_unique<LambdaAst>("foo", vector<string>{"a"}, make_unique<VarAst>("foo"));
        vector<unique_ptr<Ast>> body;
        body.push_back(make_unique<BooleanAst>(false));
        CallAst call_ast(std::move(lambda_ast), std::move(body));
        REQUIRE(*parser3.parse_let() == call_ast);
    }
    SECTION("test parse vardef")
    {
        // Parser parser1(new TokenStream(new InputStream("a = 1")));
        // REQUIRE(*parser1.parse_vardef() == VarDefAst("a", shared_ptr<Ast>(new NumberAst(1.0))));
        // Parser parser2(new TokenStream(new InputStream("a")));
        // REQUIRE(*parser2.parse_vardef() == VarDefAst("a", nullptr));
    }
    SECTION("test parse toplevel")
    {
        Parser parser1(new TokenStream(new InputStreamStr("1;\"a\";foo")));
        vector<unique_ptr<Ast>> prog1;
        prog1.push_back(make_unique<NumberAst>(1.0));
        prog1.push_back(make_unique<StringAst>("a"));
        prog1.push_back(make_unique<VarAst>("foo"));
        ProgAst prog_ast1(std::move(prog1));
        REQUIRE(*parser1.parse_toplevel() == prog_ast1);
        Parser parser2(new TokenStream(new InputStreamStr("")));
        REQUIRE(*parser2.parse_toplevel() == ProgAst(vector<unique_ptr<Ast>>()));
        Parser parser3(new TokenStream(new InputStreamStr("a 1 2")));
        REQUIRE_THROWS(parser3.parse_toplevel());
    }
    SECTION("test parse call")
    {
        Parser parser1(new TokenStream(new InputStreamStr("1;\"a\";foo")));
        vector<unique_ptr<Ast>> prog1;
        prog1.push_back(make_unique<NumberAst>(1.0));
        prog1.push_back(make_unique<StringAst>("a"));
        prog1.push_back(make_unique<VarAst>("foo"));
        ProgAst prog_ast1(std::move(prog1));
        REQUIRE(*parser1() == prog_ast1);
        Parser parser2(new TokenStream(new InputStreamStr("")));
        REQUIRE(*parser2() == ProgAst(vector<unique_ptr<Ast>>()));
        Parser parser3(new TokenStream(new InputStreamStr("a 1 2")));
        REQUIRE_THROWS(parser3());
    }
    SECTION("test parse prog")
    {
        Parser parser1(new TokenStream(new InputStreamStr("{1;2}")));
        vector<unique_ptr<Ast>> prog1;
        prog1.push_back(make_unique<NumberAst>(1.0));
        prog1.push_back(make_unique<NumberAst>(2.0));
        CHECK(*parser1.parse_prog() == ProgAst(std::move(prog1)));
        Parser parser2(new TokenStream(new InputStreamStr("{}")));
        CHECK(*parser2.parse_prog() == ProgAst(vector<unique_ptr<Ast>>()));
        Parser parser3(new TokenStream(new InputStreamStr("{1;\"bc\"}")));
        vector<unique_ptr<Ast>> prog3;
        prog3.push_back(make_unique<NumberAst>(1.0));
        prog3.push_back(make_unique<StringAst>("bc"));
        CHECK(*parser3.parse_prog() == ProgAst(std::move(prog3)));
    }
    SECTION("test parse if")
    {
        Parser parser1(new TokenStream(new InputStreamStr("if 1 then 2 else 3")));
        IfAst if_ast1(unique_ptr<NumberAst>(new NumberAst(1.0)), unique_ptr<NumberAst>(new NumberAst(2.0)), unique_ptr<NumberAst>(new NumberAst(3.0)));
        CHECK(*parser1.parse_if() == if_ast1);
        Parser parser2(new TokenStream(new InputStreamStr("if 1 then {2} else 3")));
        vector<unique_ptr<Ast>> prog2;
        prog2.push_back(unique_ptr<NumberAst>(new NumberAst(2.0)));
        IfAst if_ast2(unique_ptr<NumberAst>(new NumberAst(1.0)), make_unique<ProgAst>(std::move(prog2)), unique_ptr<NumberAst>(new NumberAst(3.0)));
        CHECK(*parser2.parse_if() == if_ast2);
        Parser parser3(new TokenStream(new InputStreamStr("if 1 then 2")));
        CHECK(*parser3.parse_if() == IfAst(unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(2.0)), unique_ptr<Ast>(new BooleanAst(false))));
    }
    SECTION("test parse atom")
    {
        Parser parser1(new TokenStream(new InputStreamStr("(1)")));
        CHECK(*parser1.parse_atom() == NumberAst(1.0));
        Parser parser2(new TokenStream(new InputStreamStr("{1;2}")));
        vector<unique_ptr<Ast>> prog2;
        prog2.push_back(unique_ptr<NumberAst>(new NumberAst(1.0)));
        prog2.push_back(unique_ptr<NumberAst>(new NumberAst(2.0)));
        CHECK(*parser2.parse_atom() == ProgAst(std::move(prog2)));
        Parser parser3(new TokenStream(new InputStreamStr("if 1 then 2 else 3")));
        CHECK(*parser3.parse_atom() == IfAst(unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(new NumberAst(2.0)), unique_ptr<Ast>(new NumberAst(3.0))));
        Parser parser4(new TokenStream(new InputStreamStr("true")));
        REQUIRE(*parser4.parse_atom() == BooleanAst(true));
        Parser parser5(new TokenStream(new InputStreamStr("false")));
        REQUIRE(*parser5.parse_atom() == BooleanAst(false));
        Parser parser6(new TokenStream(new InputStreamStr("lambda foo() \"abc\"")));
        REQUIRE(*parser6.parse_atom() == LambdaAst("foo", vector<string>(), unique_ptr<Ast>(new StringAst("abc"))));
        Parser parser7(new TokenStream(new InputStreamStr("let (a = 1) 1")));
        vector<unique_ptr<VarDef>> vardef7;
        vardef7.push_back(make_unique<VarDef>("a", unique_ptr<NumberAst>(new NumberAst(1.0))));
        LetAst let_ast(std::move(vardef7), unique_ptr<NumberAst>(new NumberAst(1.0)));
        REQUIRE(*parser7.parse_atom() == let_ast);
        Parser parser8(new TokenStream(new InputStreamStr("123.1")));
        REQUIRE(*parser8.parse_atom() == NumberAst(123.1));
        Parser parser9(new TokenStream(new InputStreamStr("a")));
        REQUIRE(*parser9.parse_atom() == VarAst("a"));
        Parser parser10(new TokenStream(new InputStreamStr("\"a\"")));
        REQUIRE(*parser10.parse_atom() == StringAst("a"));
        Parser parser11(new TokenStream(new InputStreamStr("")));
        REQUIRE_THROWS(parser11.parse_atom());
        Parser parser12(new TokenStream(new InputStreamStr("&")));
        REQUIRE_THROWS(parser12.parse_atom());
    }
    SECTION("test parse bool")
    {
        Parser parser1(new TokenStream(new InputStreamStr("true")));
        REQUIRE(*parser1.parse_bool() == BooleanAst(true));
        Parser parser2(new TokenStream(new InputStreamStr("false")));
        REQUIRE(*parser2.parse_bool() == BooleanAst(false));
    }
    SECTION("test parse expression")
    {
        Parser parser1(new TokenStream(new InputStreamStr("1()")));
        REQUIRE(*parser1.parse_expression() == CallAst(unique_ptr<Ast>(new NumberAst(1.0)), vector<unique_ptr<Ast>>()));
        Parser parser2(new TokenStream(new InputStreamStr("if 1 then 2()()()")));
        CallAst *call_ast1 = new CallAst(unique_ptr<Ast>(new NumberAst(2.0)), vector<unique_ptr<Ast>>());
        CallAst *call_ast2 = new CallAst(unique_ptr<Ast>(call_ast1), vector<unique_ptr<Ast>>());
        IfAst *if_ast = new IfAst(unique_ptr<Ast>(new NumberAst(1.0)), unique_ptr<Ast>(call_ast2), unique_ptr<Ast>(new BooleanAst(false)));
        REQUIRE(*parser2.parse_expression() == CallAst(unique_ptr<Ast>(if_ast), vector<unique_ptr<Ast>>()));
        Parser parser3(new TokenStream(new InputStreamStr("1 + ")));
        REQUIRE_THROWS(parser3.parse_expression());
    }
    SECTION("test parse call")
    {
        Parser parser1(new TokenStream(new InputStreamStr("(b, c)")));
        vector<unique_ptr<Ast>> args1;
        args1.push_back(unique_ptr<Ast>(new VarAst("b")));
        args1.push_back(unique_ptr<Ast>(new VarAst("c")));
        CallAst call_ast(unique_ptr<Ast>(new VarAst("a")), std::move(args1));
        REQUIRE(*parser1.parse_call(unique_ptr<Ast>(new VarAst("a"))) == call_ast);
    }
    SECTION("test maybe call")
    {
        Parser parser1(new TokenStream(new InputStreamStr("a(b, c)")));
        vector<unique_ptr<Ast>> args1;
        args1.push_back(unique_ptr<Ast>(new VarAst("b")));
        args1.push_back(unique_ptr<Ast>(new VarAst("c")));
        CallAst call_ast(unique_ptr<Ast>(new VarAst("a")), std::move(args1));
        unique_ptr<Ast> result = parser1.maybe_call([](Parser &p) {
            return p.parse_atom();
        });
        REQUIRE(*result == call_ast);
        Parser parser2(new TokenStream(new InputStreamStr("a")));
        unique_ptr<Ast> result2 = parser2.maybe_call([](Parser &p) {
            return p.parse_atom();
        });
        REQUIRE(*result2 == VarAst("a"));
    }
    SECTION("test maybe binary")
    {
        auto parser = [](Parser &p) {
            return p.parse_atom();
        };
        Parser parser1(new TokenStream(new InputStreamStr("a + b * c")));
        BinaryAst binary_ast("+", make_unique<VarAst>("a"),
                             make_unique<BinaryAst>("*", make_unique<VarAst>("b"), make_unique<VarAst>("c")));
        REQUIRE(*parser1.maybe_binary(parser(parser1), 0) == binary_ast);
        Parser parser2(new TokenStream(new InputStreamStr("a + b = c")));
        AssignAst assign_ast(make_unique<BinaryAst>("+", make_unique<VarAst>("a"), make_unique<VarAst>("b")),
                             make_unique<VarAst>("c"));
        REQUIRE(*parser2.maybe_binary(parser(parser2), 0) == assign_ast);
        Parser parser3(new TokenStream(new InputStreamStr("a + b + c")));
        BinaryAst binary_ast3("+", make_unique<BinaryAst>("+", make_unique<VarAst>("a"), make_unique<VarAst>("b")), make_unique<VarAst>("c"));
        REQUIRE(*parser3.maybe_binary(parser(parser3), 0) == binary_ast3);
    }
}