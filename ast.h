// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#pragma once

#include <vector>
#include <string>
#include <memory>

namespace ptb { namespace ast {

typedef enum {
    no_node = 0,
    integer_node,
    string_node,
    op_arithm_node,
    op_logical_node,
    program_node,
    if_stmt_node,
    while_stmt_node,
    return_stmt_node,
    assign_stmt_node,
    variable_node,
    variable_decl_node,
    function_decl_node,
    call_node,
    type_node,
    argument_node,
    read_stmt_node,
    write_stmt_node,
} ast_type;

struct node {
    const ast_type type;
    node(ast_type type_=no_node) : type(type_) { }
    virtual ~node() {}
    bool is_valid() { return type != no_node; }
};


typedef std::unique_ptr<node> node_ptr;


struct program : node {
    std::vector<node_ptr> declarations;
    program(std::vector<node_ptr> decls) :
        node(program_node), declarations(std::move(decls)) {
    }
};


struct while_stmt : node {
    node_ptr eval_expr;
    std::vector<node_ptr> statements;
    int scope_id;
    while_stmt(node_ptr cond_, std::vector<node_ptr> stmts) : node(while_stmt_node),
        eval_expr(std::move(cond_)), statements(std::move(stmts)) {}
};


struct return_stmt : node {
    node_ptr expr;
    return_stmt(node_ptr expr_) : node(return_stmt_node),
        expr(std::move(expr_)) {}
};

struct assign_stmt : node {
    node_ptr lvalue;
    node_ptr rvalue;
    assign_stmt(node_ptr lv, node_ptr rv) : node(assign_stmt_node),
        lvalue(std::move(lv)), rvalue(std::move(rv)) {}
};


struct type : node {
    int type_id;
    type(int id) : node(type_node), type_id(id) {}
};


struct argument : node {
    const std::string name;
    node_ptr type_expr;
    argument(const std::string name_, node_ptr type_) : node(argument_node),
        name(name_), type_expr(std::move(type_)) {}
};

struct variable : node {
    const std::string name;
    variable(const std::string& name_) : node(variable_node), name(name_) {
    }
};

struct variable_decl : node {
    const std::string name;
    node_ptr type_expr;
    node_ptr value;
    variable_decl(const std::string& name_, node_ptr type_, node_ptr value_) : node(variable_decl_node),
        name(name_), type_expr(std::move(type_)), value(std::move(value_)) {
    }
};


struct function_decl : node {
    const std::string name;
    node_ptr return_type;
    std::vector<node_ptr> arguments;
    std::vector<node_ptr> statements;
    bool is_prototype;
    bool is_main() {
        static const std::string m("@agora_eu_vou");
        return name == m;
    }

    function_decl(const std::string& name_, node_ptr rtype,
        std::vector<node_ptr> args,
        std::vector<node_ptr> stmts, bool isproto) :
        node(function_decl_node), name(name_), return_type(std::move(rtype)),
        arguments(std::move(args)), statements(std::move(stmts)), is_prototype(isproto) {
    }
};


struct call : node {
    const std::string name;
    std::vector<node_ptr> param_list;
    bool is_stmt;
    call(const std::string& name_, std::vector<node_ptr> parlist, bool is_stmt_) :
        node(call_node), name(name_), param_list(std::move(parlist)), is_stmt(is_stmt_) {
    }
};

struct integer : node {
    int value;

    integer(int value_) : node(integer_node), value(value_) {}
};

struct lstring : node {
    const std::string value;
    lstring(const std::string& str) : node(string_node), value(str) {}
};

struct op_arithm : node {
    char op;
    node_ptr left;
    node_ptr right;
    op_arithm(char op_, node_ptr lhs, node_ptr rhs) :
        node(op_arithm_node), op(op_),
        left(std::move(lhs)), right(std::move(rhs)) {}
};

struct op_logical : node {
    char op;
    node_ptr left;
    node_ptr right;
    op_logical(char op_, node_ptr lhs, node_ptr rhs) :
        node(op_logical_node), op(op_),
        left(std::move(lhs)), right(std::move(rhs)) {}
};


struct if_stmt : node {
    node_ptr eval_expr;
    std::vector<node_ptr> true_statements;
    std::vector<node_ptr> false_statements;
    int true_scope_id;
    int false_scope_id;
    if_stmt(node_ptr eval, std::vector<node_ptr> truec, std::vector<node_ptr> falsec) :
        node(if_stmt_node), eval_expr(std::move(eval)),
        true_statements(std::move(truec)), false_statements(std::move(falsec)) {
    }
};

struct read_stmt : node {
    std::string identifier;
    read_stmt(const std::string &str) : node(read_stmt_node), identifier(str) {}
};

struct write_stmt : node {
    node_ptr expr;
    write_stmt(node_ptr expr_) : node(write_stmt_node), expr(std::move(expr_)) {}
};

#define AST_MAKE_(name)                                                     \
    template<typename...Args>                                               \
    inline node_ptr make_##name(Args&&... args) {                           \
        return std::unique_ptr<name>(new name(std::forward<Args>(args)...));\
    }                                                                       \
    inline name* to_##name(const node_ptr &ptr) {                           \
        return static_cast<name*>(ptr.get());                               \
    }


AST_MAKE_(node)
AST_MAKE_(integer)
AST_MAKE_(lstring)
AST_MAKE_(if_stmt)
AST_MAKE_(while_stmt)
AST_MAKE_(return_stmt)
AST_MAKE_(variable)
AST_MAKE_(assign_stmt)
AST_MAKE_(read_stmt)
AST_MAKE_(write_stmt)
AST_MAKE_(call)
AST_MAKE_(op_arithm)
AST_MAKE_(op_logical)
AST_MAKE_(program)
AST_MAKE_(type)
AST_MAKE_(argument)
AST_MAKE_(variable_decl)
AST_MAKE_(function_decl)


} // ast
} // ptb
