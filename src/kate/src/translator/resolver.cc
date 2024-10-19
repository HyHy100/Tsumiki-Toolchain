#include "resolver.h"

namespace kate::tlr {
  void Resolver::resolve(ast::Module* module)
  {
    module->setSem(std::make_unique<sem::Module>());
    m_currentScope = &module->sem()->scope();
    
    for (auto& decl : module->global_declarations()) {
      base::Match(
        decl.get(),
        [&](ast::StructDecl* struct_) {
          resolve(struct_);
        },
        [&](ast::BufferDecl* buffer) {
          resolve(buffer);
        },
        [&](ast::FuncDecl* func_decl) {
          resolve(func_decl);
        },
        [&](ast::VarDecl* var_decl) {
          resolve(var_decl->type().get());
        },
        [&](ast::UniformDecl* uniform_decl) {
          // ...
        },
        [&](base::Default) {
          assert(false);
        }
      );
    }
  }

  void Resolver::resolve(ast::UniformDecl* uniform)
  {
    resolve(uniform->type().get());
  }

  void Resolver::resolve(ast::StructDecl* struct_)
  {
    for (auto& m : struct_->members()) {
      for (auto& m2 : struct_->members())
        if (&m2 != &m && m2->name() == m->name()) {
          assert(false); // duplicate member name.
        }

      resolve(m->type().get());
    }
  }

  void Resolver::resolve(ast::FuncDecl* func)
  {
    for (auto& arg : func->args()) {
      resolve(arg.get());
    }

    resolve(func->block().get());
  }

  void Resolver::resolve(ast::FuncArg* func_arg)
  {
    resolve(func_arg->type().get());
  }

  void Resolver::resolve(ast::BlockStat* block)
  {
    auto current_scope = m_currentScope;

    auto sem = std::make_unique<sem::BlockStat>();
    sem->scope().setParent(current_scope);

    m_currentScope = &sem->scope();
    
    block->setSem(std::move(sem));

    for (auto& stat : block->stats()) {
      base::Match(
        stat.get(),
        [&](ast::IfStat* stat) {
          resolve(stat);
        },
        [&](ast::ForStat* for_stat) {
          resolve(for_stat);
        },
        [&](ast::BlockStat* block_stat) {
          resolve(block_stat);
        },
        [&](ast::VarStat* var_stat) {
          resolve(var_stat);
        },
        [&](ast::ExprStat* expr_stat) {
          resolve(expr_stat);
        },
        [&](ast::BreakStat* break_stat) {
          resolve(break_stat);
        },
        [&](ast::WhileStat* while_stat) {
          resolve(while_stat);
        },
        [&](ast::ReturnStat* return_stat) {
          resolve(return_stat);
        },
        [](base::Default) {
          assert(false);
        }
      );
    }

    m_currentScope = current_scope;
  }

  void Resolver::resolve(ast::IfStat* if_stat)
  {
    resolve(if_stat->condition().get());

    resolve(if_stat->block().get());

    if (if_stat->elseBlock())
      resolve(if_stat->elseBlock().get());
  }

  std::optional<ast::LitExpr::Value> Resolver::comptime_eval(ast::Expr* expr) 
  {
    if (auto bexpr = expr->as<ast::BinaryExpr>())
      return comptime_eval(bexpr);
    else if (auto litexpr = expr->as<ast::LitExpr>())
        return litexpr->value();
    else
      return std::nullopt;
  }

  template<typename T>
  std::optional<T> comptime_binary_expr_eval(ast::BinaryExpr::Type type, T lhs, T rhs) {
    switch (type) {
      case ast::BinaryExpr::Type::kAdd:
        return lhs + rhs;
      case ast::BinaryExpr::Type::kSubtract:
        return lhs - rhs;
      case ast::BinaryExpr::Type::kMul:
        return lhs * rhs;
      case ast::BinaryExpr::Type::kDiv:
        return lhs / rhs;
      case ast::BinaryExpr::Type::kBitXor:
        return lhs ^ rhs;
      case ast::BinaryExpr::Type::kBitOr:
        return lhs | rhs;
      case ast::BinaryExpr::Type::kBitAnd:
        return lhs & rhs;
      default:
        return std::nullopt;
    }
  }

