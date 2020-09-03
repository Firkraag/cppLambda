#include "evaluator.hpp"
#include "input_stream.hpp"
#include "token_stream.hpp"
#include "parse.hpp"
#include "primitive.hpp"
#include "optimizer.hpp"
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

    unique_ptr<VarAst> top_level = make_unique<VarAst>(string("TOPLEVEL"));
    // cout << parser()->to_js() << endl;
    auto cps_ast = parser()->to_cps([&top_level](unique_ptr<Ast> ast) {
        vector<unique_ptr<Ast>> args;
        args.push_back(std::move(ast));
        return make_unique<CallAst>(std::move(top_level), std::move(args));
    });
    // cout << cps_ast->to_js() << endl;
    // shared_ptr<Ast> cps_ast = make_shared<CallAst>(make_shared<VarAst>("TOPLEVEL"), vector<shared_ptr<Ast>>{parser()});
    auto result = optimize(std::move(cps_ast));
    // cout << "cps_ast use_count = " << cps_ast.use_count() << endl;
    // cout << (result.first.get() == cps_ast.get()) << endl;
    // cout << "top_level use_count = " << top_level.use_count() << endl;
    auto optimized_code = std::move(result.first);
    auto global_environment = result.second;
    vector<string> global_variable_list;
    for (auto &&var : global_environment->get_vars())
    {
        auto name = var.first;
        auto define = var.second;
        if (define->assigned)
        {
            global_variable_list.push_back(name);
        }
    }
    string global_variables;
    if (!global_variable_list.empty())
    {
        global_variables = "let " + join(global_variable_list.begin(), global_variable_list.end(), ", ") + ";";
    }

    auto js_code = optimized_code->to_js();
    js_code = "\"use strict\";" + global_variables + js_code;
    std::cout << js_code << endl;
    return 0;
}
