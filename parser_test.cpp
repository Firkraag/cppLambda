#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include "parse.hpp"
#include <typeinfo>
TEST_CASE("test parse")
{
    SECTION("test skip punc")
    {
        InputStream input_stream1(";");
        TokenStream token_stream1(input_stream1);
        Parser parser1 = Parser(token_stream1);
        parser1.skip_punc(';');
        REQUIRE(parser1.token_stream.eof());
        InputStream input_stream2(";");
        TokenStream token_stream2(input_stream2);
        Parser parser2 = Parser(token_stream2);
        REQUIRE_THROWS(parser2.skip_punc('e'));
    }
    SECTION("test is punc")
    {
        InputStream input_stream1(";");
        TokenStream token_stream1(input_stream1);
        Parser parser1 = Parser(token_stream1);
        REQUIRE(parser1.is_punc(';'));
    }
    SECTION("test skip keyword")
    {
        InputStream input_stream1("else");
        TokenStream token_stream1(input_stream1);
        Parser parser1 = Parser(token_stream1);
        parser1.skip_keyword("else");
        REQUIRE(parser1.token_stream.eof());
        InputStream input_stream2("e");
        TokenStream token_stream2(input_stream2);
        Parser parser2 = Parser(token_stream2);
        REQUIRE_THROWS(parser2.skip_keyword("e"));
    }
    SECTION("test is keyword")
    {
        InputStream input_stream1("else");
        TokenStream token_stream1(input_stream1);
        Parser parser1 = Parser(token_stream1);
        REQUIRE(parser1.is_keyword("else"));
    }
    SECTION("test skip operator")
    {
        InputStream input_stream1(">=");
        TokenStream token_stream1(input_stream1);
        Parser parser1 = Parser(token_stream1);
        parser1.skip_operator(">=");
        REQUIRE(parser1.token_stream.eof());
        InputStream input_stream2(">=");
        TokenStream token_stream2(input_stream2);
        Parser parser2 = Parser(token_stream2);
        REQUIRE_THROWS(parser2.skip_operator("<"));
    }
    SECTION("test is operator")
    {
        InputStream input_stream1(">=");
        TokenStream token_stream1(input_stream1);
        Parser parser1 = Parser(token_stream1);
        REQUIRE(parser1.is_operator(">="));
    }
    SECTION("test parse varname")
    {
        InputStream input_stream1("foo");
        TokenStream token_stream1(input_stream1);
        Parser parser1 = Parser(token_stream1);
        REQUIRE(parser1.parse_varname() == "foo");
        InputStream input_stream2("1");
        TokenStream token_stream2(input_stream2);
        Parser parser2 = Parser(token_stream2);
        REQUIRE_THROWS(parser2.parse_varname());
    }
    SECTION("test delimited")
    {
        auto parser = [](Parser &parser) -> string {
            return parser.parse_varname();
        };
        InputStream input_stream1("{a, b, c}");
        TokenStream token_stream1(input_stream1);
        Parser parser1 = Parser(token_stream1);
        REQUIRE(parser1.delimited('{', '}', ',', parser) == vector<string>{"a", "b", "c"});
        InputStream input_stream2("{}");
        TokenStream token_stream2(input_stream2);
        Parser parser2 = Parser(token_stream2);
        REQUIRE(parser2.delimited('{', '}', ',', parser).empty());
        InputStream input_stream3("{a,}");
        TokenStream token_stream3(input_stream3);
        Parser parser3 = Parser(token_stream3);
        REQUIRE(parser3.delimited('{', '}', ',', parser) == vector<string>{"a"});
        InputStream input_stream4("{a,b,}");
        TokenStream token_stream4(input_stream4);
        Parser parser4 = Parser(token_stream4);
        REQUIRE(parser4.delimited('{', '}', ',', parser) == vector<string>{"a", "b"});
        Parser parser5(TokenStream(InputStream("(a, b)")));
        REQUIRE(parser5.delimited('(', ')', ',', parser) == vector<string>{"a", "b"});
    }
    SECTION("test parse lambda")
    {
        Parser parser1(TokenStream(InputStream("lambda (a, b) 1")));
        vector<string> params{"a", "b"};
        REQUIRE(*parser1.parse_lambda("lambda") == LambdaAst("", params, shared_ptr<Ast>(new NumberAst(1.0))));
        Parser parser2(TokenStream(InputStream("lambda foo() \"abc\"")));
        REQUIRE(*parser2.parse_lambda("lambda") == LambdaAst("foo", vector<string>(), shared_ptr<Ast>(new StringAst("abc"))));
    }
    SECTION("test parse let")
    {
        Parser parser1(TokenStream(InputStream("let (a = 1) 1")));
        LetAst let_ast1(vector<shared_ptr<VarDef>>{shared_ptr<VarDef>(new VarDef("a", shared_ptr<Ast>(new NumberAst(1.0))))}, shared_ptr<Ast>(new NumberAst(1.0)));
        REQUIRE(*parser1.parse_let() == let_ast1);
        Parser parser2(TokenStream(InputStream("let (a) 1")));
        LetAst let_ast2(vector<shared_ptr<VarDef>>{shared_ptr<VarDef>(new VarDef("a", nullptr))}, shared_ptr<Ast>(new NumberAst(1.0)));
        REQUIRE(*parser2.parse_let() == let_ast2);
        Parser parser3(TokenStream(InputStream("let foo (a) foo")));
        LambdaAst *lambda_ast = new LambdaAst("foo", vector<string>{"a"}, shared_ptr<Ast>(new VarAst("foo")));
        CallAst call_ast(shared_ptr<LambdaAst>(lambda_ast), vector<shared_ptr<Ast>>{shared_ptr<Ast>(new BooleanAst(false))});
        REQUIRE(*parser3.parse_let() == call_ast);
    }
    SECTION("test parse vardef")
    {
        // Parser parser1(TokenStream(InputStream("a = 1")));
        // REQUIRE(*parser1.parse_vardef() == VarDefAst("a", shared_ptr<Ast>(new NumberAst(1.0))));
        // Parser parser2(TokenStream(InputStream("a")));
        // REQUIRE(*parser2.parse_vardef() == VarDefAst("a", nullptr));
    }
    SECTION("test parse toplevel")
    {
        Parser parser1(TokenStream(InputStream("1;\"a\";foo")));
        ProgAst prog_ast1(vector<shared_ptr<Ast>>{
            shared_ptr<Ast>(new NumberAst(1.0)),
            shared_ptr<Ast>(new StringAst("a")),
            shared_ptr<Ast>(new VarAst("foo")),
        });
        REQUIRE(*parser1.parse_toplevel() == prog_ast1);
        Parser parser2(TokenStream(InputStream("")));
        REQUIRE(*parser2.parse_toplevel() == ProgAst(vector<shared_ptr<Ast>>()));
        Parser parser3(TokenStream(InputStream("a 1 2")));
        REQUIRE_THROWS(parser3.parse_toplevel());
    }
    SECTION("test parse call")
    {
        Parser parser1(TokenStream(InputStream("1;\"a\";foo")));
        ProgAst prog_ast1(vector<shared_ptr<Ast>>{
            shared_ptr<Ast>(new NumberAst(1.0)),
            shared_ptr<Ast>(new StringAst("a")),
            shared_ptr<Ast>(new VarAst("foo")),
        });
        REQUIRE(*parser1() == prog_ast1);
        Parser parser2(TokenStream(InputStream("")));
        REQUIRE(*parser2() == ProgAst(vector<shared_ptr<Ast>>()));
        Parser parser3(TokenStream(InputStream("a 1 2")));
        REQUIRE_THROWS(parser3());
    }
    SECTION("test parse prog")
    {
        Parser parser1(TokenStream(InputStream("{1;2}")));
        CHECK(*parser1.parse_prog() == ProgAst(vector<shared_ptr<Ast>>{shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new NumberAst(2.0))}));
        Parser parser2(TokenStream(InputStream("{}")));
        CHECK(*parser2.parse_prog() == ProgAst(vector<shared_ptr<Ast>>()));
        Parser parser3(TokenStream(InputStream("{1;\"bc\"}")));
        CHECK(*parser3.parse_prog() == ProgAst(vector<shared_ptr<Ast>>{shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new StringAst("bc"))}));
    }
    SECTION("test parse if")
    {
        Parser parser1(TokenStream(InputStream("if 1 then 2 else 3")));
        CHECK(*parser1.parse_if() == IfAst(shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new NumberAst(2.0)), shared_ptr<Ast>(new NumberAst(3.0))));
        Parser parser2(TokenStream(InputStream("if 1 then {2} else 3")));
        CHECK(*parser2.parse_if() == IfAst(shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new ProgAst(vector<shared_ptr<Ast>>{shared_ptr<Ast>(new NumberAst(2.0))})), shared_ptr<Ast>(new NumberAst(3.0))));
        Parser parser3(TokenStream(InputStream("if 1 then 2")));
        CHECK(*parser3.parse_if() == IfAst(shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new NumberAst(2.0)), shared_ptr<Ast>(new BooleanAst(false))));
    }
    SECTION("test parse atom")
    {
        Parser parser1(TokenStream(InputStream("(1)")));
        CHECK(*parser1.parse_atom() == NumberAst(1.0));
        Parser parser2(TokenStream(InputStream("{1;2}")));
        CHECK(*parser2.parse_atom() == ProgAst(vector<shared_ptr<Ast>>{shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new NumberAst(2.0))}));
        Parser parser3(TokenStream(InputStream("if 1 then 2 else 3")));
        CHECK(*parser3.parse_atom() == IfAst(shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(new NumberAst(2.0)), shared_ptr<Ast>(new NumberAst(3.0))));
        Parser parser4(TokenStream(InputStream("true")));
        REQUIRE(*parser4.parse_atom() == BooleanAst(true));
        Parser parser5(TokenStream(InputStream("false")));
        REQUIRE(*parser5.parse_atom() == BooleanAst(false));
        Parser parser6(TokenStream(InputStream("lambda foo() \"abc\"")));
        REQUIRE(*parser6.parse_atom() == LambdaAst("foo", vector<string>(), shared_ptr<Ast>(new StringAst("abc"))));
        Parser parser7(TokenStream(InputStream("let (a = 1) 1")));
        LetAst let_ast(vector<shared_ptr<VarDef>>{shared_ptr<VarDef>(new VarDef("a", shared_ptr<Ast>(new NumberAst(1.0))))}, shared_ptr<Ast>(new NumberAst(1.0)));
        REQUIRE(*parser7.parse_atom() == let_ast);
        Parser parser8(TokenStream(InputStream("123.1")));
        REQUIRE(*parser8.parse_atom() == NumberAst(123.1));
        Parser parser9(TokenStream(InputStream("a")));
        REQUIRE(*parser9.parse_atom() == VarAst("a"));
        Parser parser10(TokenStream(InputStream("\"a\"")));
        REQUIRE(*parser10.parse_atom() == StringAst("a"));
        Parser parser11(TokenStream(InputStream("")));
        REQUIRE_THROWS(parser11.parse_atom());
        Parser parser12(TokenStream(InputStream("&")));
        REQUIRE_THROWS(parser12.parse_atom());
    }
    SECTION("test parse bool")
    {
        Parser parser1(TokenStream(InputStream("true")));
        REQUIRE(*parser1.parse_bool() == BooleanAst(true));
        Parser parser2(TokenStream(InputStream("false")));
        REQUIRE(*parser2.parse_bool() == BooleanAst(false));
    }
    SECTION("test parse expression")
    {
        Parser parser1(TokenStream(InputStream("1()")));
        REQUIRE(*parser1.parse_expression() == CallAst(shared_ptr<Ast>(new NumberAst(1.0)), vector<shared_ptr<Ast>>()));
        Parser parser2(TokenStream(InputStream("if 1 then 2()()()")));
        CallAst *call_ast1 = new CallAst(shared_ptr<Ast>(new NumberAst(2.0)), vector<shared_ptr<Ast>>());
        CallAst *call_ast2 = new CallAst(shared_ptr<Ast>(call_ast1), vector<shared_ptr<Ast>>());
        IfAst *if_ast = new IfAst(shared_ptr<Ast>(new NumberAst(1.0)), shared_ptr<Ast>(call_ast2), shared_ptr<Ast>(new BooleanAst(false)));
        REQUIRE(*parser2.parse_expression() == CallAst(shared_ptr<Ast>(if_ast), vector<shared_ptr<Ast>>()));
        Parser parser3(TokenStream(InputStream("1 + ")));
        REQUIRE_THROWS(parser3.parse_expression());
    }
    SECTION("test parse call")
    {
        Parser parser1(TokenStream(InputStream("(b, c)")));
        shared_ptr<Ast> func(new VarAst("a"));
        CallAst call_ast(func, vector<shared_ptr<Ast>>{
                                   shared_ptr<Ast>(new VarAst("b")),
                                   shared_ptr<Ast>(new VarAst("c")),
                               });
        REQUIRE(*parser1.parse_call(func) == call_ast);
    }
    SECTION("test maybe call")
    {
        Parser parser1(TokenStream(InputStream("a(b, c)")));
        CallAst call_ast(shared_ptr<Ast>(new VarAst("a")), vector<shared_ptr<Ast>>{shared_ptr<Ast>(new VarAst("b")), shared_ptr<Ast>(new VarAst("c"))});
        shared_ptr<Ast> result = parser1.maybe_call([](Parser &p) -> shared_ptr<Ast> {
            return p.parse_atom();
        });
        REQUIRE(*result == call_ast);
        Parser parser2(TokenStream(InputStream("a")));
        shared_ptr<Ast> result2 = parser2.maybe_call([](Parser &p) -> shared_ptr<Ast> {
            return p.parse_atom();
        });
        REQUIRE(*result2 == VarAst("a"));
    }
    SECTION("test maybe binary")
    {
        auto parser = [](Parser &p) -> shared_ptr<Ast> {
            return p.parse_atom();
        };
        Parser parser1(TokenStream(InputStream("a + b * c")));
        BinaryAst binary_ast("+", make_shared<VarAst>("a"),
                             make_shared<BinaryAst>("*", make_shared<VarAst>("b"), make_shared<VarAst>("c")));
        REQUIRE(*parser1.maybe_binary(parser(parser1), 0) == binary_ast);
        Parser parser2(TokenStream(InputStream("a + b = c")));
        AssignAst assign_ast(make_shared<BinaryAst>("+", make_shared<VarAst>("a"), make_shared<VarAst>("b")),
                             make_shared<VarAst>("c"));
        REQUIRE(*parser2.maybe_binary(parser(parser2), 0) == assign_ast);
        Parser parser3(TokenStream(InputStream("a + b + c")));
        BinaryAst binary_ast3("+", make_shared<BinaryAst>("+", make_shared<VarAst>("a"), make_shared<VarAst>("b")), make_shared<VarAst>("c"));
        REQUIRE(*parser3.maybe_binary(parser(parser3), 0) == binary_ast3);
    }
}