#include "ast.h"

namespace kate::tlr::ast {
  const std::string& Decl::name() const
  {
    return m_name;
  }

  Module::Module(std::vector<CRef<Decl>>&& declaration_list)
    : m_global_declarations { std::move(declaration_list) }
  {
  }

  CRef<TreeNode> Module::clone()
  {
    return context().make<Module>(
      context().clone(m_global_declarations)
    );
  }

  std::vector<CRef<Decl>>& Module::global_declarations()
  {
    return m_global_declarations;
  }

  LitExpr::LitExpr(Type type, uint64_t value)
    : m_type { type }, m_value { value }
  {
  }

  ast::CRef<ast::TreeNode> LitExpr::clone()
  {
    return ast::context().make<ast::LitExpr>(m_type, m_value);
  }

  IdExpr::IdExpr(const std::string& ident) 
    : m_ident { ident }
  {
  }

  ast::CRef<ast::TreeNode> IdExpr::clone()
  {
    return ast::context().make<IdExpr>(
      m_ident
    );
  }

  const std::string& IdExpr::ident() const
  {
    return m_ident;
  }

  BinaryExpr::BinaryExpr(
    BinaryExpr::Type type,
    CRef<Expr>&& lhs,
    CRef<Expr>&& rhs
  ) : m_type { type },
      m_lhs { std::move(lhs) },
      m_rhs { std::move(rhs) }
  {
  }

  CRef<TreeNode> BinaryExpr::clone()
  {
    return context().make<BinaryExpr>(
      m_type,
      context().clone(m_lhs),
      context().clone(m_rhs)
    );
  }

  void BinaryExpr::setLhs(CRef<Expr>&& lhs)
  {
    m_lhs = std::move(lhs);
  }

  void BinaryExpr::setRhs(CRef<Expr>&& rhs)
  {
    m_rhs = std::move(rhs);
  }

  BinaryExpr::Type BinaryExpr::type() const
  {
    return m_type;
  }

  CRef<Expr>& BinaryExpr::lhs()
  {
    return m_lhs;
  }

  CRef<Expr>& BinaryExpr::rhs()
  {
    return m_rhs;
  }

  ReturnStat::ReturnStat(CRef<Expr>&& expr)
      : m_expr { std::move(expr) }
  {
  }

  CRef<TreeNode> ReturnStat::clone()
  {
    return context().make<ReturnStat>(
        context().clone(m_expr)
    );
  }

  CRef<Expr>& ReturnStat::expr()
  {
    return m_expr;
  }

  Attr::Attr(Attr::Type type, std::vector<CRef<Expr>>&& args)
      : m_type { type },
          m_args { std::move(args) }
  {
  }

  CRef<TreeNode> Attr::clone()
  {
    return context().make<Attr>(
      m_type,
      context().clone(m_args)
    );
  }

  FuncArg::FuncArg(
    const std::string& name,
    CRef<Type>&& type,
    std::vector<CRef<Attr>>&& attrs
  ) : m_attrs { std::move(attrs) },
      m_type { std::move(type) }
  {
    m_name = name;
  }

  CRef<Type>& FuncArg::type()
  {
    return m_type;
  }

  CRef<TreeNode> FuncArg::clone()
  {
    return context().make<FuncArg>(
      m_name,
      context().clone(m_type),
      context().clone(m_attrs)
    );
  }

  std::vector<CRef<Attr>>& FuncArg::attrs()
  {
    return m_attrs;
  }

  FuncDecl::FuncDecl(
    const std::string& name,
    CRef<BlockStat>&& block,
    std::vector<CRef<FuncArg>>&& args,
    std::vector<CRef<Attr>>&& attributes
  ) : m_attrs { std::move(attributes) },
    m_args { std::move(args) }
  {
    m_name = name;
  }

  CRef<BlockStat>& FuncDecl::block()
  {
    return m_block;
  }

