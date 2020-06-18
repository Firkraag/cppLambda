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
using namespace std;

class Parser
{
private:
    static map<string, int> PRECEDENCE;

public:
    TokenStream &token_stream;
    Parser(TokenStream &token_stream) : token_stream(token_stream) {}
    void skip_punc(char c)
    {
        if (is_punc(c))
        {
            token_stream.next();
        }
        else
        {
            throw token_stream.error_msg("Expecting punctuation: " + to_string(c));
        }
    }
    bool is_punc(char c)
    {
        auto token = token_stream.peek();
        return token == Token(PunctuationToken, c);
    }
    void skip_keyword(const string &keyword)
    {
        if (is_keyword(keyword))
        {
            token_stream.next();
        }
        else
        {
            throw token_stream.error_msg("Expecting keyword: " + keyword);
        }
    }
    bool is_keyword(const string &keyword)
    {
        auto token = token_stream.peek();
        return token == Token(KeywordToken, keyword);
    }
    void skip_operator(const string &operator_)
    {
        if (is_operator(operator_))
        {
            token_stream.next();
        }
        else
        {
            throw token_stream.error_msg("Expecting operator: " + operator_);
        }
    }
    bool is_operator(const string &operator_)
    {
        auto token = token_stream.peek();
        return token == Token(OperatorToken, operator_);
    }
    vector<string> delimited(char start, char stop, char separator, string (*parser)(Parser &p))
    {
        vector<string> list;
        bool first = true;
        skip_punc(start);
        while (!token_stream.eof())
        {
            if (is_punc(stop))
            {
                break;
            }
            if (first)
            {
                first = false;
            }
            else
            {
                skip_punc(separator);
            }
            if (is_punc(stop))
            {
                break;
            }
            list.push_back(parser(*this));
        }
        return list;
    }
    vector<VarDefAst> delimited(char start, char stop, char separator, VarDefAst (*parser)(Parser &p))
    {
        vector<VarDefAst> list;
        bool first = true;
        skip_punc(start);
        while (!token_stream.eof())
        {
            if (is_punc(stop))
            {
                break;
            }
            if (first)
            {
                first = false;
            }
            else
            {
                skip_punc(separator);
            }
            if (is_punc(stop))
            {
                break;
            }
            list.push_back(parser(*this));
        }
        return list;
    }
    vector<Ast> delimited(char start, char stop, char separator, Ast (*parser)(Parser &p))
    {
        vector<Ast> list;
        bool first = true;
        skip_punc(start);
        while (!token_stream.eof())
        {
            if (is_punc(stop))
            {
                break;
            }
            if (first)
            {
                first = false;
            }
            else
            {
                skip_punc(separator);
            }
            if (is_punc(stop))
            {
                break;
            }
            list.push_back(parser(*this));
        }
        skip_punc(stop);
        return list;
    }
    LambdaAst parse_lambda(const string &keyword)
    {
        skip_keyword(keyword);
        string name;
        if (token_stream.peek().first == VariableToken)
        {
            name = get<string>(token_stream.next().second);
        }
        else
        {
            name = "";
        }
        // auto f = bind(&Parser::parse_varname, *this);
        auto params = delimited('(', ')', ',', [](Parser &p) {
            return p.parse_varname();
        });
        auto body = parse_expression();
        return LambdaAst(name, params, body);
    }
    Ast parse_let(void)
    {
        skip_keyword("let");
        if (token_stream.peek().first == VariableToken)
        {
            auto name = get<string>(token_stream.next().second);
            vector<VarDefAst> vardefs = delimited('(', ')', ',', [](Parser &p) -> VarDefAst {
                return p.parse_vardef();
            });
            vector<string> varnames;
            vector<Ast> defines;
            for (VarDefAst &vardef : vardefs)
            {
                varnames.push_back(vardef.name);
                if (vardef.define)
                {
                    defines.push_back(vardef.define.value());
                }
                else
                {
                    defines.push_back(BooleanAst(false));
                }
            }
            return CallAst(
                LambdaAst(
                    name,
                    varnames,
                    parse_expression()),
                defines);
        }
        vector<VarDefAst> vardefs = delimited('(', ')', ',', [](Parser &p) {
            return p.parse_vardef();
        });
        Ast body = parse_expression();
        return LetAst(vardefs, body);
    }
    VarDefAst parse_vardef(void)
    {
        string name = parse_varname();
        optional<Ast> define;
        if (is_operator("="))
        {
            token_stream.next();
            define = parse_expression();
        }
        return VarDefAst(name, define);
    }
    string parse_varname(void)
    {
        // TokenStream &token_stream = parser.token_stream;
        Token token = token_stream.next();
        if (token.first == VariableToken)
        {
            return get<string>(token.second);
        }
        else
        {
            throw token_stream.error_msg("Expecting variable name");
        }
    }
    ProgAst parse_toplevel(void)
    {
        vector<Ast> prog;
        while (!token_stream.eof())
        {
            prog.push_back(parse_expression());
            if (!token_stream.eof())
            {
                skip_punc(';');
            }
        }
        return ProgAst(prog);
    }
    IfAst parse_if(void)
    {
        skip_keyword("if");
        Ast cond = parse_expression();
        if (!is_punc('{'))
        {
            skip_keyword("then");
        }
        Ast then = parse_expression();
        Ast else_ = BooleanAst(false);
        if (is_keyword("else"))
        {
            skip_keyword("else");
            else_ = parse_expression();
        }
        return IfAst(cond, then, else_);
    }
    Ast parse_atom(void)
    {
        auto parser = [](Parser &parser) -> Ast {
            if (parser.is_punc('('))
            {
                parser.skip_punc('(');
                Ast exp = parser.parse_expression();
                parser.skip_punc(')');
                return exp;
            }
            if (parser.is_punc('{'))
            {
                return parser.parse_prog();
            }
            if (parser.is_keyword("if"))
            {
                return parser.parse_if();
            }
            if (parser.is_keyword("let"))
            {
                return parser.parse_let();
            }
            if (parser.is_keyword("true") || parser.is_keyword("false"))
            {
                return parser.parse_bool();
            }
            if (parser.is_keyword("lambda"))
            {
                return parser.parse_lambda("lambda");
            }
            auto token = parser.token_stream.next();
            switch (token.first)
            {
            case NumToken:
                return NumberAst(get<double>(token.second));
                break;
            case StringToken:
                return StringAst(get<string>(token.second));
                break;
            case VariableToken:
                return VarAst(get<string>(token.second));
                break;
            default:
                break;
            }
            throw parser.token_stream.error_msg("Unexpected token");
        };
        return maybe_call(parser);
    }
    ProgAst parse_prog(void)
    {
        vector<Ast> prog = delimited('{', ')', ';', [](Parser &p) {
            return p.parse_expression();
        });
        return ProgAst(prog);
    }
    Ast parse_bool(void)
    {
        auto token = token_stream.next();
        assert(token.first == KeywordToken);
        return BooleanAst(get<string>(token.second) == "true");
    }
    Ast parse_expression(void)
    {
        auto parser = [](Parser &p) -> Ast {
            Ast ast = p.maybe_binary(p.parse_atom(), 0);
            if (BinaryAst *binary_ast = dynamic_cast<BinaryAst *>(&ast))
            {
                // BinaryAst binary_ast = cast(BinaryAst, ast);
                if (binary_ast->operator_ == "||")
                {
                    const string iife_param = generate_unique_symbol("left");
                    vector<string> params = {iife_param};
                    LambdaAst func = LambdaAst("", params, IfAst(VarAst(iife_param), VarAst(iife_param), binary_ast->right));
                    vector<Ast> args = {binary_ast->left};
                    ast = CallAst(func, args);
                }
                else if (binary_ast->operator_ == "&&")
                {
                    ast = IfAst(binary_ast->left, binary_ast->right, BooleanAst(false));
                }
            }
            return ast;
        };
        return maybe_call(parser);
    }
    CallAst parse_call(Ast func)
    {
        return CallAst(func, delimited('(', ')', ',', [](Parser &p) -> Ast {
                           return p.parse_expression();
                       }));
    }
    Ast maybe_call(Ast (*parser)(Parser &))
    {
        Ast expr = parser(*this);
        return is_punc('(') ? parse_call(expr) : expr;
    }
    Ast maybe_binary(const Ast &left, int my_prec)
    {
        auto token = token_stream.peek();
        if (token.first != OperatorToken)
        {
            return left;
        }
        string token_value = get<string>(token.second);
        int his_prec = PRECEDENCE[token_value];
        if (his_prec > my_prec)
        {
            token_stream.next();
            auto right = maybe_binary(parse_atom(), his_prec);
            Ast binary;
            if (token_value == "=")
            {
                binary = AssignAst(left, right);
            }
            else
            {
                binary = BinaryAst(token_value, left, right);
            }
            return maybe_binary(binary, my_prec);
        }
        return left;
    }
};
map<string, int> Parser::PRECEDENCE{
    {"=", 1},
    {"||", 2},
    {"&&", 3},
    {"<", 7},
    {">", 7},
    {"<=", 7},
    {">=", 7},
    {"==", 7},
    {"!=", 7},
    {"+", 10},
    {"-", 10},
    {"*", 20},
    {"/", 20},
    {"%", 20},
};
#endif