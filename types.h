// -----------------------------------------------------------------------------
// Pararatibum - A linguagem do momento
// -----------------------------------------------------------------------------

#pragma once

namespace ptb { namespace types {

enum {
    voidt = 0,
    integer,
    string,
    boolean,
    character,
    // flags
    function = 0x8000,
};

} // types
} // ptb
