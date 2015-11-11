// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#pragma once

#include <map>
#include <string>
#include <fstream>
#include "ast.h"

namespace ptb {

class dotexport
{
public:
    dotexport();
    void run(const ast::node_ptr &ast);
private:
    int export_node(const ast::node_ptr &node);
    void set_value(int id, const std::string &str);
    void link_nodes(int from, int to);
    int m_node_counter;
    int get_next_node() { return m_node_counter++; }
    std::map<int, std::string> m_nodes;
    std::ofstream m_out;
};

}
