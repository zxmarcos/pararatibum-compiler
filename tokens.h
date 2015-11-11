// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#pragma once

#include <map>

namespace ptb
{

enum tok {
    eof = 0,
    integer,
    identifier,
    string,

    // delimitadores
    l_par,
    r_par,
    l_bracket,
    r_bracket,
    l_curlbracket,
    r_curlbracket,

    comma,
    colon,
    semicolon,
    dot,

    // operações
    plus,
    minus,
    mul,
    div,
    mod,
    assign,

    // comparações
    neg,
    eq,
    ne,
    ge,
    le,
    gt,
    lt,
    b_or,
    b_and,

    // tipos
    char_,
    int_,
    bool_,
    string_,
    void_,

    main_,

    true_,
    false_,
    write_,
    read_,

    itos_,
    stoi_,

    if_,
    else_,
    return_,
    while_,
};

static std::map<int, const char*> token_name =
{
    { eof, "eof" },
    { integer, "integer" },
    { identifier, "identifier" },
    { string, "string" },
    { l_par, "l_par" },
    { r_par, "r_par" },
    { l_bracket, "l_bracket" },
    { r_bracket, "r_bracket" },
    { l_curlbracket, "l_curlbracket" },
    { r_curlbracket, "r_curlbracket" },
    { comma, "comma" },
    { colon, "colon" },
    { semicolon, "semicolon" },
    { dot, "dot" },
    { plus, "plus" },
    { minus, "minus" },
    { mul, "mul" },
    { div, "div" },
    { mod, "mod" },
    { assign, "assign" },
    { neg, "neg" },
    { eq, "eq" },
    { ne, "ne" },
    { ge, "ge" },
    { le, "le" },
    { gt, "gt" },
    { lt, "lt" },
    { b_or, "b_or" },
    { b_and, "b_and" },
    { char_, "char_" },
    { int_, "int_" },
    { bool_, "bool_" },
    { string_, "string_" },
    { void_, "void_" },
    { main_, "main_" },
    { true_, "true_" },
    { false_, "false_" },
    { write_, "write_" },
    { read_, "read_" },
    { itos_, "itos_" },
    { stoi_, "stoi_" },
    { if_, "if_" },
    { else_, "else_" },
    { return_, "return_" },
    { while_, "while_" },
};

namespace util {

inline bool is_keyword(int token) {
    switch (token) {
    case char_:
    case int_:
    case bool_:
    case string_:
    case void_:
    case main_:
    case true_:
    case false_:
    case write_:
    case read_:
    case itos_:
    case stoi_:
    case if_:
    case else_:
    case return_:
    case while_:
        return true;
    }
    return false;
}

inline bool is_operator(int token) {
    switch (token) {
    case plus:
    case minus:
    case mul:
    case div:
    case mod:
    case assign:
    case neg:
    case eq:
    case ne:
    case ge:
    case le:
    case gt:
    case lt:
    case b_or:
    case b_and:
        return true;
    }
    return false;
}

inline bool is_punctuation(int token) {
    switch (token) {
    case l_par:
    case r_par:
    case l_bracket:
    case r_bracket:
    case l_curlbracket:
    case r_curlbracket:
    case comma:
    case colon:
    case semicolon:
    case dot:
        return true;
    }
    return false;
}

inline bool is_type(int token) {
    switch (token) {
    case int_:
    case char_:
    case bool_:
    case void_:
    case string_:
        return true;
    }
    return false;
}
}

}
