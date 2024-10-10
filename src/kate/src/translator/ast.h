#pragma once

#include <limits>
#include <vector>
#include <unordered_map>
#include <memory>
#include <type_traits>
#include <cstdint>
#include <functional>
#include <cassert>
#include <cstdint>
#include <fmt/format.h>

#include "base/rtti.h"

namespace kate::tlr::ast {
  template<typename T>
  class CRef {
  public:
    CRef()
    {
      m_id = std::numeric_limits<decltype(m_id)>::max();
    }

    CRef(uint64_t id) 
    {
      m_id = id;
    }

    CRef(const CRef<T>&) = delete;

    CRef(CRef<T>&& rhs)
    {
      m_id = rhs.m_id;
      rhs.m_id = std::numeric_limits<decltype(m_id)>::max();
    }
    
    template<typename U>
    CRef(CRef<U>&& rhs)
    {
      m_id = rhs.m_id;
      rhs.m_id = std::numeric_limits<decltype(m_id)>::max();
    }

    void operator=(CRef<T>&& rhs)
    {
      m_id = rhs.m_id;
      rhs.m_id = std::numeric_limits<decltype(m_id)>::max();
    }

    void operator=(const CRef<T>&) = delete;

    T* operator->()
    {
      return get();
    }

    template<typename U>
    CRef<U> convertTo()
    {
      auto id = m_id;
      m_id = std::numeric_limits<decltype(m_id)>::max();
      return CRef<U>(id);
    }

    T* get();

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

    virtual CRef<TreeNode> clone() = 0;
  };

  class ASTContext {
  public:
    ASTContext() = default;

    template<typename _Ty, typename... _Types, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0>
    inline CRef<_Ty> make(_Types&&... _Args) {
      auto id = getID();

      m_ctx[id] = std::make_unique<_Ty>(std::forward<_Types>(_Args)...);

      return CRef<_Ty>(id);
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

      if (it == m_ctx.end()) {
        assert(false); 
        return nullptr;
      }

      return it->second.get();
    }

    template<typename T, typename... Args>
    std::vector<CRef<T>> clone(std::vector<CRef<T>>& nodes)
    {
      std::vector<CRef<T>> v;

      for (auto& node : nodes)
        v.push_back(clone(node));

      return v;
    }

    template<typename Type>
    CRef<Type> clone(CRef<Type>& node)
    {
      if (!node) return {};

      auto n = node->clone();
      return n.template convertTo<Type>();
    }

    template<typename Type>
    CRef<Type> clone(Type* node)
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

  ASTContext& context();

  template<typename T>
  T* CRef<T>::get()
  {
    assert(m_id != std::numeric_limits<uint64_t>::max());

    if (m_id == std::numeric_limits<uint64_t>::max()) return nullptr;

    return static_cast<T*>(context().get(m_id));
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
    Module(
      std::vector<CRef<Decl>>&& declaration_list
    );

    CRef<TreeNode> clone() override;

    std::vector<CRef<Decl>>& global_declarations();
  private:
    std::vector<CRef<Decl>> m_global_declarations;
  };

  class Expr : public base::rtti::Castable<Expr, TreeNode> {};

  class IdExpr : public base::rtti::Castable<IdExpr, Expr> {
  public:
    IdExpr(const std::string& ident);

    ast::CRef<ast::TreeNode> clone() override;

    const std::string& ident() const;
  private:
    std::string m_ident;
  };

  class LitExpr : public base::rtti::Castable<LitExpr, Expr> {
  public:
    enum class Type {
      kI32,
      kI64,
      kU32,
      kU64,
      kF32,
      kF64
    };

    template<typename T, typename V = std::decay_t<T>>
    LitExpr(Type type, V value)
    {
      switch (type) {
        case Type::kU32:
          //assert(std::is_same_v<V, uint32_t>);
          m_value = value;
          break;
        case Type::kU64:
          //assert(std::is_same_v<V, uint64_t>);
          m_value = value;
          break;
        case Type::kI32:
          //assert(std::is_same_v<V, int32_t>);
          m_value = reinterpret_cast<uint32_t>(value);
          break;
        case Type::kI64:
          //assert(std::is_same_v<V, int64_t>);
          m_value = reinterpret_cast<uint64_t>(value);
          break;
        case Type::kF32:
          //assert(std::is_same_v<V, float>);
          m_value = reinterpret_cast<uint64_t>(value);
          break;
        case Type::kF64:
          //assert(std::is_same_v<V, double>);
          m_value = reinterpret_cast<uint64_t>(value);
          break;
      }

      m_type = type;
    }

    LitExpr(Type type, uint64_t value);

    ast::CRef<ast::TreeNode> clone() override;
  private:
    Type m_type;
    uint64_t m_value;
  };

  class UnaryExpr final : public base::rtti::Castable<UnaryExpr, Expr> {
  public:
    enum class Type {
      kMinus,
      kPlus,
      kNot,
      kFlip
    };

