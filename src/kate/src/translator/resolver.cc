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
        if (&m2 != &m && m2->name() == m->name())
          assert(false); // duplicate member name.

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
        [&](ast::CallStat* call_stat) {
          resolve(call_stat);
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

  void Resolver::resolve(ast::ForStat* for_stat)
  {
    resolve(for_stat->initializer().get());

    resolve(for_stat->condition().get());

    resolve(for_stat->continuing().get());

    resolve(for_stat->block().get());
  }

  void Resolver::resolve(ast::VarStat* var_stat)
  {
    if (var_stat->expr())
      resolve(var_stat->expr().get());
  }

  void Resolver::resolve(ast::CallStat* call_stat)
  {
    resolve(call_stat->expr()->id().get());
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
    return resolve(array_type->type().get());
  }

  types::Type* Resolver::resolve(ast::TypeId* type_id)
  {
    if (auto ty = types::system().findType(type_id->id()))
      return ty;

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

    switch (lit->type()) {
      case ast::LitExpr::Type::kF32:
        lit->sem()->setType(types::system().findType("f32"));
        break;
      case ast::LitExpr::Type::kF64:
        lit->sem()->setType(types::system().findType("f64"));
        break;
      case ast::LitExpr::Type::kI32:
        lit->sem()->setType(types::system().findType("i32"));
        break;
      case ast::LitExpr::Type::kI64:
        lit->sem()->setType(types::system().findType("i64"));
        break;
      case ast::LitExpr::Type::kU32:
        lit->sem()->setType(types::system().findType("u32"));
        break;
      case ast::LitExpr::Type::kU64:
        lit->sem()->setType(types::system().findType("u64"));
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