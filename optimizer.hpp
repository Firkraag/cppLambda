#ifndef OPTIMIZER_H_
#define OPTIMIZER_H_
#include "ast.hpp"
#include <memory>
std::pair<unique_ptr<Ast>, VarDefineEnv> optimize(unique_ptr<Ast> ast);
#endif