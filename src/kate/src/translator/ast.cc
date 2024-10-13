#include "ast.h"
#include "sem.h"

#include <fmt/format.h>

namespace kate::tlr::ast {
  ASTContext& context() 
  {
    static ASTContext nctx;
    return nctx;
  }

  uint64_t ASTContext::getID()
  {
    static uint64_t id = 0;
    fmt::println("id: {}.", id);
    return id++;
  }

  void Decl::setSem(std::unique_ptr<sem::Decl>&& sem)
  {
    m_sem = std::move(sem);
  }

  sem::Decl* Decl::sem()
  {
    return m_sem.get();
  }

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

  sem::Module* Module::sem()
  {
    return m_sem.get();
  }

  void Module::setSem(std::unique_ptr<sem::Module>&& sem)
  {
    m_sem = std::move(sem);
  }

  sem::Expr* Expr::sem()
  {
    return m_sem.get();
  }

  void Expr::setSem(std::unique_ptr<sem::Expr>&& sem)
  {
    m_sem = std::move(sem);
  }

  LitExpr::LitExpr(Type type, uint64_t value)
    : m_type { type }, m_value { value }
  {
  }

  LitExpr::Type LitExpr::type() const
  {
    return m_type;
  }

  uint64_t LitExpr::value() const
  {
    return m_value;
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

  UnaryExpr::UnaryExpr(
    UnaryExpr::Type type,
    CRef<Expr> operand
  ) : m_type { type },
      m_operand { std::move(operand) }
  {
  }

  CRef<TreeNode> UnaryExpr::clone()
  {
    return context().make<UnaryExpr>(
      m_type,
      context().clone(m_operand)
    );
  }

  CRef<Expr>& UnaryExpr::operand()
  {
    return m_operand;
  }

  UnaryExpr::Type UnaryExpr::type() const
  {
    return m_type;
  }

  BinaryExpr::BinaryExpr(
    CRef<Expr>&& lhs,
    BinaryExpr::Type type,
    CRef<Expr>&& rhs
  ) : m_type { type },
      m_lhs { std::move(lhs) },
      m_rhs { std::move(rhs) }
  {
  }

  CRef<TreeNode> BinaryExpr::clone()
  {
    return context().make<BinaryExpr>(
      context().clone(m_lhs),
      m_type,
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
    CRef<Type>&& type,
    const std::string& name,
    CRef<BlockStat>&& block,
    std::vector<CRef<FuncArg>>&& args,
    std::vector<CRef<Attr>>&& attributes
  ) : m_attrs { std::move(attributes) },
      m_args { std::move(args) },
      m_block { std::move(block) },
      m_type { std::move(type) }
  {
    m_name = name;
  }

  CRef<Type>& FuncDecl::type()
  {
    return m_type;
  }

  CRef<BlockStat>& FuncDecl::block()
  {
    return m_block;
  }

  std::vector<CRef<FuncArg>>& FuncDecl::args()
  {
    return m_args;
  }

  CRef<TreeNode> FuncDecl::clone()
  {
    return context().make<FuncDecl>(
      context().clone(m_type),
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

  VarDecl::VarDecl(
    const std::string& name,
    CRef<Type>&& type
  ) : m_type { std::move(type) }
  {
    m_name = name;
  }

  CRef<Type>& VarDecl::type()
  {
    return m_type;
  }

  CRef<TreeNode> VarDecl::clone()
  {
    return context().make<VarDecl>(
      m_name,
      context().clone(m_type)
    );
  }

  VarStat::VarStat(
    CRef<VarDecl>&& vardecl,
    CRef<Expr>&& initializer
  ) : m_vardecl { std::move(vardecl) },
      m_initializer { std::move(initializer) }
  {
  }

  CRef<VarDecl>& VarStat::decl()
  {
    return m_vardecl;
  }

  CRef<Expr>& VarStat::expr()
  {
    return m_initializer;
  }

  CRef<TreeNode> VarStat::clone()
  {
    return context().make<VarStat>(
      context().clone(m_vardecl),
      (m_initializer) ? 
      context().clone(m_initializer) : CRef<Expr>()
    );
  }

  TypeId::TypeId(const std::string& id) 
    : m_id { id }
  {
  }

  CRef<TreeNode> TypeId::clone()
  {
    return context().make<TypeId>(m_id);
  }

  std::string& TypeId::id()
  {
    return m_id;
  }

  ArrayType::ArrayType(
    CRef<Type>&& type,
    CRef<Expr>&& arraySizeExpr
  ) : m_type { std::move(type) },
      m_arraySizeExpr { std::move(arraySizeExpr) }
  {
  }

  CRef<TreeNode> ArrayType::clone()
  {
    return context().make<ArrayType>(
      context().clone(m_type),
      context().clone(m_arraySizeExpr)
    );
  }

  CRef<Expr>& ArrayType::arraySizeExpr()
  {
    return m_arraySizeExpr;
  }

  CRef<Type>& ArrayType::type()
  {
    return m_type;
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

  sem::BlockStat* BlockStat::sem()
  {
    return m_sem.get();
  }

  void BlockStat::setSem(std::unique_ptr<sem::BlockStat>&& sem)
  {
    m_sem = std::move(sem);
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

  CallExpr::CallExpr(
    CRef<IdExpr>&& id,
    std::vector<CRef<Expr>>&& args
  ) : m_id { std::move(id) },
      m_args { std::move(args) }
  {
  }

  CRef<TreeNode> CallExpr::clone()
  {
    return ast::context().make<ast::CallExpr>(
      ast::context().clone(m_id),
      ast::context().clone(m_args)
    );
  }

  CRef<IdExpr>& CallExpr::id()
  {
    return m_id;
  }

  std::vector<CRef<Expr>>& CallExpr::args()
  {
    return m_args;
  }

  CallStat::CallStat(
    CRef<CallExpr>&& call_expr
  ) : m_call_expr { std::move(call_expr) }
  {
  }

  CRef<TreeNode> CallStat::clone()
  {
    return context().make<CallStat>(
      context().clone(m_call_expr)
    );
  }

  CRef<CallExpr>& CallStat::expr()
  {
    return m_call_expr;
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

  CRef<BlockStat>& WhileStat::block()
  {
    return m_block;
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
    BufferArgs args,
    CRef<Type>&& type,
    std::vector<CRef<Attr>>&& attributes
  ) : m_args { std::move(args) },
    m_type { std::move(type) },
    m_attributes { std::move(attributes) }
  {
    m_name = name;
  }

  std::vector<CRef<Attr>>& BufferDecl::attributes()
  {
    return m_attributes;
  }

  CRef<TreeNode> BufferDecl::clone()
  {
    return context().make<BufferDecl>(
      m_name,
      m_args,
      context().clone(m_type),
      context().clone(m_attributes)
    );
  }

  CRef<Type>& BufferDecl::type()
  {
    return m_type;
  }

  UniformDecl::UniformDecl(
    CRef<Type>&& type,
    const std::string& name,
    std::vector<CRef<Attr>>&& attributes
  ) : m_type { std::move(type) },
      m_name { name },
      m_attributes { std::move(attributes) }
  {
  }

  CRef<TreeNode> UniformDecl::clone()
  {
    return context().make<UniformDecl>(
      context().clone(m_type),
      m_name,
      context().clone(m_attributes)
    );
  }

  const std::string& UniformDecl::name() const
  {
    return m_name;
  }

  CRef<Type>& UniformDecl::type()
  {
    return m_type;
  }

  std::vector<CRef<Attr>>& UniformDecl::attributes()
  {
    return m_attributes;
  }

  const BufferArgs& BufferDecl::args() const
  {
    return m_args;
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
TS_RTTI_TYPE(ast::UnaryExpr)
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
TS_RTTI_TYPE(ast::CallExpr)
TS_RTTI_TYPE(ast::CallStat)
TS_RTTI_TYPE(ast::ArrayType)
TS_RTTI_TYPE(ast::TypeId)
TS_RTTI_TYPE(ast::UniformDecl)