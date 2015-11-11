// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#pragma once

#include <stdexcept>
#include <string>
#include <memory>
#include "lexer.h"
#include "ast.h"

namespace ptb
{

struct parser_error : public std::runtime_error {
    parser_error(const std::string& w) : std::runtime_error(w) {
    }
};

class parser
{
    lexer& m_lex;

    ast::node_ptr m_program;
    bool m_main_defined;
public:
    parser(lexer& lex);

    void run();
    const ast::node_ptr& get_ast() { return m_program; }
private:
    bool is_token(int token) {
        return (m_lex.get_token() == token);
    }
    bool is_next_token(int token) {
        return (m_lex.peek_next_token() == token);
    }

    void next() { m_lex.consume(); }
//    void match(int token, const std::string &str);

    void expect_error_(std::string const& expected);
    ast::node_ptr parse_program();
    ast::node_ptr parse_decl();

    std::vector<ast::node_ptr> parse_decl_list();
    std::vector<ast::node_ptr> parse_stmt_list();
    ast::node_ptr parse_stmt();
    ast::node_ptr parse_if_stmt();
    ast::node_ptr parse_while_stmt();
    ast::node_ptr parse_return_stmt();
    ast::node_ptr parse_identifier_stmt();
    ast::node_ptr parse_var_decl();

    ast::node_ptr parse_expr();
    ast::node_ptr parse_expr_0();
    ast::node_ptr parse_expr_1();
    ast::node_ptr parse_atom();
    ast::node_ptr parse_identifier();

    ast::node_ptr parse_read_stmt();
    ast::node_ptr parse_write_stmt();

    ast::node_ptr parse_type();

    std::vector<ast::node_ptr> parse_param_list();
    std::vector<ast::node_ptr> parse_arg_list();
    int parse_logical_op();
};

}
