// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

// Aqui é onde os paranauês acontecem...

#include <sstream>
#include "cppfmt/format.h"
#include "jvmcodegen.h"
#include "ast.h"
#include "types.h"
#include "tokens.h"

namespace ptb {

jvmcodegen::jvmcodegen()
{
}

void jvmcodegen::run(const ast::node_ptr &program, symbol_table_ptr& symtable)
{
    m_out.open("ptb.j");
    if (!m_out.is_open()) {
        throw jvmcodegen_error("Nao foi possivel abrir o arquivo ptb.j para escrita");
    }
    m_symtable = symtable;
    // empilha o escopo global
    m_stack.push(m_symtable->get_scope(0));
    reset_locals();

    gen_node(program);

    m_out.close();
}

void jvmcodegen::gen_node(const ast::node_ptr &node) {

    switch (node->type) {
//    case ast::no_node: gen_node(node); break;
    case ast::integer_node: gen_integer(node); break;
    case ast::string_node: gen_string(node); break;
    case ast::if_stmt_node: gen_if_stmt(node); break;
    case ast::while_stmt_node: gen_while_stmt(node); break;
    case ast::return_stmt_node: gen_return_stmt(node); break;
    case ast::variable_node: gen_variable(node); break;
    case ast::assign_stmt_node: gen_assign_stmt(node); break;
    case ast::call_node: gen_call(node); break;
    case ast::op_arithm_node: gen_op_arithm(node); break;
    case ast::op_logical_node: gen_op_logical(node); break;
    case ast::program_node: gen_program(node); break;
    case ast::type_node: gen_type(node); break;
    case ast::argument_node: gen_argument(node); break;
    case ast::variable_decl_node: gen_variable_decl(node); break;
    case ast::function_decl_node: gen_function_decl(node); break;
    case ast::read_stmt_node: gen_read_stmt(node); break;
    case ast::write_stmt_node: gen_write_stmt(node); break;
    }
}
void jvmcodegen::gen_program(const ast::node_ptr &node)
{
    m_out << fmt::sprintf(".class public ptb\n");
    m_out << fmt::sprintf(".super java/lang/Object\n");

    m_out << fmt::sprintf("; construtor padrao\n");
    m_out << fmt::sprintf(".method public <init>()V\n");
    m_out << fmt::sprintf("aload_0 ; empilha o this\n");
    m_out << fmt::sprintf("invokespecial java/lang/Object/<init>()V ; super()\n");
    m_out << fmt::sprintf("return\n");
    m_out << fmt::sprintf(".end method\n\n");

    auto program = ast::to_program(node);
    for (size_t i = 0; i < program->declarations.size(); i++) {
        gen_node(program->declarations[i]);
    }
}

void jvmcodegen::gen_integer(const ast::node_ptr &node)
{
    auto num = ast::to_integer(node);
    if (num->value >= 0 && num->value <= 5) {
        m_out << fmt::sprintf("iconst_%d\n", num->value);
    } else if (num->value == -1) {
        m_out << fmt::sprintf("iconst_m1\n");
    } else {
        m_out << fmt::sprintf("ldc %d\n", num->value);
    }
}

void jvmcodegen::gen_string(const ast::node_ptr &node)
{
    auto str = ast::to_lstring(node);
    m_out << fmt::sprintf("ldc %s\n", str->value);
}

void jvmcodegen::gen_if_stmt(const ast::node_ptr &node)
{
    auto ifstmt = ast::to_if_stmt(node);

    int else_label = get_next_label();
    int end_label = get_next_label();

    gen_node(ifstmt->eval_expr);
    m_out << fmt::sprintf("ifeq L%d\n", else_label);

    m_stack.push(m_symtable->get_scope(ifstmt->true_scope_id));
    for (const auto& stmt : ifstmt->true_statements) {
        gen_node(stmt);
    }
    m_out << fmt::sprintf("goto L%d\n", end_label);
    m_out << fmt::sprintf("L%d:\n", else_label);
    m_stack.pop();

    m_stack.push(m_symtable->get_scope(ifstmt->false_scope_id));
    for (const auto& stmt : ifstmt->false_statements) {
        gen_node(stmt);
    }
    m_stack.pop();

    m_out << fmt::sprintf("L%d:\n", end_label);
}

void jvmcodegen::gen_while_stmt(const ast::node_ptr &node)
{
    auto whilestmt = ast::to_while_stmt(node);
    m_stack.push(m_symtable->get_scope(whilestmt->scope_id));

    int cond_label = get_next_label();
    int end_label = get_next_label();

    m_out << fmt::sprintf("L%d:\n", cond_label);
    gen_node(whilestmt->eval_expr);
    m_out << fmt::sprintf("ifeq L%d\n", end_label);

    for (const auto& stmt : whilestmt->statements) {
        gen_node(stmt);
    }
    m_out << fmt::sprintf("goto L%d\n", cond_label);
    m_out << fmt::sprintf("L%d:\n", end_label);
    m_stack.pop();
}

void jvmcodegen::gen_return_stmt(const ast::node_ptr &node)
{
    auto ret = ast::to_return_stmt(node);
    gen_node(ret->expr);
    int type = compute_type(ret->expr);
    if (type == types::integer) {
        m_out << fmt::sprintf("ireturn\n");
    } else if (type == types::voidt) {
        m_out << fmt::sprintf("return\n");
    } else if (type == types::string) {
        m_out << fmt::sprintf("areturn\n");
    }
}

void jvmcodegen::gen_variable(const ast::node_ptr &node)
{
    auto var = ast::to_variable(node);
    auto sym = m_stack.top()->get(var->name);

    if (sym.c_type() == types::integer) {
        m_out << fmt::sprintf("iload %d\n", sym.local);
    } else if (sym.c_type() == types::string) {
        m_out << fmt::sprintf("aload %d\n", sym.local);
    }
    // TODO: string, booleans, etc...
}

void jvmcodegen::gen_assign_stmt(const ast::node_ptr &node)
{
    auto assign = ast::to_assign_stmt(node);
    auto identifier = ast::to_variable(assign->lvalue);

    auto sym = m_stack.top()->get(identifier->name);
    // escopo global?
    if (m_stack.top()->id <= 0) {
    } else {
        gen_node(assign->rvalue);
        switch (sym.c_type()) {
            case types::integer:
                m_out << fmt::sprintf("istore %d\n", sym.local);
                break;
            case types::string:
                m_out << fmt::sprintf("astore %d\n", sym.local);
                break;
            default:
                throw jvmcodegen_error(
                    fmt::sprintf("Tipo de dado nao suportado para atribuicao %d",
                        sym.c_type()));
        }
    }
}

void jvmcodegen::gen_call(const ast::node_ptr &node)
{
    auto call = ast::to_call(node);
    auto sym = m_stack.top()->get(call->name);

    if (!call->param_list.empty()) {
        size_t i = call->param_list.size() - 1;
        while (true) {
            gen_node(call->param_list[i]);
            if (i == 0)
                break;
            i--;
        }
    }
    m_out << fmt::sprintf("invokestatic ptb/%s\n", sym.signature);
    if (call->is_stmt) {
        // se a função retornar alguma coisa, descarta o resultado
        if (sym.c_type() != types::voidt) {
            m_out << fmt::sprintf("pop\n");
        }
    }
}

void jvmcodegen::gen_op_arithm(const ast::node_ptr &node)
{
    auto op = ast::to_op_arithm(node);
    gen_node(op->left);
    gen_node(op->right);
    switch (op->op) {
        case '+': m_out << fmt::sprintf("iadd\n"); break;
        case '-': m_out << fmt::sprintf("isub\n"); break;
        case '*': m_out << fmt::sprintf("imul\n"); break;
        case '/': m_out << fmt::sprintf("idiv\n"); break;
        case '%': m_out << fmt::sprintf("irem\n"); break;
        default:
            throw jvmcodegen_error(
                fmt::sprintf("Operacao aritmetica invalida %c", op->op));
    }
}

void jvmcodegen::gen_op_logical(const ast::node_ptr &node)
{
    auto op = ast::to_op_logical(node);
    gen_node(op->left);
    gen_node(op->right);
    int true_label = get_next_label();
    int end_label = get_next_label();
    switch (op->op) {
        case tok::eq:
            m_out << fmt::sprintf("if_icmpeq L%d\n", true_label);
            break;
        case tok::ne:
            m_out << fmt::sprintf("if_icmpne L%d\n", true_label);
            break;
        case tok::ge:
            m_out << fmt::sprintf("if_icmpge L%d\n", true_label);
            break;
        case tok::le:
            m_out << fmt::sprintf("if_icmple L%d\n", true_label);
            break;
        case tok::gt:
            m_out << fmt::sprintf("if_icmpgt L%d\n", true_label);
            break;
        case tok::lt:
            m_out << fmt::sprintf("if_icmplt L%d\n", true_label);
            break;
        case tok::b_and:
            m_out << fmt::sprintf("iand\n");
            m_out << fmt::sprintf("ifeq L%d\n", true_label);
            break;
        case tok::b_or:
            m_out << fmt::sprintf("ior\n");
            m_out << fmt::sprintf("ifeq L%d\n", true_label);
            break;
        default:
            throw jvmcodegen_error(fmt::sprintf("Operacao logica invalida %s", token_name[op->op]));
    }
    m_out << fmt::sprintf("iconst_0\n");
    m_out << fmt::sprintf("goto L%d\n", end_label);
    m_out << fmt::sprintf("L%d:\n", true_label);
    m_out << fmt::sprintf("iconst_1\n");
    m_out << fmt::sprintf("L%d:\n", end_label);
}

void jvmcodegen::gen_type(const ast::node_ptr &node) {}

void jvmcodegen::gen_argument(const ast::node_ptr &node)
{
    auto arg = ast::to_argument(node);
    auto& sym = m_stack.top()->get(arg->name);
    sym.local = get_next_local();
}

void jvmcodegen::gen_variable_decl(const ast::node_ptr &node)
{
    auto var = ast::to_variable_decl(node);
    auto& sym = m_stack.top()->get(var->name);
    // escopo global?
    if (m_stack.top()->id <= 0) {
    } else {
        sym.local = get_next_local();
        switch (sym.c_type()) {
            case types::integer:
                if (var->value->is_valid()) {
                    gen_node(var->value);
                } else {
                    m_out << fmt::sprintf("iconst_0\n");
                }
                m_out << fmt::sprintf("istore %d\n", sym.local);
                break;
            case types::string:
                if (var->value->is_valid()) {
                    gen_node(var->value);
                } else {
                    m_out << fmt::sprintf("ldc ""\n");
                }
                m_out << fmt::sprintf("astore %d\n", sym.local);
                break;
            default:
                throw jvmcodegen_error("Apenas inteiros sao suportados pelo gerador");
        }
    }
}

void jvmcodegen::gen_function_decl(const ast::node_ptr &node)
{
    auto func = ast::to_function_decl(node);
    auto curr_scope = m_stack.top();
    auto& sym = curr_scope->get(func->name);
    if (!sym.is_valid()) {
        throw jvmcodegen_error(fmt::sprintf("%s simbolo nao encontrado!", func->name));
    }

    // empilha o escopo da função
    m_stack.push(m_symtable->get_scope(sym.scope_id));
    reset_locals();
    reset_labels();

    m_out << fmt::sprintf(".method public static ");
    std::stringstream ss;
    if (func->is_main()) {
        m_out << fmt::sprintf("main([Ljava/lang/String;)V\n");
    } else {
        ss << fmt::sprintf("%s(", func->name);
        for (size_t i = 0; i < func->arguments.size(); i++) {
            auto arg = ast::to_argument(func->arguments[i]);
            auto& asym = m_stack.top()->get(arg->name);
            ss << fmt::sprintf("%s", jvm_type(asym.c_type()));
            if (i != func->arguments.size() - 1)
                ss << m_out << fmt::sprintf(",");
        }
        ss << fmt::sprintf(")%s", jvm_type(sym.type & ~types::function));

        // salva a assinatura para realizar chamadas..
        sym.signature = ss.str();
        m_out << fmt::sprintf(ss.str());
        m_out << fmt::sprintf("\n");
    }

    int locals = compute_locals(node);
    if (func->is_main())
        locals += 1;
    m_out << fmt::sprintf(".limit locals %d\n", locals);
    m_out << fmt::sprintf(".limit stack 15\n");

    for (const auto& arg : func->arguments) {
        gen_node(arg);
    }
    for (const auto& stmt : func->statements) {
        gen_node(stmt);
    }
    m_stack.pop();

    if (!func->is_main()) {
        if (sym.c_type() == types::integer) {
            m_out << fmt::sprintf("ireturn\n");
        } else if (sym.c_type() == types::voidt) {
            m_out << fmt::sprintf("return\n");
        } else if (sym.c_type() == types::string) {
            m_out << fmt::sprintf("areturn\n");
        }
    } else {
        m_out << fmt::sprintf("return\n");
    }
    m_out << fmt::sprintf(".end method\n\n");
}

void jvmcodegen::gen_read_stmt(const ast::node_ptr &node)
{
    auto read = ast::to_read_stmt(node);

    auto& sym = m_stack.top()->get(read->identifier);
    m_out << fmt::sprintf("new java/util/Scanner\n");
    m_out << fmt::sprintf("dup\n");
    m_out << fmt::sprintf("getstatic java/lang/System/in Ljava/io/InputStream;\n");
    m_out << fmt::sprintf("invokespecial java/util/Scanner/<init>(Ljava/io/InputStream;)V\n");
    if (sym.type == types::integer) {
        m_out << fmt::sprintf("invokevirtual java/util/Scanner/nextInt()I\n");
        m_out << fmt::sprintf("istore %d\n", sym.local);
    } else if (sym.type == types::string) {
        m_out << fmt::sprintf("invokevirtual java/util/Scanner/nextLine()Ljava/lang/String;\n");
        m_out << fmt::sprintf("astore %d\n", sym.local);
    }
}

void jvmcodegen::gen_write_stmt(const ast::node_ptr &node)
{
    auto write = ast::to_write_stmt(node);

    m_out << fmt::sprintf("getstatic java/lang/System/out Ljava/io/PrintStream;\n");
    int type = compute_type(write->expr);
    gen_node(write->expr);
    if (type == types::string) {
        m_out << fmt::sprintf("invokevirtual java/io/PrintStream/print(Ljava/lang/String;)V\n");
    } else if (type == types::integer) {
        m_out << fmt::sprintf("invokevirtual java/io/PrintStream/print(I)V\n");
    }
}

int jvmcodegen::compute_locals(const ast::node_ptr &node)
{
    switch (node->type) {
    case ast::no_node: return 0;
    case ast::integer_node: return 0;
    case ast::string_node: return 0;
    case ast::if_stmt_node: {
        auto ifstmt = ast::to_if_stmt(node);
        int count = 0;
        for (const auto& stmt : ifstmt->true_statements) {
            count += compute_locals(stmt);
        }
        for (const auto& stmt : ifstmt->false_statements) {
            count += compute_locals(stmt);
        }
        return count;
    }
    case ast::while_stmt_node: {
        auto whilestmt = ast::to_while_stmt(node);
        int count = 0;
        for (const auto& stmt : whilestmt->statements) {
            count += compute_locals(stmt);
        }
        return count;
    }
    case ast::return_stmt_node: return 0;
    case ast::variable_node: return 0;
    case ast::assign_stmt_node: return 0;
    case ast::call_node: return 0;
    case ast::op_arithm_node: return 0;
    case ast::op_logical_node: return 0;
    case ast::program_node: return 0;
    case ast::type_node: return 0;
    case ast::argument_node: return 1;
    case ast::variable_decl_node: return 1;
    case ast::function_decl_node: {
        auto func = ast::to_function_decl(node);
        int count = 0;
        count += func->arguments.size();
        for (const auto& stmt : func->statements) {
            count += compute_locals(stmt);
        }
        return count;
    }
    }
    return 0;
}

// Transforma um tipo nativo no tipo correspondente da JVM
std::string jvmcodegen::jvm_type(int type)
{
    switch (type) {
    case types::integer: return "I";
    case types::boolean: return "Z";
    case types::character: return "C";
    case types::voidt: return "V";
    case types::string: return "Ljava/lang/String;";
    }
    // void é o padrão...
    return "V";
}

// Copiado do analisador semântico
// TODO: refatorar...
int jvmcodegen::compute_type(const ast::node_ptr &expr)
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
