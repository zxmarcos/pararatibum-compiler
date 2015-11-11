// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#pragma once

#include <stdexcept>
#include <string>
#include <stack>
#include "ast.h"
#include "symtable.h"

namespace ptb {

struct semantic_error : public std::runtime_error {
    semantic_error(const std::string& w) : std::runtime_error(w) {
    }
};

class analyzer
{
public:
    analyzer();

    void run(const ast::node_ptr &program);
    symbol_table_ptr get_symtable();

private:
    int m_scope_counter;
    void analyze_node(const ast::node_ptr &node);
    void analyze_function_decl(const ast::node_ptr &node);
    void analyze_variable_decl(const ast::node_ptr &node);

    void analyze_integer(const ast::node_ptr &node);
    void analyze_lstring(const ast::node_ptr &node);
    void analyze_if_stmt(const ast::node_ptr &node);
    void analyze_while_stmt(const ast::node_ptr &node);
    void analyze_return_stmt(const ast::node_ptr &node);
    void analyze_variable(const ast::node_ptr &node);
    void analyze_assign_stmt(const ast::node_ptr &node);
    void analyze_call(const ast::node_ptr &node);
    void analyze_op_arithm(const ast::node_ptr &node);
    void analyze_op_logical(const ast::node_ptr &node);
    void analyze_program(const ast::node_ptr &node);
    void analyze_type(const ast::node_ptr &node);
    void analyze_argument(const ast::node_ptr &node);
    void analyze_read_stmt(const ast::node_ptr &node);
    void analyze_write_stmt(const ast::node_ptr &node);

    int compute_type(const ast::node_ptr &expr);
    symbol_table_ptr m_symtable;
    int get_next_scope() { return m_scope_counter++; }
    std::stack<scope_ptr> m_stack;
    scope_ptr m_global;
};

}

