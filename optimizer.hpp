#ifndef OPTIMIZER_H_
#define OPTIMIZER_H_
#include "ast.hpp"
#include <memory>
std::pair<shared_ptr<Ast>, VarDefineEnv> optimize(shared_ptr<Ast>&);
#endif