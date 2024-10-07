#include "parser.h"

namespace kate::sc {
  Parser::Parser(
    const ParserOptions& options
  ) : m_options { options },
    offset { -1 }
  {
  }

  ast::CRef<ast::Module> Parser::parse(const std::string_view& source) 
  {
    m_lexer.tokenize(source);

    std::vector<ast::CRef<ast::Decl>> global_decls;

    while (should_continue()) {
      auto decl = parse_global_declaration();

      if (!decl.matched) return {};

      global_decls.push_back(std::move(decl.value));
    }

    return ast::context().make<ast::Module>(std::move(global_decls));
  }

  Result<ast::CRef<ast::Decl>> Parser::parse_global_declaration()
  {
    if (auto func = parse_func_decl(); func.matched)
      return func;

    // if all global declarations failed, then
    // synchronize to the next '}' and fail.
    sync_to(Token::Type::kRBrace);

    return {};
  }

  Result<ast::CRef<ast::FuncDecl>> Parser::parse_func_decl()
  {
    if (matches("fn")) {
      auto function_name = matches(Token::Type::kIdent);

      if (!function_name)
        return error("expected function name.");

      if (!matches(Token::Type::kLeftParen))
        return error("expected a '(' after function name.");

      std::vector<ast::CRef<ast::FuncArg>> function_args;

      while (should_continue() && !matches(Token::Type::kRightParen)) {
        auto ident = matches(Token::Type::kIdent);

        if (!ident) return error("missing argument identifier.");

        if (!matches(Token::Type::kColon))
          return error("missing ':' after function argument name.");

        auto type = expect_type();

        if (!type.matched) return Failure::kError;

        // (Renan): if we are here, then we have a valid argument.
        function_args.push_back(
          ast::context().make<ast::FuncArg>(
            std::string(std::get<std::string_view>(ident->value())),
            std::move(type.value)
          )
        );
      }

      if (!current()->is(Token::Type::kRightParen))
        return error("expected a ')' after function arguments.");

      return ast::context().make<ast::FuncDecl>(
        std::string(std::get<std::string_view>(function_name->value())),
        std::move(function_args)
      );
    }

    return Failure::kNoMatch;
  }

  Result<ast::CRef<ast::Type>> Parser::expect_type()
  {
    // TODO: Support type arguments.
    auto ident = matches(Token::Type::kIdent);

    if (!ident) return error("expected type identifier.");

    return ast::context().make<ast::Type>(std::string(std::get<std::string_view>(ident->value())));
  }

  Failure Parser::error(
      const std::string_view& message
  )
  {
    if (m_options.error_callback) {
      std::string composed_message = "parser error: (";
      composed_message += m_lexer[offset].location().line;
      composed_message += ":";
      composed_message += m_lexer[offset].location().column;
      composed_message += "): ";
      composed_message += message;

      m_options.error_callback(composed_message);
    }

    return Failure::kError;
  }

  // Sync the parser to the next token of a certain type.
  void Parser::sync_to(Token::Type tok)
  {
    while (!matches(tok) && offset < m_lexer.tokenCount())
      offset++;
  }

  const Token* Parser::current()
  {
    static Token eof_token = Token(Token::Type::kEOF, 0, {});

    if (offset >= m_lexer.tokenCount()) return &eof_token;

    return &m_lexer[offset];
  }

  const Token* Parser::matches(Token::Type type)
  {
    static Token eof_token = Token(Token::Type::kEOF, 0, {});

    if (offset + 1 >= m_lexer.tokenCount()) 
      return &eof_token;

    return (m_lexer[1 + offset].type() == type) ? &m_lexer[++offset] : nullptr;
  }

  const Token* Parser::matches(std::string_view identifier)
  {
    if (offset + 1 >= m_lexer.tokenCount()) 
      return nullptr;

    return (m_lexer[1 + offset].type() == Token::Type::kIdent) ? &m_lexer[++offset] : nullptr;
  }

  bool Parser::should_continue()
  {
    return m_lexer[offset].is(Token::Type::kEOF);
  }

  template<typename T>
  Result<T>::Result(Failure failure) {
    if (failure == Failure::kNoMatch) {
      errored = false;
      matched = false;
    } else {
      errored = true;
      matched = false;
    }
  }

  template<typename T>
  template<typename U>
  Result<T>::Result(Result<U>&& rhs) 
  {
    value = std::move(rhs.value);
    matched = rhs.matched;
    errored = rhs.errored;
  }

  template<typename T>
  Result<T>::Result() : 
    errored{ false }, 
    matched { false }
  {
  }

  template<typename T>
  Result<T>::Result(T&& ptr) : 
    errored { false }, 
    matched { true }, 
    value { std::move(ptr) } 
  {
  }

  template<typename T>
  Result<T>::Result(bool _erroed, bool _matched) : 
    errored { _erroed }, 
    matched { _matched }
  {
  }

  template<typename T>
  Result<T>::Result(bool _erroed, bool _matched, T value) : 
    errored { _erroed }, 
    matched { _matched }, 
    value { std::move(value) }
  {
  }

  template<typename T>
  T&& Result<T>::unwrap() 
  {
    return std::move(value);
  }

  template<typename T>
  T& Result<T>::operator->()
  {
    return value;
  }

  template<typename T>
  bool Result<T>::ok()
  {
    return !errored;
  }
}