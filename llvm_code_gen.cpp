#include <fstream>
#include "ast_code_gen.hpp"
#include "parse.hpp"
#include "token_stream.hpp"
#include "input_stream.hpp"

int main(int argc, char const *argv[])
{
    TheModule = std::make_unique<Module>("my cool jit", TheContext);
    std::ifstream lambda_source_file(argv[1]);
    std::string code((std::istreambuf_iterator<char>(lambda_source_file)),
                     std::istreambuf_iterator<char>());
    Parser parser(new TokenStream(new InputStream(new string(std::move(code)))));
    std::map<std::string, llvm::Instruction *> closure;
    FunctionType *FT =
        FunctionType::get(Type::getDoubleTy(TheContext), vector<Type *>(0, Type::getDoubleTy(TheContext)), false);

    llvm::Function *TheFunction =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", TheModule.get());
    // Create a new basic block to start insertion into.
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);
    if (Value *RetVal = parser()->codegen(closure))
    {
        Builder.CreateRet(RetVal);
        verifyFunction(*TheFunction);
        TheFunction->print(errs());
        return 0;
    }
    // Error reading body, remove function.
    TheFunction->eraseFromParent();


    return 1;
}
