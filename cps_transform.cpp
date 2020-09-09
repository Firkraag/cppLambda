#include "input_stream.hpp"
#include "token_stream.hpp"
#include "parse.hpp"
#include "primitive.hpp"
#include <functional>
#include <memory>
#include <fstream>
#include <streambuf>
int main(int argc, char const *argv[])
{
    std::ifstream lambda_source_file(argv[1]);
    std::string code((std::istreambuf_iterator<char>(lambda_source_file)),
                     std::istreambuf_iterator<char>());
    Parser parser(new TokenStream(new InputStreamStr(new string(std::move(code)))));
    shared_ptr<Environment> environment = std::make_shared<Environment>();
    auto cps_ast = parser()->to_cps([](unique_ptr<Ast> ast) {
        vector<unique_ptr<Ast>> args;
        args.push_back(std::move(ast));
        return make_unique<CallAst>(make_unique<VarAst>("TOPLEVEL"), std::move(args));
    });
    auto js_code = cps_ast->to_js();
    std::cout << js_code << endl;
    return 0;
}
