#include "parser.h"
#include "sem.h"

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

    while (should_continue()) {
      auto decl = parse_global_declaration();

      if (!decl.matched) return {};

      m_global_decls.push_back(decl);
    }

    return ast::context().make<ast::Module>(std::move(m_global_decls));
  }

  Result<ast::CRef<ast::Decl>> Parser::parse_global_declaration()
  {
    auto attrs = parse_attributes();

    Result<ast::CRef<ast::Decl>> decl;

    decl = parse_func_decl(attrs.value); 
    if (decl.matched)
      return decl;

    decl = parse_buffer_decl(attrs.value); 
    if (decl.matched)
      return decl;

    decl = struct_declaration(); 
    if (decl.matched)
      return decl;

    decl = parse_uniform_decl(attrs.value); 
    if (decl.matched)
      return decl;

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
            case Token::Type::kLeftBracket:
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
          case Token::Type::kPercent:
          case Token::Type::kPlus:
          case Token::Type::kDot:
          case Token::Type::kLeftBracket:
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

    auto identifier = identifier_expr(); 
    
    matches(Token::Type::kLeftParen);

    auto expr_list = parse_expression_list();

    for (auto& e : expr_list.value) 
      assert(e.get());

    if (expr_list.errored) return Failure::kError;

    if (!matches(Token::Type::kRightParen))
      return error("missing ')' after function call argument list.");
  
    return ast::context().make<ast::CallExpr>(
      std::move(identifier),
      std::move(expr_list.value)
    );
  }

  Result<ast::CRef<ast::StructDecl>> Parser::struct_declaration()
  {
    if (matches("struct")) {
      auto name = parse_name();

      if (name.errored) return Failure::kError;

      if (!name.matched) return error("missing name when declaring struct.");

      auto members = struct_members();

      if (members.errored) return Failure::kError;

      if (!members.matched) return error("missing struct body, KSL does not support forward declarations.");

      matches(Token::Type::kSemicolon);

      return ast::context().make<ast::StructDecl>(
        name.value,
        std::move(members.value)
      );
    }

    return Failure::kError;
  }

  Result<std::vector<ast::CRef<ast::StructMember>>> Parser::struct_members()
  {
    if (matches(Token::Type::kLBrace)) {
      std::vector<ast::CRef<ast::StructMember>> members;

      size_t i = 0;

      while (should_continue() && !matches(Token::Type::kRBrace)) {
        if (i++ > 0 && !matches(Token::Type::kComma))
          return error("missing ',' while declaring struct members.");

        auto attrs = parse_attributes();

        if (attrs.errored) return Failure::kError;

        auto name = parse_name();

        if (name.errored) return Failure::kError;

        if (!name.matched) 
          return error("missing name in struct member.");

        if (!matches(Token::Type::kColon)) 
          return error("missing ':' after name in struct member.");

        auto type = expect_type();

        if (type.errored) return Failure::kError;

        if (!type.matched) return error("missing type after ':' in struct member.");
        
        members.push_back(
          ast::context().make<ast::StructMember>(
            std::move(type.value),
            name.value,
            std::move(attrs.value)
          )
        );
      }

      return std::move(members);
    }

    return Failure::kNoMatch;
  }

  Result<ast::CRef<ast::VarStat>> Parser::var_statement()
  {
    if (matches("var")) {
      auto name = parse_name();

      if (name.errored) return Failure::kError;

      if (!name.matched) 
        return error("missing name identifier in variable statement.");

      ast::CRef<ast::Type> type;

      if (matches(Token::Type::kColon)) {
        auto type_result = expect_type();

        if (type_result.errored) 
          return Failure::kError;

        if (!type_result.matched) 
          return error("missing type after ':' in variable declaration statement.");

        type = std::move(type_result.value);
      }

      ast::CRef<ast::Expr> initializer;

      if (matches(Token::Type::kEqual)) {
        auto initializer_result = parse_expr();

        if (initializer_result.errored) 
          return Failure::kError;

        if (!initializer_result.matched) 
          error("missing initializer expression after '=' in variable statement.");

        initializer = std::move(initializer_result.value);
      } 

      if (!matches(Token::Type::kSemicolon))
        return error("missing ';' after variable declaration statement.");

      return ast::context().make<ast::VarStat>(
        ast::context().make<ast::VarDecl>(
          name.value,
          std::move(type)
        ),
        std::move(initializer)
      );
    }

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

    // try a return statement.
    stat = parse_return_stat();

    if (stat.errored) return Failure::kError;

    if (stat.matched) return std::move(stat);

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

    stat = var_statement();

    if (stat.errored) return Failure::kError;

    if (stat.matched) return std::move(stat);

    // try a expression statement.
    // expression should always be the last ones to be parsed here.
    stat = parse_expr_stat();

    if (stat.errored) return Failure::kError;

    if (stat.matched) return std::move(stat);

    // throw an error if all statements failed.
    return error("Invalid statement.");
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
    expr_list.push_back(std::move(expr.value));

    while (matches(Token::Type::kComma)) {
      expr = parse_expr();

      if (!expr.matched)
        return error("Missing a expression after ',' while parsing a expression list.");

      expr_list.push_back(std::move(expr.value));
    }

    for (auto& e : expr_list) {
      assert(e.get());
    }

    return expr_list;
  }

  Result<ast::CRef<ast::ArrayExpr>> Parser::array_expr()
  {
    if (matches(Token::Type::kLeftBracket)) {
      std::vector<ast::CRef<ast::Expr>> expr_list;

      for (size_t i = 0; should_continue() && !matches(Token::Type::kRightBracket); i++) {
        if (i > 0)
          if (!matches(Token::Type::kComma))
            return error("Expected a ',' between expressions when parsing an array literal.");

        auto expr = parse_expr();

        if (expr.errored) return Failure::kError;

        if (!expr.matched) return error("Expected expression in array literal.");

        expr_list.push_back(std::move(expr.value));
      }

      if (expr_list.size() == 0)
        return error("Empty array literals is not allowed.");

      return ast::context().make<ast::ArrayExpr>(
        std::move(expr_list)
      );
    }

    return Failure::kNoMatch;
  }

  Result<ast::CRef<ast::LitExpr>> Parser::literal_expr()
  {
    ast::LitExpr::Value value;

    memset(&value, 0, sizeof(value));

    if (auto tok = matches(Token::Type::kInt16)) {
        value.type = ast::LitExpr::Value::Type::kI16;
        value.value.i64 = static_cast<int16_t>(tok->value_as<int64_t>());
    } else if (auto tok = matches(Token::Type::kInt32)) {
        value.type = ast::LitExpr::Value::Type::kI32;
        value.value.i64 = static_cast<int32_t>(tok->value_as<int64_t>());
    } else if (auto tok = matches(Token::Type::kInt64)) {
        value.type = ast::LitExpr::Value::Type::kI64;
        value.value.i64 = static_cast<int64_t>(tok->value_as<int64_t>());
    } else if (auto tok = matches(Token::Type::kUint16)) {
        value.type = ast::LitExpr::Value::Type::kU16;
        value.value.u64 = static_cast<uint16_t>(tok->value_as<uint64_t>());
    } else if (auto tok = matches(Token::Type::kUint32)) {
        value.type = ast::LitExpr::Value::Type::kU32;
        value.value.u64 = static_cast<uint32_t>(tok->value_as<uint64_t>());
    } else if (auto tok = matches(Token::Type::kUint64)) {
        value.type = ast::LitExpr::Value::Type::kU64;
        value.value.u64 = static_cast<uint64_t>(tok->value_as<uint64_t>());
    } else if (auto tok = matches(Token::Type::kFlt32)) {
        value.type = ast::LitExpr::Value::Type::kF32;
        value.value.f64 = static_cast<float>(tok->value_as<double>());
    } else if (auto tok = matches(Token::Type::kFlt64)) {
        value.type = ast::LitExpr::Value::Type::kF64;
        value.value.f64 = static_cast<double>(tok->value_as<double>());
    } else return Failure::kNoMatch;

    return ast::context().make<ast::LitExpr>(value);
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
    Result<ast::CRef<ast::Expr>> expr;

    expr = unary_expr();

    if (expr.matched) return expr;

    if (expr.errored) return Failure::kError;

    expr = call_expr();

    if (expr.matched) return expr;

    if (expr.errored) return Failure::kError;

    expr = literal_expr();
    
    if (expr.matched) return expr;

    if (expr.errored) return Failure::kError;

    expr = identifier_expr(); 
    
    if (expr.matched) return expr;

    if (expr.errored) return Failure::kError;

    expr = array_expr();

    if (expr.matched) return expr;

    if (expr.errored) return Failure::kError;

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
      case Token::Type::kLeftBracket:
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
      if (op && op->type() == Token::Type::kLeftBracket) is_index_accessor = true;

      advance();

      auto rhs_expr = primary_expr();

      if (rhs_expr.errored || !rhs_expr.matched)
          return error("error while parsing expression.");

      auto rhs = rhs_expr.unwrap();

      if (is_index_accessor) {
          matches(Token::Type::kRightBracket);
          is_index_accessor = false;
      }

      lookahead = peek(1);

      while (lookahead && (
          (is_operator(*lookahead) && get_precedence(*lookahead) > get_precedence(*op))
          || (get_associativity(*lookahead) == Associativity::kRight 
                          && get_precedence(*lookahead) == get_precedence(*op))))
      {
          if (lookahead && lookahead->type() == Token::Type::kLeftBracket)
              is_index_accessor = true;

          auto rhs_expr2 = parse_expression_1(
              std::move(rhs),
              get_precedence(*op) + ((get_precedence(*lookahead) > get_precedence(*op)) ? 1 : 0)
          );

          if (rhs_expr2.errored || !rhs_expr2.matched)
              return error("error while parsing expression.");

          rhs = rhs_expr2;

          if (is_index_accessor) {
              matches(Token::Type::kRightBracket);
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
        case Token::Type::kLeftBracket:
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
      else if (ident.value == "location")
        type = ast::Attr::Type::kLocation;
      else if (ident.value == "input")
        type = ast::Attr::Type::kInput;
      else if (ident.value == "builtin")
        type = ast::Attr::Type::kBuiltin;
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
          std::move(expr_list.value)
        )
      );
    }

    return attribute_list;
  }

  Result<ast::CRef<ast::ReturnStat>> Parser::parse_return_stat()
  {
    if (matches("return")) {
      auto expr = parse_expr();

      if (expr.errored) return Failure::kError;

      if (!expr.matched) return error("missing expression in 'return' statement.");

      if (!matches(Token::Type::kSemicolon))
        return error("missing ';' after 'return' statement.");

      auto ret = ast::context().make<ast::ReturnStat>(std::move(expr.value));

      return std::move(ret);
    }

    return Failure::kNoMatch;
  }

  Result<ast::CRef<ast::UniformDecl>> Parser::parse_uniform_decl(
    std::vector<ast::CRef<ast::Attr>>& attributes
  )
  {
    if (matches("uniform")) {
      auto name = parse_name();

      if (name.errored) 
        return Failure::kError;

      if (!name.matched) 
        return error("missing name in uniform declaration.");

      if (!matches(Token::Type::kColon))
        return error("missing ':' after uniform name.");

      auto type = expect_type();

      if (type.errored) 
        return Failure::kError;

      if (!type.matched)
        return error("missing type in uniform declaration.");

      if (!matches(Token::Type::kSemicolon))
        return error("missing ';' after uniform declaration.");

      return ast::context().make<ast::UniformDecl>(
        std::move(type),
        name.value,
        std::move(attributes)
      );
    }

    return Failure::kNoMatch;
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

      auto type = expect_type();

      if (type.errored) return Failure::kError;

      if (!type.matched) 
        return error("missing type in buffer declaration.");

      if (!matches(Token::Type::kSemicolon)) 
        return error("missing semicolon after buffer declaration.");

      return ast::context().make<ast::BufferDecl>(
        name,
        args,
        std::move(type),
        std::move(attributes)
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
        auto attrs = parse_attributes();

        if (attrs.errored) return Failure::kError;

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

      ast::CRef<ast::Type> type;

      if (matches(Token::Type::kColon)) {
        auto type_result = expect_type();

        if (type_result.errored) return Failure::kError;

        if (!type_result.matched) 
          return error("missing type after ':' in function return type.");

        type = std::move(type_result.value);
      } else type = ast::context().make<ast::TypeId>("void");

      auto block = parse_block();

      if (block.errored) 
        return Failure::kError;

      if (!block.matched) 
        return error("missing block in function declaration.");

      return ast::context().make<ast::FuncDecl>(
        std::move(type),
        function_name,
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
    if (matches(Token::Type::kLeftBracket)) {
      auto e = parse_expr();

      if (e.errored) return Failure::kError;

      if (!matches(Token::Type::kRightBracket)) 
        return error("missing ']' in array size.");

      auto type = expect_type();

      if (type.errored) return Failure::kError;
      else if (!type.matched)
        return error("missing type in array.");

      return static_cast<ast::CRef<ast::Type>>(
        ast::context().make<ast::ArrayType>(
          std::move(type.value),
          std::move(e.value)
        )
      );
    }

    auto struct_members_ = struct_members();

    if (struct_members_.errored) return Failure::kError;

    if (struct_members_.matched) {
      static size_t n = 0;

      auto struct_name = fmt::format("priv_{}", ++n);

      m_global_decls.push_back(
        ast::context().make<ast::StructDecl>(
          struct_name,
          std::move(struct_members_.value)
        )
      );

      return static_cast<ast::CRef<ast::Type>>(
        ast::context().make<ast::TypeId>(struct_name)
      );
    }

    auto ident = parse_name();

    if (ident.errored) return Failure::kError;

    if (!ident.matched) return error("expected type identifier.");

    return static_cast<ast::CRef<ast::Type>>(
      ast::context().make<ast::TypeId>(ident.value)
    );
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