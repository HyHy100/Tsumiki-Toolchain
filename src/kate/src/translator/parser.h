#pragma once

#include <optional>

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
        bool should_continue();

        const Token* current();

        const Token* matches(Token::Type type);

        const Token* matches(std::string_view identifier);

        void sync_to(Token::Type tok);

        Result<ast::CRef<ast::Decl>> parse_global_declaration();

        Result<ast::CRef<ast::FuncDecl>> parse_func_decl();

        Result<ast::CRef<ast::Type>> expect_type();

        Result<ast::CRef<ast::Stat>> statement();

        Result<ast::CRef<ast::BlockStat>> parse_block();

        Result<std::vector<ast::CRef<ast::Attr>>> parse_attributes();

        Result<std::vector<ast::CRef<ast::Expr>>> parse_expression_list();

        Result<ast::CRef<ast::Expr>> parse_expr();

        Result<ast::CRef<ast::LitExpr>> literal_expr();

        Result<std::string> parse_name();

        Failure error(const std::string_view& message);

        ParserOptions m_options;

        Lexer m_lexer;

        int64_t offset;
    };
}