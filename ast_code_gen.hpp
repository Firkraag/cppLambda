#ifndef AST_CODE_GEN_H_
#define AST_CODE_GEN_H_
#include "ast.hpp"
#include <cstdlib>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/Support/raw_os_ostream.h>
using llvm::APFloat;
using llvm::BasicBlock;
using llvm::CallInst;
using llvm::ConstantExpr;
using llvm::ConstantFP;
using llvm::errs;
using llvm::FunctionType;
using llvm::IRBuilder;
using llvm::LLVMContext;
using llvm::Module;
using llvm::Type;
using llvm::verifyFunction;

extern LLVMContext TheContext;
extern IRBuilder<> Builder;
extern std::unique_ptr<Module> TheModule;
#endif