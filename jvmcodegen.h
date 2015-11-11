// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#pragma once
#include <stdexcept>
#include <string>
#include <stack>
#include <fstream>
#include "ast.h"
#include "symtable.h"

namespace ptb {

struct jvmcodegen_error : public std::runtime_error {
    jvmcodegen_error(const std::string& w) : std::runtime_error(w) {
    }
};

class jvmcodegen
{
public:
    jvmcodegen();

    void run(const ast::node_ptr &program, symbol_table_ptr &symtable);
private:
    symbol_table_ptr m_symtable;
    std::ofstream m_out;

    void gen_node(const ast::node_ptr &node);
    void gen_integer(const ast::node_ptr &node);
    void gen_string(const ast::node_ptr &node);
    void gen_if_stmt(const ast::node_ptr &node);
    void gen_while_stmt(const ast::node_ptr &node);
    void gen_return_stmt(const ast::node_ptr &node);
    void gen_variable(const ast::node_ptr &node);
    void gen_assign_stmt(const ast::node_ptr &node);
    void gen_call(const ast::node_ptr &node);
    void gen_op_arithm(const ast::node_ptr &node);
    void gen_op_logical(const ast::node_ptr &node);
    void gen_program(const ast::node_ptr &node);
    void gen_type(const ast::node_ptr &node);
    void gen_argument(const ast::node_ptr &node);
    void gen_variable_decl(const ast::node_ptr &node);
    void gen_function_decl(const ast::node_ptr &node);
    void gen_read_stmt(const ast::node_ptr &node);
    void gen_write_stmt(const ast::node_ptr &node);

    int compute_type(const ast::node_ptr &expr);
    int compute_locals(const ast::node_ptr &node);
    std::string jvm_type(int type);

    std::stack<scope_ptr> m_stack;
    int m_local_counter;
    int m_label_counter;

    int get_next_local() { return m_local_counter++; }
    int get_next_label() { return m_label_counter++; }
    void reset_locals() { m_local_counter = 0; }
    void reset_labels() { m_label_counter = 0; }
};

}
