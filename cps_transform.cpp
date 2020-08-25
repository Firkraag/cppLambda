#include "evaluator.hpp"
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
    Parser parser(new TokenStream(new InputStream(new string(std::move(code)))));
    shared_ptr<Environment> environment = std::make_shared<Environment>();
    auto cps_ast = parser()->to_cps([](shared_ptr<Ast> ast) {
        return make_shared<CallAst>(
            make_shared<VarAst>("TOPLEVEL"),
            vector<shared_ptr<Ast>>{ast});
    });
    auto js_code = cps_ast->to_js();
    std::cout << js_code << endl;
    return 0;
}
