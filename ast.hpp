#ifndef AST_H_
#define AST_H_
#include <vector>
#include <iostream>
#include <optional>
#include <memory>
#include <algorithm>
#include <any>
#include "environment.hpp"
#include "object.hpp"
// using namespace std;
using std::any;
using std::cout;
using std::endl;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::vector;
using std::weak_ptr;
class VarDefine;
class LambdaAst;
using VarDefineEnv = shared_ptr<EnvironmentBase<shared_ptr<VarDefine>>>;
class Ast : public enable_shared_from_this<Ast>
{
protected:
    bool constant = false;

public:
    virtual ~Ast() {};
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) = 0;
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) = 0;
    virtual bool operator==(const Ast &ast) const = 0;
    virtual string to_js() = 0;
    virtual shared_ptr<Ast> to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)>) = 0;
    virtual bool has_side_effect() = 0;
    virtual shared_ptr<Ast> optimize(shared_ptr<LambdaAst> closure)
    {
        return shared_from_this();
    }
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment) {}
};
class LiteralAst : public Ast
{
protected:
    shared_ptr<Object> value;
    LiteralAst(shared_ptr<Object> value) : value(value) {}

public:
    const shared_ptr<Object> &get_value() const
    {
        return value;
    }
};
class NumberAst : public LiteralAst
{
    // private:
    //     double value;

