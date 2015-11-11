// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#pragma once
#include <stdexcept>
#include <string>
#include "ast.h"

namespace ptb {

struct optimizer_error : public std::runtime_error {
    optimizer_error(const std::string& w) : std::runtime_error(w) {
    }
};

class optimizer
{
public:
    optimizer();

    void run(const ast::node_ptr &node);

private:
    void fold_pass(const ast::node_ptr &node);
    int fold_expr(const ast::node_ptr &node);
    int can_fold(const ast::node_ptr &node);
};

}
