#pragma once

#include <string_view>
#include <vector>
#include <variant>

namespace kate::sc {
    struct SourceLocation {
        size_t line = 0;
        size_t column = 0;   
    };

    class Token {
    public:
        enum class Type {
            kInteger,       // integers
            kFloat,         // float

            kColon,         // :

            kEqual,         // =
            kPlusEqual,     // +=
            kMinusEqual,    // -=
            kDivideEqual,   // /=
            kPercentEqual,  // %=
            kMulEq,         // *=

            kLnBrk,

            kIncrement,     // ++
            kDecrement,     // --

            kAnd,       // &
            kOr,        // |

            kPlus,      // +
            kMinus,     // -
            kAsterisk,  // *
            kSlash,     // /
            kPercent,   // %

            kGTEq,      // >=
            kLTEq,      // <=

            kGT,        // >
            kLT,        // <

            kLBracket,  // [
            kRBracket,  // ]

            kLBrace,    // {
            kRBrace,    // }            

            kComma,     // ,
            kDot,       // .

            kSemicolon, // ;

            kWhitespace,

            kQMark,     // ?
            
            kLeftParen, // (
            kRightParen,// )

            kAt,        // @

            kIdent,     // identifier

            kEOF,

            kCount
        };

        using value_t = std::variant<std::string_view, uint64_t, int64_t, double>;

        Token(
            Type type,
            const value_t& value,
            const SourceLocation& loc
        );

        const SourceLocation& location() const;

        const value_t& value() const;

        Type type() const;
    private:
        SourceLocation m_loc;
        Type m_type;
        value_t m_value;
    };
    
    class Lexer {
    public:
        Lexer();

        void tokenize(const std::string_view& source);

        const std::vector<Token>& tokens();

        size_t tokenCount() const;

        const Token& operator[](size_t index);
    private:
        std::vector<Token> m_tokens;
    };
}