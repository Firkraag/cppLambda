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
        auto parser = [](Parser &parser) {
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
    }
    SECTION("test parse lambda")
    {
        InputStream input_stream1("lambda (a, b) 1");
        TokenStream token_stream1(input_stream1);
        Parser parser1(token_stream1);
        vector<string> params{"a", "b"};
        // REQUIRE(parser1.parse_lambda("lambda") == LambdaAst("", params, NumberAst(1.0)));
    }
    SECTION("test parse atom")
    {
        InputStream input_stream1("(1)");
        TokenStream token_stream1(input_stream1);
        Parser parser1(token_stream1);
        // auto result = parser1.parse_atom();

        // // cout << result.first << endl;
        // // cout << result.second << endl;
        // REQUIRE(result == NumberAst(1.0));
        InputStream input_stream97("0.0");
        TokenStream token_stream97(input_stream97);
        Parser parser97(token_stream97);
        Ast result97 = parser97.parse_atom();
        const NumberAst *number_ast = dynamic_cast<const NumberAst *>(&result97);
        cout << typeid(result97).name();
        // cout << number_ast;
        // cout << number_ast->value;
        // cout << result97;
        CHECK(NumberAst(0.0) == result97);
        InputStream input_stream98("a");
        TokenStream token_stream98(input_stream98);
        Parser parser98(token_stream98);
        CHECK(parser98.parse_atom() == VarAst("a"));
        InputStream input_stream99("\"a\"");
        TokenStream token_stream99(input_stream99);
        Parser parser99(token_stream99);
        CHECK(parser99.parse_atom() == StringAst("a"));
        InputStream input_stream100("");
        TokenStream token_stream100(input_stream100);
        Parser parser100(token_stream100);
        REQUIRE_THROWS(parser100.parse_atom());
        InputStream input_stream101("&");
        TokenStream token_stream101(input_stream101);
        Parser parser101(token_stream101);
        REQUIRE_THROWS(parser101.parse_atom());
    }
}