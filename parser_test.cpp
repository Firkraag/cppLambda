#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include "parse.hpp"
#include <typeinfo>
TEST_CASE("test parse")
{
    SECTION("test skip punc")
    {
        Parser parser1 = Parser(new TokenStream(new InputStream(";")));
        parser1.skip_punc(';');
        REQUIRE(parser1.token_stream->eof());
        Parser parser2 = Parser(new TokenStream(new InputStream(";")));
        REQUIRE_THROWS(parser2.skip_punc('e'));
    }
    SECTION("test is punc")
    {
        Parser parser1 = Parser(new TokenStream(new InputStream(";")));
        REQUIRE(parser1.is_punc(';'));
    }
    SECTION("test skip keyword")
    {
        Parser parser1 = Parser(new TokenStream(new InputStream("else")));
        parser1.skip_keyword("else");
        REQUIRE(parser1.token_stream->eof());
        Parser parser2 = Parser(new TokenStream(new InputStream("e")));
        REQUIRE_THROWS(parser2.skip_keyword("e"));
    }
    SECTION("test is keyword")
    {
        Parser parser1 = Parser(new TokenStream(new InputStream("else")));
        REQUIRE(parser1.is_keyword("else"));
    }
    SECTION("test skip operator")
    {
        Parser parser1 = Parser(new TokenStream(new InputStream(">=")));
        parser1.skip_operator(">=");
        REQUIRE(parser1.token_stream->eof());
        Parser parser2 = Parser(new TokenStream(new InputStream(">=")));
        REQUIRE_THROWS(parser2.skip_operator("<"));
    }
    SECTION("test is operator")
    {
        Parser parser1 = Parser(new TokenStream(new InputStream(">=")));
        REQUIRE(parser1.is_operator(">="));
    }
    SECTION("test parse varname")
    {
        Parser parser1 = Parser(new TokenStream(new InputStream("foo")));
        REQUIRE(parser1.parse_varname() == "foo");
        Parser parser2 = Parser(new TokenStream(new InputStream("1")));
        REQUIRE_THROWS(parser2.parse_varname());
    }
    SECTION("test delimited")
    {
        auto parser = [](Parser &parser) -> string {
            return parser.parse_varname();
        };
        Parser parser1 = Parser(new TokenStream(new InputStream("{a, b, c}")));
        REQUIRE(parser1.delimited('{', '}', ',', parser) == vector<string>{"a", "b", "c"});
        Parser parser2 = Parser(new TokenStream(new InputStream("{}")));
        REQUIRE(parser2.delimited('{', '}', ',', parser).empty());
        Parser parser3 = Parser(new TokenStream(new InputStream("{a,}")));
        REQUIRE(parser3.delimited('{', '}', ',', parser) == vector<string>{"a"});
        Parser parser4 = Parser(new TokenStream(new InputStream("{a,b,}")));
        REQUIRE(parser4.delimited('{', '}', ',', parser) == vector<string>{"a", "b"});
        Parser parser5(new TokenStream(new InputStream("(a, b)")));
        REQUIRE(parser5.delimited('(', ')', ',', parser) == vector<string>{"a", "b"});
    }
    SECTION("test parse lambda")
    {
        Parser parser1(new TokenStream(new InputStream("lambda (a, b) 1")));
        vector<string> params{"a", "b"};
        REQUIRE(*parser1.parse_lambda("lambda") == LambdaAst("", params, shared_ptr<Ast>(new NumberAst(1.0))));
        Parser parser2(new TokenStream(new InputStream("lambda foo() \"abc\"")));
        REQUIRE(*parser2.parse_lambda("lambda") == LambdaAst("foo", vector<string>(), shared_ptr<Ast>(new StringAst("abc"))));
    }
    SECTION("test parse let")
    {
        Parser parser1(new TokenStream(new InputStream("let (a = 1) 1")));
        LetAst let_ast1(vector<shared_ptr<VarDef>>{shared_ptr<VarDef>(new VarDef("a", shared_ptr<Ast>(new NumberAst(1.0))))}, shared_ptr<Ast>(new NumberAst(1.0)));
        REQUIRE(*parser1.parse_let() == let_ast1);
        Parser parser2(new TokenStream(new InputStream("let (a) 1")));
        LetAst let_ast2(vector<shared_ptr<VarDef>>{shared_ptr<VarDef>(new VarDef("a", nullptr))}, shared_ptr<Ast>(new NumberAst(1.0)));
        REQUIRE(*parser2.parse_let() == let_ast2);
        Parser parser3(new TokenStream(new InputStream("let foo (a) foo")));
        LambdaAst *lambda_ast = new LambdaAst("foo", vector<string>{"a"}, shared_ptr<Ast>(new VarAst("foo")));
        CallAst call_ast(shared_ptr<LambdaAst>(lambda_ast), vector<shared_ptr<Ast>>{shared_ptr<Ast>(new BooleanAst(false))});
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
        Parser parser1(new TokenStream(new InputStream("1;\"a\";foo")));
        ProgAst prog_ast1(vector<shared_ptr<Ast>>{
            shared_ptr<Ast>(new NumberAst(1.0)),
            shared_ptr<Ast>(new StringAst("a")),
            shared_ptr<Ast>(new VarAst("foo")),
        });
        REQUIRE(*parser1.parse_toplevel() == prog_ast1);
        Parser parser2(new TokenStream(new InputStream("")));
        REQUIRE(*parser2.parse_toplevel() == ProgAst(vector<shared_ptr<Ast>>()));
        Parser parser3(new TokenStream(new InputStream("a 1 2")));
        REQUIRE_THROWS(parser3.parse_toplevel());
    }
    SECTION("test parse call")
    {
        Parser parser1(new TokenStream(new InputStream("1;\"a\";foo")));
        ProgAst prog_ast1(vector<shared_ptr<Ast>>{
            shared_ptr<Ast>(new NumberAst(1.0)),
            shared_ptr<Ast>(new StringAst("a")),
            shared_ptr<Ast>(new VarAst("foo")),
        });
        REQUIRE(*parser1() == prog_ast1);
        Parser parser2(new TokenStream(new InputStream("")));
        REQUIRE(*parser2() == ProgAst(vector<shared_ptr<Ast>>()));
        Parser parser3(new TokenStream(new InputStream("a 1 2")));
        REQUIRE_THROWS(parser3());
    }
    SECTION("test parse prog")
    {
        Parser parser1(new TokenStream(new InputStream("{1;2}")));
        CHECK(*parser1.parse_prog() == ProgAst(vector<shared_ptr<Ast>>{shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new NumberAst(2.0))}));
        Parser parser2(new TokenStream(new InputStream("{}")));
        CHECK(*parser2.parse_prog() == ProgAst(vector<shared_ptr<Ast>>()));
        Parser parser3(new TokenStream(new InputStream("{1;\"bc\"}")));
        CHECK(*parser3.parse_prog() == ProgAst(vector<shared_ptr<Ast>>{shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new StringAst("bc"))}));
    }
    SECTION("test parse if")
    {
        Parser parser1(new TokenStream(new InputStream("if 1 then 2 else 3")));
        CHECK(*parser1.parse_if() == IfAst(shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new NumberAst(2.0)), shared_ptr<Ast>(new NumberAst(3.0))));
        Parser parser2(new TokenStream(new InputStream("if 1 then {2} else 3")));
        CHECK(*parser2.parse_if() == IfAst(shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new ProgAst(vector<shared_ptr<Ast>>{shared_ptr<Ast>(new NumberAst(2.0))})), shared_ptr<Ast>(new NumberAst(3.0))));
        Parser parser3(new TokenStream(new InputStream("if 1 then 2")));
        CHECK(*parser3.parse_if() == IfAst(shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new NumberAst(2.0)), shared_ptr<Ast>(new BooleanAst(false))));
    }
    SECTION("test parse atom")
    {
        Parser parser1(new TokenStream(new InputStream("(1)")));
        CHECK(*parser1.parse_atom() == NumberAst(1.0));
        Parser parser2(new TokenStream(new InputStream("{1;2}")));
        CHECK(*parser2.parse_atom() == ProgAst(vector<shared_ptr<Ast>>{shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new NumberAst(2.0))}));
        Parser parser3(new TokenStream(new InputStream("if 1 then 2 else 3")));
        CHECK(*parser3.parse_atom() == IfAst(shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new NumberAst(2.0)), shared_ptr<Ast>(new NumberAst(3.0))));
        Parser parser4(new TokenStream(new InputStream("true")));
        REQUIRE(*parser4.parse_atom() == BooleanAst(true));
        Parser parser5(new TokenStream(new InputStream("false")));
        REQUIRE(*parser5.parse_atom() == BooleanAst(false));
        Parser parser6(new TokenStream(new InputStream("lambda foo() \"abc\"")));
        REQUIRE(*parser6.parse_atom() == LambdaAst("foo", vector<string>(), shared_ptr<Ast>(new StringAst("abc"))));
        Parser parser7(new TokenStream(new InputStream("let (a = 1) 1")));
        LetAst let_ast(vector<shared_ptr<VarDef>>{shared_ptr<VarDef>(new VarDef("a", shared_ptr<Ast>(new NumberAst(1.0))))}, shared_ptr<Ast>(new NumberAst(1.0)));
        REQUIRE(*parser7.parse_atom() == let_ast);
        Parser parser8(new TokenStream(new InputStream("123.1")));
        REQUIRE(*parser8.parse_atom() == NumberAst(123.1));
        Parser parser9(new TokenStream(new InputStream("a")));
        REQUIRE(*parser9.parse_atom() == VarAst("a"));
        Parser parser10(new TokenStream(new InputStream("\"a\"")));
        REQUIRE(*parser10.parse_atom() == StringAst("a"));
        Parser parser11(new TokenStream(new InputStream("")));
        REQUIRE_THROWS(parser11.parse_atom());
        Parser parser12(new TokenStream(new InputStream("&")));
        REQUIRE_THROWS(parser12.parse_atom());
    }
    SECTION("test parse bool")
    {
        Parser parser1(new TokenStream(new InputStream("true")));
        REQUIRE(*parser1.parse_bool() == BooleanAst(true));
        Parser parser2(new TokenStream(new InputStream("false")));
        REQUIRE(*parser2.parse_bool() == BooleanAst(false));
    }
    SECTION("test parse expression")
    {
        Parser parser1(new TokenStream(new InputStream("1()")));
        REQUIRE(*parser1.parse_expression() == CallAst(shared_ptr<Ast>(new NumberAst(1.0)), vector<shared_ptr<Ast>>()));
        Parser parser2(new TokenStream(new InputStream("if 1 then 2()()()")));
        CallAst *call_ast1 = new CallAst(shared_ptr<Ast>(new NumberAst(2.0)), vector<shared_ptr<Ast>>());
        CallAst *call_ast2 = new CallAst(shared_ptr<Ast>(call_ast1), vector<shared_ptr<Ast>>());
        IfAst *if_ast = new IfAst(shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(call_ast2), shared_ptr<Ast>(new BooleanAst(false)));
        REQUIRE(*parser2.parse_expression() == CallAst(shared_ptr<Ast>(if_ast), vector<shared_ptr<Ast>>()));
        Parser parser3(new TokenStream(new InputStream("1 + ")));
        REQUIRE_THROWS(parser3.parse_expression());
    }
    SECTION("test parse call")
    {
        Parser parser1(new TokenStream(new InputStream("(b, c)")));
        CallAst call_ast(unique_ptr<Ast>(new VarAst("a")), vector<shared_ptr<Ast>>{
                                   shared_ptr<Ast>(new VarAst("b")),
                                   shared_ptr<Ast>(new VarAst("c")),
                               });
        REQUIRE(*parser1.parse_call(unique_ptr<Ast>(new VarAst("a"))) == call_ast);
    }
    SECTION("test maybe call")
    {
        Parser parser1(new TokenStream(new InputStream("a(b, c)")));
        CallAst call_ast(shared_ptr<Ast>(new VarAst("a")), vector<shared_ptr<Ast>>{shared_ptr<Ast>(new VarAst("b")), shared_ptr<Ast>(new VarAst("c"))});
        unique_ptr<Ast> result = parser1.maybe_call([](Parser &p) {
            return p.parse_atom();
        });
        REQUIRE(*result == call_ast);
        Parser parser2(new TokenStream(new InputStream("a")));
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
        Parser parser1(new TokenStream(new InputStream("a + b * c")));
        BinaryAst binary_ast("+", make_shared<VarAst>("a"),
                             make_shared<BinaryAst>("*", make_shared<VarAst>("b"), make_shared<VarAst>("c")));
        REQUIRE(*parser1.maybe_binary(parser(parser1), 0) == binary_ast);
        Parser parser2(new TokenStream(new InputStream("a + b = c")));
        AssignAst assign_ast(make_shared<BinaryAst>("+", make_shared<VarAst>("a"), make_shared<VarAst>("b")),
                             make_shared<VarAst>("c"));
        REQUIRE(*parser2.maybe_binary(parser(parser2), 0) == assign_ast);
        Parser parser3(new TokenStream(new InputStream("a + b + c")));
        BinaryAst binary_ast3("+", make_shared<BinaryAst>("+", make_shared<VarAst>("a"), make_shared<VarAst>("b")), make_shared<VarAst>("c"));
        REQUIRE(*parser3.maybe_binary(parser(parser3), 0) == binary_ast3);
    }
}