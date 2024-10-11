#pragma once

#include <vector>

#include "ast.h"

#include "types.h"

namespace kate::tlr::sem {
  class Decl {
  public:
    Decl(ast::Decl* decl);

    std::string_view name();
  private:
    ast::Decl* m_decl;
  };

  class Expr {
  public:
    Expr(ast::Expr* expr);

    void setType(types::Type* type);

    types::Type* type();
  private:
    ast::Expr* m_expr;
    types::Type* m_type;
  };

  class Scope {
  public:
    Scope();

    Scope(Scope* parent);

    Scope* parent();

    void setParent(Scope* parent);

    void addDecl(Decl* decl);

    Decl* findDecl(
      const std::string_view& name,
      bool recursive = true
    );
  private:
    Scope* m_parent;

    std::vector<Decl*> m_decls;
  };

  class BlockStat {
  public:
    BlockStat() = default;

    ~BlockStat() = default;

    Scope& scope();
  private:
    Scope m_scope;
  };

  class Module {
  public:
    Module() = default;

    ~Module() = default;

    Scope& scope();
  private:
    Scope m_scope;
  };

  class Type {
  public:
    Type() = delete;

    Type(types::Type* type);

    types::Type* type();
  private:
    types::Type* m_type;
  };
}