    // protected:
    //     bool constant = true;

public:
    NumberAst(double value) : LiteralAst(make_shared<Double>(value)) {}
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment);
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback);
    virtual shared_ptr<Ast> to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)>);
    virtual bool has_side_effect()
    {
        return false;
    }
};
class BooleanAst : public LiteralAst
{

public:
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment);
    BooleanAst(bool value) : LiteralAst(make_shared<Boolean>(value)) {};
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback);
    virtual shared_ptr<Ast> to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)>);
    virtual bool has_side_effect()
    {
        return false;
    }
    // virtual ostream &operator<<(ostream &output);
};
class StringAst : public LiteralAst
{

public:
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment);
    StringAst(string value) : LiteralAst(make_shared<String>(value)) {};
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback);
    virtual shared_ptr<Ast> to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)>);
    virtual bool has_side_effect()
    {
        return false;
    }
};
class VarAst : public Ast
{
private:
    string name;

public:
    weak_ptr<VarDefine> define;
    // VarDefineEnv env;
    VarAst(string name, shared_ptr<VarDefine> define = nullptr) : name(name), define(define) {}

    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment);
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    const string &get_name() const
    {
        return name;
    }
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback);
    virtual shared_ptr<Ast> to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)>);
    virtual bool has_side_effect()
    {
        return false;
    }
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    void set_name(const string &name) {
        this->name = name;
    }
};
class VarDef
{
public:
    string name;
    shared_ptr<Ast> define;
    VarDef(string name, shared_ptr<Ast> define)
        : name(name), define(define) {}
    bool operator==(const VarDef &vardef_ast) const;
};
class LambdaAst : public Ast
{
private:
    string name;
    vector<string> params;
    shared_ptr<Ast> body;
    vector<string> iife_params{};
    VarDefineEnv env = nullptr;

public:
    LambdaAst(string name, vector<string> params, shared_ptr<Ast> body)
        : name(name), params(params), body(body) {}
    LambdaAst(string name, vector<string> params, shared_ptr<Ast> body, vector<string> iife_params)
        : name(name), params(params), body(body), iife_params(iife_params) {}
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment);
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback);
    virtual bool has_side_effect()
    {
        return false;
    }
    const vector<string> &get_params() const
    {
        return params;
    }
    const string &get_name() const
    {
        return name;
    }
    const shared_ptr<Ast> &get_body() const
    {
        return body;
    }
    const VarDefineEnv &get_env() const
    {
        return env;
    }
    vector<string> &get_iife_params()
    {
        return iife_params;
    }
    virtual shared_ptr<Ast> to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)>);
    virtual shared_ptr<Ast> optimize(shared_ptr<LambdaAst> closure);
};
class LetAst : public Ast
{
private:
    vector<shared_ptr<VarDef>> vardefs;
    shared_ptr<Ast> body;

public:
    LetAst(vector<shared_ptr<VarDef>> vardefs, shared_ptr<Ast> body) : vardefs(vardefs), body(body) {}
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment);
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback);
    virtual shared_ptr<Ast> to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)>);
    virtual bool has_side_effect()
    {
        return any_of(vardefs.begin(), vardefs.end(), [](shared_ptr<VarDef> vardef) {
            return vardef->define == nullptr ? false : vardef->define->has_side_effect();
            }) ||
            body->has_side_effect();
    }
};
class CallAst : public Ast
{

private:
    shared_ptr<Ast> func;
    vector<shared_ptr<Ast>> args;

public:
    CallAst(shared_ptr<Ast> func, vector<shared_ptr<Ast>> args) : func(func), args(args) {}
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment);
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback);
    virtual shared_ptr<Ast> to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)>);
    virtual bool has_side_effect()
    {
        return true;
    }
    virtual shared_ptr<Ast> optimize(shared_ptr<LambdaAst> closure);
    const vector<shared_ptr<Ast>> &get_args() const
    {
        return args;
    }
    const shared_ptr<Ast> &get_func() const
    {
        return func;
    }
};
class ProgAst : public Ast
{

private:
    vector<shared_ptr<Ast>> prog;

public:
    ProgAst(vector<shared_ptr<Ast>> prog) : prog(prog) {};
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment);
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback);
    virtual shared_ptr<Ast> to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)>);
    virtual shared_ptr<Ast> optimize(shared_ptr<LambdaAst> closure);
    const vector<shared_ptr<Ast>> &get_prog() const
    {
        return prog;
    }
    virtual bool has_side_effect()
    {
        return any_of(prog.begin(), prog.end(), [](shared_ptr<Ast> expr) {
            return expr->has_side_effect();
            });
    }
};
class IfAst : public Ast
{
private:
    shared_ptr<Ast> cond;
    shared_ptr<Ast> then;
    shared_ptr<Ast> else_;

public:
    IfAst(shared_ptr<Ast> cond, shared_ptr<Ast> then, shared_ptr<Ast> else_) : cond(cond), then(then), else_(else_) {};
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment);
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback);
    virtual shared_ptr<Ast> to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)>);
    virtual shared_ptr<Ast> optimize(shared_ptr<LambdaAst> closure);
    virtual bool has_side_effect()
    {
        return cond->has_side_effect() || then->has_side_effect() || else_->has_side_effect();
    }
    const shared_ptr<Ast> &get_cond() const
    {
        return cond;
    }
    const shared_ptr<Ast> &get_then() const
    {
        return then;
    }
    const shared_ptr<Ast> &get_else() const
    {
        return else_;
    }
};
class BinaryAst : public Ast
{

private:
    string operator_;
    shared_ptr<Ast> left;
    shared_ptr<Ast> right;

public:
    BinaryAst(string operator_, shared_ptr<Ast> left, shared_ptr<Ast> right)
        : operator_(operator_), left(left), right(right) {}
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment);
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback);
    virtual shared_ptr<Ast> to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)>);
    virtual shared_ptr<Ast> optimize(shared_ptr<LambdaAst> closure);
    const string &get_operator() const
    {
        return operator_;
    }
    const shared_ptr<Ast> &get_left() const
    {
        return left;
    }
    const shared_ptr<Ast> &get_right() const
    {
        return right;
    }
    virtual bool has_side_effect()
    {
        return left->has_side_effect() || right->has_side_effect();
    }
};
class AssignAst : public Ast
{
private:
    shared_ptr<Ast> left;
    shared_ptr<Ast> right;

public:
    AssignAst(shared_ptr<Ast> left, shared_ptr<Ast> right)
        : left(left), right(right) {}
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment);
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback);
    virtual shared_ptr<Ast> to_cps(function<shared_ptr<Ast>(shared_ptr<Ast>)>);
    const string &get_operator() const;
    virtual bool has_side_effect()
    {
        return true;
    }
    virtual shared_ptr<Ast> optimize(shared_ptr<LambdaAst> closure);
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
};
enum class VarKind
{
    GlobalVar,
    LambdaParam,
    IifeParam,
};
class VarDefine
{
private:
public:
    VarKind var_kind;
    weak_ptr<Ast> current_value;

    int assigned = 0;
    vector<shared_ptr<VarAst>> refs{};
    VarDefine(VarKind var_kind) : var_kind(var_kind) {};
};
extern bool OptimizeChange;
#endif