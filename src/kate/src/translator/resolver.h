#pragma once

#include "ast.h"
#include "sem.h"

namespace kate::tlr {
  class Resolver {
  public:
    Resolver() = default;

    ~Resolver() = default;
    
    void resolve(ast::Module* module);
  private:
    void resolve(ast::UniformDecl* uniform_);

    void resolve(ast::StructDecl* struct_);

    void resolve(ast::FuncDecl* func);

    void resolve(ast::FuncArg* func_arg);

    void resolve(ast::BlockStat* block);

    void resolve(ast::IfStat* if_stat);

    void resolve(ast::ForStat* for_stat);

    void resolve(ast::VarStat* var_stat);

    void resolve(ast::ExprStat* expr_stat);

    void resolve(ast::BreakStat* break_stat);

    void resolve(ast::WhileStat* while_stat);

    void resolve(ast::ReturnStat* return_stat);

    void resolve(ast::BufferDecl* buffer);

    types::Type* resolve(ast::Type* type);

    void resolve(ast::Expr* expr);

    void resolve(ast::LitExpr* lit);

    void resolve(ast::BinaryExpr* bexpr);

    void resolve(ast::UnaryExpr* uexpr);

    sem::Decl* resolve(ast::IdExpr* idexpr);

    void resolve(ast::CallExpr* callexpr);

    void resolve(ast::StructMember* struct_member);

    types::Type* resolve(ast::ArrayType* array_type);

    types::Type* resolve(ast::TypeId* type_id);

    sem::Scope* m_currentScope;
  };
}