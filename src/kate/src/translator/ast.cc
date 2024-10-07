#include "ast.h"

namespace kate::sc::ast {
    const std::string& Decl::name() const
    {
        return m_name;
    }

    void Module::addGlobalDecl(CtxRef<Decl>&& decl)
    {
        m_globalDecls.push_back(std::move(decl));
    }

    std::vector<CtxRef<Decl>>& Module::getGlobalDecls()
    {
        return m_globalDecls;
    }

    BinaryExpr::BinaryExpr(
        BinaryExpr::Type type,
        CtxRef<Expr>&& lhs,
        CtxRef<Expr>&& rhs
    ) : m_type { type },
        m_lhs { std::move(lhs) },
        m_rhs { std::move(rhs) }
    {
    }

    CtxRef<TreeNode> BinaryExpr::clone()
    {
        return GetNodeContext().make<BinaryExpr>(
            m_type,
            GetNodeContext().clone(m_lhs),
            GetNodeContext().clone(m_rhs)
        );
    }

    void BinaryExpr::setLhs(CtxRef<Expr>&& lhs)
    {
        m_lhs = std::move(lhs);
    }

    void BinaryExpr::setRhs(CtxRef<Expr>&& rhs)
    {
        m_rhs = std::move(rhs);
    }

    BinaryExpr::Type BinaryExpr::type() const
    {
        return m_type;
    }

    CtxRef<Expr>& BinaryExpr::lhs()
    {
        return m_lhs;
    }

    CtxRef<Expr>& BinaryExpr::rhs()
    {
        return m_rhs;
    }

    ReturnStat::ReturnStat(CtxRef<Expr>&& expr)
        : m_expr { std::move(expr) }
    {
    }

    CtxRef<TreeNode> ReturnStat::clone()
    {
        return GetNodeContext().make<ReturnStat>(
            GetNodeContext().clone(m_expr)
        );
    }

    CtxRef<Expr>& ReturnStat::expr()
    {
        return m_expr;
    }

    Attr::Attr(Attr::Type type, std::vector<CtxRef<Expr>>&& args)
        : m_type { type },
            m_args { std::move(args) }
    {
    }

    CtxRef<TreeNode> Attr::clone()
    {
        return GetNodeContext().make<Attr>(
            m_type,
            GetNodeContext().clone(m_args)
        );
    }

    FuncArg::FuncArg(
        const std::string& name,
        std::vector<CtxRef<Attr>>&& attrs
    ) : m_attrs { std::move(attrs) }
    {
        m_name = name;
    }

    CtxRef<TreeNode> FuncArg::clone()
    {
        return GetNodeContext().make<FuncArg>(
            m_name,
            GetNodeContext().clone(m_attrs)
        );
    }

    std::vector<CtxRef<Attr>>& FuncArg::attrs()
    {
        return m_attrs;
    }

    FuncDecl::FuncDecl(
        const std::string& name,
        std::vector<CtxRef<Attr>>&& attrs
    ) : m_attrs { attrs }
    {
        m_name = name;
    }

    CtxRef<TreeNode> FuncDecl::clone()
    {
        return GetNodeContext().make<FuncDecl>(
            m_name,
            GetNodeContext().clone(m_attrs)
        );
    }

    std::vector<CtxRef<Attr>>& FuncDecl::attrs()
    {
        return m_attrs;
    }

    VarDecl::VarDecl(const std::string& name)
    {
        m_name = name;
    }

    CtxRef<TreeNode> VarDecl::clone()
    {
        return GetNodeContext().make<VarDecl>(
            m_name
        );
    }

    VarStat::VarStat(
        CtxRef<VarDecl>&& vardecl,
        CtxRef<Expr>&& initializer
    ) : m_vardecl { std::move(vardecl) },
        m_initializer { std::move(initializer) }
    {
    }

    CtxRef<TreeNode> VarStat::clone()
    {
        return GetNodeContext().make<VarStat>(
            GetNodeContext().clone(m_vardecl),
            (m_initializer) ? 
            GetNodeContext().clone(m_initializer) : CtxRef<Expr>()
        );
    }

    Type::Type(
        const std::string& id,
        std::vector<CtxRef<Expr>>&& generic_expression_list
    ) : m_id { id },
        m_generic_expression_list { std::move(generic_expression_list) }
    {
    }

    CtxRef<TreeNode> Type::clone()
    {
        return GetNodeContext().make<Type>(
            m_id,
            GetNodeContext().clone(m_generic_expression_list)
        );
    }

    std::vector<CtxRef<Expr>>& Type::generic_expression_list()
    {
        return m_generic_expression_list;
    }

    StructMember::StructMember(
        CtxRef<Type>&& type,
        const std::string& name,
        std::vector<CtxRef<Attr>>&& attrs
    ) : m_type { std::move(type) },
        m_attrs { std::move(attrs) }
    {
        m_name = name;
    }

