#pragma once

#include "../ast.h"
#include "../sem.h"
#include "../types.h"

#include <iostream>
#include <sstream>

namespace kate::tlr {
  class GLSLPrinter {
  public:
    void print(ast::Module* module);
  private:
    void print(ast::UniformDecl* uniform_);

    void print(ast::StructDecl* struct_);

    void print(ast::FuncDecl* func);

    void print(ast::FuncArg* func_arg);

    void print(ast::BlockStat* block);

    void print(ast::Stat* stat);

    void print(ast::IfStat* if_stat);

    void print(ast::ForStat* for_stat);

    void print(ast::VarStat* var_stat);

    void print(ast::ExprStat* expr_stat);

    void print(ast::BreakStat* break_stat);

    void print(ast::WhileStat* while_stat);

    void print(ast::ReturnStat* return_stat);

    void print(ast::BufferDecl* buffer);

    void print(ast::Type* type);

    void print(ast::Expr* expr);

    void print(ast::LitExpr* lit);

    void print(ast::BinaryExpr* bexpr);

    void print(ast::UnaryExpr* uexpr);

    void print(ast::IdExpr* idexpr);

    void print(ast::ArrayExpr* array_expr);

    void print(ast::CallExpr* callexpr);

    void print(ast::ArrayType* array_type);

    void print(ast::TypeId* type_id);

    void print_type_prefix(types::Type* type);

    void print_type_postfix(types::Type* type);

    std::string maybe_translate_ksl_type_to_glsl(
      const std::string& type
    );

    std::stringstream& out();

    size_t m_ident_level;
    std::stringstream m_stream;
  };
}