// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#include <iostream>
#include <exception>
#include <stdexcept>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "analyzer.h"
#include "dotexport.h"
#include "tokens.h"
#include "jvmcodegen.h"

using namespace std;

int main(int argc, char **argv)
{
    try {
        fmt::printf("Compilador de PararaTibum - A linguagem do momento\n");
        if (argc < 2) {
            fmt::printf("Utilizar ptbc <arquivo>\n");
            return 0;
        }
        ptb::lexer lex;
        lex.open(argv[1]);
        ptb::parser parser(lex);
        ptb::analyzer semantic;
        ptb::code_gen gen;
        ptb::dotexport dotter;
        ptb::jvmcodegen jvmcg;

        parser.run();
        const auto& ast = parser.get_ast();
        semantic.run(ast);
        dotter.run(ast);
        gen.translate(ast);
        auto symtbl = semantic.get_symtable();
        jvmcg.run(ast, symtbl);
        fmt::printf("Program compilado com sucesso!\n");
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;

}