    CtxRef<TreeNode> StructMember::clone()
    {
        return GetNodeContext().make<StructMember>(
            GetNodeContext().clone(m_type),
            m_name,
            GetNodeContext().clone(m_attrs)
        );
    }

    CtxRef<Type>& StructMember::type()
    {
        return m_type;
    }

    const std::string& StructMember::name()
    {
        return m_name;
    }

    std::vector<CtxRef<Attr>>& StructMember::attrs()
    {
        return m_attrs;
    }

    BlockStat::BlockStat(
        std::vector<CtxRef<Stat>>&& stats
    ) : m_stats { std::move(stats) }
    {
    }

    CtxRef<TreeNode> BlockStat::clone()
    {
        return GetNodeContext().make<BlockStat>(
            GetNodeContext().clone(m_stats)
        );
    }

    std::vector<CtxRef<Stat>>& BlockStat::stats()
    {
        return m_stats;
    }

    ExprStat::ExprStat(CtxRef<Expr>&& expr) 
        : m_expr { std::move(expr) }
    {
    }

    CtxRef<TreeNode> ExprStat::clone()
    {
        return GetNodeContext().make<ExprStat>(
            GetNodeContext().clone(m_expr)
        );
    }

    CtxRef<Expr>& ExprStat::expr()
    {
        return m_expr;
    }

    IfStat::IfStat(
        CtxRef<Expr>&& condition,
        CtxRef<BlockStat>&& block,
        CtxRef<BlockStat>&& elseBlock
    ) : m_condition { std::move(condition) },
        m_block { std::move(block) },
        m_elseBlock { std::move(elseBlock) }
    {
    }

    CtxRef<TreeNode> IfStat::clone()
    {
        return GetNodeContext().make<IfStat>(
            GetNodeContext().clone(m_condition),
            GetNodeContext().clone(m_block),
            m_elseBlock ? GetNodeContext().clone(m_elseBlock) : CtxRef<BlockStat>()
        );
    }

    CtxRef<Expr>& IfStat::condition()
    {
        return m_condition;
    }

    CtxRef<BlockStat>& IfStat::block()
    {
        return m_block;
    }

    CtxRef<BlockStat>& IfStat::elseBlock()
    {
        return m_elseBlock;
    }

    ForStat::ForStat(
        CtxRef<ExprStat>&& initializer,
        CtxRef<Expr>&& condition,
        CtxRef<ExprStat>&& continuing,
        CtxRef<BlockStat>&& block
    ) : m_initializer { std::move(initializer) },
        m_condition { std::move(condition) },
        m_continuing { std::move(continuing) },
        m_block { std::move(block) }
    {
    }

    CtxRef<TreeNode> ForStat::clone()
    {
        return GetNodeContext().make<ForStat>(
            GetNodeContext().clone(m_initializer),
            GetNodeContext().clone(m_condition),
            GetNodeContext().clone(m_continuing),
            GetNodeContext().clone(m_block)
        );
    }

    CtxRef<ExprStat>& ForStat::initializer()
    {
        return m_initializer;
    }

    CtxRef<Expr>& ForStat::condition()
    {
        return m_condition;
    }

    CtxRef<ExprStat>& ForStat::continuing()
    {
        return m_continuing;
    }

    CtxRef<BlockStat>& ForStat::block()
    {
        return m_block;
    }

    WhileStat::WhileStat(CtxRef<Expr>&& condition, CtxRef<BlockStat>&& block)
        : m_condition { std::move(condition) },
            m_block { std::move(block) }
    {
    }

    CtxRef<TreeNode> WhileStat::clone()
    {
        return GetNodeContext().make<WhileStat>(
            GetNodeContext().clone(m_condition),
            GetNodeContext().clone(m_block)
        );
    }

    CtxRef<Expr>& WhileStat::condition()
    {
        return m_condition;
    }

    CtxRef<TreeNode> BreakStat::clone()
    {
        return GetNodeContext().make<BreakStat>();
    }

    StructDecl::StructDecl(
        const std::string& name,
        std::vector<CtxRef<StructMember>>&& members,
        std::vector<CtxRef<Attr>>&& attrs
    ) : m_members { std::move(members) },
        m_attrs { std::move(attrs) }
    {
        m_name = name;
    }

    CtxRef<TreeNode> StructDecl::clone()
    {
        return GetNodeContext().make<StructDecl>(
            m_name,
            GetNodeContext().clone(m_members),
            GetNodeContext().clone(m_attrs)
        );
    }

    std::vector<CtxRef<StructMember>>& StructDecl::members()
    {
        return m_members;
    }

    std::vector<CtxRef<Attr>>& StructDecl::attrs()
    {
        return m_attrs;
    }
}

using namespace kate::sc;

TS_RTTI_TYPE(ast::TreeNode)
TS_RTTI_TYPE(ast::Attr)
TS_RTTI_TYPE(ast::Decl)

TS_RTTI_TYPE(ast::Expr)
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
