#pragma once

#include "lexer.h"

namespace kate::sc {
    class Parser {
    public:
        Parser() = default;

        void parse(const std::string_view& source);
    private:
        Lexer m_lexer;
    };
}