#pragma once

#include <string_view>
#include <vector>
#include <variant>

namespace kate::tlr {
  struct SourceLocation {
    size_t line = 0;
    size_t column = 0;   
  };

  class Token {
  public:
    enum class Type {
      kInt16,
      kUint16,
      kInt32,
      kInt64,
      kUint32,
      kUint64,
      kFlt32,
      kFlt64,
      kIncrement, // ++
      kDecrement, // --
      kColon,     // :
      kEqual,     // =
      kPlusEq,    // +=
      kMinusEq,   // -=
      kDivideEq,  // /=
      kPercentEq, // %=
      kMulEq,     // *=
      kAnd,       // &
      kOr,        // |
      kOrEq,      // |=
      kTilde,     // ~
      kXor,       // ^
      kXorEq,     // ^=
      kAndEq,     // &=
      kRS,        // >>
      kLS,        // <<
      kRSEq,      // >>=
      kLSEq,      // <<=
      kEqEq,      // ==
      kNotEq,     // !=
      kAndAnd,    // &&
      kExclamation,// !
      kOrOr,      // ||
      kPlus,      // +
      kMinus,     // -
      kAsterisk,  // *
      kSlash,     // /
      kPercent,   // %
      kGTEq,      // >=
      kLTEq,      // <=
      kGT,        // >
      kLT,        // <
      kLeftBracket,  // [
      kRightBracket,  // ]
      kLBrace,    // {
      kRBrace,    // }
      kComma,     // ,
      kDot,       // .
      kSemicolon, // ;
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

    template<typename T>
    const T& value_as() const {
      return std::get<T>(value());
    }

    Type type() const;

    bool is(Type type) const;
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