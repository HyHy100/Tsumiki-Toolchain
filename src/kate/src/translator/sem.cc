#include "sem.h"

namespace kate::tlr::sem {
  Decl::Decl(ast::Decl* decl)
   : m_decl { decl }
  {
  }

  std::string_view Decl::name()
  {
    return m_decl->name();
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

  Scope::Scope(Scope* parent)
    : m_parent { parent }
  {
  }

  Scope* Scope::parent()
  {
    return m_parent;
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
}