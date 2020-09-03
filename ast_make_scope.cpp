#include "ast.hpp"
void VarAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    shared_ptr<VarDefine> var_define;
    auto scope = closure_environment->lookup(name);
    if (scope == nullptr)
    {
        var_define = make_shared<VarDefine>(VarKind::GlobalVar);
        global_environment->define(name, var_define);
    }
    else
    {
        var_define = scope->get(name);
    }
    var_define->refs.push_back(this);
    this->define = var_define;
}
void LambdaAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    // this->closure = closure;
    // auto closure_env = closure == nullptr ? global_environment : closure->get_env();
    auto current_environment = env = closure_environment->extend();
    if (!name.empty())
    {
        current_environment->define(name, make_shared<VarDefine>(VarKind::LambdaParam));
    }
    for (auto &&param : params)
    {
        current_environment->define(param, make_shared<VarDefine>(VarKind::LambdaParam));
    }
    for (auto &&param : iife_params)
    {
        current_environment->define(param, make_shared<VarDefine>(VarKind::IifeParam));
    }

    body->make_scope(current_environment, global_environment);
}
void CallAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    func->make_scope(closure_environment, global_environment);
    for (auto &&arg : args)
    {
        arg->make_scope(closure_environment, global_environment);
    }
}
void ProgAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    for (auto &&item : prog)
    {
        item->make_scope(closure_environment, global_environment);
    }
}
void IfAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    cond->make_scope(closure_environment, global_environment);
    then->make_scope(closure_environment, global_environment);
    else_->make_scope(closure_environment, global_environment);
}
void BinaryAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    left->make_scope(closure_environment, global_environment);
    right->make_scope(closure_environment, global_environment);
}
void AssignAst::make_scope(VarDefineEnv closure_environment, VarDefineEnv global_environment)
{
    left->make_scope(closure_environment, global_environment);
    right->make_scope(closure_environment, global_environment);
    const VarAst *left_ast = dynamic_cast<const VarAst *>(left.get());
    if (left_ast != nullptr)
    {
        left_ast->define->assigned++;
        left_ast->define->current_value = right.get();
    }
}