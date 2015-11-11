// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

// Gramática BNF da linguagem PararaTibum
//
// program ::= decl_list
//
// decl_list ::= decl decl_list
//
// decl ::= func_decl | var_decl | main_decl
//
// var_decl ::= type 'ident' ';'
//            | type 'ident' ':=' expr ';'
//            ;
//
// func_decl ::= type 'ident' '(' arg_list ')' ';'
//             | type 'ident' '(' arg_list ')' '{' stmt_list '}'
//             ;
//
// main_decl ::= '@agora_eu_vou' '(' ')' '{' stmt_list '}'
//
// type ::= '^ela' | '^essa' | '^menino' | '^novinha' | '^deixa'
//
// arg_list ::= type 'ident'
//            | type 'ident' ',' arg_list
//            |
//            ;
//
// stmt_list ::= stmt
//             | stmt stmt_list
//             |
//             ;
//
// stmt ::= if_stmt | while_stmt | return_stmt | identifier_stmt
//        | var_decl | read_stmt | write_stmt
//
//
// identifier_stmt ::= 'ident' '=' expr ';'
//                   | 'ident' '(' param_list ')' ';'
//                   ;
//
// if_stmt ::= 'if' '(' expr ')' '{' stmt_list '}'
//           | 'if' '(' expr ')' '{' stmt_list '}' else '{' stmt_list '}'
//           ;
//
// while_stmt ::= 'while' '(' expr ')' '{' stmt_list '}'
//
// return_stmt ::= 'return' expr ';'
//
// log ::= '>' | '<' | '=' | '>=' | '<=' | '!=' | 'tu' | 'eu'
//
// expr   ::= expr_0 log expr_0 | expr_0
//
// expr_0 ::= expr_1 '+' expr_1 | expr_1 '-' expr_1 | expr_1
//
// expr_1 ::= atom '*' atom | atom '/' atom | atom '%' atom | atom
//
// atom ::= '(' expr ')' | 'number' | 'literal' | identifier | '^esqueca' | '^faz'
//
// identifier ::= 'ident'
//              | 'ident' '(' param_list ')'
//              ;
//
// param_list ::= expr
//              | expr ',' param_list
//              |
//              ;
//
// read_stmt ::=  '^mexer_com' '(' 'ident' ')' ';'
// write_stmt ::= '^mostrar' '(' expr ')' ';'

#include <iostream>
#include <cppfmt/format.h>
#include "lexer.h"
#include "parser.h"
#include "tokens.h"
#include "types.h"

