#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2.hpp"
#include "token_stream.hpp"
TEST_CASE("test token stream")
{
    SECTION("test is keyword")
    {

        for (auto keyword : TokenStream::KEYWORDS)
        {
            REQUIRE(TokenStream::is_keyword(keyword));
        }
        REQUIRE(!TokenStream::is_keyword("aaa"));
        REQUIRE(TokenStream::is_keyword("λ"));
    }
    SECTION("test is digit")
    {
        for (size_t i = 0; i < strlen(TokenStream::DIGITS); i++)
        {
            REQUIRE(TokenStream::is_digit(TokenStream::DIGITS[i]));
        }
        REQUIRE(!TokenStream::is_digit('a'));
    }
    SECTION("test is identifier start")
    {
        REQUIRE(TokenStream::is_identifier_start('a'));
        REQUIRE(TokenStream::is_identifier_start('_'));
        REQUIRE(!TokenStream::is_identifier_start('1'));
    }
    SECTION("test is identifier")
    {
        REQUIRE(TokenStream::is_identifier('a'));
        REQUIRE(TokenStream::is_identifier('_'));
        REQUIRE(TokenStream::is_identifier('1'));
        REQUIRE(TokenStream::is_identifier('!'));
        REQUIRE(!TokenStream::is_identifier('#'));
    }
    SECTION("test is opearator char")
    {
        for (size_t i = 0; i < strlen(TokenStream::OPERATORS); i++)
        {
            REQUIRE(TokenStream::is_operator(TokenStream::OPERATORS[i]));
        }
        REQUIRE(!TokenStream::is_operator('a'));
    }
    SECTION("test is punctuation")
    {
        for (size_t i = 0; i < strlen(TokenStream::PUNCTUATIONS); i++)
        {
            REQUIRE(TokenStream::is_punctuation(TokenStream::PUNCTUATIONS[i]));
        }
        REQUIRE(!TokenStream::is_punctuation('a'));
    }
    SECTION("test is whitespace")
    {
        for (size_t i = 0; i < strlen(TokenStream::WHITESPACES); i++)
        {
            REQUIRE(TokenStream::is_whitespace(TokenStream::WHITESPACES[i]));
        }
        REQUIRE(!TokenStream::is_whitespace('a'));
    }
    SECTION("test read while")
    {
        InputStream input_stream("ab123=");
        TokenStream token_stream1(input_stream);
        auto result = token_stream1.read_while([](char c) -> bool {
            return isalnum(c);
        });
        REQUIRE(result == "ab123");
    }
    SECTION("test read number")
    {

        InputStream input_stream1("0.0");
        TokenStream token_stream1(input_stream1);
        auto result1 = token_stream1.read_number();
        REQUIRE(result1 == Token(NumToken, 0.0));
        REQUIRE(result1 == Token(NumToken, (double) 0));
        InputStream input_stream2("123=");
        TokenStream token_stream2(input_stream2);
        auto result2 = token_stream2.read_number();
        REQUIRE(result2 == Token(NumToken, 123.0));
        InputStream input_stream3("123.3.=");
        TokenStream token_stream3(input_stream3);
        auto result3 = token_stream3.read_number();
        REQUIRE(result3 == Token(NumToken, 123.3));
        InputStream input_stream4("123.1");
        TokenStream token_stream4(input_stream4);
        auto result4 = token_stream4.read_number();
        REQUIRE(result4 == Token(NumToken, 123.1));
    }
    SECTION("test skip comment")
    {
        InputStream input_stream("# abc\ndef");
        TokenStream token_stream(input_stream);
        token_stream.skip_comment();
        REQUIRE(token_stream.input_stream.peek() == 'd');
    }
    SECTION("test read identifier")
    {
        InputStream input_stream1("a=1");
        TokenStream token_stream1(input_stream1);
        REQUIRE(token_stream1.read_identifier() == Token(VariableToken, "a=1"));
        InputStream input_stream2("a = 1");
        TokenStream token_stream2(input_stream2);
        REQUIRE(token_stream2.read_identifier() == Token(VariableToken, "a"));
        InputStream input_stream3("let(a = 1");
        TokenStream token_stream3(input_stream3);
        REQUIRE(token_stream3.read_identifier() == Token(KeywordToken, "let"));
    }
    SECTION("test read string")
    {
        InputStream input_stream1("\"ab\"");
        TokenStream token_stream1(input_stream1);
        REQUIRE(token_stream1.read_string() == Token(StringToken, "ab"));
        InputStream input_stream2("\"ab\\c\"");
        TokenStream token_stream2(input_stream2);
        REQUIRE(token_stream2.read_string() == Token(StringToken, "abc"));
        InputStream input_stream3("\"abc");
        TokenStream token_stream3(input_stream3);
        REQUIRE_THROWS(token_stream3.read_string());
    }
    SECTION("test read next")
    {
        InputStream input_stream(" # comment\n123 abc \"nba\" let a=2  >= ;");
        TokenStream token_stream(input_stream);
        REQUIRE(token_stream.read_next() == Token(NumToken, 123.0));
        REQUIRE(token_stream.read_next() == Token(VariableToken, "abc"));
        REQUIRE(token_stream.read_next() == Token(StringToken, "nba"));
        REQUIRE(token_stream.read_next() == Token(KeywordToken, "let"));
        REQUIRE(token_stream.read_next() == Token(VariableToken, "a=2"));
        REQUIRE(token_stream.read_next() == Token(OperatorToken, ">="));
        REQUIRE(token_stream.read_next() == Token(PunctuationToken, ';'));
        REQUIRE(token_stream.read_next().first == NullToken);
        InputStream input_stream2("\x08");
        TokenStream token_stream2(input_stream2);
        REQUIRE_THROWS(token_stream2.read_next());
    }
    SECTION("test peek and next")
    {
        InputStream input_stream1(" # comment\n123 abc \"nba\" let a=2  >= ;");
        TokenStream token_stream1(input_stream1);
        REQUIRE(token_stream1.peek() == Token(NumToken, 123.0));
        REQUIRE(token_stream1.peek() == Token(NumToken, 123.0));
        REQUIRE(token_stream1.next() == Token(NumToken, 123.0));

        InputStream input_stream2(" # comment\n123 abc \"nba\" let a=2  >= ;");
        TokenStream token_stream2(input_stream2);
        REQUIRE(token_stream2.next() == Token(NumToken, 123.0));
        REQUIRE(token_stream2.next() == Token(VariableToken, "abc"));
        REQUIRE(token_stream2.next() == Token(StringToken, "nba"));
        REQUIRE(token_stream2.next() == Token(KeywordToken, "let"));
        REQUIRE(token_stream2.next() == Token(VariableToken, "a=2"));
        REQUIRE(token_stream2.next() == Token(OperatorToken, ">="));
    }
    SECTION("test eof")
    {
        InputStream input_stream1("# comment\n");
        TokenStream token_stream1(input_stream1);
        REQUIRE(token_stream1.eof());
    }
}
