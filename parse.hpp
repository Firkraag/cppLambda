#ifndef PARSE_H_
#define PARSE_H_
#include "token_stream.hpp"
#include "ast.hpp"
#include "utils.hpp"
#include <cassert>
#include <iostream>
#include <set>
#include <memory>
#include <functional>
#include <tuple>
#include <utility>
#include <variant>
#include <string>
#include <cstring>
#include <map>
using namespace std;

class Parser
{
private:
    static map<string, int> PRECEDENCE;

public:
    unique_ptr<TokenStream> token_stream;
    Parser(TokenStream* token_stream) : token_stream(token_stream) {}
    Parser(string code) : token_stream(new TokenStream(new InputStreamStr(new string(code)))) {}
    void skip_punc(char c);
    bool is_punc(char c);
    void skip_keyword(const string &keyword);
    bool is_keyword(const string &keyword);
    void skip_operator(const string &operator_);
    bool is_operator(const string &operator_);
    vector<string> delimited(char start, char stop, char separator, string (*parser)(Parser &p));
    vector<unique_ptr<VarDef>> delimited(char start, char stop, char separator, unique_ptr<VarDef> (*parser)(Parser &p));
    vector<unique_ptr<Ast>> delimited(char start, char stop, char separator, unique_ptr<Ast> (*parser)(Parser &p));
    unique_ptr<LambdaAst> parse_lambda(const string &keyword);
    unique_ptr<Ast> parse_let(void);
    unique_ptr<VarDef> parse_vardef(void);
    string parse_varname(void);
    unique_ptr<ProgAst> parse_toplevel(void);
    unique_ptr<IfAst> parse_if(void);
    unique_ptr<Ast> parse_atom(void);
    unique_ptr<ProgAst> parse_prog(void);
    unique_ptr<BooleanAst> parse_bool(void);
    unique_ptr<Ast> parse_expression(void);
    unique_ptr<CallAst> parse_call(unique_ptr<Ast> func);
    unique_ptr<Ast> maybe_call(unique_ptr<Ast> (*parser)(Parser &));
    unique_ptr<Ast> maybe_binary(unique_ptr<Ast> left, int my_prec);
    unique_ptr<Ast> operator()();
    bool eof() const;
};
#endif