#include "ast_code_gen.hpp"
#include "utils.hpp"
LLVMContext TheContext;
IRBuilder<> Builder(TheContext);
std::unique_ptr<Module> TheModule;
static Type *DoubleTy = Type::getDoubleTy(TheContext);
static Type *DoublePtr = Type::getDoublePtrTy(TheContext);
static Type *Int32 = Type::getInt32Ty(TheContext);
static Type *Int32Ptr = Type::getInt32PtrTy(TheContext);
// static std::map<std::string, AllocaInst *> NamedValues;
// static std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;
/// LogError* - These are little helper functions for error handling.
std::unique_ptr<Ast> LogError(const char *Str)
{
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}
static Value *LogErrorV(const char *Str)
{
    LogError(Str);
    return nullptr;
}
llvm::Value *NumberAst::codegen(std::map<std::string, llvm::Instruction *> &closure)
{
    const Double *d = dynamic_cast<const Double *>(value.get());
    return ConstantFP::get(TheContext, APFloat(d->get_value()));
}

llvm::Value *VarAst::codegen(std::map<std::string, llvm::Instruction *> &closure)
{
    // Look this variable up in the function.
    Value *V = closure[name];
    if (!V)
        return LogErrorV("Unknown variable name");

    // Load the value.
    return Builder.CreateLoad(V, name.c_str());
}

llvm::Value *LambdaAst::codegen(std::map<std::string, llvm::Instruction *> &closure)
{
    FunctionType *FT =
        FunctionType::get(Type::getDoubleTy(TheContext), vector<Type *>(params.size(), Type::getDoubleTy(TheContext)), false);

    llvm::Function *TheFunction =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, TheModule.get());
    unsigned Idx = 0;
    for (auto &Arg : TheFunction->args())
        Arg.setName(params[Idx++]);
    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);
    std::map<std::string, llvm::Instruction *> self_closure;
    for (auto it = closure.begin(); it != closure.end(); it++)
    {
        self_closure[it->first] = it->second;
    }
    for (auto &Arg : TheFunction->args())
    {
        llvm::Constant *allocsize = llvm::ConstantExpr::getSizeOf(DoubleTy);
        auto malloc_value =
            llvm::CallInst::CreateMalloc(Builder.GetInsertBlock(), Int32, DoubleTy, allocsize, nullptr, nullptr, Arg.getName());
        Builder.CreateStore(&Arg, malloc_value);
        self_closure[Arg.getName()] = malloc_value;
    }
    if (Value *RetVal = body->codegen(self_closure))
    {
    auto BB = Builder.GetInsertBlock();
    Builder.SetInsertPoint(BB);
        Builder.CreateRet(RetVal);
        RetVal->print(errs());
        verifyFunction(*TheFunction);
        TheFunction->print(errs());
        return TheFunction;
    }
    // Error reading body, remove function.
    TheFunction->eraseFromParent();

    return nullptr;
}
// // Make the module, which holds all the code.
// TheModule = std::make_unique<Module>("my cool jit", TheContext);
// Type *Double = Type::getDoubleTy(TheContext);
// Type *DoublePtr = Type::getDoublePtrTy(TheContext);
// Type *Int8 = Type::getInt8Ty(TheContext);
// Type *Int8Ptr = Type::getInt8PtrTy(TheContext);

// FunctionType *FT =
//     FunctionType::get(Type::getDoubleTy(TheContext), vector<Type *>(), false);

// Function *TheFunction =
//     Function::Create(FT, Function::ExternalLinkage, "foo", TheModule.get());
// BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
// Builder.SetInsertPoint(BB);
// auto result = ConstantFP::get(TheContext, APFloat(1.0));
// Builder.CreateRet(result);
// llvm::Constant *allocsize = llvm::ConstantExpr::getSizeOf(Double);
// auto malloc = llvm::CallInst::CreateMalloc(BB, Int8Ptr, Double, allocsize, nullptr, nullptr, "arr");
// BB->getInstList().push_back(malloc);
// TheFunction->print(errs());
llvm::Value *ProgAst::codegen(std::map<std::string, llvm::Instruction *> &closure)
{
    Value *result;
    for (auto &&expr : prog)
    {
        result = expr->codegen(closure);
    }
    return result;
}
llvm::Value *CallAst::codegen(std::map<std::string, llvm::Instruction *> &closure)
{

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    // BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
    llvm::Function *function = static_cast<llvm::Function *>(func->codegen(closure));
    // If argument mismatch error.
    if (function->arg_size() != args.size())
        return LogErrorV("Incorrect # arguments passed");

    auto BB = Builder.GetInsertBlock();
    Builder.SetInsertPoint(BB);
    std::vector<Value *> ArgsV;
    for (unsigned i = 0, e = args.size(); i != e; ++i)
    {
        ArgsV.push_back(args[i]->codegen(closure));
        if (!ArgsV.back())
            return nullptr;
    }

    return Builder.CreateCall(function, ArgsV, "calltmp");
}