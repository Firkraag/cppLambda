#include "ast.hpp"
#include "utils.hpp"
#include <memory>
using std::make_unique;
using std::unique_ptr;

static bool is_bool(const Ast &ast)
{
    const BinaryAst *binary_ast = dynamic_cast<const BinaryAst *>(&ast);

    if (binary_ast != nullptr)
    {
        auto &op = binary_ast->get_operator();
        if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=")
        {
            return true;
        }
        if (op == "&&" || op == "||")
        {
            return is_bool(binary_ast->get_left()) && is_bool(binary_ast->get_right());
        }
    }
    return false;
}
static string make_var(const string &name)
{
    return name;
}
string NumberAst::to_js()
{
    return value->to_string();
}
string BooleanAst::to_js()
{
    return value ? "true" : "false";
}
string StringAst::to_js()
{
    return "\"" + value->to_string() + "\"";
}

string VarAst::to_js()
{
    return make_var(name);
}
string LambdaAst::to_js()
{
    string code = "(function ";
    string func_name = name.empty() ? generate_unique_symbol("anonymous") : name;
    code += make_var(func_name);
    vector<string> js_param_list;
    for (auto &&var : params)
    {
        js_param_list.push_back(make_var(var));
    }

    code += "(" + join(js_param_list.begin(), js_param_list.end(), ", ") + ") {";
    if (!iife_params.empty())
    {
        code += "let " + join(iife_params.begin(), iife_params.end(), ", ") + ";";
    }
    code += "GUARD(arguments, " + func_name + ");";
    code += "return " + body->to_js() + " })";
    return code;
}
string LetAst::to_js()
{
    if (vardefs.empty())
    {
        return body->to_js();
    }
    vector<string> lambda_params = {vardefs[0]->name};
    vector<unique_ptr<VarDef>> lambda_body_vardefs;
    for (auto vardef = vardefs.begin() + 1; vardef < vardefs.end(); vardef++)
    {
        lambda_body_vardefs.push_back(std::move(*vardef));
    }

    unique_ptr<LetAst> lambda_body = make_unique<LetAst>(std::move(lambda_body_vardefs), std::move(body));
    auto lambda_ast = make_unique<LambdaAst>("", lambda_params, std::move(lambda_body));
    vector<unique_ptr<Ast>> iife_args;
    iife_args.push_back(vardefs[0]->define == nullptr ? make_unique<BooleanAst>(false) : std::move(vardefs[0]->define));
    // auto iife = make_unique<CallAst>(lambda_ast, iife_args);
    return "(" + CallAst(std::move(lambda_ast), std::move(iife_args)).to_js() + ")";
}
string CallAst::to_js()
{
    string func_code = func->to_js();
    vector<string> args_js_list;
    for (auto &&arg : args)
    {
        args_js_list.push_back(arg->to_js());
    }
    return func_code + "(" + join(args_js_list.begin(), args_js_list.end(), ", ") + ")";
}
string ProgAst::to_js()
{
    if (prog.empty())
    {
        return "(false)";
    }
    vector<string> prog_js_list;
    for (auto &&exp : prog)
    {
        prog_js_list.push_back(exp->to_js());
    }
    return "(" + join(prog_js_list.begin(), prog_js_list.end(), ", ") + ")";
}
string IfAst::to_js()
{
    string cond_code = cond->to_js();
    string then_code = then->to_js();
    string else_code = else_->to_js();

    if (!is_bool(*cond))
    {
        cond_code += " !== false";
    }
    return "(" + cond_code + " ? " + then_code + " : " + else_code + ")";
}
string BinaryAst::to_js()
{
    return left->to_js() + " " + operator_ + " " + right->to_js();
}
string AssignAst::to_js()
{
    return left->to_js() + " = " + right->to_js();
}