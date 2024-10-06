#pragma once

#include <limits>
#include <vector>
#include <unordered_map>
#include <memory>
#include <type_traits>
#include <cstdint>
#include <functional>

#include "base/rtti.h"

namespace kate::sc::ast {
    template<typename T>
    class CtxRef {
    public:
        CtxRef()
        {
            m_id = std::numeric_limits<decltype(m_id)>::max();
        }

        CtxRef(uint64_t id) 
        {
            m_id = id;
        }

        CtxRef(const CtxRef<T>&) = delete;

        CtxRef(CtxRef<T>&& rhs)
        {
            m_id = rhs.m_id;
            rhs.m_id = std::numeric_limits<decltype(m_id)>::max();
        }
        
        template<typename U>
        CtxRef(CtxRef<U>&& rhs)
        {
            static_assert(std::is_convertible_v<U*, T*> || std::is_convertible_v<T*, U*>);
            m_id = rhs.m_id;
            rhs.m_id = std::numeric_limits<decltype(m_id)>::max();
        }

        void operator=(CtxRef<T>&& rhs)
        {
            m_id = rhs.m_id;
            rhs.m_id = std::numeric_limits<decltype(m_id)>::max();
        }

        void operator=(const CtxRef<T>&) = delete;

        T* operator->()
        {
            return get();
        }

        template<typename U>
        CtxRef<U> convertTo()
        {
            auto id = m_id;
            m_id = std::numeric_limits<decltype(m_id)>::max();
            return CtxRef<U>(id);
        }

        T* get();

        ~CtxRef()
        {
        }

        operator bool() const
        {
            return m_id != std::numeric_limits<decltype(m_id)>::max();
        }

        uint64_t m_id;
    };

    class TreeNode : public base::rtti::Castable<TreeNode, base::rtti::Base> {
    public:
        TreeNode() = default;

        virtual ~TreeNode() = default;

        TreeNode(const TreeNode&) = delete;

        virtual CtxRef<TreeNode> clone() = 0;
    };

    class NodeContext {
    public:
        NodeContext() = default;

        template<typename _Ty, typename... _Types, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0>
        inline CtxRef<_Ty> make(_Types&&... _Args) {
            auto id = getID();

            m_ctx[id] = std::make_unique<_Ty>(std::forward<_Types>(_Args)...);

            return CtxRef<_Ty>(id);
        }

        void remove(uint64_t id)
        {
            auto it = m_ctx.find(id);

            if (it == m_ctx.end()) return;

            m_ctx.erase(it);
        }

        bool swap(TreeNode* src, TreeNode* dst) 
        {
            uint64_t whichId { 
                std::numeric_limits<uint64_t>::max()
            };

            uint64_t withId {
                std::numeric_limits<uint64_t>::max()
            };

            for (auto& [id, node] : m_ctx) {
                if (node.get() == src) 
                    whichId = id;

                if (node.get() == dst) 
                    withId = id;

                if (whichId != std::numeric_limits<uint64_t>::max() && 
                    withId != std::numeric_limits<uint64_t>::max())
                    break;
            }

            if (whichId == std::numeric_limits<uint64_t>::max() || 
                withId == std::numeric_limits<uint64_t>::max())
                return false;

            std::swap(m_ctx[whichId], m_ctx[withId]);

            return true;
        }

        TreeNode* get(uint64_t id)
        {
            auto it = m_ctx.find(id);

            if (it == m_ctx.end()) return nullptr;

            return it->second.get();
        }

        template<typename T, typename... Args>
        std::vector<CtxRef<T>> clone(std::vector<CtxRef<T>>& nodes)
        {
            std::vector<CtxRef<T>> v;

            for (auto& node : nodes)
                v.push_back(clone(node));

            return v;
        }

        template<typename Type>
        CtxRef<Type> clone(CtxRef<Type>& node)
        {
            if (!node) return {};

            auto n = node->clone();
            return n.template convertTo<Type>();
        }

        template<typename Type>
        CtxRef<Type> clone(Type* node)
        {
            if (!node) return {};

            auto n = node->clone();
            return n.template convertTo<Type>();
        }

