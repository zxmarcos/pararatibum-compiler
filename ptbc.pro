TEMPLATE = app
CONFIG -= app_bundle
CONFIG += c++11
CONFIG += exceptions

SOURCES += main.cpp \
    lexer.cpp \
    parser.cpp \
    cppfmt/format.cpp \
    codegen.cpp \
    analyzer.cpp \
    dotexport.cpp \
    jvmcodegen.cpp \
    symtable.cpp \
    optimizer.cpp

HEADERS += \
    lexer.h \
    tokens.h \
    parser.h \
    cppfmt/format.h \
    ast.h \
    codegen.h \
    analyzer.h \
    dotexport.h \
    symtable.h \
    jvmcodegen.h \
    types.h \
    optimizer.h

