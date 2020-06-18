#ifndef AST_H_
#define AST_H_
#include <vector>
#include <iostream>
#include <optional>
using namespace std;
class Ast
{
public:
    virtual ~Ast(){};
    virtual bool operator==(const Ast &ast) const
    {
        // cout << "Ast comparasion equality" << endl;
        return false;
    }
    virtual ostream &operator<<(ostream &output) {
        output << "Ast()";
        return output;
    }
};
class LiteralAst : public Ast
{
};
class NumberAst : public LiteralAst
{
public:
    double value;
    NumberAst(double value) : value(value){};
    virtual bool operator==(const Ast &ast) const
    {
        const NumberAst *number_ast = dynamic_cast<const NumberAst *>(&ast);
        return number_ast != NULL && number_ast->value == value;
    }
    virtual ostream &operator<<(ostream &output)
    {
        // const NumberAst *number_ast = dynamic_cast<const NumberAst *>(&ast);
        output << "NumberAst(value=" << value << ")";
        return output;
    }
};
class BooleanAst : public LiteralAst
{
public:
    bool value;
    BooleanAst(bool value) : value(value){};
    virtual bool operator==(const Ast &ast) const
    {
        const BooleanAst *boolean_ast = dynamic_cast<const BooleanAst *>(&ast);
        return boolean_ast != NULL && boolean_ast->value == value;
    }
    virtual ostream &operator<<(ostream &output)
    {
        output << "BooleanAst(value=" << value << ")";
        return output;
    }
};
class StringAst : public LiteralAst
{
public:
    string value;
    StringAst(string value) : value(value){};
    virtual bool operator==(const Ast &ast) const
    {
        const StringAst *string_ast = dynamic_cast<const StringAst *>(&ast);
        return string_ast != NULL && string_ast->value == value;
    }
    virtual ostream &operator<<(ostream &output)
    {
        output << "String(value=" << value << ")";
        return output;
    }
};
class VarAst : public Ast
{
public:
    string name;
    VarAst(string name) : name(name) {}
    virtual bool operator==(const Ast &ast) const
    {
        const VarAst *var_ast = dynamic_cast<const VarAst *>(&ast);
        return var_ast != NULL && var_ast->name == name;
    }
    virtual ostream &operator<<(ostream &output)
    {
        output << "VarAst(name=" << name << ")";
        return output;
    }
};
class VarDefAst : public Ast
{
public:
    string name;
    optional<Ast> define;
    VarDefAst(string name, optional<Ast> define)
        : name(name), define(define) {}
};
class LambdaAst : public Ast
{
public:
    string name;
    vector<string> params;
    Ast body;
    LambdaAst(string name, vector<string> params, Ast body)
        : name(name), params(params), body(body) {}
    virtual bool operator==(const Ast &ast) const
    {
        const LambdaAst *lambda_ast = dynamic_cast<const LambdaAst *>(&ast);
        return lambda_ast != NULL && name == lambda_ast->name && params == lambda_ast->params && body == lambda_ast->body;
    }
    // virtual ostream &operator<<(ostream &output)
    // {
    //     output << "LambdaAst(name=" << name << ", params=" << params << ", body=" << body << ")";
    //     return output;
    // }
};
class LetAst : public Ast
{
public:
    vector<VarDefAst> vardefs;
    Ast body;
    LetAst(vector<VarDefAst> vardefs, Ast body) : vardefs(vardefs), body(body) {}
    virtual bool operator==(const Ast &ast) const
    {
        const LetAst *let_ast = dynamic_cast<const LetAst *>(&ast);
        return let_ast != NULL && let_ast->vardefs == vardefs && let_ast->body == body;
    }
    // virtual ostream &operator<<(ostream &output)
    // {
    //     output << "LetAst(vardefs=" << vardefs << ", body=" << body << ")";
    //     return output;
    // }
};
class CallAst : public Ast
{
public:
    Ast func;
    vector<Ast> args;
    CallAst(Ast func, vector<Ast> args) : func(func), args(args) {}
    virtual bool operator==(const Ast &ast) const
    {
        const CallAst *call_ast = dynamic_cast<const CallAst *>(&ast);
        return call_ast != NULL && call_ast->func == func && call_ast->args == args;
    }
    // virtual ostream &operator<<(ostream &output)
    // {
    //     output << "CallAst(func=" << func << ", args=" << args << ")";
    //     return output;
    // }
};
class ProgAst : public Ast
{
public:
    vector<Ast> prog;
    ProgAst(vector<Ast> prog) : prog(prog){};
    virtual bool operator==(const Ast &ast) const
    {
        const ProgAst *prog_ast = dynamic_cast<const ProgAst *>(&ast);
        return prog_ast != NULL && prog_ast->prog == prog;
    }
    // virtual ostream &operator<<(ostream &output)
    // {
    //     output << "ProgAst(prog=" << prog << ")";
    //     return output;
    // }
};
class IfAst : public Ast
{
public:
    Ast cond;
    Ast then;
    Ast else_;
    IfAst(Ast cond, Ast then, Ast else_) : cond(cond), then(then), else_(else_){};
    virtual bool operator==(const Ast &ast) const
    {
        const IfAst *if_ast = dynamic_cast<const IfAst *>(&ast);
        return if_ast != NULL && if_ast->cond == cond && if_ast->then == then && if_ast->else_ == else_;
    }
    // virtual ostream &operator<<(ostream &output)
    // {
    //     output << "IfAst(cond=" << cond << ", then=" << then << ", else = " << else_ << ")";
    //     return output;
    // }
};
class BinaryAst : public Ast
{
public:
    string operator_;
    Ast left;
    Ast right;
    BinaryAst(string operator_, Ast left, Ast right)
        : operator_(operator_), left(left), right(right) {}
    virtual bool operator==(const Ast &ast) const
    {
        const BinaryAst *binary_ast = dynamic_cast<const BinaryAst *>(&ast);
        return binary_ast != NULL && binary_ast->left == left && binary_ast->right == right && binary_ast->operator_ == operator_;
    }
    // virtual ostream &operator<<(ostream &output)
    // {
    //     output << "BinaryAst(operator=" << operator_ << ", left=" << left << ", right=" << right << ")";
    //     return output;
    // }
};
class AssignAst : public Ast
{
public:
    Ast left;
    Ast right;
    AssignAst(Ast left, Ast right)
        : left(left), right(right) {}
    virtual bool operator==(const Ast &ast) const
    {
        const AssignAst *assign_ast = dynamic_cast<const AssignAst *>(&ast);
        return assign_ast != NULL && assign_ast->left == left && assign_ast->right == right;
    }
    // virtual ostream &operator<<(ostream &output)
    // {
    //     output << "AssignAst(left=" << left << ", right=" << right << ")";
    //     return output;
    // }
};
#endif