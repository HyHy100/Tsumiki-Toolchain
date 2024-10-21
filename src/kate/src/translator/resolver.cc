#include "resolver.h"

namespace kate::tlr {
  Resolver::Resolver()
    : m_current_function { nullptr },
      m_currentScope { nullptr }
  {
  }

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
    std::vector<types::Custom::Member> members;

    for (auto& m : struct_->members()) {
      for (auto& m2 : struct_->members())
        if (&m2 != &m && m2->name() == m->name()) {
          assert(false); // TODO: duplicate member name.
          return;
        }

      resolve(m->type().get());

      m->setSem(
        std::make_unique<sem::Decl>(
          m.get(),
          m->type()->sem()->type()
        )
      );

      members.push_back(
        types::Custom::Member(
          m->type()->sem()->type(),
          m->name()
        )
      );
    }

    struct_->setSem(
      std::make_unique<sem::Decl>(
        struct_,
        types::system().addType(
          struct_->name(),
          std::make_unique<types::Custom>(
            struct_->name(),
            std::move(members)
          )
        )
      )
    );
  }

  void Resolver::resolve(ast::FuncDecl* func)
  {    
    m_current_function = func;

    resolve(func->type().get());

    for (auto& arg : func->args()) resolve(arg.get());

    resolve(func->block().get());

    func->setSem(std::make_unique<sem::Decl>(func, func->type()->sem()->type()));

    m_currentScope->addDecl(func->sem());

    m_current_function = nullptr;
  }

  void Resolver::resolve(ast::FuncArg* func_arg)
  {
    resolve(func_arg->type().get());

    func_arg->setSem(
      std::make_unique<sem::Decl>(
        func_arg, 
        func_arg->type()->sem()->type()
      )
    );

    m_currentScope->addDecl(func_arg->sem());
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
      default:
        if constexpr (std::is_integral_v<T>) {
          switch (type) {
            case ast::BinaryExpr::Type::kBitXor:
              return lhs ^ rhs;
            case ast::BinaryExpr::Type::kBitOr:
              return lhs | rhs;
            case ast::BinaryExpr::Type::kBitAnd:
              return lhs & rhs;
          }
        }

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
    if (auto& ty = var_stat->decl()->type())
      resolve(ty.get());

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

    resolve(var_stat->decl()->type().get());

    auto sem = std::make_unique<sem::Decl>(
      var_stat->decl().get(), 
      var_stat->decl()->type()->sem()->type()
    );

    var_stat->decl()->setSem(std::move(sem));

    m_currentScope->addDecl(var_stat->decl()->sem());
  }

  void Resolver::resolve(ast::ExprStat* expr_stat)
  {
    resolve(expr_stat->expr().get());
  }

  void Resolver::resolve(ast::WhileStat* while_stat)
  {
    resolve(while_stat->condition().get());

    resolve(while_stat->block().get());
  }

  void Resolver::resolve(ast::ReturnStat* return_stat)
  {
    resolve(return_stat->expr().get());

    if (return_stat->expr()->sem()->type()->mangledName() != m_current_function->type()->sem()->type()->mangledName()) {
      // TODO: Handle error.
      assert(false);
      return;
    }
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
      type_name = subty->mangledName() + "[]"; // unsized array.

    if (auto ty = types::system().findType(type_name))
      return ty;

    auto ty = types::system().addType(
      type_name,
      std::make_unique<types::Array>(subty, array_size ?  array_size->value.u64 : 0)
    );

    array_type->setSem(std::make_unique<sem::Expr>(array_type));

    array_type->sem()->setType(ty);

    return ty;
  }

  types::Type* Resolver::resolve(ast::TypeId* type_id)
  {
    if (auto ty = types::system().findType(type_id->id())) {
      type_id->setSem(std::make_unique<sem::Expr>(type_id));

      type_id->sem()->setType(ty);

      return ty;
    }

    // TODO: Handle error here.
    assert(false);
    return nullptr;
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

    if (bexpr->type() == ast::BinaryExpr::Type::kMemberAccess) {   
      if (auto* ident = bexpr->rhs()->as<ast::IdExpr>()) {
        auto* lhs_type = bexpr->lhs()->sem()->type();

        if (auto* user_type = lhs_type->as<types::Custom>()) {
          for (auto& member : user_type->members()) {
            if (member.name() == ident->ident()) {
              bexpr->setSem(std::make_unique<sem::Expr>(bexpr));
              bexpr->sem()->setType(member.type());

              break;
            }
          }

          if (!bexpr->sem()) {
            // TODO: Handle error.
            assert(false);
            return;
          }
        } else {
          // (Renan): Support for swizzles is currently ongoing work.
          // TODO: Handle error.
          assert(false);
          return;
        }
      } else {
        // (Renan): This code path should never be reachable,
        // because the parser must require 'rhs' to be an identifier.
        assert(false);
      }
    } else if (bexpr->type() == ast::BinaryExpr::Type::kIndexAccessor) {
      if (auto* array_type = bexpr->lhs()->sem()->type()->as<types::Array>()) {
        // first we need to resolve rhs
        resolve(bexpr->rhs().get());

        // (Renan): this check here must be removed in the long term,
        // because ideally we need to check if the given type is convertible
        // to uint instead.
        if (bexpr->rhs()->sem()->type()->mangledName() != "uint" &&
            bexpr->rhs()->sem()->type()->mangledName() != "int") {
          // TODO: Handle error.
          assert(false);
          return;
        }

        bexpr->setSem(std::make_unique<sem::Expr>(bexpr));
        bexpr->sem()->setType(array_type->type());
      } else {
        // TODO: Handle error.
        assert(false);
        return;
      }
    } else {
      resolve(bexpr->rhs().get());

      auto lhs_type = bexpr->lhs()->sem()->type();
      auto rhs_type = bexpr->rhs()->sem()->type();

      bexpr->setSem(std::make_unique<sem::Expr>(bexpr));

      if (lhs_type == rhs_type) {
        bexpr->sem()->setType(lhs_type);
        return;
      }

      // TODO: implement implicit conversions (?)
      assert(false);
    }
  }

  void Resolver::resolve(ast::UnaryExpr* uexpr)
  {
    resolve(uexpr->operand().get());

    uexpr->setSem(std::make_unique<sem::Expr>(uexpr));
    
    // propagate type of operand
    uexpr->sem()->setType(uexpr->operand()->sem()->type());
  }

  sem::Decl* Resolver::resolve(ast::IdExpr* idexpr)
  {
    auto decl = m_currentScope->findDecl(idexpr->ident());

    idexpr->setSem(std::make_unique<sem::Expr>(idexpr));
    idexpr->sem()->setType(decl->type());

    return decl;
  }

  void Resolver::resolve(ast::CallExpr* callexpr)
  {
    // TODO: Implement type conversion validation.
    auto name = callexpr->id()->ident();

    auto& call_args = callexpr->args();

    for (auto& arg : call_args)
      resolve(arg.get());

    if (auto* constructor_type = types::system().findType(name)) {
      if (auto* array_type = constructor_type->as<types::Array>()) {
        // TODO: Handle error.
        // Array constructores are not supported yet.
        assert(false);
        return;
      } else if (auto* user_type = constructor_type->as<types::Custom>()) {
        auto& members = user_type->members();
        
        if (members.size() != call_args.size()) {
          // TODO: Handle error.
          assert(false);
          return;
        }

        for (size_t i = 0; i < members.size(); i++) {
          // check if types are compatible
          if (members[i].type()->mangledName() != call_args[i]->sem()->type()->mangledName()) {
            // TODO: Handle error.
            assert(false);
            return;
          }  
        }
      } else {
        // If it's a scalar.
        if (constructor_type->numSlots() == 1) {
          if (call_args.size() > 1) {
            // TODO: Handle error.
            assert(false);
            return;
          }

          // then check if the only arguments is of the same type of the constructor identifier.
          if (call_args[0]->sem()->type() != constructor_type) {
            assert(false);
            return;
          }
        } 
        // otherwise we have a mat/vec
        else if (call_args.size() == 1) {
          // it's fine if there's a single argument being passed,
          auto* arg_type = call_args[0]->sem()->type();

          // we just need to validate if it's a scalar.
          if (!arg_type->is<types::Scalar>()) {
            // TODO: Handle error.
            assert(false);
            return;
          }
        } else {
          uint64_t num_scalars_in_arguments = 0;

          for (auto& arg : call_args) {
            auto* arg_type = arg->sem()->type();

            // Only matrix / scalar / vector types are allowed here.
            if (!(arg_type->is<types::Mat>() || arg_type->is<types::Scalar>() || arg_type->is<types::Vec>())) {
              // TODO: Handle error.
              assert(false);
              return;
            }

            num_scalars_in_arguments += arg->sem()->type()->numSlots();
          }

          if (num_scalars_in_arguments != constructor_type->numSlots()) {
            // TODO: Handle error.
            assert(false);
            return;
          }
        }
      }

      // If we are here then all previous validations succedded.
      callexpr->setSem(std::make_unique<sem::Expr>(callexpr));
      callexpr->sem()->setType(constructor_type);
    } else {
      // Otherwise we have a function here.
      auto* semDecl = m_currentScope->findDecl(name);

      if (!semDecl) {
        // TODO: Handle error.
        assert(false);
        return;
      }

      // Validate if we have a function declaration.
      if (auto* func_decl = semDecl->decl()->as<ast::FuncDecl>()) {
        if (call_args.size() != func_decl->args().size()) {
          // TODO: Handle error.
          assert(false);
          return;
        }

        for (size_t i = 0; i < call_args.size(); i++) {
          auto& arg = call_args[i];

          if (arg->sem()->type()->mangledName() != func_decl->args()[i]->type()->sem()->type()->mangledName()) {
            // TODO: Handle error.
            assert(false);
            return;
          }
        }

        // If we are here then all previous checks succeded.
        callexpr->setSem(std::make_unique<sem::Expr>(callexpr));
        callexpr->sem()->setType(func_decl->type()->sem()->type());
      } else {
        // TODO: Handle error.
        assert(false);
        return;
      }
    }
  }

  void Resolver::resolve(ast::StructMember* struct_member)
  {
    resolve(struct_member->type().get());
  }
}