#include "ast.hpp"
#include "optimizer.hpp"
#include <memory>
// shared_ptr<Ast> Optimizer::optimize_aux(vector<shared_ptr<Ast>>::iterator begin, vector<shared_ptr<Ast>>::iterator end)
// {
//     if (begin == end)
//     {
//         changes++;
//         return make_shared<BooleanAst>(false);
//     }
//     if (end - begin == 1)
//     {
//         return optimize(*begin);
//     }
//     if ((*begin)->has_side_effect())
//     {
//         return make_shared<ProgAst>(vector<shared_ptr<Ast>>{optimize(*begin), optimize_aux(begin + 1, end)});
//     }
//     changes++;
//     return optimize_aux(begin + 1, end);
//     // if
//     //     not has_side_effect(prog[0]) : self.changes += 1 return self._optimize_aux(ProgAst(prog [1:])) return ProgAst([ self._optimize_aux(prog[0]), self._optimize_aux(ProgAst(prog [1:])) ])
// }

// shared_ptr<Ast> Optimizer::optimize(shared_ptr<ProgAst> prog_ast)
// {
//     cout << "side effect" << endl;
//     auto prog = prog_ast->get_prog();
//     return optimize_aux(prog.begin(), prog.end());
// }
// shared_ptr<Ast> optimize(shared_ptr<IfAst> if_ast) {
//     auto cond = optimize(if_ast->get_cond());
//     auto then = optimize(if_ast->get_then());
//     auto else_ = optimize(if_ast->get_else());

//     return make_shared<IfAst>(cond, then, else_);
// }
std::pair<shared_ptr<Ast>, VarDefineEnv> optimize(shared_ptr<Ast>& ast) {
    VarDefineEnv global_environment;
    OptimizeChange = true;
    while (OptimizeChange) {
        global_environment = make_shared<EnvironmentBase<shared_ptr<VarDefine>>>();
        ast->make_scope(global_environment, global_environment);
        OptimizeChange = false;
        ast = ast->optimize(nullptr);
    }
    return std::make_pair(ast, global_environment);
}