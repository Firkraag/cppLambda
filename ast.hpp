#ifndef AST_H_
#define AST_H_
#include <vector>
#include <iostream>
#include <optional>
#include <memory>
#include <algorithm>
#include <any>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include "environment.hpp"
#include "object.hpp"
using llvm::Value;
using std::any;
using std::cout;
using std::endl;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;
using std::weak_ptr;
class VarDefine;
class LambdaAst;
using VarDefineEnv = shared_ptr<EnvironmentBase<shared_ptr<VarDefine>>>;
class Ast
{
protected:
    bool constant = false;

public:
    virtual ~Ast(){};
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) const = 0;
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const = 0;
    virtual bool operator==(const Ast &ast) const = 0;
    virtual string to_js() = 0;
    virtual unique_ptr<Ast> to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)>) = 0;
    virtual bool has_side_effect() = 0;
    virtual unique_ptr<Ast> optimize(LambdaAst *closure) = 0;
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment) {}
    virtual llvm::Value *codegen(std::map<std::string, llvm::Instruction *> &closure)
    {
        return nullptr;
    }
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

public:
    NumberAst(double value) : LiteralAst(make_shared<Double>(value)) {}
    NumberAst(shared_ptr<Object> object) : LiteralAst(object){};
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) const;
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const;
    virtual unique_ptr<Ast> to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)>);
    virtual unique_ptr<Ast> optimize(LambdaAst *closure);
    virtual bool has_side_effect()
    {
        return false;
    }
    virtual llvm::Value *codegen(std::map<std::string, llvm::Instruction *> &closure);
};
class BooleanAst : public LiteralAst
{

public:
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) const;
    BooleanAst(bool value) : LiteralAst(make_shared<Boolean>(value)){};
    BooleanAst(shared_ptr<Object> object) : LiteralAst(object){};
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const;
    virtual unique_ptr<Ast> optimize(LambdaAst *closure);
    virtual unique_ptr<Ast> to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)>);
    virtual bool has_side_effect()
    {
        return false;
    }
    // virtual llvm::Value *codegen();
};
class StringAst : public LiteralAst
{
public:
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) const;
    StringAst(const string &value) : LiteralAst(make_shared<String>(value)){};
    StringAst(shared_ptr<Object> object) : LiteralAst(object){};
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual unique_ptr<Ast> optimize(LambdaAst *closure);
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const;
    virtual unique_ptr<Ast> to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)>);
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
    shared_ptr<VarDefine> define;
    // VarDefineEnv env;
    VarAst(string name, shared_ptr<VarDefine> define = nullptr) : name(name), define(define) {}

    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) const;
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    const string &get_name() const
    {
        return name;
    }
    virtual unique_ptr<Ast> optimize(LambdaAst *closure);
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const;
    virtual unique_ptr<Ast> to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)>);
    virtual bool has_side_effect()
    {
        return false;
    }
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    void set_name(const string &name)
    {
        this->name = name;
    }
    virtual llvm::Value *codegen(std::map<std::string, llvm::Instruction *> &closure);
};
class VarDef
{
public:
    string name;
    unique_ptr<Ast> define;
    VarDef(string name, unique_ptr<Ast> define)
        : name(name), define(std::move(define)) {}
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
    LambdaAst(string name, vector<string> params, unique_ptr<Ast> body)
        : name(name), params(std::move(params)), body(std::move(body)) {}
    LambdaAst(string name, vector<string> params, unique_ptr<Ast> body, vector<string> iife_params)
        : name(name), params(params), body(std::move(body)), iife_params(iife_params) {}
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) const;
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const;
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
    const Ast &get_body() const
    {
        return *body;
    }
    Ast &get_body()
    {
        return *body;
    }
    const VarDefineEnv &get_env() const
    {
        return env;
    }
    vector<string> &get_iife_params()
    {
        return iife_params;
    }
    virtual unique_ptr<Ast> to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)>);
    virtual unique_ptr<Ast> optimize(LambdaAst *closure);
    virtual llvm::Value *codegen(std::map<std::string, llvm::Instruction *> &closure);
};
class LetAst : public Ast
{
private:
    vector<unique_ptr<VarDef>> vardefs;
    unique_ptr<Ast> body;

public:
    LetAst(vector<unique_ptr<VarDef>> vardefs, unique_ptr<Ast> body) : vardefs(std::move(vardefs)), body(std::move(body)) {}
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) const;
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const;
    virtual unique_ptr<Ast> optimize(LambdaAst *closure);
    virtual unique_ptr<Ast> to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)>);
    virtual bool has_side_effect()
    {
        return any_of(vardefs.begin(), vardefs.end(), [](unique_ptr<VarDef> &vardef) {
                   return vardef->define == nullptr ? false : vardef->define->has_side_effect();
               }) ||
               body->has_side_effect();
    }
    virtual llvm::Value *codegen(std::map<std::string, llvm::Instruction *> &closure);
};
class CallAst : public Ast
{

private:
    unique_ptr<Ast> func;
    vector<unique_ptr<Ast>> args;

public:
    CallAst(unique_ptr<Ast> func, vector<unique_ptr<Ast>> args) : func(std::move(func)), args(std::move(args)) {}
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) const;
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const;
    virtual unique_ptr<Ast> to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)>);
    virtual bool has_side_effect()
    {
        return true;
    }
    virtual unique_ptr<Ast> optimize(LambdaAst *closure);
    const vector<unique_ptr<Ast>> &get_args() const
    {
        return args;
    }
    vector<unique_ptr<Ast>> &get_args()
    {
        return args;
    }
    const Ast &get_func() const
    {
        return *func;
    }
    Ast &get_func()
    {
        return *func;
    }
    unique_ptr<Ast> move_func()
    {
        return std::move(func);
    }
    virtual llvm::Value *codegen(std::map<std::string, llvm::Instruction *> &closure);
};
class ProgAst : public Ast
{

private:
    vector<unique_ptr<Ast>> prog;

public:
    ProgAst(vector<unique_ptr<Ast>> prog) : prog(std::move(prog)){};
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) const;
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const;
    virtual unique_ptr<Ast> to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)>);
    virtual unique_ptr<Ast> optimize(LambdaAst *closure);
    const vector<unique_ptr<Ast>> &get_prog() const
    {
        return prog;
    }
    virtual bool has_side_effect()
    {
        return any_of(prog.begin(), prog.end(), [](unique_ptr<Ast> &expr) {
            return expr->has_side_effect();
        });
    }
    virtual llvm::Value *codegen(std::map<std::string, llvm::Instruction *> &closure);
};
class IfAst : public Ast
{
private:
    unique_ptr<Ast> cond;
    unique_ptr<Ast> then;
    unique_ptr<Ast> else_;

public:
    IfAst(unique_ptr<Ast> cond, unique_ptr<Ast> then, unique_ptr<Ast> else_) : cond(std::move(cond)), then(std::move(then)), else_(std::move(else_)){};
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) const;
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const;
    virtual unique_ptr<Ast> to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)>);
    virtual unique_ptr<Ast> optimize(LambdaAst *closure);
    virtual bool has_side_effect()
    {
        return cond->has_side_effect() || then->has_side_effect() || else_->has_side_effect();
    }
    const Ast &get_cond() const
    {
        return *cond;
    }
    const Ast &get_then() const
    {
        return *then;
    }
    const Ast &get_else() const
    {
        return *else_;
    }
    virtual llvm::Value *codegen(std::map<std::string, llvm::Instruction *> &closure);
};
class BinaryAst : public Ast
{

private:
    string operator_;
    unique_ptr<Ast> left;
    unique_ptr<Ast> right;

public:
    BinaryAst(string operator_, unique_ptr<Ast> left, unique_ptr<Ast> right)
        : operator_(operator_), left(std::move(left)), right(std::move(right)) {}
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) const;
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const;
    virtual unique_ptr<Ast> to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)>);
    virtual unique_ptr<Ast> optimize(LambdaAst *closure);
    const string &get_operator() const
    {
        return operator_;
    }
    const Ast &get_left() const
    {
        return *left;
    }
    const Ast &get_right() const
    {
        return *right;
    }
    unique_ptr<Ast> move_left()
    {
        return std::move(left);
    }
    unique_ptr<Ast> move_right()
    {
        return std::move(right);
    }
    virtual bool has_side_effect()
    {
        return left->has_side_effect() || right->has_side_effect();
    }
    virtual llvm::Value *codegen(std::map<std::string, llvm::Instruction *> &closure);
};
class AssignAst : public Ast
{
private:
    unique_ptr<Ast> left;
    unique_ptr<Ast> right;

public:
    AssignAst(unique_ptr<Ast> left, unique_ptr<Ast> right)
        : left(std::move(left)), right(std::move(right)) {}
    virtual shared_ptr<Object> evaluate(shared_ptr<Environment> environment) const;
    virtual bool operator==(const Ast &ast) const;
    virtual string to_js();
    virtual void evaluate_callback(shared_ptr<Environment> environment, function<void(shared_ptr<Object>)> callback) const;
    virtual unique_ptr<Ast> to_cps(function<unique_ptr<Ast>(unique_ptr<Ast>)>);
    const string &get_operator() const;
    virtual bool has_side_effect()
    {
        return true;
    }
    virtual unique_ptr<Ast> optimize(LambdaAst *closure);
    virtual void make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment);
    virtual llvm::Value *codegen(std::map<std::string, llvm::Instruction *> &closure);
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
    const Ast *current_value;

    int assigned = 0;
    vector<VarAst *> refs{};
    VarDefine(VarKind var_kind) : var_kind(var_kind){};
};
extern bool OptimizeChange;
#endif