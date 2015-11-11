// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#include <map>
#include <string>
#include <memory>
#include "cppfmt/format.h"
#include "types.h"

#pragma once

namespace ptb {

struct symbol {
    int type;
    int c_type() const { return type & ~types::function; }
    std::string name;
    int scope_id;
    bool is_valid() {
        return type != 0;
    }
    symbol() : type(0) {}
    symbol(const std::string &name_, int type_) :
        type(type_), name(name_), scope_id(0) {}
    symbol(const std::string &name_, int type_, int sid) :
        type(type_), name(name_), scope_id(sid) {}

    // informações utilizadas pelo gerador de código
    int local = -1;
    std::string signature;
};

// Baseado na implementação do livro do Dragão
struct scope {
    std::shared_ptr<scope> prev;
    std::map<std::string, symbol> symbols;
    int id;

    scope(std::shared_ptr<scope> prev_, int id_) : prev(prev_), id(id_) {
    }

    void insert(const std::string &name, const symbol &sym) {
        symbols[name] = sym;
//        fmt::printf("Inserindo simbolo %s no escopo %d\n", name, id);
    }

    symbol& get(const std::string &name) {
        auto cscope = this;
        while (cscope != nullptr) {
            if (cscope->symbols.find(name) != cscope->symbols.end()) {
//                fmt::printf("Encontrou simbolo %s no escopo %d\n", name, cscope->id);
                return cscope->symbols[name];
            }
            cscope = cscope->prev.get();
        }
        static symbol defsym;
        return defsym;
    }
};

typedef std::shared_ptr<scope> scope_ptr;

struct symbol_table {
    std::map<int, scope_ptr> scopes;
    scope_ptr get_scope(int id);
    void put_scope(int id, scope_ptr scope);
};

typedef std::shared_ptr<symbol_table> symbol_table_ptr;

} // ptb


