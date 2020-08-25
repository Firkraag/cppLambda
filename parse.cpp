#include "parse.hpp"

map<string, int> Parser::PRECEDENCE =
    {
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
void Parser::skip_punc(char c)
{
    if (is_punc(c))
    {
        token_stream->next();
    }
    else
    {
        throw token_stream->error_msg("Expecting punctuation: " + to_string(c));
    }
}
bool Parser::is_punc(char c)
{
    auto token = token_stream->peek();
    return token == Token(PunctuationToken, c);
}
void Parser::skip_keyword(const string &keyword)
{
    if (is_keyword(keyword))
    {
        token_stream->next();
    }
    else
    {
        throw token_stream->error_msg("Expecting keyword: " + keyword);
    }
}
bool Parser::is_keyword(const string &keyword)
{
    auto token = token_stream->peek();
    return token == Token(KeywordToken, keyword);
}
void Parser::skip_operator(const string &operator_)
{
    if (is_operator(operator_))
    {
        token_stream->next();
    }
    else
    {
        throw token_stream->error_msg("Expecting operator: " + operator_);
    }
}
bool Parser::is_operator(const string &operator_)
{
    auto token = token_stream->peek();
    return token == Token(OperatorToken, operator_);
}
vector<string> Parser::delimited(char start, char stop, char separator, string (*parser)(Parser &p))
{
    vector<string> list;
    bool first = true;
    skip_punc(start);
    while (!token_stream->eof())
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
vector<shared_ptr<VarDef>> Parser::delimited(char start, char stop, char separator, shared_ptr<VarDef> (*parser)(Parser &p))
{
    vector<shared_ptr<VarDef>> list;
    bool first = true;
    skip_punc(start);
    while (!token_stream->eof())
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
vector<shared_ptr<Ast>> Parser::delimited(char start, char stop, char separator, shared_ptr<Ast> (*parser)(Parser &p))
{
    vector<shared_ptr<Ast>> list;
    bool first = true;
    skip_punc(start);
    while (!token_stream->eof())
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
unique_ptr<LambdaAst> Parser::parse_lambda(const string &keyword)
{
    skip_keyword(keyword);
    string name;
    if (token_stream->peek().first == VariableToken)
    {
        name = get<string>(token_stream->next().second);
    }
    else
    {
        name = "";
    }
    vector<string> params = delimited('(', ')', ',', [](Parser &p) {
        return p.parse_varname();
    });
    return make_unique<LambdaAst>(name, params, parse_expression());
}
unique_ptr<Ast> Parser::parse_let(void)
{
    skip_keyword("let");
    if (token_stream->peek().first == VariableToken)
    {
        auto name = get<string>(token_stream->next().second);
        vector<shared_ptr<VarDef>> vardefs = delimited('(', ')', ',', [](Parser &p) -> shared_ptr<VarDef> {
            return p.parse_vardef();
        });
        vector<string> varnames;
        vector<shared_ptr<Ast>> defines;
        for (auto &vardef : vardefs)
        {
            varnames.push_back(vardef->name);
            if (vardef->define)
            {
                defines.push_back(vardef->define);
            }
            else
            {
                defines.push_back(make_unique<BooleanAst>(false));
            }
        }
        shared_ptr<Ast> func(new LambdaAst(name, std::move(varnames), parse_expression()));
        return make_unique<CallAst>(func, std::move(defines));
    }
    vector<shared_ptr<VarDef>> vardefs = delimited('(', ')', ',', [](Parser &p) {
        return p.parse_vardef();
    });
    return make_unique<LetAst>(std::move(vardefs), parse_expression());
}
shared_ptr<VarDef> Parser::parse_vardef(void)
{
    string name = parse_varname();
    shared_ptr<Ast> define;
    if (is_operator("="))
    {
        token_stream->next();
        define = parse_expression();
    }
    return make_shared<VarDef>(name, define);
}
string Parser::parse_varname(void)
{
    // TokenStream &token_stream = parser.token_stream;
    Token token = token_stream->next();
    if (token.first == VariableToken)
    {
        return get<string>(token.second);
    }
    else
    {
        throw token_stream->error_msg("Expecting variable name");
    }
}
unique_ptr<ProgAst> Parser::parse_toplevel(void)
{
    vector<shared_ptr<Ast>> prog;
    while (!token_stream->eof())
    {
        prog.push_back(parse_expression());
        if (!token_stream->eof())
        {
            skip_punc(';');
        }
    }
    return make_unique<ProgAst>(std::move(prog));
}
unique_ptr<IfAst> Parser::parse_if(void)
{
    skip_keyword("if");
    shared_ptr<Ast> cond = parse_expression();
    if (!is_punc('{'))
    {
        skip_keyword("then");
    }
    shared_ptr<Ast> then = parse_expression();
    shared_ptr<Ast> else_(new BooleanAst(false));
    if (is_keyword("else"))
    {
        skip_keyword("else");
        else_ = parse_expression();
    }
    return make_unique<IfAst>(cond, then, else_);
}
unique_ptr<Ast> Parser::parse_atom(void)
{
    auto parser = [](Parser &parser) -> unique_ptr<Ast> {
        if (parser.is_punc('('))
        {
            parser.skip_punc('(');
            auto exp = parser.parse_expression();
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
        auto token = parser.token_stream->next();
        switch (token.first)
        {
        case NumToken:
            return make_unique<NumberAst>(get<double>(token.second));
            break;
        case StringToken:
            return make_unique<StringAst>(get<string>(token.second));
            break;
        case VariableToken:
            return make_unique<VarAst>(get<string>(token.second));
            break;
        default:
            break;
        }
        throw parser.token_stream->error_msg("Unexpected token");
    };
    return maybe_call(parser);
}
unique_ptr<ProgAst> Parser::parse_prog(void)
{
    vector<shared_ptr<Ast>> prog = delimited('{', '}', ';', [](Parser &p) -> shared_ptr<Ast> {
        return p.parse_expression();
    });
    return make_unique<ProgAst>(std::move(prog));
}
unique_ptr<BooleanAst> Parser::parse_bool(void)
{
    auto token = token_stream->next();
    assert(token.first == KeywordToken);
    return make_unique<BooleanAst>(get<string>(token.second) == "true");
}
unique_ptr<Ast> Parser::parse_expression(void)
{
    auto parser = [](Parser &p) -> unique_ptr<Ast> {
        unique_ptr<Ast> ast = p.maybe_binary(p.parse_atom(), 0);
        if (BinaryAst *binary_ast = dynamic_cast<BinaryAst *>(ast.get()))
        {
            if (binary_ast->get_operator() == "||")
            {
                const string iife_param = generate_unique_symbol("left");
                vector<string> params = {iife_param};
                shared_ptr<IfAst> if_ast(new IfAst(shared_ptr<VarAst>(new VarAst(iife_param)), shared_ptr<VarAst>(new VarAst(iife_param)), binary_ast->get_right()));
                shared_ptr<LambdaAst> func = shared_ptr<LambdaAst>(new LambdaAst("", params, if_ast));
                vector<shared_ptr<Ast>> args = {binary_ast->get_left()};
                ast = make_unique<CallAst>(func, args);
            }
            else if (binary_ast->get_operator() == "&&")
            {
                ast = make_unique<IfAst>(binary_ast->get_left(), binary_ast->get_right(), shared_ptr<BooleanAst>(new BooleanAst(false)));
            }
        }
        return ast;
    };
    return maybe_call(parser);
}
unique_ptr<CallAst> Parser::parse_call(unique_ptr<Ast> func)
{
    return make_unique<CallAst>(std::move(func), delimited('(', ')', ',', [](Parser &p) -> shared_ptr<Ast> {
                                    return p.parse_expression();
                                }));
}
unique_ptr<Ast> Parser::maybe_call(unique_ptr<Ast> (*parser)(Parser &))
{
    auto expr = parser(*this);
    return is_punc('(') ? parse_call(std::move(expr)) : std::move(expr);
}
unique_ptr<Ast> Parser::maybe_binary(unique_ptr<Ast> left, int my_prec)
{
    auto token = token_stream->peek();
    if (token.first != OperatorToken)
    {
        return left;
    }
    string token_value = get<string>(token.second);
    int his_prec = PRECEDENCE[token_value];
    if (his_prec > my_prec)
    {
        token_stream->next();
        auto right = maybe_binary(parse_atom(), his_prec);
        unique_ptr<Ast> binary;
        if (token_value == "=")
        {
            binary = make_unique<AssignAst>(std::move(left), std::move(right));
        }
        else
        {
            binary = make_unique<BinaryAst>(token_value, std::move(left), std::move(right));
        }
        return maybe_binary(std::move(binary), my_prec);
    }
    return left;
}
unique_ptr<Ast> Parser::operator()()
{
    return parse_toplevel();
}
