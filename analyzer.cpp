// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

// O analisador semântico verifica a consistência do programa e cria
// a tabela de simbolos que será utilizada pelo gerador de código

#include <iostream>
#include <cppfmt/format.h>
#include "analyzer.h"
#include "tokens.h"
#include "types.h"

namespace ptb {

analyzer::analyzer()
{
    m_scope_counter = 0;
}

void analyzer::run(const ast::node_ptr &node)
{
    m_scope_counter = 0;
    m_symtable = std::make_shared<symbol_table>();

    if (node->type != ast::program_node) {
        throw semantic_error("AST nao e um programa valido!");
    }

    int global_sid = get_next_scope();

    // cria o escopo global
    m_global = std::make_shared<scope>(nullptr, global_sid);
    m_stack.push(m_global);
    m_symtable->put_scope(global_sid, m_global);

    auto program = ast::to_program(node);
    if (program->declarations.empty()) {
        throw semantic_error("O programa nao contem nenhuma declaracao!");
    }
    for (size_t i = 0; i < program->declarations.size(); i++) {
        analyze_node(program->declarations[i]);
    }
}

symbol_table_ptr analyzer::get_symtable()
{
    return m_symtable;
}

void analyzer::analyze_node(const ast::node_ptr &node)
{
    switch (node->type) {
    case ast::function_decl_node:
        analyze_function_decl(node);
        break;
    case ast::variable_decl_node:
        analyze_variable_decl(node);
        break;
    case ast::variable_node:
        analyze_variable(node);
        break;
    case ast::op_arithm_node:
        analyze_op_arithm(node);
        break;
    case ast::op_logical_node:
        analyze_op_logical(node);
        break;
    case ast::if_stmt_node:
        analyze_if_stmt(node);
        break;
    case ast::while_stmt_node:
        analyze_while_stmt(node);
        break;
    case ast::assign_stmt_node:
        analyze_assign_stmt(node);
        break;
    case ast::argument_node:
        analyze_argument(node);
        break;
    case ast::return_stmt_node:
        analyze_return_stmt(node);
        break;
    case ast::write_stmt_node:
        analyze_write_stmt(node);
        break;
    case ast::read_stmt_node:
        analyze_read_stmt(node);
        break;
    default:
        break;
    }
}

// Analisa a declaração de funções
void analyzer::analyze_function_decl(const ast::node_ptr &node)
{
    auto func = ast::to_function_decl(node);

    auto curr_scope = m_stack.top();

    // Verifica se o simbolo já existe na tabela de símbolos
    auto sym = curr_scope->get(func->name);
    if (!sym.is_valid()) {

        int sid = get_next_scope();

        // insere o simbolo no escopo atual
        curr_scope->insert(func->name,
                           symbol(func->name,
                                 compute_type(func->return_type) | types::function, sid));

        // cria o novo escopo
        auto fscope = std::make_shared<scope>(curr_scope, sid);
        m_symtable->put_scope(sid, fscope);
        // empilha o escopo e insere os argumentos nele
        m_stack.push(fscope);
        for (size_t i = 0; i < func->arguments.size(); i++) {
            analyze_node(func->arguments[i]);
        }
        m_stack.pop();
        sym = curr_scope->get(func->name);
    }
    // empilha o escopo da função e analisa os statements
    m_stack.push(m_symtable->get_scope(sym.scope_id));
    for (size_t i = 0; i < func->statements.size(); i++) {
        analyze_node(func->statements[i]);
    }
    m_stack.pop();
}

// Analisa a declaração de variáveis
void analyzer::analyze_variable_decl(const ast::node_ptr &node)
{
    auto var = ast::to_variable_decl(node);

    int type = compute_type(var->type_expr);
    if (var->value->is_valid()) {
        if (type != compute_type(var->value)) {
            throw semantic_error("Atribuicao invalida encontrada, tipos incompativeis!");
        }
    }
    auto curr_scope = m_stack.top();
    auto sym = curr_scope->get(var->name);
    if (!sym.is_valid()) {
        curr_scope->insert(var->name, symbol(var->name, type));
    }
    analyze_node(var->value);
}

void analyzer::analyze_integer(const ast::node_ptr &node) {}
void analyzer::analyze_lstring(const ast::node_ptr &node) {}

void analyzer::analyze_if_stmt(const ast::node_ptr &node)
{
    auto ifstmt = ast::to_if_stmt(node);

    auto curr_scope = m_stack.top();

    analyze_node(ifstmt->eval_expr);


    if (!ifstmt->true_statements.empty()) {
        int sid = get_next_scope();
        // cria o novo escopo para o escopo verdadeiro
        auto fscope = std::make_shared<scope>(curr_scope, sid);
        m_symtable->put_scope(sid, fscope);
        // empilha o escopo e analisa os statements nele
        m_stack.push(fscope);
        for (size_t i = 0; i < ifstmt->true_statements.size(); i++) {
            analyze_node(ifstmt->true_statements[i]);
        }
        m_stack.pop();
        ifstmt->true_scope_id = sid;
    }
    if (!ifstmt->false_statements.empty()) {
        int sid = get_next_scope();
        // cria o novo escopo para o escopo verdadeiro
        auto fscope = std::make_shared<scope>(curr_scope, sid);
        m_symtable->put_scope(sid, fscope);
        // empilha o escopo e analisa os statements nele
        m_stack.push(fscope);
        for (size_t i = 0; i < ifstmt->false_statements.size(); i++) {
            analyze_node(ifstmt->false_statements[i]);
        }
        m_stack.pop();
        ifstmt->false_scope_id = sid;
    }
}

void analyzer::analyze_while_stmt(const ast::node_ptr &node)
{
    auto whilestmt = ast::to_while_stmt(node);

    auto curr_scope = m_stack.top();

    analyze_node(whilestmt->eval_expr);

    if (!whilestmt->statements.empty()) {
        int sid = get_next_scope();
        // cria o novo escopo para o escopo verdadeiro
        auto fscope = std::make_shared<scope>(curr_scope, sid);
        m_symtable->put_scope(sid, fscope);
        // empilha o escopo e insere os argumentos nele
        m_stack.push(fscope);
        for (size_t i = 0; i < whilestmt->statements.size(); i++) {
            analyze_node(whilestmt->statements[i]);
        }
        m_stack.pop();
        whilestmt->scope_id = sid;
    }
}

void analyzer::analyze_return_stmt(const ast::node_ptr &node)
{
    // TODO: verificar o tipo de retorno da função atual
    auto ret = ast::to_return_stmt(node);
    analyze_node(ret->expr);
}

void analyzer::analyze_variable(const ast::node_ptr &node)
{
    auto var = ast::to_variable(node);

    auto curr_scope = m_stack.top();
    if (!curr_scope->get(var->name).is_valid()) {
        throw semantic_error(fmt::sprintf("Variavel %s nao declarada!", var->name));
    }
}

void analyzer::analyze_assign_stmt(const ast::node_ptr &node)
{
    auto assign = ast::to_assign_stmt(node);
    analyze_node(assign->lvalue);
    analyze_node(assign->rvalue);

    int left = compute_type(assign->lvalue);
    int right = compute_type(assign->rvalue);

    if (right != left) {
        throw semantic_error("Tipos incompativeis");
    }
}

void analyzer::analyze_call(const ast::node_ptr &node)
{
    auto call = ast::to_call(node);

    auto curr_scope = m_stack.top();
    auto sym = curr_scope->get(call->name);
    if (!sym.is_valid()) {
        throw semantic_error(fmt::sprintf("Funcao %s nao declarada!", call->name));
    }
    if (!(sym.type & types::function)) {
        throw semantic_error(fmt::sprintf("Identificador %s nao e uma funcao!", call->name));
    }
    // TODO: verificar quantidade de parametros
    for (size_t i = 0; i < call->param_list.size(); i++) {
        analyze_node(call->param_list[i]);
    }
}

void analyzer::analyze_op_arithm(const ast::node_ptr &node) {
    auto op = ast::to_op_arithm(node);
    analyze_node(op->left);
    analyze_node(op->right);
}

void analyzer::analyze_op_logical(const ast::node_ptr &node) {
    auto op = ast::to_op_logical(node);
    analyze_node(op->left);
    analyze_node(op->right);
}

void analyzer::analyze_program(const ast::node_ptr &node) {}

void analyzer::analyze_type(const ast::node_ptr &node) {}

void analyzer::analyze_argument(const ast::node_ptr &node)
{
    auto arg = ast::to_argument(node);

    auto curr_scope = m_stack.top();
    int type = compute_type(arg->type_expr);
    curr_scope->insert(arg->name, symbol(arg->name, type));
}

void analyzer::analyze_read_stmt(const ast::node_ptr &node)
{
    auto read = ast::to_read_stmt(node);
    auto curr_scope = m_stack.top();
    if (!curr_scope->get(read->identifier).is_valid()) {
        throw semantic_error(fmt::sprintf("Variavel %s nao declarada!", read->identifier));
    }
}

void analyzer::analyze_write_stmt(const ast::node_ptr &node)
{
    auto write = ast::to_write_stmt(node);
    analyze_node(write->expr);
}

// Computa os tipos
int analyzer::compute_type(const ast::node_ptr &expr)
{
    switch (expr->type) {
        case ast::call_node: {
            auto curr_scope = m_stack.top();
            auto sym = curr_scope->get(ast::to_call(expr)->name);
            if (!sym.is_valid())
                return -1;
            return sym.type & ~types::function;
        }
        case ast::variable_node: {
            auto curr_scope = m_stack.top();
            auto sym = curr_scope->get(ast::to_variable(expr)->name);
            if (!sym.is_valid())
                return -1;
            return sym.type;
        }
        case ast::integer_node:
            return types::integer;
        case ast::string_node:
            return types::string;
        case ast::type_node:
            return ast::to_type(expr)->type_id;
        case ast::op_logical_node: {
            auto op = ast::to_op_logical(expr);
            int lhs_type = compute_type(op->left);
            int rhs_type = compute_type(op->right);
            if (lhs_type == types::integer && rhs_type == types::integer)
                return types::integer;
            return -1;
        }
        case ast::op_arithm_node: {
            auto op = ast::to_op_arithm(expr);
            int lhs_type = compute_type(op->left);
            int rhs_type = compute_type(op->right);
            if (lhs_type == types::integer && rhs_type == types::integer)
                return types::integer;
            return -1;
        }
        default: return -1;
    }
}

}
