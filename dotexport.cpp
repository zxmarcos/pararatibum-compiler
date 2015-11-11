// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

// Exporta uma AST para o formato DOT para ser visualizado no Graphivz
// Muito útil para debuggar a árvore gerada.

#include <iostream>
#include <cppfmt/format.h>
#include <fstream>
#include "dotexport.h"
#include "tokens.h"
#include "types.h"
#include "ast.h"

namespace ptb {

// Remove as aspas duplas de uma std::string
std::string unquote(const std::string &str)
{
    if (*str.begin() == '"' && *str.rbegin() == '"') {
        return str.substr(1, str.length() - 2);
    }
    return str;
}


dotexport::dotexport()
{
    m_node_counter = 0;
}

void dotexport::run(const ast::node_ptr &ast)
{
    m_node_counter = 0;

    m_out.open("ast.dot");

    m_out << fmt::sprintf("digraph {\n");
    m_out << fmt::sprintf("graph [fontname = \"helvetica\"]\n");
    m_out << fmt::sprintf("node [fontname = \"helvetica\"]\n");
    m_out << fmt::sprintf("edge [fontname = \"helvetica\"]\n");

    export_node(ast);

    for (auto pair : m_nodes) {
        m_out << fmt::sprintf("%d [label=\"%s\",shape=box]\n", pair.first, pair.second);
    }
    m_out << fmt::sprintf("}\n");
}

int dotexport::export_node(const ast::node_ptr &node)
{
    using namespace ast;
    if (!node->is_valid())
        return -1;

    int id = m_node_counter++;

    switch (node->type) {
    case integer_node:
        set_value(id, fmt::sprintf("%d", to_integer(node)->value));
        return id;

    case string_node:
        set_value(id, unquote(to_lstring(node)->value));
        return id;

    case op_arithm_node: {
        auto op = to_op_arithm(node);
        int lhs = export_node(op->left);
        int rhs = export_node(op->right);
        set_value(id, fmt::sprintf("%c", op->op));
        link_nodes(id, lhs);
        link_nodes(id, rhs);
        return id;
    }
    case op_logical_node: {
        static const std::map<int, std::string> log_names = {
            { neg, "!"},
            { eq, "="},
            { ne, "!="},
            { ge, ">="},
            { le, "<="},
            { gt, ">"},
            { lt, "<"},
            { b_or, "&&"},
            { b_and, "||"},
        };

        auto op = to_op_logical(node);
        int lhs = export_node(op->left);
        int rhs = export_node(op->right);
        set_value(id, fmt::sprintf("%s", log_names.at(op->op)));
        link_nodes(id, lhs);
        link_nodes(id, rhs);
        return id;
    }
    case program_node: {
        auto program = to_program(node);
        set_value(id, "program");
        for (size_t i = 0; i < program->declarations.size(); i++) {
            link_nodes(id, export_node(program->declarations[i]));
        }
        return id;
    }

    case if_stmt_node: {
        auto stmt = to_if_stmt(node);
        int eval = export_node(stmt->eval_expr);
        link_nodes(id, eval);
        int tmp = get_next_node();
        link_nodes(id, tmp);
        set_value(tmp, "true");
        for (size_t i = 0; i < stmt->true_statements.size(); i++) {
            link_nodes(tmp, export_node(stmt->true_statements[i]));
        }

        tmp = get_next_node();
        link_nodes(id, tmp);
        set_value(tmp, "false");
        for (size_t i = 0; i < stmt->false_statements.size(); i++) {
            link_nodes(tmp, export_node(stmt->false_statements[i]));
        }

        set_value(id, "if");
        return id;
    }
    case while_stmt_node: {
        auto stmt = to_while_stmt(node);
        int eval = export_node(stmt->eval_expr);
        link_nodes(id, eval);
        int tmp = get_next_node();
        link_nodes(id, tmp);
        set_value(tmp, "body");
        for (size_t i = 0; i < stmt->statements.size(); i++) {
            link_nodes(tmp, export_node(stmt->statements[i]));
        }
        set_value(id, "while");
        return id;
    }
    case return_stmt_node: {
        auto stmt = to_return_stmt(node);
        int expr = export_node(stmt->expr);
        link_nodes(id, expr);
        set_value(id, "return");
        return id;
    }
    case assign_stmt_node: {
        auto op = to_assign_stmt(node);
        int lhs = export_node(op->lvalue);
        int rhs = export_node(op->rvalue);
        set_value(id, ":=");
        link_nodes(id, lhs);
        link_nodes(id, rhs);
        return id;
    }
    case variable_node:
        set_value(id, to_variable(node)->name);
        return id;

    case variable_decl_node: {
        auto var = to_variable_decl(node);
        int lhs = export_node(var->type_expr);
        int rhs = export_node(var->value);
        set_value(id, fmt::sprintf("var: %s", var->name));
        link_nodes(id, lhs);
        link_nodes(id, rhs);
        return id;
    }
    case function_decl_node: {
        auto func = to_function_decl(node);
        set_value(id, fmt::sprintf("func: %s", func->name));
        link_nodes(id, export_node(func->return_type));
        int tmp = get_next_node();
        set_value(tmp, "args");
        link_nodes(id, tmp);
        for (size_t i = 0; i < func->arguments.size(); i++) {
            link_nodes(tmp, export_node(func->arguments[i]));
        }
        if (!func->statements.empty()) {
            tmp = get_next_node();
            link_nodes(id, tmp);
            set_value(tmp, "body");
            for (size_t i = 0; i < func->statements.size(); i++) {
                link_nodes(tmp, export_node(func->statements[i]));
            }
        }
        return id;
    }
    case call_node: {
        auto call = to_call(node);
        set_value(id, fmt::sprintf("call: %s", call->name));
        for (size_t i = 0; i < call->param_list.size(); i++) {
            link_nodes(id, export_node(call->param_list[i]));
        }
        return id;
    }

    case type_node:
        switch (to_type(node)->type_id) {
        case types::integer: set_value(id, "int"); break;
        case types::boolean: set_value(id, "bool"); break;
        case types::string: set_value(id, "string"); break;
        case types::voidt: set_value(id, "void"); break;
        case types::character: set_value(id, "char"); break;
        }
        return id;
    case argument_node: {
        auto arg = to_argument(node);
        int tmp = get_next_node();
        set_value(id, "arg");
        set_value(tmp, arg->name);
        int type = export_node(arg->type_expr);
        link_nodes(tmp, type);
        link_nodes(id, tmp);
        return id;
    }
    case read_stmt_node: {
        auto read = to_read_stmt(node);
        set_value(id, fmt::sprintf("read: %s", read->identifier));
        return id;
    }
    case write_stmt_node: {
        auto write = to_write_stmt(node);
        set_value(id, "write");
        int expr = export_node(write->expr);
        link_nodes(id, expr);
        return id;
    }
    case no_node:
        return id;
    }
    // ???
    return -1;
}

void dotexport::set_value(int id, const std::string &str)
{
    m_nodes[id] = str;
}

void dotexport::link_nodes(int from, int to)
{
    if (from < 0 || to < 0)
        return;
    m_out << fmt::sprintf("%d -> %d\n", from, to);
}

}
