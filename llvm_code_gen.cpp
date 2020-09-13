#include <fstream>
#include "ast_code_gen.hpp"
#include "parse.hpp"
#include "token_stream.hpp"
#include "input_stream.hpp"
#include "llvm/Support/TargetSelect.h"
static void InitializeModuleAndPassManager()
{
    TheModule = std::make_unique<Module>("my cool jit", TheContext);
    TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());
}

int main(int argc, char const *argv[])
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    TheJIT = make_unique<llvm::orc::KaleidoscopeJIT>();
    InitializeModuleAndPassManager();
    if (argc == 1)
    {
        std::cout << "ready> ";
        Parser parser(new TokenStream(new InputStreamStdin()));
        std::map<std::string, llvm::Instruction *> closure;
        // FunctionType *FT =
        //     FunctionType::get(Type::getDoubleTy(TheContext), vector<Type *>(0, Type::getDoubleTy(TheContext)), false);

        // llvm::Function *TheFunction =
        //     llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", TheModule.get());
        // // Create a new basic block to start insertion into.
        // BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
        // Builder.SetInsertPoint(BB);
        while (!parser.token_stream->eof())
        {
            auto ast = parser.parse_expression();
            parser.skip_punc(';');
            auto lambda_ast = dynamic_cast<const LambdaAst *>(ast.get());
            if (lambda_ast != nullptr)
            {
                auto lambda_ir = ast->codegen(closure);
                lambda_ir->print(errs());
                std::cout << endl;
                TheJIT->addModule(std::move(TheModule));
                InitializeModuleAndPassManager();
            }
            else
            {
                auto toplevel_lambda = make_unique<LambdaAst>(
                    "__anon_expr", vector<string>(), std::move(ast));
                toplevel_lambda->codegen(closure);
                auto H = TheJIT->addModule(std::move(TheModule));
                InitializeModuleAndPassManager();
                auto ExprSymbol = TheJIT->findSymbol("__anon_expr");
                double (*FP)() = (double (*)())(intptr_t)cantFail(ExprSymbol.getAddress());
                cout << "Evaluate to " << FP() << endl;
                TheJIT->removeModule(H);
            }
            std::cout << "ready> ";
        }
    }
    else
    {
        std::ifstream lambda_source_file(argv[1]);
        std::string code((std::istreambuf_iterator<char>(lambda_source_file)),
                         std::istreambuf_iterator<char>());
        std::map<std::string, llvm::Instruction *> closure;
        auto result = Parser(code)()->codegen(closure);
        result->print(errs());
        cout << endl;

        // FunctionType *FT =
        //     FunctionType::get(Type::getDoubleTy(TheContext), vector<Type *>(0, Type::getDoubleTy(TheContext)), false);

        // llvm::Function *TheFunction =
        //     llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", TheModule.get());
        // // Create a new basic block to start insertion into.
        // BasicBlock *BB = BasicBlock::Create(TheContext, "entry", TheFunction);
        // Builder.SetInsertPoint(BB);
        // if (Value *RetVal = parser()->codegen(closure))
        // {
        //     Builder.CreateRet(RetVal);
        //     verifyFunction(*TheFunction);
        //     TheFunction->print(errs());
        //     return 0;
    }
    return 0;
}
