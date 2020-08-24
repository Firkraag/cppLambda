#ifndef TOKEN_STREAM_H_
#define TOKEN_STREAM_H_
#include "input_stream.hpp"
#include <iostream>
#include <set>
#include <functional>
#include <variant>
using namespace std;
enum TokenType
{
    NumToken,
    KeywordToken,
    VariableToken,
    StringToken,
    OperatorToken,
    PunctuationToken,
    NullToken,
};
typedef variant<double, char, string> TokenValue;
typedef pair<enum TokenType, TokenValue> Token;
class TokenStream
{
private:
    Token current;

public:
    InputStream input_stream;
    static set<string> KEYWORDS;
    static const char *DIGITS;
    static const char *OPERATORS;
    static const char *PUNCTUATIONS;
    static const char *WHITESPACES;

    TokenStream(InputStream input_stream);
    const string error_msg(const string &msg);
    static bool is_keyword(const string &word);
    static bool is_digit(char c);
    static bool is_identifier_start(char c);
    static bool is_identifier(char c);
    static bool is_operator(char c);
    static bool is_punctuation(char c);
    static bool is_whitespace(char c);
    const string read_while(function<bool(char)> predicate);
    const Token read_number(void);
    const Token read_identifier(void);
    const Token read_string(void);
    void skip_comment(void);
    const Token read_next(void);
    const Token peek(void);
    const Token next(void);
    bool eof(void);
};
#endif