        template<typename T>
        void foreach(std::function<void(T&)> cb)
        {
            std::vector<TreeNode*> nodes;

            for (auto& [_, ptr] : m_ctx) nodes.push_back(ptr.get());

            for (auto& ptr : nodes) {                
                if (T* cptr = ptr->as<T>()) {
                    cb(*cptr);
                }
            }
        }

        void reset()
        {
            m_ctx = std::unordered_map<uint64_t, std::unique_ptr<TreeNode>>();
        }
    private:
        uint64_t getID()
        {
            static uint64_t id = 0;

            return id++;
        }

        std::unordered_map<uint64_t, std::unique_ptr<TreeNode>> m_ctx;
    };

    static NodeContext& GetNodeContext()
    {
        static NodeContext nctx;
        return nctx;
    }

    template<typename T>
    T* CtxRef<T>::get()
    {
        if (m_id == std::numeric_limits<uint64_t>::max()) return nullptr;

        return static_cast<T*>(GetNodeContext().get(m_id));
    }

    class Decl : public base::rtti::Castable<Decl, TreeNode> {
    public:
        virtual ~Decl() = default;

        const std::string& name() const;
    protected:
        std::string m_name;
    };

    class Module : public base::rtti::Castable<Module, TreeNode> {
    public:
        Module() = default;

        ~Module() = default;

        CtxRef<TreeNode> clone() override;

        void addGlobalDecl(CtxRef<Decl>&& decl);

        std::vector<CtxRef<Decl>>& getGlobalDecls();
    private:
        std::vector<CtxRef<Decl>> m_globalDecls;
    };

    class Expr : public base::rtti::Castable<Expr, TreeNode> {};

    class Attr final : public base::rtti::Castable<Attr, TreeNode> {
    public:
        enum class Type {
            kGroup,
            kBinding,
            kCompute,
            kVertex,
            kFragment
        };

        Attr(Type type, std::vector<CtxRef<Expr>>&& args = {});

        CtxRef<TreeNode> clone() override;
    private:
        Type m_type;
        std::vector<CtxRef<Expr>> m_args;
    };

    class FuncArg final : public base::rtti::Castable<FuncArg, Decl> {
    public:
        FuncArg(
            const std::string& name,
            std::vector<CtxRef<Attr>>&& attrs
        );

        CtxRef<TreeNode> clone() override;

        std::vector<CtxRef<Attr>>& attrs();
    private:
        std::vector<CtxRef<Attr>> m_attrs;
    };

    class FuncDecl final : public base::rtti::Castable<FuncDecl, Decl> {
    public:
        FuncDecl(
            const std::string& name,
            std::vector<CtxRef<Attr>>&& attributes = {}
        );

        CtxRef<TreeNode> clone() override;

        std::vector<CtxRef<Attr>>& attrs();
    private:
        std::vector<CtxRef<Attr>>& m_attrs;
    };

    class Type final : public base::rtti::Castable<Type, Expr> {
    public:
        Type(
            const std::string& id,
            std::vector<CtxRef<Expr>>&& generic_expression_list = {}
        );

        CtxRef<TreeNode> clone() override;

        std::vector<CtxRef<Expr>>& generic_expression_list();
    private:
        std::string m_id;
        std::vector<CtxRef<Expr>> m_generic_expression_list;
    };

    class StructMember final : public base::rtti::Castable<StructMember, Decl> {
    public:
        StructMember(
            CtxRef<Type>&& type,
            const std::string& name,
            std::vector<CtxRef<Attr>>&& attrs = {}
        );

        CtxRef<TreeNode> clone() override;

        CtxRef<Type>& type();

        const std::string& name();

        std::vector<CtxRef<Attr>>& attrs();
    private:
        CtxRef<Type> m_type;
        std::vector<CtxRef<Attr>> m_attrs;
    };

    class StructDecl final : public base::rtti::Castable<StructDecl, Decl> {
    public:
        StructDecl(
            const std::string& name,
            std::vector<CtxRef<StructMember>>&& members
        );

        CtxRef<TreeNode> clone() override;

        std::vector<CtxRef<StructMember>>& members();
    private:
        std::vector<CtxRef<StructMember>> m_members;
    };
}