  std::optional<ast::LitExpr::Value> Resolver::comptime_eval(ast::BinaryExpr* bexpr) 
  {
    auto lhs_val = comptime_eval(bexpr->lhs().get());
    
    if (!lhs_val) 
      return std::nullopt;
    
    auto rhs_val = comptime_eval(bexpr->rhs().get());

    if (!rhs_val) 
      return std::nullopt;

    ast::LitExpr::Value::Type type;

    // if it's a 'float <expr> float' binary expression
    if ((lhs_val->type & ast::LitExpr::Value::Type::kFloatMask) &&
          (rhs_val->type & ast::LitExpr::Value::Type::kFloatMask)) 
    {
      // then we check if types are equal
      if (lhs_val->type == rhs_val->type)
        type = lhs_val->type;
      else
        // if types are not equal then we assume one of them is a 64 bit floating point value. 
        type = ast::LitExpr::Value::kF64;

      auto result = comptime_binary_expr_eval(bexpr->type(), lhs_val->value.f64, lhs_val->value.f64);

      if (!result)
        return std::nullopt;

      return ast::LitExpr::Value {
        .type = type,
        .value.f64 = result.value()
      };
    }
    // if it's a 'int <expr> int' binary expression
    else if ((lhs_val->type & ast::LitExpr::Value::Type::kIntMask) &&
          (rhs_val->type & ast::LitExpr::Value::Type::kIntMask)) 
    {
      // then we check if types are equal
      if (lhs_val->type == rhs_val->type)
        type = lhs_val->type;
      else
        // if types are not equal then we assume one of them is a 64 bit signed integer value. 
        type = ast::LitExpr::Value::kI64;

      auto result = comptime_binary_expr_eval(bexpr->type(), lhs_val->value.i64, lhs_val->value.i64);

      if (!result)
        return std::nullopt;

      return ast::LitExpr::Value {
        .type = type,
        .value.i64 = result.value()
      };
    }
    // if it's a 'uint <expr> uint' binary expression
    else if ((lhs_val->type & ast::LitExpr::Value::Type::kUnsignedIntMask) &&
          (rhs_val->type & ast::LitExpr::Value::Type::kUnsignedIntMask)) 
    {
      // then we check if types are equal
      if (lhs_val->type == rhs_val->type)
        type = lhs_val->type;
      else
        // if types are not equal then we assume one of them is a 64 unsigned integer value. 
        type = ast::LitExpr::Value::kU64;

      auto result = comptime_binary_expr_eval(bexpr->type(), lhs_val->value.u64, lhs_val->value.u64);

      if (!result)
        return std::nullopt;

      return ast::LitExpr::Value {
        .type = type,
        .value.u64 = result.value()
      };
    } 

    // compile time evaluation failed.
    return std::nullopt;
  }

  void Resolver::resolve(ast::ForStat* for_stat)
  {
    if (auto& cond = for_stat->initializer())
      resolve(for_stat->initializer().get());

    if (auto& cond = for_stat->condition())
      resolve(for_stat->condition().get());

    if (auto& cond = for_stat->continuing())
      resolve(for_stat->continuing().get());

    resolve(for_stat->block().get());
  }

  void Resolver::resolve(ast::VarStat* var_stat)
  {
    if (auto& ty = var_stat->decl()->type()) {
      resolve(ty.get());
    }

    if (auto& expr = var_stat->expr()) {
      resolve(expr.get());

      // if variable declaration statement is missing a type,
      // then we try to infer it from initializer.
      if (!var_stat->decl()->type()) {
        auto ty = var_stat->expr()->sem()->type();
        
        auto sem = std::make_unique<sem::Expr>(var_stat->expr().get());
        sem->setType(ty);

        var_stat->decl()->type()->setSem(std::move(sem));
      }
    }
  }

  void Resolver::resolve(ast::ExprStat* expr_stat)
  {
    resolve(expr_stat->expr().get());
  }

  void Resolver::resolve(ast::BreakStat* break_stat)
  {
  }

  void Resolver::resolve(ast::WhileStat* while_stat)
  {
    resolve(while_stat->condition().get());

    resolve(while_stat->block().get());
  }

  void Resolver::resolve(ast::ReturnStat* return_stat)
  {
    resolve(return_stat->expr().get());
  }

