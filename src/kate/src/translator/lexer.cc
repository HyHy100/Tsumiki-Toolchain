#include "lexer.h"

#include "base/numeric.h"

#include <cstdio>
#include <charconv>
#include <stdexcept>
#include <iostream>
#include <utility>

namespace kate::tlr {
  Token::Token(
    Token::Type type,
    const value_t& value,
    const SourceLocation& loc
  ) : m_loc { loc },
    m_value { value },
    m_type { type }
  {
  }

  const Token::value_t& Token::value() const
  {
    return m_value;
  }

  const SourceLocation& Token::location() const
  {
    return m_loc;
  }

  Token::Type Token::type() const
  {
    return m_type;
  }

  bool Token::is(Type type) const
  {
    return (m_type == type);
  }

  Lexer::Lexer()
  {
  }

  void Lexer::tokenize(const std::string_view& source)
  {
    size_t offset = 0;

    SourceLocation loc;

    auto advance = [&] {
      auto tok = source.at(offset++);

      if (tok == '\n') {
        loc.line++;
        loc.column = 0;
      } else loc.column++;

      return tok;
    };
    
    auto can_peek = [&](size_t off) {
      return offset + off < source.size();
    };

    auto peek = [&](size_t off) {
      if (!can_peek(off)) return static_cast<char>(EOF);
    
      return source.at(offset + off);
    };

    auto matches = [&](size_t offset_, char c) {
      return can_peek(offset_) && peek(offset_) == c;
    };

    auto is_hex_digit = [&](size_t off) {
      if (!can_peek(1)) return false;

      auto c = peek(offset);

      return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    };

    auto is_ident = [](char c) {
      return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
    };

    auto is_number = [](char c) {
      return c >= '0' && c <= '9';
    };

    auto is_ident_or_number = [&](char c) {
      return is_number(c) || is_ident(c);
    };

    auto show_error_and_die = [&](std::string_view str) {
      std::cerr << "ERROR (" << loc.line << ":" << loc.column << "): " << str << std::endl;

      std::exit(1);
    };

    bool found_eof = false;

    while (can_peek(0) && !found_eof) {
      char c = peek(0);
      
      if (is_number(c)) {
        std::string_view number;

        uint64_t value = 0;

        number = std::string_view(&source[offset], 1);
        auto start = offset;

        advance();

        // 0[x]
        //  ^
        // ___|
        if (c == '0' && (matches(0, 'x') || matches(0, 'X'))) {
          // advance
          advance();

          start = offset;
          number = { &source[start], 1 };

          while (is_hex_digit(0)) {
            number = { &source[start], offset - start + 1 };
            advance();
          }

          if (start == offset) 
            show_error_and_die("Failed to parse hexadecimal integer. Missing integer part of hex.");

          auto [_, ec] = std::from_chars(number.data(), number.data() + number.length(), value, 16);
        
          if (ec != std::errc()) {
            if (ec == std::errc::invalid_argument)
              show_error_and_die("Failed to parse hexadecimal integer. Not a number.");
            else if (ec == std::errc::result_out_of_range)
              show_error_and_die("Failed to parse hexadecimal integer. Number is larger than an i64.");
            else show_error_and_die("Failed to parse hexadecimal integer.");
          }
        } else {
          while (is_number(peek(0))) {
            number = { &source[start], offset - start + 1 };
            advance();
          }

          // If after constructing the number we have a '.', then it's a fractional
          if (matches(0, '.')) {
            advance(); // .

            while (is_number(peek(0))) {
              number = { &source[start], offset - start + 1 };
              
              advance();
            }   

            double dvalue;

            auto [_, ec] = std::from_chars(number.data(), number.data() + number.length(), dvalue);
          
            if (ec != std::errc()) {
              if (ec == std::errc::invalid_argument)
                show_error_and_die("Failed to parse fp64. Not a number.");
              else if (ec == std::errc::result_out_of_range)
                show_error_and_die("Failed to parse fp64. Number is larger than the maximum limit.");
              else
                show_error_and_die("Failed to parse fp64.");
            }

            if (matches(0, 'f')) {
              advance();

              if (base::in_range<float>(dvalue))
                m_tokens.emplace_back(
                  Token::Type::kFlt32,
                  dvalue,
                  loc
                );
              else
                show_error_and_die("Number is larger than maximum float32 limit");
            } else {
              if (matches(0, 'd')) 
                advance();

              m_tokens.emplace_back(
                Token::Type::kFlt64,
                dvalue,
                loc
              );
            }

            continue;
          } else {
            // if it's not a fp value, then parse it to an uint64
            auto [_, ec] = std::from_chars(number.data(), number.data() + number.length(), value, 10);
          
            if (ec != std::errc()) {
              if (ec == std::errc::invalid_argument)
                show_error_and_die("Failed to parse integer. Not a number.");
              else if (ec == std::errc::result_out_of_range)
                show_error_and_die("Failed to parse integer. Number is larger than the i64 max limit.");
              else
                show_error_and_die("Failed to parse integer.");
            }
          }
        }

        if (matches(0, 'u')) {
          advance();

          if (matches(0, 'l')) {
            advance();

            m_tokens.emplace_back(
              Token::Type::kUint32,
              static_cast<int64_t>(value),
              loc
            );
          } else if (matches(0, 's')) {
            advance();

            if (base::in_range<uint16_t>(value)) {
              m_tokens.emplace_back(
                Token::Type::kUint16,
                static_cast<int64_t>(value),
                loc
              );
            } else show_error_and_die("Value overflows u16 limits.");
          } else {
            if (base::in_range<uint32_t>(value)) {
              m_tokens.emplace_back(
                Token::Type::kUint32,
                static_cast<int64_t>(value),
                loc
              );
            } else show_error_and_die("Value overflows u32 limits.");
          }
        } else if (matches(0, 'l')) {
          advance();

          if (base::in_range<int64_t>(value))
            m_tokens.emplace_back(
              Token::Type::kInt64,
              static_cast<int64_t>(value),
              loc
            );
          else show_error_and_die("Value overflows i64 limits.");
        } else if (matches(0, 's')) {
          advance();

          if (base::in_range<int16_t>(value))
            m_tokens.emplace_back(
              Token::Type::kInt16,
              static_cast<int64_t>(value),
              loc
            );
          else show_error_and_die("Value overflows i16 limits.");
        } else {
          matches(0, 'i'); // optionally skip 'i'

          if (base::in_range<int32_t>(value)) {
            m_tokens.emplace_back(
              Token::Type::kInt32,
              static_cast<int64_t>(value),
              loc
            );
          } else show_error_and_die("Value overflows i32 limits.");
        }

        continue;
      }

      if (is_ident(c)) {
        std::string_view identifier;

        auto start = offset;

        while (is_ident_or_number(peek(0))) {
          identifier = { &source[start], offset - start + 1 };
          
          advance();
        }

        m_tokens.emplace_back(
          Token::Type::kIdent,
          identifier,
          loc
        );

        continue;
      }

      switch (c) {
        case '=':
          m_tokens.emplace_back(
            Token::Type::kEqual,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case '?':
          m_tokens.emplace_back(
            Token::Type::kQMark,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case '>':
          if (matches(1, '>')) {
            if (matches(2, '=')) {
              m_tokens.emplace_back(
                Token::Type::kLSEq,
                std::string_view { &source[offset], 2 },
                loc
              );

              advance();
            } else {
              m_tokens.emplace_back(
                Token::Type::kLS,
                std::string_view { &source[offset], 2 },
                loc
              );
            }

            advance();
          } else if (matches(1, '=')) {
            m_tokens.emplace_back(
              Token::Type::kGTEq,
              std::string_view { &source[offset], 2 },
              loc
            );

            advance();
          } else {
            m_tokens.emplace_back(
              Token::Type::kGT,
              std::string_view { &source[offset], 1 },
              loc
            );
          }

          advance();
          break;
        case '<':
          if (matches(1, '<')) {
            if (matches(2, '=')) {
              m_tokens.emplace_back(
                Token::Type::kRSEq,
                std::string_view { &source[offset], 2 },
                loc
              );

              advance();
            } else {
              m_tokens.emplace_back(
                Token::Type::kRS,
                std::string_view { &source[offset], 2 },
                loc
              );
            }

            advance();
          } else if (matches(1, '=')) {
            m_tokens.emplace_back(
              Token::Type::kLTEq,
              std::string_view { &source[offset], 2 },
              loc
            );

            advance();
          } else {
            m_tokens.emplace_back(
              Token::Type::kLT,
              std::string_view { &source[offset], 1 },
              loc
            );
          }

          advance();
          break;
        case '~':
          m_tokens.emplace_back(
            Token::Type::kTilde,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case '(':
          m_tokens.emplace_back(
            Token::Type::kLeftParen,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case '%':
          if (matches(1, '=')) {
            m_tokens.emplace_back(
              Token::Type::kPercentEq,
              std::string_view { &source[offset], 2 },
              loc
            );

            advance();
          } else {
            m_tokens.emplace_back(
              Token::Type::kPercent,
              std::string_view { &source[offset], 1 },
              loc
            );
          }

          advance();
          break;
        case '|':
          if (matches(1, '=')) {
            m_tokens.emplace_back(
              Token::Type::kOrEq,
              std::string_view { &source[offset], 2 },
              loc
            );

            advance();
          } else {
            m_tokens.emplace_back(
              Token::Type::kOr,
              std::string_view { &source[offset], 1 },
              loc
            );
          }
          advance();
          break;
        case '&':
          if (matches(1, '=')) {
            m_tokens.emplace_back(
              Token::Type::kAndEq,
              std::string_view { &source[offset], 2 },
              loc
            );

            advance();
          } else {
            m_tokens.emplace_back(
              Token::Type::kAnd,
              std::string_view { &source[offset], 1 },
              loc
            );
          }
          advance();
          break;
        case '@':
          m_tokens.emplace_back(
            Token::Type::kAt,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case ':':
          m_tokens.emplace_back(
            Token::Type::kColon,
            std::string_view { &source[offset], 1 },
            loc
          );

          advance();

          break;
        case '/':
          if (matches(1, '=')) {
            m_tokens.emplace_back(
              Token::Type::kDivideEq,
              std::string_view { &source[offset], 2 },
              loc
            );
            advance(); // /
            advance(); // =
          } else if (matches(1, '/')) {
            advance(); advance(); // '//'

            while (!matches(0, '\n')) advance();

            advance(); // \n
          } else {
            m_tokens.emplace_back(
              Token::Type::kSlash,
              std::string_view { &source[offset], 1 },
              loc
            );
            advance();
          }
          break;
        case '!':
          m_tokens.emplace_back(
            Token::Type::kExclamation,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case ')':
          m_tokens.emplace_back(
            Token::Type::kRightParen,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case '^':
          if (matches(1, '=')) {
            m_tokens.emplace_back(
              Token::Type::kXorEq,
              std::string_view { &source[offset], 2 },
              loc
            );

            advance();
          } else {
            m_tokens.emplace_back(
              Token::Type::kXor,
              std::string_view { &source[offset], 1 },
              loc
            );
          }

          advance();
          break;
        case '\'':
        case '+':
          if (matches(1, '=')) {
            m_tokens.emplace_back(
              Token::Type::kPlusEq,
              std::string_view { &source[offset], 2 },
              loc
            );
            advance();
          } else if (matches(1, '+')) {
            m_tokens.emplace_back(
              Token::Type::kIncrement,
              std::string_view { &source[offset], 2 },
              loc
            );
            advance();
          } else {
            m_tokens.emplace_back(
              Token::Type::kPlus,
              std::string_view { &source[offset], 1 },
              loc
            );
          }
          advance();
          break;
        case '-':
          if (matches(1, '=')) {
            m_tokens.emplace_back(
              Token::Type::kMinusEq,
              std::string_view { &source[offset], 2 },
              loc
            );
            advance();
          } else if (matches(1, '-')) {
            m_tokens.emplace_back(
              Token::Type::kDecrement,
              std::string_view { &source[offset], 2 },
              loc
            );

            advance();
          } else {
            m_tokens.emplace_back(
              Token::Type::kMinus,
              std::string_view { &source[offset], 1 },
              loc
            );
          }

          advance();
          break;
        case '*':
          if (matches(1, '=')) {
            m_tokens.emplace_back(
              Token::Type::kMulEq,
              std::string_view { &source[offset], 2 },
              loc
            );
            
            advance();
          } else {
            m_tokens.emplace_back(
              Token::Type::kAsterisk,
              std::string_view { &source[offset], 1 },
              loc
            );
          }
          
          advance();
          break;
        case '.':
          m_tokens.emplace_back(
            Token::Type::kDot,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case ',':
          m_tokens.emplace_back(
            Token::Type::kComma,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case '\r': {           
          if (matches(1, '\n')) {
            advance();
          }
          advance();
          break;
        }
        case '\n':
          advance();
          break;
        case '{':
          m_tokens.emplace_back(
            Token::Type::kLBrace,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case '[':
          m_tokens.emplace_back(
            Token::Type::kLeftBracket,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case ']':
          m_tokens.emplace_back(
            Token::Type::kRightBracket,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case ';':
          m_tokens.emplace_back(
            Token::Type::kSemicolon,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case '}':
          m_tokens.emplace_back(
            Token::Type::kRBrace,
            std::string_view { &source[offset], 1 },
            loc
          );
          advance();
          break;
        case '\0':
        case EOF:
          m_tokens.emplace_back(
            Token::Type::kEOF,
            std::string_view { &source[offset], 1 },
            loc
          );

          advance();

          found_eof = true;
          break;
        case '\t':
        case ' ':
          advance();
          break;
        default:
          static auto err = std::string { "ERROR: Unhandled token '" } + peek(0) + "'";
          
          show_error_and_die(err);
      }
    }

    // Add dummy EOF token if not found
    if (!found_eof) {
      m_tokens.emplace_back(
        Token::Type::kEOF,
        std::string_view("\0"),
        loc
      );
    }

    for (auto& tok : m_tokens) {
      std::visit([](auto& v) {
        std::cerr << v << " ";
      }, tok.value());
    }

    std::cerr << std::endl;
  }

  const std::vector<Token>& Lexer::tokens()
  {
    return m_tokens;
  }

  size_t Lexer::tokenCount() const
  {
    return m_tokens.size();
  }

  const Token& Lexer::operator[](size_t index)
  {
    return m_tokens.at(index);
  }
}