
#include <memory>
#include "cppfmt/format.h"
#include "symtable.h"


namespace ptb {

scope_ptr symbol_table::get_scope(int id)
{
    if (scopes.find(id) != scopes.end())
        return scopes[id];
    return nullptr;
}

void symbol_table::put_scope(int id, scope_ptr scope)
{
    if (scopes.find(id) != scopes.end()) {
        fmt::printf("Escopo %d ja existe\n", id);
        return;
    }
    if (scope)
        scopes[id] = scope;
}


}