    UnaryExpr(
      Type type,
      CRef<Expr> operand
    );

    CRef<TreeNode> clone() override;

    CRef<Expr>& operand();
  private:
    Type m_type;
    CRef<Expr> m_operand;
  };

  class BinaryExpr final : public base::rtti::Castable<BinaryExpr, Expr> {
  public:
    enum class Type {
      kAdd,
      KSub,
      kDiv,
      kMul,
      kMod,

      kMemberAccess,
      kSwizzle,

      kCompoundAdd,
      kCompoundSub,
      kCompoundDiv,
      kCompoundMul,
      kCompoundMod,

      kComma,

      kOrEqual,
      kXorEqual,
      kAndEqual,
      kRightShiftEqual,
      kLeftShiftEqual,
      kModulusEqual,
      kDivideEqual,
      kMultiplyEqual,
      kSubtractEqual,
      kAddEqual,
      kEqual,
      kOrOr,
      kAndAnd,
      kBitOr,
      kBitXor,
      kBitAnd,
      kEqualEqual,
      kNotEqual,
      kGreaterThan,
      kGreaterThanEqual,
      kLessThan,
      kLessThanEqual,
      kLeftShift,
      kRightShift,
      kSubtract,
      kMultiply,
      kDivide,
      kModulus,
      kIncrement,
      kDecrement,
      kIndexAccessor,

      kCount
    };

    BinaryExpr(
      CRef<Expr>&& lhs, 
      Type type, 
      CRef<Expr>&& rhs
    );

    CRef<TreeNode> clone() override;

    void setLhs(CRef<Expr>&& lhs);

    void setRhs(CRef<Expr>&& rhs);

    Type type() const;

    CRef<Expr>& lhs();

    CRef<Expr>& rhs();
  private:
    CRef<Expr> m_lhs;
    CRef<Expr> m_rhs;

    Type m_type;
  };

  class Attr final : public base::rtti::Castable<Attr, TreeNode> {
  public:
    enum class Type {
      kGroup,
      kBinding,
      kCompute,
      kVertex,
      kFragment,
      kWorkgroupSize,
      kLocation,
      kCount
    };

    Attr(Type type, std::vector<CRef<Expr>>&& args = {});

    CRef<TreeNode> clone() override;
  private:
    Type m_type;
    std::vector<CRef<Expr>> m_args;
  };

  class Type;

  class FuncArg final : public base::rtti::Castable<FuncArg, Decl> {
  public:
    FuncArg(
      const std::string& name,
      CRef<Type>&& type,
      std::vector<CRef<Attr>>&& attrs = {}
    );

    CRef<TreeNode> clone() override;

    CRef<Type>& type();

    std::vector<CRef<Attr>>& attrs();
  private:
    CRef<Type> m_type;
    std::vector<CRef<Attr>> m_attrs;
  };

  class BlockStat;

  class FuncDecl final : public base::rtti::Castable<FuncDecl, Decl> {
  public:
    FuncDecl(
      const std::string& name,
      CRef<BlockStat>&& block,
      std::vector<CRef<FuncArg>>&& args = {},
      std::vector<CRef<Attr>>&& attributes = {}
    );

    CRef<TreeNode> clone() override;

    std::vector<CRef<Attr>>& attrs();

    CRef<BlockStat>& block();
  private:
    std::vector<CRef<FuncArg>> m_args;

    std::vector<CRef<Attr>> m_attrs;

    CRef<BlockStat> m_block;
  };

  class VarDecl final : public base::rtti::Castable<VarDecl, Decl> {
  public:
    VarDecl(
      const std::string& name,
      CRef<Type>&& type
    );

    CRef<TreeNode> clone() override;

    CRef<Type>& type();
  private:
    CRef<Type> m_type;
  };

  class Type final : public base::rtti::Castable<Type, Expr> {
  public:
    Type(const std::string& id);

    Type(CRef<Type>&& type);

    Type(CRef<Type>&& type, CRef<Expr>&& size);

    CRef<Expr>& sizeExpr();

    bool isUnsizedArray();

    CRef<TreeNode> clone() override;
  private:
    std::string m_id;
    bool m_is_array;
    CRef<Type> m_subtype;
    CRef<Expr> m_size_expr;
  };

  class StructMember final : public base::rtti::Castable<StructMember, Decl> {
  public:
    StructMember(
      CRef<Type>&& type,
      const std::string& name,
      std::vector<CRef<Attr>>&& attrs = {}
    );

    CRef<TreeNode> clone() override;

    CRef<Type>& type();

    const std::string& name();

    std::vector<CRef<Attr>>& attrs();
  private:
    CRef<Type> m_type;
    std::vector<CRef<Attr>> m_attrs;
  };

  class Stat : public base::rtti::Castable<Stat, TreeNode> {};

