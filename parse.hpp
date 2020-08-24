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
    TokenStream token_stream;
    Parser(TokenStream token_stream) : token_stream(token_stream) {}
    void skip_punc(char c);
    bool is_punc(char c);
    void skip_keyword(const string &keyword);
    bool is_keyword(const string &keyword);
    void skip_operator(const string &operator_);
    bool is_operator(const string &operator_);
    vector<string> delimited(char start, char stop, char separator, string (*parser)(Parser &p));
    vector<shared_ptr<VarDef>> delimited(char start, char stop, char separator, shared_ptr<VarDef> (*parser)(Parser &p));
    vector<shared_ptr<Ast>> delimited(char start, char stop, char separator, shared_ptr<Ast> (*parser)(Parser &p));
    shared_ptr<LambdaAst> parse_lambda(const string &keyword);
    shared_ptr<Ast> parse_let(void);
    shared_ptr<VarDef> parse_vardef(void);
    string parse_varname(void);
    shared_ptr<ProgAst> parse_toplevel(void);
    shared_ptr<IfAst> parse_if(void);
    shared_ptr<Ast> parse_atom(void);
    shared_ptr<ProgAst> parse_prog(void);
    shared_ptr<BooleanAst> parse_bool(void);
    shared_ptr<Ast> parse_expression(void);
    shared_ptr<CallAst> parse_call(shared_ptr<Ast> func);
    shared_ptr<Ast> maybe_call(shared_ptr<Ast> (*parser)(Parser &));
    shared_ptr<Ast> maybe_binary(shared_ptr<Ast> left, int my_prec);
    shared_ptr<Ast> operator()();
};
#endif