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
        TokenStream token_stream1(new InputStream("ab123="));
        auto result = token_stream1.read_while([](char c) -> bool {
            return isalnum(c);
        });
        REQUIRE(result == "ab123");
    }
    SECTION("test read number")
    {

        TokenStream token_stream1(new InputStream("0.0"));
        auto result1 = token_stream1.read_number();
        REQUIRE(result1 == Token(NumToken, 0.0));
        REQUIRE(result1 == Token(NumToken, (double) 0));
        TokenStream token_stream2(new InputStream("123"));
        auto result2 = token_stream2.read_number();
        REQUIRE(result2 == Token(NumToken, 123.0));
        TokenStream token_stream3(new InputStream("123.3.="));
        auto result3 = token_stream3.read_number();
        REQUIRE(result3 == Token(NumToken, 123.3));
        TokenStream token_stream4(new InputStream("123.1"));
        auto result4 = token_stream4.read_number();
        REQUIRE(result4 == Token(NumToken, 123.1));
    }
    SECTION("test skip comment")
    {
        TokenStream token_stream(new InputStream("# abc\ndef"));
        token_stream.skip_comment();
        REQUIRE(token_stream.input_stream->peek() == 'd');
    }
    SECTION("test read identifier")
    {
        TokenStream token_stream1(new InputStream("a=1"));
        REQUIRE(token_stream1.read_identifier() == Token(VariableToken, "a=1"));
        TokenStream token_stream2(new InputStream("a = 1"));
        REQUIRE(token_stream2.read_identifier() == Token(VariableToken, "a"));
        TokenStream token_stream3(new InputStream("let(a = 1"));
        REQUIRE(token_stream3.read_identifier() == Token(KeywordToken, "let"));
    }
    SECTION("test read string")
    {
        TokenStream token_stream1(new InputStream("\"ab\""));
        REQUIRE(token_stream1.read_string() == Token(StringToken, "ab"));
        TokenStream token_stream2(new InputStream("\"ab\\c\""));
        REQUIRE(token_stream2.read_string() == Token(StringToken, "abc"));
        TokenStream token_stream3(new InputStream("\"abc"));
        REQUIRE_THROWS(token_stream3.read_string());
    }
    SECTION("test read next")
    {
        TokenStream token_stream(new InputStream(" # comment\n123 abc \"nba\" let a=2  >= ;"));
        REQUIRE(token_stream.read_next() == Token(NumToken, 123.0));
        REQUIRE(token_stream.read_next() == Token(VariableToken, "abc"));
        REQUIRE(token_stream.read_next() == Token(StringToken, "nba"));
        REQUIRE(token_stream.read_next() == Token(KeywordToken, "let"));
        REQUIRE(token_stream.read_next() == Token(VariableToken, "a=2"));
        REQUIRE(token_stream.read_next() == Token(OperatorToken, ">="));
        REQUIRE(token_stream.read_next() == Token(PunctuationToken, ';'));
        REQUIRE(token_stream.read_next().first == NullToken);
        TokenStream token_stream2(new InputStream("\x08"));
        REQUIRE_THROWS(token_stream2.read_next());
    }
    SECTION("test peek and next")
    {
        TokenStream token_stream1(new InputStream(" # comment\n123 abc \"nba\" let a=2  >= ;"));
        REQUIRE(token_stream1.peek() == Token(NumToken, 123.0));
        REQUIRE(token_stream1.peek() == Token(NumToken, 123.0));
        REQUIRE(token_stream1.next() == Token(NumToken, 123.0));

        TokenStream token_stream2(new InputStream(" # comment\n123 abc \"nba\" let a=2  >= ;"));
        REQUIRE(token_stream2.next() == Token(NumToken, 123.0));
        REQUIRE(token_stream2.next() == Token(VariableToken, "abc"));
        REQUIRE(token_stream2.next() == Token(StringToken, "nba"));
        REQUIRE(token_stream2.next() == Token(KeywordToken, "let"));
        REQUIRE(token_stream2.next() == Token(VariableToken, "a=2"));
        REQUIRE(token_stream2.next() == Token(OperatorToken, ">="));
        TokenStream token_stream3(new InputStream("{1;2}"));
        REQUIRE(token_stream3.next() == Token(PunctuationToken, '{'));
        REQUIRE(token_stream3.next() == Token(NumToken, 1.0));
        REQUIRE(token_stream3.next() == Token(PunctuationToken, ';'));
        REQUIRE(token_stream3.next() == Token(NumToken, 2.0));
        REQUIRE(token_stream3.next() == Token(PunctuationToken, '}'));
    }
    SECTION("test eof")
    {
        TokenStream token_stream1(new InputStream("# comment\n"));
        REQUIRE(token_stream1.eof());
    }
}
