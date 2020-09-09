#include "ast_code_gen.hpp"
#include "utils.hpp"
LLVMContext TheContext;
IRBuilder<> Builder(TheContext);
std::unique_ptr<Module> TheModule;
std::unique_ptr<llvm::orc::KaleidoscopeJIT> TheJIT;
static Type *DoubleTy = Type::getDoubleTy(TheContext);
static Type *DoublePtr = Type::getDoublePtrTy(TheContext);
static Type *Int32 = Type::getInt32Ty(TheContext);
static Type *Int32Ptr = Type::getInt32PtrTy(TheContext);
static map<string, vector<string>> FunctionProtos;

// CrateEntryBlockAlloca - Create an alloca instruction in the entry block of
// the function. This is used for mutable variables etc.
static llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function *TheFunction, const std::string &VarName)
{
    IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(Type::getDoubleTy(TheContext), nullptr, VarName.c_str());
}
static llvm::Function *getFunctionProto(std::string func_name, vector<string> params)
{

    // If not, check whether we can codegen the declaration from some existing
    // prototype.
    FunctionType *FT =
        FunctionType::get(DoubleTy, vector<Type *>(params.size(), DoubleTy), false);

    llvm::Function *TheFunction =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, func_name, TheModule.get());
    // Set names for all arguments.
    unsigned Idx = 0;
    for (auto &Arg : TheFunction->args())
        Arg.setName(params[Idx++]);
    return TheFunction;
}
/// LogError* - These are little helper functions for error handling.
static std::unique_ptr<Ast> LogError(const char *Str)
{
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}
static Value *LogErrorV(const char *Str)
{
    LogError(Str);
    return nullptr;
}
llvm::Value *NumberAst::codegen(std::map<std::string, llvm::AllocaInst *> &closure)
{
    const Double *d = dynamic_cast<const Double *>(value.get());
    return ConstantFP::get(TheContext, APFloat(d->get_value()));
}

llvm::Value *VarAst::codegen(std::map<std::string, llvm::AllocaInst *> &closure)
{
    // Look this variable up in the function.
    llvm::AllocaInst *V = closure[name];
    if (!V)
        return LogErrorV("Unknown variable name");

    // Load the value.
    return Builder.CreateLoad(V, name.c_str());
}

llvm::Value *LambdaAst::codegen(std::map<std::string, llvm::AllocaInst *> &closure)
{
    std::map<std::string, llvm::AllocaInst *> self_closure(closure);
    FunctionProtos[name] = params;
    llvm::Function *TheFunction = getFunctionProto(name, params);
    if (!TheFunction)
    {
        return nullptr;
    }
    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);

    Builder.SetInsertPoint(BB);
    for (auto &Arg : TheFunction->args())
    {
        // Create an alloca for this variable.
        llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName());

        // Store the initial value into the alloca.
        Builder.CreateStore(&Arg, Alloca);
        // llvm::Constant *allocsize = llvm::ConstantExpr::getSizeOf(DoubleTy);
        // auto malloc_value =
        //     llvm::CallInst::CreateMalloc(Builder.GetInsertBlock(), Int32, DoubleTy, allocsize, nullptr, nullptr, Arg.getName());
        // Builder.CreateStore(&Arg, malloc_value);
        // self_closure[Arg.getName()] = malloc_value;
        self_closure[Arg.getName()] = Alloca;
    }
    if (Value *RetVal = body->codegen(self_closure))
    {
        Builder.CreateRet(RetVal);
        verifyFunction(*TheFunction);
        return TheFunction;
    }
    // Error reading body, remove function.
    TheFunction->eraseFromParent();

    return nullptr;
}
llvm::Value *ProgAst::codegen(std::map<std::string, llvm::AllocaInst *> &closure)
{
    Value *result = nullptr;
    for (auto &&expr : prog)
    {
        result = expr->codegen(closure);
    }
    return result;
}
llvm::Value *CallAst::codegen(std::map<std::string, llvm::AllocaInst *> &closure)
{

    const VarAst *var_ast = dynamic_cast<const VarAst *>(func.get());
    string name = var_ast->get_name();
    llvm::Function *function = nullptr;
    // Look up the name in the global module table.
    auto FI = FunctionProtos.find(name);
    if (FI != FunctionProtos.end())
        function = getFunctionProto(name, FI->second);
    if (!function)
    {
        return LogErrorV("Unknown function referenced");
    }

    // If argument mismatch error.
    if (function->arg_size() != args.size())
        return LogErrorV("Incorrect # arguments passed");

    std::vector<Value *> ArgsV;
    for (unsigned i = 0, e = args.size(); i != e; ++i)
    {
        ArgsV.push_back(args[i]->codegen(closure));
        if (!ArgsV.back())
            return nullptr;
    }

    return Builder.CreateCall(function, ArgsV, "calltmp");
}
llvm::Value *BinaryAst::codegen(std::map<std::string, llvm::AllocaInst *> &closure)
{
    Value *L = left->codegen(closure);
    Value *R = right->codegen(closure);

    if (!L || !R)
    {
        return nullptr;
    }
    if (operator_ == "+")
    {
        return Builder.CreateFAdd(L, R, "addtmp");
    }
    if (operator_ == "-")
    {
        return Builder.CreateFSub(L, R, "subtmp");
    }
    if (operator_ == "*")
    {
        return Builder.CreateFMul(L, R, "multmp");
    }
    if (operator_ == "/")
    {
        return Builder.CreateFDiv(L, R, "divtmp");
    }
    if (operator_ == "<")
    {
        L = Builder.CreateFCmpULT(L, R, "cmptmp");
        // Convert bool 0/1 to double 0.0 or 1.0
        return Builder.CreateUIToFP(L, Type::getDoubleTy(TheContext), "booltmp");
    }
    if (operator_ == ">")
    {
        L = Builder.CreateFCmpULT(R, L, "cmptmp");
        // Convert bool 0/1 to double 0.0 or 1.0
        return Builder.CreateUIToFP(L, Type::getDoubleTy(TheContext), "booltmp");
    }
    return LogErrorV("invalid binary operator");
}