  class BlockStat final : public base::rtti::Castable<BlockStat, Stat> {
  public:
    BlockStat(std::vector<CRef<Stat>>&& stats);

    CRef<TreeNode> clone() override;

    std::vector<CRef<Stat>>& stats();
  private:
    std::vector<CRef<Stat>> m_stats;
  };

  class VarStat final : public base::rtti::Castable<VarStat, Stat> {
  public:
    VarStat(
      CRef<VarDecl>&& vardecl,
      CRef<Expr>&& initializer = {}
    );

    CRef<TreeNode> clone() override;
  private:
    CRef<VarDecl> m_vardecl;
    CRef<Expr> m_initializer;
  };

  class Expr;
  class BlockStat;

  class ExprStat final : public base::rtti::Castable<ExprStat, Stat> {
  public:
    ExprStat(CRef<Expr>&& expr);

    CRef<TreeNode> clone() override;

    CRef<Expr>& expr();
  private:
    CRef<Expr> m_expr;
  };

  class ReturnStat final : public base::rtti::Castable<ReturnStat, Stat> {
  public:
    ReturnStat(CRef<Expr>&& expr);

    CRef<TreeNode> clone() override;

    CRef<Expr>& expr();
  private:
    CRef<Expr> m_expr;
  };

  class BreakStat final : public base::rtti::Castable<BreakStat, Stat> {
  public:
    BreakStat() = default;

    ~BreakStat() = default;

    CRef<TreeNode> clone() override;
  };

  class IfStat final : public base::rtti::Castable<IfStat, Stat> {
  public:
    IfStat(
      CRef<Expr>&& condition,
      CRef<BlockStat>&& block,
      CRef<BlockStat>&& elseBlock
    );

    CRef<TreeNode> clone() override;

    CRef<Expr>& condition();

    CRef<BlockStat>& block();

    CRef<BlockStat>& elseBlock();
  private:
    CRef<Expr> m_condition;
    CRef<BlockStat> m_block;
    CRef<BlockStat> m_elseBlock;
  };

  class ForStat final : public base::rtti::Castable<ForStat, Stat> {
  public:
    ForStat(
      CRef<ExprStat>&& initializer,
      CRef<Expr>&& condition,
      CRef<ExprStat>&& continuing,
      CRef<BlockStat>&& block
    );

    CRef<TreeNode> clone() override;

    CRef<ExprStat>& initializer();

    CRef<Expr>& condition();

    CRef<ExprStat>& continuing();

    CRef<BlockStat>& block();
  private:
    CRef<ExprStat> m_initializer;
    CRef<Expr> m_condition;
    CRef<ExprStat> m_continuing;
    CRef<BlockStat> m_block;
  };

  class CallExpr final : public base::rtti::Castable<CallExpr, Stat> {
  public:
    CallExpr(
      const std::string& identifier,
      std::vector<CRef<Expr>> args
    );

    CRef<TreeNode> clone() override;

    const std::string& identifier() const;

    std::vector<CRef<Expr>>& args();
  private:
    std::string m_identifier;
    std::vector<CRef<Expr>> m_args;
  };

  class CallStat final : public base::rtti::Castable<CallStat, Stat> {
  public:
    CallStat(
      CRef<CallExpr>&& call_expr
    );

    CRef<TreeNode> clone() override;

    CRef<CallExpr>& expr();
  private:
    CRef<CallExpr> m_call_expr;
  };

  class WhileStat final : public base::rtti::Castable<WhileStat, Stat> {
  public:
    WhileStat(CRef<Expr>&& condition, CRef<BlockStat>&& block);

    CRef<TreeNode> clone() override;

    CRef<Expr>& condition();
  private:
    CRef<Expr> m_condition;
    CRef<BlockStat> m_block;
  };

  class StructDecl final : public base::rtti::Castable<StructDecl, Decl> {
  public:
    StructDecl(
      const std::string& name,
      std::vector<CRef<StructMember>>&& members,
      std::vector<CRef<Attr>>&& attrs = {}
    );

    CRef<TreeNode> clone() override;

    std::vector<CRef<StructMember>>& members();

    std::vector<CRef<Attr>>& attrs();
  private:
    std::vector<CRef<StructMember>> m_members;
    std::vector<CRef<Attr>> m_attrs;
  };

  enum class AccessMode {
    kWrite,
    kRead,
    kReadWrite,
    kCount
  };

  struct BufferArgs {
    AccessMode access_mode = AccessMode::kReadWrite;
  };

  class BufferDecl final : public base::rtti::Castable<BufferDecl, Decl> {
  public:
    BufferDecl(
      const std::string& name,
      BufferArgs args,
      CRef<Type>&& type
    );

    CRef<TreeNode> clone() override;

    CRef<Type>& type();

    const BufferArgs& args() const;
  private:
    BufferArgs m_args;
    CRef<Type> m_type;
  };
}