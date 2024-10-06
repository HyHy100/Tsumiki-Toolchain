#include "parser.h"

namespace kate::sc {
    void Parser::parse(const std::string_view& source) {
        m_lexer.tokenize(source);

        
    }
}