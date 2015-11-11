// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

// Otimizações: Dead Code Elimination, Constant Folding

#include "optimizer.h"
#include "tokens.h"

namespace ptb {

enum {
    no_folding = 0,
    can_be_folded,
    already_folded,
};

optimizer::optimizer()
{
}

void optimizer::run(const ast::node_ptr &node)
{

}

void optimizer::fold_pass(const ast::node_ptr &node)
{
}

int optimizer::fold_expr(const ast::node_ptr &node)
{
    switch (node->type) {
        case ast::integer_node: return ast::to_integer(node)->value;
        case ast::op_arithm_node: {
            auto op = ast::to_op_arithm(node);
            switch (op->op) {
                case '+': return fold_expr(op->left) + fold_expr(op->right);
                case '-': return fold_expr(op->left) - fold_expr(op->right);
                case '*': return fold_expr(op->left) * fold_expr(op->right);
                case '/': {
                    int den = fold_expr(op->right);
                    if (den == 0) {
                        throw optimizer_error("Divisao por zero encontrada!");
                    }
                    return fold_expr(op->left) / den;
                }
            }
            break;
        }
        case ast::op_logical_node: {
            auto op = ast::to_op_logical(node);
            switch (op->op) {
                case tok::eq: return fold_expr(op->left) == fold_expr(op->right);
                case tok::ge: return fold_expr(op->left) >= fold_expr(op->right);
                case tok::gt: return fold_expr(op->left) > fold_expr(op->right);
                case tok::le: return fold_expr(op->left) <= fold_expr(op->right);
                case tok::lt: return fold_expr(op->left) < fold_expr(op->right);
                case tok::ne: return fold_expr(op->left) != fold_expr(op->right);
                case tok::b_and: return fold_expr(op->left) && fold_expr(op->right);
                case tok::b_or: return fold_expr(op->left) || fold_expr(op->right);
            }
            break;
        }
    }
}

int optimizer::can_fold(const ast::node_ptr &node)
{
    switch (node->type) {
    case ast::no_node: return 0;
    case ast::integer_node: return already_folded;
    case ast::string_node: return 0;
    case ast::op_arithm_node: {
        auto op = ast::to_op_arithm(node);
        if (can_fold(op->left) && can_fold(op->right))
            return can_be_folded;
        return 0;
    }
    case ast::op_logical_node: {
        auto op = ast::to_op_logical(node);
        if (can_fold(op->left) && can_fold(op->right))
            return can_be_folded;
        return 0;
    }
    case ast::program_node: return 0;
    case ast::if_stmt_node: return 0;
    case ast::while_stmt_node: return 0;
    case ast::return_stmt_node: return 0;
    case ast::assign_stmt_node: return 0;
    case ast::variable_node: return 0;
    case ast::variable_decl_node: return 0;
    case ast::function_decl_node: return 0;
    case ast::call_node: return 0;
    case ast::type_node: return 0;
    case ast::argument_node: return 0;
    case ast::read_stmt_node: return 0;
    case ast::write_stmt_node: return 0;
    }
}

}
