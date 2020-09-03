#include "ast.hpp"

template <typename T>
static bool vector_pointer_equality(const vector<T> &v1, const vector<T> &v2)
{
    return std::equal(v1.begin(), v1.end(), v2.begin(), v2.end(), [](const T &p1, const T &p2) {
        return *p1 == *p2;
    });
}
bool NumberAst::operator==(const Ast &ast) const
{
    const NumberAst *number_ast = dynamic_cast<const NumberAst *>(&ast);
    return number_ast != NULL && *(number_ast->value) == *value;
}
bool BooleanAst::operator==(const Ast &ast) const
{
    const BooleanAst *boolean_ast = dynamic_cast<const BooleanAst *>(&ast);
    return boolean_ast != NULL && *(boolean_ast->value) == *value;
}
bool StringAst::operator==(const Ast &ast) const
{
    const StringAst *string_ast = dynamic_cast<const StringAst *>(&ast);
    return string_ast != NULL && *(string_ast->value) == *value;
}
bool VarAst::operator==(const Ast &ast) const
{
    const VarAst *var_ast = dynamic_cast<const VarAst *>(&ast);
    return var_ast != NULL && var_ast->name == name;
}
bool VarDef::operator==(const VarDef &vardef_ast) const
{
    // const VarDefAst *vardef_ast = dynamic_cast<const VarDefAst *>(&ast);
    if (name != vardef_ast.name)
    {
        return false;
    }
    if (define == nullptr)
    {
        if (vardef_ast.define == nullptr)
        {
            return true;
        }
        return false;
    }
    else if (vardef_ast.define == nullptr)
    {
        return false;
    }
    else
    {
        return *define == *(vardef_ast.define);
    }
}
bool LambdaAst::operator==(const Ast &ast) const
{
    const LambdaAst *lambda_ast = dynamic_cast<const LambdaAst *>(&ast);
    return lambda_ast != NULL && name == lambda_ast->get_name() && params == lambda_ast->params && *body == *(lambda_ast->body);
}
bool LetAst::operator==(const Ast &ast) const
{
    const LetAst *let_ast = dynamic_cast<const LetAst *>(&ast);
    return let_ast != NULL and *(let_ast->body) == *body and vector_pointer_equality(vardefs, let_ast->vardefs);
}
bool CallAst::operator==(const Ast &ast) const
{
    const CallAst *call_ast = dynamic_cast<const CallAst *>(&ast);
    return call_ast != NULL && *(call_ast->func) == *func && vector_pointer_equality(args, call_ast->args);
}
bool ProgAst::operator==(const Ast &ast) const
{
    const ProgAst *prog_ast = dynamic_cast<const ProgAst *>(&ast);
    return prog_ast != NULL && vector_pointer_equality(prog, prog_ast->prog);
}
bool IfAst::operator==(const Ast &ast) const
{
    const IfAst *if_ast = dynamic_cast<const IfAst *>(&ast);
    return if_ast != NULL && *(if_ast->cond) == *cond && *(if_ast->then) == *then && *(if_ast->else_) == *else_;
}
bool BinaryAst::operator==(const Ast &ast) const
{
    const BinaryAst *binary_ast = dynamic_cast<const BinaryAst *>(&ast);
    return binary_ast != NULL && *(binary_ast->left) == *left && *(binary_ast->right) == *right && binary_ast->operator_ == operator_;
}
bool AssignAst::operator==(const Ast &ast) const
{
    const AssignAst *assign_ast = dynamic_cast<const AssignAst *>(&ast);
    return assign_ast != NULL && *(assign_ast->left) == *left && *(assign_ast->right) == *right;
}