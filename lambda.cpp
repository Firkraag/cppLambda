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

    shared_ptr<VarAst> top_level(new VarAst(string("TOPLEVEL")), [](VarAst *var_ast) {
        delete var_ast;
    });
    // cout << parser()->to_js() << endl;
    auto cps_ast = parser()->to_cps([top_level](shared_ptr<Ast> ast) {
        return shared_ptr<CallAst>(new CallAst(top_level, vector<shared_ptr<Ast>>{ast}), [](CallAst *call_ast) {
            delete call_ast;
        });
    });
    // cout << cps_ast->to_js() << endl;
    // shared_ptr<Ast> cps_ast = make_shared<CallAst>(make_shared<VarAst>("TOPLEVEL"), vector<shared_ptr<Ast>>{parser()});
    auto result = optimize(cps_ast);
    // cout << "cps_ast use_count = " << cps_ast.use_count() << endl;
    // cout << (result.first.get() == cps_ast.get()) << endl;
    // cout << "top_level use_count = " << top_level.use_count() << endl;
    auto optimized_code = result.first;
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
