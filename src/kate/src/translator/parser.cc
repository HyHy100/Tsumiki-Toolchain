#include "parser.h"

#include <fmt/format.h>

namespace kate::tlr {
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
      fmt::println("processing....");

      auto decl = parse_global_declaration();

      if (!decl.matched) return {};

      fmt::println("Adding new global declaration: {}", decl.value->name());

      global_decls.push_back(std::move(decl.value));
    }

    return ast::context().make<ast::Module>(std::move(global_decls));
  }

  Result<ast::CRef<ast::Decl>> Parser::parse_global_declaration()
  {
    auto attrs = parse_attributes();

    fmt::println("attribute count: {}", attrs.value.size());

    if (auto func = parse_func_decl(attrs.value); func.matched)
      return func;

    if (auto buffer = parse_buffer_decl(attrs.value); buffer.matched) {
      fmt::println("buffer name2: ", buffer->name());
      return buffer;
    }

    // if all global declarations failed, then
    // synchronize to the next '}' and fail.
    sync_to(Token::Type::kRBrace);

    return {};
  }

  Result<ast::CRef<ast::Stat>> Parser::statement()
  {
    return {};
  }

  Result<ast::CRef<ast::BlockStat>> Parser::parse_block()
  {
    std::vector<ast::CRef<ast::Stat>> statements;

    if (matches(Token::Type::kLBrace)) {
      while (should_continue() && !matches(Token::Type::kRBrace)) {
        auto stat = statement();

        if (stat.errored) return Failure::kError;

        statements.push_back(std::move(stat.value));
      }

      if (!current()->is(Token::Type::kRBrace)) return error("missing '}' after end of statement block.");

      return ast::context().make<ast::BlockStat>(std::move(statements));
    }

    return Failure::kNoMatch;
  }

  Result<std::vector<ast::CRef<ast::Expr>>> Parser::parse_expression_list()
  {
    auto expr = parse_expr();

    if (!expr.matched) return Failure::kNoMatch;

    std::vector<ast::CRef<ast::Expr>> expr_list;
    expr_list.push_back(std::move(expr.value));

    while (matches(Token::Type::kComma)) {
      expr = parse_expr();

      if (!expr.matched)
        return error("missing a expression after ',' while parsing a expression list.");

      expr_list.push_back(std::move(expr.value));
    }

    return expr_list;
  }

  Result<ast::CRef<ast::LitExpr>> Parser::literal_expr()
  {
    if (auto tok = matches(Token::Type::kInt16))
      return ast::context().make<ast::LitExpr>(
        ast::LitExpr::Type::kI32,
        tok->value_as<int64_t>()
      );
    else if (auto tok = matches(Token::Type::kInt32))
      return ast::context().make<ast::LitExpr>(
        ast::LitExpr::Type::kI32,
        tok->value_as<int64_t>()
      );
    else if (auto tok = matches(Token::Type::kInt64))
      return ast::context().make<ast::LitExpr>(
        ast::LitExpr::Type::kI64,
        tok->value_as<int64_t>()
      );
    else if (auto tok = matches(Token::Type::kUint16))
      return ast::context().make<ast::LitExpr>(
        ast::LitExpr::Type::kU32,
        tok->value_as<uint64_t>()
      );
    else if (auto tok = matches(Token::Type::kUint32))
      return ast::context().make<ast::LitExpr>(
        ast::LitExpr::Type::kU32,
        tok->value_as<uint64_t>()
      );
    else if (auto tok = matches(Token::Type::kUint64))
      return ast::context().make<ast::LitExpr>(
        ast::LitExpr::Type::kU64,
        tok->value_as<uint64_t>()
      );
    else if (auto tok = matches(Token::Type::kFlt32))
      return ast::context().make<ast::LitExpr>(
        ast::LitExpr::Type::kF32,
        tok->value_as<double>()
      );
    else if (auto tok = matches(Token::Type::kFlt64))
      return ast::context().make<ast::LitExpr>(
        ast::LitExpr::Type::kF64,
        tok->value_as<double>()
      );

    return Failure::kNoMatch;
  }

  Result<ast::CRef<ast::Expr>> Parser::primary_expr()
  {
    if (auto lit = literal_expr(); lit.matched) return lit;

    if (auto idexpr = identifier_expr(); idexpr.matched) return idexpr;

    return Failure::kNoMatch;
  }

  Result<ast::CRef<ast::IdExpr>> Parser::identifier_expr()
  {
    if (auto id = matches(Token::Type::kIdent))
      return ast::context().make<ast::IdExpr>(
        std::string(id->value_as<std::string_view>())
      );

    return Failure::kNoMatch;
  }
 
  Result<ast::CRef<ast::Expr>> Parser::parse_expr()
  {
    return primary_expr(); 
  }

  Result<std::vector<ast::CRef<ast::Attr>>> Parser::parse_attributes()
  {
    // TODO (Renan): We need to implemenet a synchronization point here.
    std::vector<ast::CRef<ast::Attr>> attribute_list;

    while (matches(Token::Type::kAt)) {
      auto ident = parse_name();

      if (!ident.matched) return error("missing attribute identifier after '@'.");

      auto type = ast::Attr::Type::kCount;

      if (ident.value == "workgroup_size")
        type = ast::Attr::Type::kWorkgroupSize;
      else if (ident.value == "compute")
        type = ast::Attr::Type::kCompute;
      else if (ident.value == "vertex")
        type = ast::Attr::Type::kVertex;
      else if (ident.value == "fragment")
        type = ast::Attr::Type::kFragment;
      else if (ident.value == "group")
        type = ast::Attr::Type::kGroup;
      else if (ident.value == "binding")
        type = ast::Attr::Type::kBinding;

      Result<std::vector<ast::CRef<ast::Expr>>> expr_list;
      
      if (matches(Token::Type::kLeftParen)) {
        expr_list = parse_expression_list();

        if (expr_list.errored) return Failure::kError;

        if (!matches(Token::Type::kRightParen))
          return error("missing ')' at end of attribute parameters.");
      }

      attribute_list.push_back(
        ast::context().make<ast::Attr>(
          type,
          std::move(expr_list.value)
        )
      );
    }

    return attribute_list;
  }

  Result<ast::CRef<ast::BufferDecl>> Parser::parse_buffer_decl(
    std::vector<ast::CRef<ast::Attr>>& attributes
  )
  {
    if (matches("buffer")) {
      std::vector<ast::CRef<ast::Expr>> expr_list;

      if (matches(Token::Type::kLT)) {
        auto expr_list_result = parse_expression_list();

        if (expr_list_result.errored) return Failure::kError;

        if (!expr_list_result.matched)
          return error("missing expression list between '<..>' while declaring buffer.");

        if (!matches(Token::Type::kGT)) 
          return error("missing '>' at end of buffer argument list.");
      }

      auto name = parse_name();

      if (!name.matched) return error("missing name in buffer declaration.");

      if (!matches(Token::Type::kColon)) return error("missing ':' after buffer name.");

      auto type_ident = parse_name();

      if (!type_ident.matched) return error("missing type in buffer declaration.");

      if (!matches(Token::Type::kSemicolon)) return error("missing semicolon after buffer declaration.");

      return ast::context().make<ast::BufferDecl>(
        name.value,
        std::move(expr_list),
        ast::context().make<ast::Type>(type_ident.value)
      );
    }

    return Failure::kNoMatch;
  }

  Result<ast::CRef<ast::FuncDecl>> Parser::parse_func_decl(
    std::vector<ast::CRef<ast::Attr>>& attributes
  )
  {
    if (matches("fn")) {
      auto function_name = parse_name();

      if (!function_name.matched)
        return error("expected function name.");

      if (!matches(Token::Type::kLeftParen))
        return error("expected a '(' after function name.");

      std::vector<ast::CRef<ast::FuncArg>> function_args;

      while (should_continue() && !matches(Token::Type::kRightParen)) {
        auto ident = parse_name();

        if (!ident.matched) return error("missing argument identifier.");

        if (!matches(Token::Type::kColon))
          return error("missing ':' after function argument name.");

        auto type = expect_type();

        if (!type.matched) return Failure::kError;

        // (Renan): if we are here, then we have a valid argument.
        function_args.push_back(
          ast::context().make<ast::FuncArg>(
            ident.value,
            std::move(type.value)
          )
        );
      }

      if (!current()->is(Token::Type::kRightParen))
        return error("expected a ')' after function arguments.");

      auto block = parse_block();

      if (block.errored) return Failure::kError;

      if (!block.matched) return error("missing block in function declaration.");

      return ast::context().make<ast::FuncDecl>(
        function_name.value,
        std::move(block.value),
        std::move(function_args)
      );
    }

    return Failure::kNoMatch;
  }

  Result<std::string> Parser::parse_name()
  {
    if (auto tok = matches(Token::Type::kIdent)) 
      return std::string(tok->value_as<std::string_view>());

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
      auto composed_message = fmt::format(
        "PARSER ERROR ({}:{}): {}",
        m_lexer[offset].location().line,
        m_lexer[offset].location().column,
        message
      );

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

    return (
      m_lexer[1 + offset].type() == Token::Type::kIdent &&
      m_lexer[1 + offset].value_as<std::string_view>() == identifier
    ) ? &m_lexer[++offset] : nullptr;
  }

  bool Parser::should_continue()
  {
    return offset + 1 < m_lexer.tokenCount() && !m_lexer[offset + 1].is(Token::Type::kEOF);
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