namespace ptb
{

using namespace ast;

parser::parser(ptb::lexer& lex) : m_lex(lex)
{
    if (!m_lex.is_open())
        throw parser_error("Nenhuma arquivo carregado no analisador léxico!");
}

void parser::run()
{
    m_main_defined = false;
    try {
        m_lex.get_token();
        m_program = parse_program();
        if (!is_token(tok::eof)) {
            throw parser_error("Erro na construcao da arvore sintatica");
        }
    } catch (parser_error& error) {
        std::cerr << error.what() << std::endl;
        return;
    }
}

#define expect_error(str) expect_error_(fmt::sprintf("[em %s] %s",__func__,str))
#define match(tok,str)\
    if (!is_token(tok)) expect_error(str); next();
//void parser::match(int token, const std::string& str)
//{
//    if (!is_token(token)) {
//        expect_error(str);
//    }
//    next();
//}


void parser::expect_error_(const std::string &expected)
{
    auto token = m_lex.get_token_info();
    auto msg = fmt::sprintf("Esperado %s na linha %d:%d, encontrado %s [%s]",
                            expected, token.lineno, token.start, token.token, token_name[token.type]);
    throw parser_error(msg.c_str());
}




// program ::= decl_list
node_ptr parser::parse_program()
{
    return make_program(std::move(parse_decl_list()));
}


// decl ::= func_decl | var_decl | main_decl
//
// var_decl ::= type 'ident' ';'
//            | type 'ident' ':=' expr ';'
//            ;
//
// func_decl ::= type 'ident' '(' arg_list ')' ';'
//             | type 'ident' '(' arg_list ')' '{' stmt_list '}'
//             ;
//
// main_decl ::= '@agora_eu_vou' '(' ')' '{' stmt_list '}'
node_ptr parser::parse_decl()
{
    if (is_token(tok::main_)) {
        if (m_main_defined) {
            throw parser_error("A funcao main ja foi definida!");
        }
        auto name = m_lex.token();
        next();
        match(tok::l_par, "(");
        match(tok::r_par, ")");
        match(tok::l_curlbracket, "{");
        auto stmts = parse_stmt_list();
        match(tok::r_curlbracket, "}");
        m_main_defined = true;
        auto type = make_type(types::integer);
        auto args = std::vector<node_ptr>();
        return make_function_decl(name, std::move(type), std::move(args),
                                  std::move(stmts), false);
    }

    auto type = parse_type();
    if (is_token(tok::eof)) {
        return make_node();
    }
    if (!type->is_valid()) {
        expect_error("a declaracao de uma funcao ou variavel");
    }
    if (!is_token(tok::identifier)) {
        expect_error("um identificador");
    }
    std::string name = m_lex.token();
    next();
    if (is_token(tok::semicolon)) {
        next();
        return make_variable_decl(name, std::move(type), std::move(make_node()));
    }
    if (is_token(tok::assign)) {
        next();
        auto expr = parse_expr();
        match(tok::semicolon, ";");
        return make_variable_decl(name, std::move(type), std::move(expr));
    }
    if (is_token(tok::l_par)) {
        next();
        auto args = parse_arg_list();
        match(tok::r_par, ")");
        if (is_token(tok::semicolon)) {
            return make_function_decl(name, std::move(type), std::move(args),
                                      std::move(std::vector<node_ptr>()), true);
        }
        match(tok::l_curlbracket,"{");
        auto stmts = parse_stmt_list();
        match(tok::r_curlbracket,"}");
        return make_function_decl(name, std::move(type), std::move(args),
                                  std::move(stmts), false);
    }

    return make_node();
}



// decl_list ::= decl decl_list
std::vector<node_ptr> parser::parse_decl_list()
{
    std::vector<node_ptr> decls;
    while (auto decl = parse_decl()) {
        if (!decl->is_valid()) {
            break;
        }
        decls.push_back(std::move(decl));
    }
    return decls;
}

// stmt_list ::= stmt
//             | stmt stmt_list
//             ;
std::vector<node_ptr> parser::parse_stmt_list()
{
    std::vector<node_ptr> stmts;
    while (auto stmt = parse_stmt()) {
        if (!stmt->is_valid()) {
            break;
        }
        stmts.push_back(std::move(stmt));
    }
    return stmts;
}

// stmt ::= if_stmt | while_stmt | return_stmt | identifier_stmt
node_ptr parser::parse_stmt()
{
    if (util::is_type(m_lex.get_token())) {
        return parse_var_decl();
    }

    switch (m_lex.get_token()) {
    case tok::if_: return parse_if_stmt();
    case tok::while_: return parse_while_stmt();
    case tok::return_: return parse_return_stmt();
    case tok::identifier: return parse_identifier_stmt();
    case tok::read_: return parse_read_stmt();
    case tok::write_: return parse_write_stmt();
    default:
        return make_node();
    }
}


// stmt ::= if_stmt
node_ptr parser::parse_if_stmt()
{
    if (is_token(tok::if_)) {
        // if ( expr )
        next();
        match(tok::l_par, "(");

        auto expr = parse_expr();

        match(tok::r_par, ")");

        // { expr }
        match(tok::l_curlbracket, "{");

        auto true_stmts = parse_stmt_list();

        match(tok::r_curlbracket, "}");

        if (is_token(tok::else_)) {
            next();
            // { expr }
            match(tok::l_curlbracket, "{");

            auto false_stmts = parse_stmt_list();

            match(tok::r_curlbracket, "}");

            return make_if_stmt(std::move(expr), std::move(true_stmts),
                                std::move(false_stmts));
        }
        return make_if_stmt(std::move(expr), std::move(true_stmts),
                            std::move(std::vector<node_ptr>()));
    }
    return make_node();
}

// stmt ::= while_stmt
node_ptr parser::parse_while_stmt()
{
    if (is_token(tok::while_)) {
        // while ( expr )
        next();
        match(tok::l_par, "(");

        auto expr = parse_expr();

        match(tok::r_par, ")");

        // { expr }
        match(tok::l_curlbracket, "{");

        auto body = parse_stmt_list();

        match(tok::r_curlbracket, "}");

        return make_while_stmt(std::move(expr), std::move(body));
    }
    return make_node();
}

// stmt ::= return_stmt
node_ptr parser::parse_return_stmt()
{
    if (is_token(tok::return_)) {
        // while ( expr )
        next();
        auto expr = parse_expr();
        match(tok::semicolon, ";");
        return make_return_stmt(std::move(expr));
    }
    return make_node();
}

// stmt ::= identifier_stmt
// identifier_stmt ::= 'ident' '=' expr ';'
//                   | 'ident' '(' param_list ')' ';'
//                   ;
//
node_ptr parser::parse_identifier_stmt()
{
    if (is_token(tok::identifier)) {
        auto var_name = m_lex.token();
        next();
        // assign statement
        if (is_token(tok::assign)) {
            auto var = make_variable(var_name);
            next();
            auto expr = parse_expr();
            match(tok::semicolon, ";");
            return make_assign_stmt(std::move(var), std::move(expr));
        }
        if (is_token(tok::l_par)) {
            next();
            auto params = parse_param_list();
            match(tok::r_par, ")");
            match(tok::semicolon, ";");
            return make_call(var_name, std::move(params), true);
        }
        auto token = m_lex.get_token_info();
        auto msg = fmt::sprintf("Esperado uma atribuição ou chamada na linha %d:%d, encontrado %s",
                                token.lineno, token.start, token.token);
        throw parser_error(msg.c_str());
    }
    return make_node();
}


// var_decl ::= type 'ident' ';'
//            | type 'ident' ':=' expr ';'
//            ;
node_ptr parser::parse_var_decl()
{
    if (auto type = parse_type()) {
        if (!is_token(tok::identifier)) {
            expect_error("um identificador");
        }
        std::string name = m_lex.token();
        next();
        if (is_token(tok::semicolon)) {
            next();
            return make_variable_decl(name, std::move(type), std::move(make_node()));
        }
        if (is_token(tok::assign)) {
            next();
            auto expr = parse_expr();
            match(tok::semicolon, ";");
            return make_variable_decl(name, std::move(type), std::move(expr));
        }
    }
    return make_node();
}


// expr ::= expr_0 log expr_0 | expr_0
node_ptr parser::parse_expr()
{
    auto p = parse_expr_0();

    int log_op = parse_logical_op();
    if (log_op >= 0) {
        next();
        auto right = parse_expr();
        return make_op_logical(log_op, std::move(p), std::move(right));
    }
    return p;
}


// expr_0 ::= expr_1 '+' expr_1 | expr_1 '-' expr_1 | expr_1
node_ptr parser::parse_expr_0()
{
    auto p = parse_expr_1();

    if (is_token(tok::plus) || is_token(tok::minus)) {
        int type = m_lex.get_token();
        next();

        auto right = parse_expr_0();
        if (type == tok::plus)
            return make_op_arithm('+', std::move(p), std::move(right));

        return make_op_arithm('-', std::move(p), std::move(right));
    }
    return p;
}

// expr_1 ::= atom '*' atom | atom '/' atom | atom '%' atom | atom
node_ptr parser::parse_expr_1()
{
    auto p = parse_atom();

    if (is_token(tok::mul) || is_token(tok::div) || is_token(tok::mod)) {
        int type = m_lex.get_token();

        next();
        auto right = parse_expr_1();
        if (type == tok::mul)
            return make_op_arithm('*', std::move(p), std::move(right));
        if (type == tok::mod)
            return make_op_arithm('%', std::move(p), std::move(right));
        return make_op_arithm('/', std::move(p), std::move(right));
    }
    return p;
}

// atom ::= '(' expr ')' | 'number' | 'literal' | identifier
node_ptr parser::parse_atom()
{
    if (is_token(tok::integer)) {
        int value = atoi(m_lex.token().c_str());

        next();
        return make_integer(value);
    }
    if (is_token(tok::l_par)) {
        next();
        auto t = parse_expr();
        if (is_token(tok::r_par)) {
            next();
            return t;
        }
        else {
            auto token = m_lex.get_token_info();
            auto msg = fmt::sprintf("Esperado ( na linha %d:%d, encontrado %s",
                                    token.lineno, token.start, token.token);
            throw parser_error(msg.c_str());
        }
    }
    if (is_token(tok::string)) {
        auto lstr = m_lex.token();
        next();
        return make_lstring(lstr);
    }
    if (is_token(tok::identifier)) {
        return parse_identifier();
    }
    if (is_token(tok::true_)) {
        next();
        return make_integer(1);
    }
    if (is_token(tok::false_)) {
        next();
        return make_integer(0);
    }

    return make_node();
}

// identifier ::= 'ident'
//              | 'ident' '(' param_list ')'
//              ;
//
node_ptr parser::parse_identifier()
{
    if (!is_token(tok::identifier)) {
        expect_error("um identificador");
    }
    std::string name = m_lex.token();
    next();
    if (is_token(tok::l_par)) {
        next();
        auto params = parse_param_list();
        match(tok::r_par, ")");
        return make_call(name, std::move(params), false);
    }
    return make_variable(name);
}

// read_stmt ::=  '^mexer_com' '(' 'ident' ')' ';'
node_ptr parser::parse_read_stmt()
{
    if (is_token(tok::read_)) {
        next();
        match(tok::l_par, "(");
        if (!is_token(tok::identifier)) {
            expect_error_("identificador");
        }
        std::string id = m_lex.token();
        next();
        match(tok::r_par, ")");
        match(tok::semicolon, ";");

        return make_read_stmt(id);
    }
    return make_node();
}

// write_stmt ::= '^mostrar' '(' expr ')' ';'
node_ptr parser::parse_write_stmt()
{
    if (is_token(tok::write_)) {
        next();
        match(tok::l_par, "(");
        auto expr = parse_expr();
        match(tok::r_par, ")");
        match(tok::semicolon, ";");
        return make_write_stmt(std::move(expr));
    }
    return make_node();
}


// type ::= '^ela' | '^essa' | '^menino' | '^novinha' | '^deixa'
node_ptr parser::parse_type()
{
    int id = m_lex.get_token();
    int rtid = -1;
    switch (id) {
    case char_: rtid = types::character; break;
    case int_: rtid = types::integer; break;
    case bool_: rtid = types::boolean; break;
    case string_: rtid = types::string; break;
    case void_: rtid = types::voidt; break;
    }
    if(rtid != -1) {
        next();
        return make_type(rtid);
    }
    return make_node();
}


// param_list ::= expr
//              | expr ',' param_list
//              ;
std::vector<node_ptr> parser::parse_param_list()
{
    std::vector<node_ptr> params;
    while (auto parexpr = parse_expr()) {
        if (!parexpr->is_valid()) {
            break;
        }
        params.push_back(std::move(parexpr));
        if (is_token(',')) {
            next();
        }
    }
    return params;
}


//
// arg_list ::= type 'ident'
//            | type 'ident' ',' arg_list
//            ;
//
std::vector<node_ptr> parser::parse_arg_list()
{
    std::vector<node_ptr> args;
    while (auto argtype = parse_type()) {
        if (!argtype->is_valid()) {
            break;
        }
        next();
        if (!is_token(tok::identifier)) {
            expect_error("um identificador");
        }
        std::string name = m_lex.token();
        next();
        args.push_back(make_argument(name, std::move(argtype)));
        if (is_token(',')) {
            next();
        }
    }
    return args;
}

// log ::= '>' | '<' | '=' | '>=' | '<=' | '!=' | 'tu' | 'eu'
int parser::parse_logical_op()
{
    switch (m_lex.get_token()) {
    case tok::gt:
    case tok::lt:
    case tok::eq:
    case tok::ge:
    case tok::le:
    case tok::ne:
    case tok::b_and:
    case tok::b_or:
        return m_lex.get_token();
    }
    return -1;
}

}