llvm::Value *LetAst::codegen(std::map<std::string, llvm::AllocaInst *> &closure)
{
    std::map<std::string, llvm::AllocaInst *> self_closure(closure);
    auto TheFunction = Builder.GetInsertBlock()->getParent();
    for (auto &&vardef : vardefs)
    {

        llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, vardef->name);
        // Emit the start code first, without `variable` in scope.
        Value *InitialValue = vardef->define->codegen(self_closure);
        // Store the initial value into the alloca.
        Builder.CreateStore(InitialValue, Alloca);
        self_closure[vardef->name] = Alloca;
        // llvm::Constant *allocsize = llvm::ConstantExpr::getSizeOf(DoubleTy);
        // auto malloc_value =
        //     llvm::CallInst::CreateMalloc(Builder.GetInsertBlock(), Int32, DoubleTy, allocsize, nullptr, nullptr, Arg.getName());
        // Builder.CreateStore(&Arg, malloc_value);
        // self_closure[Arg.getName()] = malloc_value;
    }
    return body->codegen(self_closure);
}
llvm::Value *IfAst::codegen(std::map<std::string, llvm::AllocaInst *> &closure)
{
    Value *cond_value = cond->codegen(closure);
    if (!cond_value)
    {
        return nullptr;
    }

    // Convert condition to a bool by comparing non-equal to 0.0.
    cond_value = Builder.CreateFCmpONE(cond_value, ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");
    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

    // Create blocks for the then and else cases. Insert the `then` block at the
    // end of the function.
    BasicBlock *ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
    BasicBlock *ElseBB = BasicBlock::Create(TheContext, "else");
    BasicBlock *MergeBB = BasicBlock::Create(TheContext, "ifcont");

    Builder.CreateCondBr(cond_value, ThenBB, ElseBB);

    // Emit then value.
    Builder.SetInsertPoint(ThenBB);

    Value *ThenV = then->codegen(closure);
    if (!ThenV)
    {
        return nullptr;
    }

    Builder.CreateBr(MergeBB);
    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    ThenBB = Builder.GetInsertBlock();

    // Emit else block.
    TheFunction->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);

    Value *ElseV = else_->codegen(closure);
    if (!ElseV)
    {
        return nullptr;
    }

    Builder.CreateBr(MergeBB);
    // Codegen of 'Else' can change the current block, update ElseBB for the PHI.

    // Emit merge block.
    TheFunction->getBasicBlockList().push_back(MergeBB);
    Builder.SetInsertPoint(MergeBB);
    llvm::PHINode *PN = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, "iftmp");
    PN->addIncoming(ThenV, ThenBB);
    PN->addIncoming(ElseV, ElseBB);
    return PN;
}
llvm::Value *AssignAst::codegen(std::map<std::string, llvm::AllocaInst *> &closure)
{
    const VarAst *left_node = dynamic_cast<const VarAst *>(left.get());
    if (left_node == nullptr)
    {
        return nullptr;
    }
    auto result = closure.find(left_node->get_name());
    if (result == closure.end())
    {
        return nullptr;
    }
    Value *value = right->codegen(closure);
    // Store the initial value into the alloca.
    Builder.CreateStore(value, closure[left_node->get_name()]);
    return value;
}
