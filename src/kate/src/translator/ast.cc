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

TS_RTTI_TYPE(kate::sc::ast::TreeNode)
TS_RTTI_TYPE(kate::sc::ast::Attr)
TS_RTTI_TYPE(kate::sc::ast::Decl)
TS_RTTI_TYPE(kate::sc::ast::Expr)
TS_RTTI_TYPE(kate::sc::ast::FuncArg)
TS_RTTI_TYPE(kate::sc::ast::FuncDecl)
TS_RTTI_TYPE(kate::sc::ast::StructMember)
TS_RTTI_TYPE(kate::sc::ast::StructDecl)
TS_RTTI_TYPE(kate::sc::ast::Module)
TS_RTTI_TYPE(kate::sc::ast::Type)