  CRef<TreeNode> FuncDecl::clone()
  {
    return context().make<FuncDecl>(
      m_name,
      context().clone(m_block),
      context().clone(m_args),
      context().clone(m_attrs)
    );
  }

  std::vector<CRef<Attr>>& FuncDecl::attrs()
  {
    return m_attrs;
  }

  VarDecl::VarDecl(const std::string& name)
  {
    m_name = name;
  }

  CRef<TreeNode> VarDecl::clone()
  {
    return context().make<VarDecl>(
      m_name
    );
  }

  VarStat::VarStat(
    CRef<VarDecl>&& vardecl,
    CRef<Expr>&& initializer
  ) : m_vardecl { std::move(vardecl) },
      m_initializer { std::move(initializer) }
  {
  }

  CRef<TreeNode> VarStat::clone()
  {
    return context().make<VarStat>(
      context().clone(m_vardecl),
      (m_initializer) ? 
      context().clone(m_initializer) : CRef<Expr>()
    );
  }

  Type::Type(
    const std::string& id,
    std::vector<CRef<Expr>>&& generic_expression_list
  ) : m_id { id },
      m_generic_expression_list { std::move(generic_expression_list) }
  {
  }

  CRef<TreeNode> Type::clone()
  {
    return context().make<Type>(
        m_id,
        context().clone(m_generic_expression_list)
    );
  }

  std::vector<CRef<Expr>>& Type::generic_expression_list()
  {
    return m_generic_expression_list;
  }

  StructMember::StructMember(
    CRef<Type>&& type,
    const std::string& name,
    std::vector<CRef<Attr>>&& attrs
  ) : m_type { std::move(type) },
      m_attrs { std::move(attrs) }
  {
    m_name = name;
  }

  CRef<TreeNode> StructMember::clone()
  {
    return context().make<StructMember>(
      context().clone(m_type),
      m_name,
      context().clone(m_attrs)
    );
  }

  CRef<Type>& StructMember::type()
  {
    return m_type;
  }

  const std::string& StructMember::name()
  {
    return m_name;
  }

  std::vector<CRef<Attr>>& StructMember::attrs()
  {
    return m_attrs;
  }

  BlockStat::BlockStat(
      std::vector<CRef<Stat>>&& stats
  ) : m_stats { std::move(stats) }
  {
  }

  CRef<TreeNode> BlockStat::clone()
  {
    return context().make<BlockStat>(
      context().clone(m_stats)
    );
  }

  std::vector<CRef<Stat>>& BlockStat::stats()
  {
    return m_stats;
  }

  ExprStat::ExprStat(CRef<Expr>&& expr) 
    : m_expr { std::move(expr) }
  {
  }

  CRef<TreeNode> ExprStat::clone()
  {
    return context().make<ExprStat>(
      context().clone(m_expr)
    );
  }

  CRef<Expr>& ExprStat::expr()
  {
      return m_expr;
  }

  IfStat::IfStat(
    CRef<Expr>&& condition,
    CRef<BlockStat>&& block,
    CRef<BlockStat>&& elseBlock
  ) : m_condition { std::move(condition) },
      m_block { std::move(block) },
      m_elseBlock { std::move(elseBlock) }
  {
  }

  CRef<TreeNode> IfStat::clone()
  {
    return context().make<IfStat>(
      context().clone(m_condition),
      context().clone(m_block),
      m_elseBlock ? context().clone(m_elseBlock) : CRef<BlockStat>()
    );
  }

  CRef<Expr>& IfStat::condition()
  {
    return m_condition;
  }

  CRef<BlockStat>& IfStat::block()
  {
    return m_block;
  }

  CRef<BlockStat>& IfStat::elseBlock()
  {
    return m_elseBlock;
  }

  ForStat::ForStat(
    CRef<ExprStat>&& initializer,
    CRef<Expr>&& condition,
    CRef<ExprStat>&& continuing,
    CRef<BlockStat>&& block
  ) : m_initializer { std::move(initializer) },
      m_condition { std::move(condition) },
      m_continuing { std::move(continuing) },
      m_block { std::move(block) }
  {
  }

