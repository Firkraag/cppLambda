#ifndef EVALUATOR_H_
#define EVALUATOR_H_
#include <any>
#include "ast.hpp"
#include "environment.hpp"
using std::any;
// any evaluate(const Ast &ast, shared_ptr<Environment> environment);
double evaluate(const NumberAst &number_ast, shared_ptr<Environment> environment);
bool evaluate(const BooleanAst &boolean_ast, shared_ptr<Environment> environment);
string evaluate(const StringAst &string_ast, shared_ptr<Environment> environment);
any evaluate(const VarAst &var_ast, shared_ptr<Environment> environment);
// any evaluate(const BinaryAst &binary_ast, shared_ptr<Environment> environment)
// any evaluate(const AssignAst &assign_ast, shared_ptr<Environment> environment)
#endif