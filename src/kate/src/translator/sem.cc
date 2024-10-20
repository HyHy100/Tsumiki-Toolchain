#include "sem.h"

namespace kate::tlr::sem {
  Decl::Decl(ast::Decl* decl, types::Type* type)
   : m_decl { decl },
      m_type { type }
  {
  }

  types::Type* Decl::type()
  {
    return m_type;
  }

  std::string_view Decl::name()
  {
    return m_decl->name();
  }

  ast::Decl* Decl::decl()
  {
    return m_decl;
  }

  Expr::Expr(ast::Expr* expr)
    : m_expr { expr },
      m_type { nullptr }
  {
  }

  void Expr::setType(types::Type* type)
  {
    m_type = type;
  }

  types::Type* Expr::type()
  {
    return m_type;
  }

  Scope::Scope()
    : m_parent { nullptr }
  {
  }

  Scope::Scope(Scope* parent)
    : m_parent { parent }
  {
  }

  Scope* Scope::parent()
  {
    return m_parent;
  }

  void Scope::setParent(Scope* parent)
  {
    m_parent = parent;
  }

  void Scope::addDecl(Decl* decl)
  {
    m_decls.push_back(decl);
  }

  Decl* Scope::findDecl(
    const std::string_view& name,
    bool recursive
  )
  {
    for (auto& d : m_decls)
      if (d->name() == name)
        return d;

    if (recursive)
      if (m_parent)
        if (auto d = m_parent->findDecl(name))
          return d;

    return nullptr;
  }

  Scope& BlockStat::scope()
  {
    return m_scope;
  }

  Scope& Module::scope()
  {
    return m_scope;
  }

  Type::Type(
    types::Type* type
  )
  {
  }

  types::Type* Type::type()
  {
    return m_type;
  }
}