  CRef<TreeNode> ForStat::clone()
  {
    return context().make<ForStat>(
      context().clone(m_initializer),
      context().clone(m_condition),
      context().clone(m_continuing),
      context().clone(m_block)
    );
  }

  CRef<ExprStat>& ForStat::initializer()
  {
    return m_initializer;
  }

  CRef<Expr>& ForStat::condition()
  {
    return m_condition;
  }

  CRef<ExprStat>& ForStat::continuing()
  {
    return m_continuing;
  }

  CRef<BlockStat>& ForStat::block()
  {
    return m_block;
  }

  WhileStat::WhileStat(CRef<Expr>&& condition, CRef<BlockStat>&& block)
    : m_condition { std::move(condition) },
      m_block { std::move(block) }
  {
  }

  CRef<TreeNode> WhileStat::clone()
  {
    return context().make<WhileStat>(
      context().clone(m_condition),
      context().clone(m_block)
    );
  }

  CRef<Expr>& WhileStat::condition()
  {
    return m_condition;
  }

  CRef<TreeNode> BreakStat::clone()
  {
    return context().make<BreakStat>();
  }

  StructDecl::StructDecl(
    const std::string& name,
    std::vector<CRef<StructMember>>&& members,
    std::vector<CRef<Attr>>&& attrs
  ) : m_members { std::move(members) },
      m_attrs { std::move(attrs) }
  {
    m_name = name;
  }

  CRef<TreeNode> StructDecl::clone()
  {
    return context().make<StructDecl>(
      m_name,
      context().clone(m_members),
      context().clone(m_attrs)
    );
  }

  std::vector<CRef<StructMember>>& StructDecl::members()
  {
    return m_members;
  }

  std::vector<CRef<Attr>>& StructDecl::attrs()
  {
    return m_attrs;
  }

  BufferDecl::BufferDecl(
    const std::string& name,
    std::vector<CRef<Expr>>&& args,
    CRef<Type>&& type
  ) : m_args { std::move(args) },
    m_type { std::move(type) }
  {
    m_name = name;
  }

  CRef<TreeNode> BufferDecl::clone()
  {
    return context().make<BufferDecl>(
      m_name,
      context().clone(m_args),
      context().clone(m_type)
    );
  }

  CRef<Type>& BufferDecl::type()
  {
    return m_type;
  }
}

using namespace kate::tlr;

TS_RTTI_TYPE(ast::TreeNode)
TS_RTTI_TYPE(ast::Attr)
TS_RTTI_TYPE(ast::Decl)
TS_RTTI_TYPE(ast::Expr)
TS_RTTI_TYPE(ast::LitExpr)
TS_RTTI_TYPE(ast::IdExpr)
TS_RTTI_TYPE(ast::BinaryExpr)
TS_RTTI_TYPE(ast::FuncArg)
TS_RTTI_TYPE(ast::FuncDecl)
TS_RTTI_TYPE(ast::StructMember)
TS_RTTI_TYPE(ast::StructDecl)
TS_RTTI_TYPE(ast::Stat)
TS_RTTI_TYPE(ast::IfStat)
TS_RTTI_TYPE(ast::ForStat)
TS_RTTI_TYPE(ast::WhileStat)
TS_RTTI_TYPE(ast::BlockStat)
TS_RTTI_TYPE(ast::ExprStat)
TS_RTTI_TYPE(ast::ReturnStat)
TS_RTTI_TYPE(ast::BreakStat)
TS_RTTI_TYPE(ast::VarDecl)
TS_RTTI_TYPE(ast::VarStat)
TS_RTTI_TYPE(ast::Module)
TS_RTTI_TYPE(ast::Type)
TS_RTTI_TYPE(ast::BufferDecl)