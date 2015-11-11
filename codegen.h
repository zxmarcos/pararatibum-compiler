// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#pragma once

#include "ast.h"
#include <vector>
#include <functional>
#include <fstream>

namespace ptb {

class code_gen
{
public:
    code_gen();
    void translate(const ast::node_ptr &node);
private:
    std::ofstream m_out;
};

}
