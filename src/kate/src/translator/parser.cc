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
      auto decl = parse_global_declaration();

      if (!decl.matched) return {};

      fmt::println("Adding new global declaration: {}", decl.value->name());

      global_decls.push_back(decl);
    }

    return ast::context().make<ast::Module>(std::move(global_decls));
  }

  Result<ast::CRef<ast::Decl>> Parser::parse_global_declaration()
  {
    auto attrs = parse_attributes();

    if (auto func = parse_func_decl(attrs.value); func.matched)
      return func;

    if (auto buffer = parse_buffer_decl(attrs.value); buffer.matched)
      return buffer;

    // if all global declarations failed, then
    // synchronize to the next '}' and fail.
    sync_to(Token::Type::kRBrace);

    return {};
  }

  void Parser::advance(size_t n) {
    if (offset + n < m_lexer.tokenCount())
      offset += n;
  }

  const Token* Parser::peek(size_t n) {
    return (offset + n < m_lexer.tokenCount()) ? &m_lexer[offset + n] : nullptr;
  }

  int Parser::get_precedence(const Token& tk) {
    switch (tk.type()) {
            case Token::Type::kOrEq:
            case Token::Type::kXorEq:
            case Token::Type::kAndEq:
            case Token::Type::kRSEq:
            case Token::Type::kLSEq:
            case Token::Type::kPercentEq:
            case Token::Type::kDivideEq:
            case Token::Type::kMulEq:
            case Token::Type::kMinusEq:
            case Token::Type::kPlusEq:
            case Token::Type::kEqual:
                return 0;
            case Token::Type::kOrOr:
            case Token::Type::kAndAnd:
                return 1;
            case Token::Type::kEqEq:
            case Token::Type::kNotEq:
                return 2;
            case Token::Type::kOr:
            case Token::Type::kXor:
            case Token::Type::kAnd:
                return 3;
            case Token::Type::kGT:
            case Token::Type::kGTEq:
            case Token::Type::kLT:
            case Token::Type::kLTEq:
                return 4;
            case Token::Type::kLS:
            case Token::Type::kRS:
                return 5;
            case Token::Type::kPlus:
            case Token::Type::kMinus:
                return 6;
            case Token::Type::kAsterisk:
            case Token::Type::kSlash:
            case Token::Type::kPercent:
                return 7;
            case Token::Type::kDot:
            case Token::Type::kLBracket:
                return 8;
            default:
                return -1;
        }

    assert(false);
  }

  Parser::Associativity Parser::get_associativity(const Token& tk) {
      switch (tk.type()) {
          case Token::Type::kMinus:
          case Token::Type::kAsterisk:
          case Token::Type::kSlash:
          case Token::Type::kComma:
          case Token::Type::kPercent:
          case Token::Type::kPlus:
          case Token::Type::kDot:
          case Token::Type::kLBracket:
              return Associativity::kLeft;
          default:
              return Associativity::kRight;
      }

      assert(false);
  }

  Result<ast::CRef<ast::ExprStat>> Parser::parse_expr_stat()
  {
    auto expr = parse_expr();

    if (expr.errored) return Failure::kError;

    if (!expr.matched) return Failure::kNoMatch;

    if (!matches(Token::Type::kSemicolon)) 
      return error("missing ';' after expression statement.");

    return ast::context().make<ast::ExprStat>(std::move(expr));
  }

  Result<ast::CRef<ast::WhileStat>> Parser::while_statement()
  {
    if (matches("while")) {
      auto condition = parse_expr();

      if (condition.errored)
        return Failure::kError;

      if (!condition.matched)
        return error("missing condition in while statement.");

      auto block = parse_block();

      if (block.errored)
        return Failure::kError;

      if (!block.matched)
        return error("missing block in while statement.");

      return ast::context().make<ast::WhileStat>(
        std::move(condition.value),
        std::move(block.value)
      );
    }
    
    return Failure::kNoMatch;
  }

  Result<ast::CRef<ast::ForStat>> Parser::for_statement()
  {
    if (matches("for")) {
      // TODO: Think about this better.
      // about how to handle cases like
      // for ;;; {}
      // for now, deny any attempts.
      auto initializer = statement();

      if (initializer.errored)
        return Failure::kError;

      if (!initializer.matched)
        return error("missing initializer in for statement.");

      auto condition = parse_expr();

      if (condition.errored)
        return Failure::kError;

      if (!condition.matched)
        return error("missing condition in for statement.");

      if (!matches(Token::Type::kSemicolon))
        return error("missing semicolon after for statement condition.");

      auto continuing = statement();

      if (continuing.errored)
        return Failure::kError;

      if (!continuing.matched)
        return error("missing continuing expression in for statement.");

      auto block = parse_block();

      if (block.errored) return Failure::kError;

      if (!block.matched)
        return error("missing block in for statement.");

      return ast::context().make<ast::ForStat>(
        ast::context().make<ast::ExprStat>(
          std::move(initializer.value)
        ),
        std::move(condition.value),
        ast::context().make<ast::ExprStat>(
          std::move(continuing.value)
        ),
        std::move(block)
      );      
    }

    return Failure::kNoMatch;
  }

  Result<ast::CRef<ast::CallExpr>> Parser::call_expr()
  {
    if (!peek(1)->is(Token::Type::kIdent) || !peek(2)->is(Token::Type::kLeftParen))
      return Failure::kNoMatch;

    auto identifier = matches(Token::Type::kIdent); 
    
    matches(Token::Type::kLeftParen);

    auto expr_list = parse_expression_list();

    if (expr_list.errored) return Failure::kError;

    if (!matches(Token::Type::kRightParen))
      return error("missing ')' after function call argument list.");
  
    return ast::context().make<ast::CallExpr>(
      std::string(identifier->value_as<std::string_view>()),
      std::move(expr_list)
    );
  }
 
  Result<ast::CRef<ast::CallStat>> Parser::call_statement()
  {
    auto call_expr_ = call_expr();

    if (call_expr_.errored) return Failure::kError;

    if (call_expr_.matched) return call_expr_;

    return Failure::kNoMatch;
  }

  Result<ast::CRef<ast::IfStat>> Parser::if_statement()
  {
    if (matches("if")) {
      auto condition = parse_expr();

      if (condition.errored) return Failure::kError;

      if (!condition.matched) 
        return error("missing condition expression in 'if' statement.");

      auto block = parse_block();

      if (block.errored) return Failure::kError;

      if (!block.matched) 
        return error("missing block in 'if' statement.");

      ast::CRef<ast::BlockStat> else_block = {};

      if (matches("else")) {
        auto else_block_result = parse_block();

        if (else_block_result.errored) return Failure::kError;

        if (!else_block_result.matched)
          return error("missing block in 'else' statement.");

        else_block = else_block_result;
      }

      return ast::context().make<ast::IfStat>(
        std::move(condition),
        std::move(block),
        std::move(else_block)
      );
    }

    return Failure::kNoMatch;
  }

  Result<ast::CRef<ast::Stat>> Parser::statement()
  {
    Result<ast::CRef<ast::Stat>> stat;

    // try a if statement.
    stat = if_statement();

    if (stat.errored) return Failure::kError;

    if (stat.matched) return std::move(stat);

    // try a for statement.
    stat = for_statement();

    if (stat.errored) return Failure::kError;

    if (stat.matched) return std::move(stat);

    // try a while statement.
    stat = while_statement();

    if (stat.errored) return Failure::kError;

    if (stat.matched) return std::move(stat);

    stat = call_statement();

    if (stat.errored) return Failure::kError;

    if (stat.matched) return std::move(stat);

    // try a expression statement.
    // expression should always be the last ones to be parsed here.
    stat = parse_expr_stat();

    if (stat.errored) return Failure::kError;

    if (stat.matched) return std::move(stat);

    // throw an error if all statements failed.
    return error("invalid statement.");
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

      if (!current()->is(Token::Type::kRBrace)) 
        return error("missing '}' after end of statement block.");

      return ast::context().make<ast::BlockStat>(std::move(statements));
    }

    return Failure::kNoMatch;
  }

  Result<std::vector<ast::CRef<ast::Expr>>> Parser::parse_expression_list()
  {
    auto expr = parse_expr();

    if (!expr.matched) return Failure::kNoMatch;

    std::vector<ast::CRef<ast::Expr>> expr_list;
    expr_list.push_back(expr);

    while (matches(Token::Type::kComma)) {
      expr = parse_expr();

      if (!expr.matched)
        return error("missing a expression after ',' while parsing a expression list.");

      expr_list.push_back(expr);
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

  Result<ast::CRef<ast::UnaryExpr>> Parser::unary_expr()
  {
    if (matches(Token::Type::kMinus)) {
      auto expr = primary_expr();

      if (expr.errored) return Failure::kError;

      if (!expr.matched) 
        return error("missing expression after unary '-'.");
      
      return ast::context().make<ast::UnaryExpr>(
        ast::UnaryExpr::Type::kMinus,
        std::move(expr)
      );
    } else if (matches(Token::Type::kPlus)) {
      auto expr = primary_expr();

      if (expr.errored) return Failure::kError;

      if (!expr.matched) 
        return error("missing expression after unary '+'.");
    
      return ast::context().make<ast::UnaryExpr>(
        ast::UnaryExpr::Type::kPlus,
        std::move(expr)
      );
    } else if (matches(Token::Type::kExclamation)) {
      auto expr = primary_expr();

      if (expr.errored) return Failure::kError;

      if (!expr.matched) 
        return error("missing expression after unary '!'.");
      
      return ast::context().make<ast::UnaryExpr>(
        ast::UnaryExpr::Type::kNot,
        std::move(expr)
      );
    } else if (matches(Token::Type::kTilde)) {
      auto expr = primary_expr();

      if (expr.errored) return Failure::kError;

      if (!expr.matched) 
        return error("missing expression after unary '~'.");
      
      return ast::context().make<ast::UnaryExpr>(
        ast::UnaryExpr::Type::kFlip,
        std::move(expr)
      );
    }

    return Failure::kNoMatch;
  }

  Result<ast::CRef<ast::Expr>> Parser::primary_expr()
  {
    auto unary_expr_ = unary_expr();

    if (unary_expr_.matched) return unary_expr_;

    if (unary_expr_.errored) return Failure::kError;

    auto litexpr = literal_expr();
    
    if (litexpr.matched) return litexpr;

    if (litexpr.errored) return Failure::kError;

    auto call_expr_ = call_expr();

    if (call_expr_.matched) return call_expr_;

    if (call_expr_.errored) return Failure::kError;

    auto idexpr = identifier_expr(); 
    
    if (idexpr.matched) return idexpr;

    if (idexpr.errored) return Failure::kError;

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

  bool Parser::is_operator(const Token& tok)
  {
    switch (tok.type()) {
      case Token::Type::kOrEq:
      case Token::Type::kXorEq:
      case Token::Type::kAndEq:
      case Token::Type::kRSEq:
      case Token::Type::kLSEq:
      case Token::Type::kPercentEq:
      case Token::Type::kDivideEq:
      case Token::Type::kMulEq:
      case Token::Type::kMinusEq:
      case Token::Type::kPlusEq:
      case Token::Type::kEqual:
      case Token::Type::kOrOr:
      case Token::Type::kAndAnd:
      case Token::Type::kEqEq:
      case Token::Type::kNotEq:
      case Token::Type::kGT:
      case Token::Type::kGTEq:
      case Token::Type::kLT:
      case Token::Type::kLTEq:
      case Token::Type::kIncrement:
      case Token::Type::kDecrement:
      case Token::Type::kDot:
      case Token::Type::kLBracket:
        return true;
      default:
        return is_numeric_operator(tok);
    }

    assert(false);
  }

  bool Parser::is_numeric_operator(const Token& tok)
  {
    switch (tok.type()) {
      case Token::Type::kOr:
      case Token::Type::kXor:
      case Token::Type::kAnd:
      case Token::Type::kLS:
      case Token::Type::kRS:
      case Token::Type::kPlus:
      case Token::Type::kMinus:
      case Token::Type::kAsterisk:
      case Token::Type::kSlash:
      case Token::Type::kPercent:
        return true;
      default:
        return false;
    }

    assert(false);
  }

  Result<ast::CRef<ast::Expr>> Parser::parse_expression_1(
    ast::CRef<ast::Expr>&& lhs,
    size_t min_precedence
  )
  {
    auto* lookahead = peek(1);

    if (!lookahead) return std::move(lhs);

    while (lookahead && is_operator(*lookahead) && get_precedence(*lookahead) >= min_precedence) {
      auto op = lookahead;

      bool is_index_accessor = false;
      if (op && op->type() == Token::Type::kLBracket) is_index_accessor = true;

      advance();

      auto rhs_expr = primary_expr();

      if (rhs_expr.errored || !rhs_expr.matched)
          return error("error while parsing expression.");

      auto rhs = rhs_expr.unwrap();

      if (is_index_accessor) {
          matches(Token::Type::kRBracket);
          is_index_accessor = false;
      }

      lookahead = peek(1);

      while (lookahead && (
          (is_operator(*lookahead) && get_precedence(*lookahead) > get_precedence(*op))
          || (get_associativity(*lookahead) == Associativity::kRight 
                          && get_precedence(*lookahead) == get_precedence(*op))))
      {
          if (lookahead && lookahead->type() == Token::Type::kLBracket)
              is_index_accessor = true;

          auto rhs_expr2 = parse_expression_1(
              std::move(rhs),
              get_precedence(*op) + ((get_precedence(*lookahead) > get_precedence(*op)) ? 1 : 0)
          );

          if (rhs_expr2.errored || !rhs_expr2.matched)
              return error("error while parsing expression.");

          rhs = rhs_expr2;

          if (is_index_accessor) {
              matches(Token::Type::kRBracket);
              is_index_accessor = false;
          }

          lookahead = peek(1);
      }

      auto op_type = ast::BinaryExpr::Type::kCount;

      switch (op->type()) {
        case Token::Type::kOrEq:
          op_type = ast::BinaryExpr::Type::kOrEqual;
          break;
        case Token::Type::kXorEq:
          op_type = ast::BinaryExpr::Type::kXorEqual;
          break;
        case Token::Type::kAndEq:
          op_type = ast::BinaryExpr::Type::kAndEqual;
          break;
        case Token::Type::kRSEq:
          op_type = ast::BinaryExpr::Type::kRightShiftEqual;
          break;
        case Token::Type::kLSEq:
          op_type = ast::BinaryExpr::Type::kLeftShiftEqual;
          break;
        case Token::Type::kPercentEq:
          op_type = ast::BinaryExpr::Type::kModulusEqual;
          break;
        case Token::Type::kDivideEq:
          op_type = ast::BinaryExpr::Type::kDivideEqual;
          break;
        case Token::Type::kMulEq:
          op_type = ast::BinaryExpr::Type::kMultiplyEqual;
          break;
        case Token::Type::kMinusEq:
          op_type = ast::BinaryExpr::Type::kSubtractEqual;
          break;
        case Token::Type::kPlusEq:
          op_type = ast::BinaryExpr::Type::kAddEqual;
          break;
        case Token::Type::kEqual:
          op_type = ast::BinaryExpr::Type::kEqual;
          break;
        case Token::Type::kOrOr:
          op_type = ast::BinaryExpr::Type::kOrOr;
          break;
        case Token::Type::kAndAnd:
          op_type = ast::BinaryExpr::Type::kAndAnd;
          break;
        case Token::Type::kOr:
          op_type = ast::BinaryExpr::Type::kBitOr;
          break;
        case Token::Type::kXor:
          op_type = ast::BinaryExpr::Type::kBitXor;
          break;
        case Token::Type::kAnd:
          op_type = ast::BinaryExpr::Type::kBitAnd;
          break;
        case Token::Type::kEqEq:
          op_type = ast::BinaryExpr::Type::kEqualEqual;
          break;
        case Token::Type::kNotEq:
          op_type = ast::BinaryExpr::Type::kNotEqual;
          break;
        case Token::Type::kGT:
          op_type = ast::BinaryExpr::Type::kGreaterThan;
          break;
        case Token::Type::kGTEq:
          op_type = ast::BinaryExpr::Type::kGreaterThanEqual;
          break;
        case Token::Type::kLT:
          op_type = ast::BinaryExpr::Type::kLessThan;
          break;
        case Token::Type::kLTEq:
          op_type = ast::BinaryExpr::Type::kLessThanEqual;
          break;
        case Token::Type::kLS:
          op_type = ast::BinaryExpr::Type::kLeftShift;
          break;
        case Token::Type::kRS:
          op_type = ast::BinaryExpr::Type::kRightShift;
          break;
        case Token::Type::kPlus:
          op_type = ast::BinaryExpr::Type::kAdd;
          break;
        case Token::Type::kMinus:
          op_type = ast::BinaryExpr::Type::kSubtract;
          break;
        case Token::Type::kAsterisk:
          op_type = ast::BinaryExpr::Type::kMultiply;
          break;
        case Token::Type::kSlash:
          op_type = ast::BinaryExpr::Type::kDivide;
          break;
        case Token::Type::kPercent:
          op_type = ast::BinaryExpr::Type::kModulus;
          break;
        case Token::Type::kIncrement:
          op_type = ast::BinaryExpr::Type::kIncrement;
          break;
        case Token::Type::kDecrement:
          op_type = ast::BinaryExpr::Type::kDecrement;
          break;
        case Token::Type::kDot:
          op_type = ast::BinaryExpr::Type::kMemberAccess;
          break;
        case Token::Type::kLBracket:
          op_type = ast::BinaryExpr::Type::kIndexAccessor;
          break;
        default:
          return error("invalid operator.");
      }

      lhs = ast::context().make<ast::BinaryExpr>(
          std::move(lhs),
          op_type,
          std::move(rhs)
      );
    }

    return std::move(lhs);
  }
 
  Result<ast::CRef<ast::Expr>> Parser::parse_expr()
  {
    auto expr = primary_expr();

    if (expr.matched) {
        auto next = peek(1);

        if (next && is_operator(*next)) {
            expr = parse_expression_1(std::move(expr.value), 0u);
        }
    }

    return expr; 
  }

  Result<std::vector<ast::CRef<ast::Attr>>> Parser::parse_attributes()
  {
    // TODO (Renan): We need to implemenet a synchronization point here.
    std::vector<ast::CRef<ast::Attr>> attribute_list;

    while (matches(Token::Type::kAt)) {
      auto ident = parse_name();

      if (!ident.matched) 
        return error("missing attribute identifier after '@'.");

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
      else 
        return error(fmt::format("unknown attribute '{}'.", ident.value));

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
          expr_list.unwrap()
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

      ast::BufferArgs args;

      if (matches(Token::Type::kLT)) {
        if (matches("read"))
          args.access_mode = ast::AccessMode::kRead;
        else if (matches("write"))
          args.access_mode = ast::AccessMode::kWrite;
        else if (matches("read_write"))
          args.access_mode = ast::AccessMode::kReadWrite;
        else
          return error("unknown buffer access mode.");

        if (!matches(Token::Type::kGT)) 
          return error("missing '>' at end of buffer argument list.");
      }

      auto name = parse_name();

      if (!name.matched) 
        return error("missing name in buffer declaration.");

      if (!matches(Token::Type::kColon)) 
        return error("missing ':' after buffer name.");

      auto type_ident = parse_name();

      if (!type_ident.matched) 
        return error("missing type in buffer declaration.");

      if (!matches(Token::Type::kSemicolon)) 
        return error("missing semicolon after buffer declaration.");

      return ast::context().make<ast::BufferDecl>(
        name,
        args,
        ast::context().make<ast::Type>(type_ident)
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

        if (!ident.matched) 
          return error("missing argument identifier.");

        if (!matches(Token::Type::kColon))
          return error("missing ':' after function argument name.");

        auto type = expect_type();

        if (!type.matched) return Failure::kError;

        // (Renan): if we are here, then we have a valid argument.
        function_args.push_back(
          ast::context().make<ast::FuncArg>(
            ident,
            type
          )
        );
      }

      if (!current()->is(Token::Type::kRightParen))
        return error("expected a ')' after function arguments.");

      auto block = parse_block();

      if (block.errored) 
        return Failure::kError;

      if (!block.matched) 
        return error("missing block in function declaration.");

      return ast::context().make<ast::FuncDecl>(
        function_name,
        block,
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

    if (!ident) 
      return error("expected type identifier.");

    return ast::context().make<ast::Type>(std::string(std::get<std::string_view>(ident->value())));
  }

  Failure Parser::error(
      const std::string& message
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