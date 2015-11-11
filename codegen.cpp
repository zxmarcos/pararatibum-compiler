// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#include <stdexcept>
#include <memory>
#include <cppfmt/format.h>
#include "codegen.h"
#include "ast.h"
#include "tokens.h"
#include "types.h"

// Gramática BNF da linguagem

namespace ptb {

code_gen::code_gen()
{
    m_out.open("ptb.cpp");
    if (!m_out.is_open()) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo ptb.cpp para escrita");
    }
}

void code_gen::translate(const ast::node_ptr &node)
{
    switch (node->type) {
    case ast::no_node: {
        break;
    }
    case ast::integer_node: {
        m_out << fmt::sprintf("%d", ast::to_integer(node)->value);
        break;
    }
    case ast::string_node: {
        m_out << fmt::sprintf("%s", ast::to_lstring(node)->value);
        break;
    }
    case ast::if_stmt_node: {
        auto ifnode = ast::to_if_stmt(node);
        m_out << fmt::sprintf("if (");
        translate(ifnode->eval_expr);
        m_out << fmt::sprintf(") {\n");
        for (size_t i = 0; i < ifnode->true_statements.size(); i++) {
            translate(ifnode->true_statements[i]);
        }
        m_out << fmt::sprintf("}\n");
        if (ifnode->false_statements.size()) {
            m_out << fmt::sprintf("else {\n");
            for (size_t i = 0; i < ifnode->false_statements.size(); i++) {
                translate(ifnode->false_statements[i]);
            }
            m_out << fmt::sprintf("}\n");
        }
        break;
    }
    case ast::while_stmt_node: {
        auto whilenode = ast::to_while_stmt(node);
        m_out << fmt::sprintf("while (");
        translate(whilenode->eval_expr);
        m_out << fmt::sprintf(") {\n");
        for (size_t i = 0; i < whilenode->statements.size(); i++) {
            translate(whilenode->statements[i]);
        }
        m_out << fmt::sprintf("}\n");
        break;
    }
    case ast::return_stmt_node: {
        auto retnode = ast::to_return_stmt(node);
        m_out << fmt::sprintf("return ");
        translate(retnode->expr);
        m_out << fmt::sprintf(";\n");
        break;
    }
    case ast::variable_node: {
        m_out << fmt::sprintf("%s", ast::to_variable(node)->name);
        break;
    }
    case ast::assign_stmt_node: {
        auto asignode = ast::to_assign_stmt(node);
        translate(asignode->lvalue);
        m_out << fmt::sprintf("=");
        translate(asignode->rvalue);
        m_out << fmt::sprintf(";\n");
        break;
    }
    case ast::call_node: {
        auto callnode = ast::to_call(node);
        m_out << fmt::sprintf("%s(", callnode->name);
        for (size_t i = 0; i < callnode->param_list.size(); i++) {
            translate(callnode->param_list[i]);
            if (i != callnode->param_list.size()-1)
                m_out << fmt::sprintf(",");
        }
        if (callnode->is_stmt) {
            m_out << fmt::sprintf(");\n");
        } else {
            m_out << fmt::sprintf(")");
        }
        break;
    }
    case ast::op_arithm_node: {
        auto opnode = ast::to_op_arithm(node);
        translate(opnode->left);
        m_out << fmt::sprintf("%c", opnode->op);
        translate(opnode->right);
        break;
    }
    case ast::op_logical_node: {
        auto opnode = ast::to_op_logical(node);
        translate(opnode->left);
        switch (opnode->op) {
        case tok::eq: m_out << fmt::sprintf("=="); break;
        case tok::ne: m_out << fmt::sprintf("!="); break;
        case tok::ge: m_out << fmt::sprintf(">="); break;
        case tok::le: m_out << fmt::sprintf("<="); break;
        case tok::gt: m_out << fmt::sprintf(">"); break;
        case tok::lt: m_out << fmt::sprintf("<"); break;
        case tok::b_or: m_out << fmt::sprintf("||"); break;
        case tok::b_and: m_out << fmt::sprintf("&&"); break;
        }
        translate(opnode->right);
        break;
    }
    case ast::program_node: {
        m_out << fmt::sprintf("#include <iostream>\n");
        m_out << fmt::sprintf("#include <string>\n");
        auto program = ast::to_program(node);
        for (size_t i = 0; i < program->declarations.size(); i++) {
            translate(program->declarations[i]);
            m_out << fmt::sprintf("\n");
        }
        break;
    }
    case ast::type_node: {
        auto typenode = ast::to_type(node);
        switch (typenode->type_id) {
        case types::integer: m_out << fmt::sprintf("int "); break;
        case types::character: m_out << fmt::sprintf("char "); break;
        case types::voidt: m_out << fmt::sprintf("void "); break;
        case types::boolean: m_out << fmt::sprintf("bool "); break;
        case types::string: m_out << fmt::sprintf("std::string "); break;
        }
        break;
    }
    case ast::argument_node: {
        auto argnode = ast::to_argument(node);
        translate(argnode->type_expr);
        m_out << fmt::sprintf(argnode->name);
        break;
    }
    case ast::variable_decl_node: {
        auto vardnode = ast::to_variable_decl(node);
        translate(vardnode->type_expr);
        m_out << fmt::sprintf(vardnode->name);
        if (vardnode->value->is_valid()) {
            m_out << fmt::sprintf("=");
            translate(vardnode->value);
        }
        m_out << fmt::sprintf(";\n");
        break;
    }
    case ast::function_decl_node: {
        auto funcnode = ast::to_function_decl(node);
        translate(funcnode->return_type);

        m_out << fmt::sprintf("%s(", funcnode->is_main() ? "main" : funcnode->name);
        for (size_t i = 0; i < funcnode->arguments.size(); i++) {
            translate(funcnode->arguments[i]);
            if (i != funcnode->arguments.size()-1)
                m_out << fmt::sprintf(",");
        }
        m_out << fmt::sprintf(") {\n");
        for (size_t i = 0; i < funcnode->statements.size(); i++) {
            translate(funcnode->statements[i]);
        }
        m_out << fmt::sprintf("}\n");
        break;
    }
    case ast::read_stmt_node: {
        auto read = ast::to_read_stmt(node);
        m_out << fmt::sprintf("std::cin >> %s;\n", read->identifier);
        break;
    }
    case ast::write_stmt_node: {
        auto write = ast::to_write_stmt(node);
        m_out << fmt::sprintf("std::cout << (");
        translate(write->expr);
        m_out << fmt::sprintf(");\n");
        break;
    }
    default:
        m_out << fmt::sprintf("/* ast::node invalido para traducao %d*/", node->type);
        break;
    }
}


}