  void Resolver::resolve(ast::BufferDecl* buffer_decl)
  {
    resolve(buffer_decl->type().get());
  }

  types::Type* Resolver::resolve(ast::Type* type)
  {
    return base::Match(
      type,
      [&](ast::ArrayType* type) {
        return resolve(type);
      },
      [&](ast::TypeId* type) {
        return resolve(type);
      },
      [&](base::Default) -> types::Type* {
        assert(false);
        return nullptr;
      }
    );
  }

  types::Type* Resolver::resolve(ast::ArrayType* array_type)
  {
    auto subty = resolve(array_type->type().get());
    
    std::optional<ast::LitExpr::Value> array_size;

    // If it's an array, check if we have a size.
    if (auto& array_size_expr = array_type->arraySizeExpr()) {
      array_size = comptime_eval(array_size_expr.get());

      if (!array_size) {
        // TODO: Handle error.
        assert(false);
        return nullptr;
      }

      if (array_size->value.u64 == 0) {
        // TODO: Handle error.
        assert(false);
        return nullptr;
      }
    }

    std::string type_name;

    if (array_size)
      type_name = fmt::format("{}[{}]", subty->mangledName(), array_size->value.u64);
    else
      // unsized array.
      type_name = subty->mangledName() + "[]";

    if (auto ty = types::system().findType(type_name))
      return ty;

    return types::system().addType(
      type_name,
      std::make_unique<types::Array>(subty, array_size ?  array_size->value.u64 : 0)
    );
  }

  types::Type* Resolver::resolve(ast::TypeId* type_id)
  {
    if (auto ty = types::system().findType(type_id->id()))
      return ty;

    return types::system().addType(
      type_id->id(), 
      std::make_unique<types::Scalar>(type_id->id())
    );
  }

  void Resolver::resolve(ast::Expr* expr)
  {
    base::Match(
      expr,
      [&](ast::BinaryExpr* expr) {
        resolve(expr);
      },
      [&](ast::UnaryExpr* expr) {
        resolve(expr);
      },
      [&](ast::IdExpr* expr) {
        resolve(expr);
      },
      [&](ast::CallExpr* expr) {
        resolve(expr);
      },
      [&](ast::LitExpr* expr) {
        resolve(expr);
      },
      [](base::Default) {
        assert(false);
      }
    );
  }

  void Resolver::resolve(ast::LitExpr* lit)
  {
    lit->setSem(std::make_unique<sem::Expr>(lit));

    switch (lit->value().type) {
      case ast::LitExpr::Value::Type::kF32:
        lit->sem()->setType(types::system().findType("float"));
        break;
      case ast::LitExpr::Value::Type::kF64:
        lit->sem()->setType(types::system().findType("double"));
        break;
      case ast::LitExpr::Value::Type::kI16:
        lit->sem()->setType(types::system().findType("half"));
        break;
      case ast::LitExpr::Value::Type::kU16:
        lit->sem()->setType(types::system().findType("uhalf"));
        break;
      case ast::LitExpr::Value::Type::kI32:
        lit->sem()->setType(types::system().findType("int"));
        break;
      case ast::LitExpr::Value::Type::kI64:
        lit->sem()->setType(types::system().findType("long"));
        break;
      case ast::LitExpr::Value::Type::kU32:
        lit->sem()->setType(types::system().findType("uint"));
        break;
      case ast::LitExpr::Value::Type::kU64:
        lit->sem()->setType(types::system().findType("ulong"));
        break;
      default:
        assert(false);
    }
  }

  void Resolver::resolve(ast::BinaryExpr* bexpr)
  {
    resolve(bexpr->lhs().get());
    resolve(bexpr->rhs().get());
  }

  void Resolver::resolve(ast::UnaryExpr* uexpr)
  {
    resolve(uexpr->operand().get());
  }

  sem::Decl* Resolver::resolve(ast::IdExpr* idexpr)
  {
    return m_currentScope->findDecl(idexpr->ident());
  }

  void Resolver::resolve(ast::CallExpr* callexpr)
  {
    for (auto& arg : callexpr->args())
      resolve(arg.get());
  }

  void Resolver::resolve(ast::StructMember* struct_member)
  {
    resolve(struct_member->type().get());
  }
}