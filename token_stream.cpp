#include "token_stream.hpp"
#include <set>
#include <functional>
#include <cstring>
using namespace std;

static bool in_string(char c, const char *string)
{
    for (size_t i = 0; i < strlen(string); i++)
    {
        if (string[i] == c)
        {
            return true;
        }
    }
    return false;
}
set<string> TokenStream::KEYWORDS = {"if", "then", "let", "else", "lambda", "λ", "true", "false", "js"};
const char *TokenStream::DIGITS = "0123456789";
const char *TokenStream::OPERATORS = "+-*/%=&|<>!";
const char *TokenStream::PUNCTUATIONS = ",;(){}[]";
const char *TokenStream::WHITESPACES = " \t\n";
TokenStream::TokenStream(InputStream input_stream) : input_stream(input_stream)
{
    current = Token(NullToken, 1.0);
}
bool TokenStream::eof(void)
{
    return peek().first == NullToken;
}
const Token TokenStream::next(void)
{
    Token temp = current;
    current = Token(NullToken, 1.0);
    if (temp.first == NullToken)
    {
        return read_next();
    }
    return temp;
}
const Token TokenStream::peek(void)
{
    if (current.first == NullToken)
    {
        current = read_next();
    }
    return current;
}
const Token TokenStream::read_next(void)
{
    read_while(is_whitespace);
    if (input_stream.eof())
    {
        return Token(NullToken, 1.0);
    }
    char c = input_stream.peek();
    if (c == '#')
    {
        skip_comment();
        return read_next();
    }
    else if (c == '"')
    {
        return read_string();
    }
    else if (is_digit(c))
    {
        return read_number();
    }
    else if (is_identifier_start(c))
    {
        return read_identifier();
    }
    else if (is_punctuation(c))
    {
        return Token(PunctuationToken, input_stream.next());
    }
    else if (is_operator(c))
    {
        return Token(OperatorToken, read_while(is_operator));
    }
    throw input_stream.error_msg("Can't handle character: " + to_string(c));
}
void TokenStream::skip_comment(void)
{
    read_while([](char c) -> bool {
        return c != '\n';
    });
    input_stream.next();
}
const Token TokenStream::read_string(void)
{
    input_stream.next();
    bool escaped = false;
    string buffer;
    while (!input_stream.eof())
    {
        char c = input_stream.next();
        if (escaped)
        {
            buffer.push_back(c);
            escaped = false;
        }
        else if (c == '\\')
        {
            escaped = true;
        }
        else if (c == '"')
        {
            return Token(StringToken, buffer);
        }
        else
        {
            buffer.push_back(c);
        }
    }
    throw input_stream.error_msg("Has no enclosing double quote for string");
}
const Token TokenStream::read_identifier(void)
{
    const string identifier = read_while(is_identifier);
    if (is_keyword(identifier))
    {
        return Token(KeywordToken, identifier);
    }
    else
    {
        return Token(VariableToken, identifier);
    }
}
const Token TokenStream::read_number(void)
{
    bool has_dot = false;
    string number = read_while([&has_dot](char c) -> bool {
        // static bool has_dot = false;
        if (c == '.')
        {
            if (has_dot)
            {
                return false;
            }
            has_dot = true;
            return true;
        }
        return isdigit(c);
    });
    return Token(NumToken, strtod(number.c_str(), NULL));
}
const string TokenStream::read_while(function<bool(char)> predicate)
{
    string buffer;
    while (!input_stream.eof() && predicate(input_stream.peek()))
    {
        buffer.push_back(input_stream.next());
    }
    return buffer;
}
bool TokenStream::is_keyword(const string &word)
{
    auto search = KEYWORDS.find(word);
    return search != KEYWORDS.end();
}
bool TokenStream::is_digit(char c)
{
    return isdigit(c);
}
bool TokenStream::is_identifier_start(char c)
{
    return isalpha(c) || c == '_';
}
bool TokenStream::is_identifier(char c)
{
    if (isalnum(c))
    {
        return true;
    }
    return in_string(c, "_?!-<>=");
}
bool TokenStream::is_operator(char c)
{
    return in_string(c, OPERATORS);
}
bool TokenStream::is_punctuation(char c)
{
    return in_string(c, PUNCTUATIONS);
}
bool TokenStream::is_whitespace(char c)
{
    return in_string(c, WHITESPACES);
}
const string TokenStream::error_msg(const string &msg)
{
    return input_stream.error_msg(msg);
}