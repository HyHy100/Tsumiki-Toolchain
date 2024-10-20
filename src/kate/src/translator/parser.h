#pragma once

#include <optional>
#include <functional>

#include "lexer.h"
#include "ast.h"

namespace kate::tlr {
    enum class Failure {
        kNoMatch,
        kError
    };

    template<typename T>
    struct Result {
        Result(Failure failure);

        template<typename U>
        Result(Result<U>&& rhs) 
        {
          value = std::move(rhs.value);
          matched = rhs.matched;
          errored = rhs.errored;
        }

        Result();
        
        Result(T&& ptr);

        Result(bool _erroed, bool _matched);

        Result(bool _erroed, bool _matched, T value);

        T&& unwrap();

        T& operator->();

        bool ok();

        operator T&&()
        {
          return std::move(value);
        }

        bool errored;
        bool matched;
        T value;
    };

    struct ParserOptions {
        std::function<void(const std::string_view& error)> error_callback; 
    };

    class Parser {
    public:
        Parser(const ParserOptions& options);

        ast::CRef<ast::Module> parse(const std::string_view& source);
    private:
        enum class Associativity {
          kLeft, kRight
        };

        void advance(size_t n = 1);

        bool is_operator(const Token& tok);

        bool is_numeric_operator(const Token& tok);

        const Token* peek(size_t n);

        int get_precedence(const Token& expr);

        Associativity get_associativity(const Token& tk);

        bool should_continue();

        const Token* current();

        const Token* matches(Token::Type type);

        const Token* matches(std::string_view identifier);

        void sync_to(Token::Type tok);

        Result<ast::CRef<ast::Decl>> parse_global_declaration();

        Result<ast::CRef<ast::FuncDecl>> parse_func_decl(
          std::vector<ast::CRef<ast::Attr>>& attributes
        );

        Result<ast::CRef<ast::BufferDecl>> parse_buffer_decl(
          std::vector<ast::CRef<ast::Attr>>& attributes
        );

        Result<ast::CRef<ast::UniformDecl>> parse_uniform_decl(
          std::vector<ast::CRef<ast::Attr>>& attributes
        );

        Result<ast::CRef<ast::ReturnStat>> parse_return_stat();

        Result<ast::CRef<ast::Type>> expect_type();

        Result<ast::CRef<ast::Stat>> statement();

        Result<ast::CRef<ast::CallExpr>> call_expr();

        Result<ast::CRef<ast::IfStat>> if_statement();

        Result<ast::CRef<ast::ForStat>> for_statement();

        Result<ast::CRef<ast::VarStat>> var_statement();

        Result<ast::CRef<ast::WhileStat>> while_statement();

        Result<ast::CRef<ast::StructDecl>> struct_declaration();

        Result<std::vector<ast::CRef<ast::StructMember>>> struct_members();

        Result<ast::CRef<ast::BlockStat>> parse_block();

        Result<std::vector<ast::CRef<ast::Attr>>> parse_attributes();

        Result<std::vector<ast::CRef<ast::Expr>>> parse_expression_list();

        Result<ast::CRef<ast::Expr>> parse_expression_1(
          ast::CRef<ast::Expr>&& lhs,
          size_t min_precedence
        );

        Result<ast::CRef<ast::ExprStat>> parse_expr_stat();

        Result<ast::CRef<ast::Expr>> parse_expr();

        Result<ast::CRef<ast::Expr>> primary_expr();

        Result<ast::CRef<ast::UnaryExpr>> unary_expr();

        Result<ast::CRef<ast::IdExpr>> identifier_expr();

        Result<ast::CRef<ast::LitExpr>> literal_expr();

        Result<ast::CRef<ast::ArrayExpr>> array_expr();

        Result<std::string> parse_name();
        
        Failure error(const std::string& message);

        ParserOptions m_options;

      std::vector<ast::CRef<ast::Decl>> m_global_decls;

        Lexer m_lexer;

        int64_t offset;
    };
}