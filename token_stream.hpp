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
// class Token {
//     public:
//         TokenType token_type;
//         double num_value;

// };
typedef variant<double, char, string> TokenValue;
// union TokenValue {
//     double num;
//     // unique_ptr<string> string;
// };

typedef pair<enum TokenType, TokenValue> Token;
class TokenStream
{
private:

public:
    InputStream &input_stream;
    Token current;
    static set<string> KEYWORDS;
    static const char *DIGITS;
    static const char *OPERATORS;
    static const char *PUNCTUATIONS;
    static const char *WHITESPACES;

    TokenStream(InputStream &input_stream);
    const string error_msg(const string & msg);
    static bool is_keyword(const string &word);
    static bool is_digit(char c);
    static bool is_identifier_start(char c);
    static bool is_identifier(char c);
    static bool is_operator(char c);
    static bool is_punctuation(char c);
    static bool is_whitespace(char c);
    string read_while(function<bool(char)> predicate);
    Token read_number(void);
    Token read_identifier(void);
    Token read_string(void);
    void skip_comment(void);
    Token read_next(void);
    Token peek(void);
    Token next(void);
    bool eof(void);
};
#endif