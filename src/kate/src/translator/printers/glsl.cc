#include "glsl.h"
#include "base/rtti.h"

#include <cassert>

namespace kate::tlr {
  void GLSLPrinter::print(ast::Module* module)
  {
    for (auto& decl : module->global_declarations()) {
      base::Match(
        decl.get(),
        [&](ast::StructDecl* struct_) {
          print(struct_);
        },
        [&](ast::BufferDecl* buffer) {
          print(buffer);
        },
        [&](ast::FuncDecl* func_decl) {
          print(func_decl);
        },
        [&](ast::VarDecl* var_decl) {
          print(var_decl->type().get());
        },
        [&](ast::UniformDecl* uniform_decl) {
          print(uniform_decl);
        },
        [&](base::Default) {
          assert(false);
        }
      );

      out() << "\n";
    }

    std::cout << "GLSL:" << std::endl << m_stream.str() << std::endl;
  }

  std::stringstream& GLSLPrinter::out()
  {
    return m_stream;
  }

  void GLSLPrinter::print(ast::UniformDecl* uniform)
  {
    out() << "uniform ";
    print(uniform->type().get());
    out() << uniform->name();
    out() << ";\n\n";
  }

  void GLSLPrinter::print(ast::StructDecl* struct_)
  {
    out() << "struct " << struct_->name() << " {\n";
    for (auto& m : struct_->members()) {
      out() << "\t";

      print(m->type().get());

      out() << " " << m->name() << ";\n";
    }
    out() << "};\n";
  }

  void GLSLPrinter::print(ast::FuncDecl* func)
  {
    print(func->type().get());
    out() << " " << func->name() << "(";

    for (auto i = 0; i < func->args().size(); i++) {
      if (i > 0) out() << ", ";

      print(func->args()[i].get());
    }

    out() << ") ";

    print(func->block().get());
  }

  void GLSLPrinter::print(ast::FuncArg* func_arg)
  {
  
  }

  void GLSLPrinter::print(ast::BlockStat* block)
  {
    out() << "{\n";

    for (auto& stat : block->stats())
      print(stat.get());

    out() << "}\n";
  }

  void GLSLPrinter::print(ast::IfStat* if_stat)
  {
    out() << "if (";
    print(if_stat->condition().get());
    out() << ") ";
    print(if_stat->block().get());
  }

  void GLSLPrinter::print(ast::ForStat* for_stat)
  {
    out() << "for (";
    print(for_stat->initializer().get());
    out() << ";";
    print(for_stat->condition().get());
    out() << ";";
    print(for_stat->continuing().get());
    out() << ") ";
    print(for_stat->block().get());
  }

  void GLSLPrinter::print(ast::VarStat* var_stat)
  {
    auto& var_type = var_stat->decl()->type();

    if (var_type) 
      print(var_type.get());

    out() << " " << var_stat->decl()->name();

    if (var_stat->expr()) {
      out() << " = ";

      print(var_stat->expr().get());
    }

    out() << ";\n";
  }

  void GLSLPrinter::print(ast::ExprStat* expr_stat)
  {
    print(expr_stat->expr().get());
    out() << ";\n";
  }

  void GLSLPrinter::print(ast::BreakStat* break_stat)
  {
    out() << "break;\n";
  }

  void GLSLPrinter::print(ast::WhileStat* while_stat)
  {
    out() << "while (";
    print(while_stat->condition().get());
    out() << ") ";
    print(while_stat->block().get());
  }

  void GLSLPrinter::print(ast::ReturnStat* return_stat)
  {
    out() << "return ";
    print(return_stat->expr().get());
    out() << ";\n";
  }

  void GLSLPrinter::print(ast::BufferDecl* buffer)
  {
    out() << "buffer " << buffer->name() << " {\n";
    out() << "\t";
    print(buffer->type().get());
    out() << " data;\n";
    out() << "};\n";
  }

  void GLSLPrinter::print(ast::Type* type)
  {
    base::Match(
      type,
      [&](ast::ArrayType* arrayType) {
        print(arrayType);
      },
      [&](ast::TypeId* typeId) {
        print(typeId);
      },
      [&](base::Default) {
        assert(false);
      }
    );
  }

  void GLSLPrinter::print(ast::Stat* stat)
  {
    base::Match(
      stat,
      [&](ast::IfStat* stat) {
        print(stat);
      },
      [&](ast::ForStat* for_stat) {
        print(for_stat);
      },
      [&](ast::BlockStat* block_stat) {
        print(block_stat);
      },
      [&](ast::VarStat* var_stat) {
        print(var_stat);
      },
      [&](ast::ExprStat* expr_stat) {
        print(expr_stat);
      },
      [&](ast::BreakStat* break_stat) {
        print(break_stat);
      },
      [&](ast::WhileStat* while_stat) {
        print(while_stat);
      },
      [&](ast::ReturnStat* return_stat) {
        print(return_stat);
      },
      [](base::Default) {
        assert(false);
      }
    );
  }

  void GLSLPrinter::print(ast::Expr* expr)
  {
    base::Match(
      expr,
      [&](ast::BinaryExpr* expr) {
        print(expr);
      },
      [&](ast::UnaryExpr* expr) {
        print(expr);
      },
      [&](ast::IdExpr* expr) {
        print(expr);
      },
      [&](ast::CallExpr* expr) {
        print(expr);
      },
      [&](ast::LitExpr* expr) {
        print(expr);
      },
      [&](ast::TypeId* expr) {
        print(expr);
      },
      [&](ast::ArrayType* expr) {
        print(expr);
      },
      [&](base::Default) {
        std::cout << "Failed at type: " << expr->typeName() << std::endl;
        assert(false);
      }
    );
  }

  void GLSLPrinter::print(ast::LitExpr* lit)
  {
    auto v = lit->value();

    switch (v.type) {
      case ast::LitExpr::Value::Type::kI16:
        out() << v.value.i64;
        break;
      case ast::LitExpr::Value::Type::kU16:
        out() << v.value.u64;
        break;
      case ast::LitExpr::Value::Type::kI32:
        out() << v.value.i64;
        break;
      case ast::LitExpr::Value::Type::kI64:
        out() << v.value.i64;
        break;
      case ast::LitExpr::Value::Type::kF32:
        out() << v.value.f64;
        break;
      case ast::LitExpr::Value::Type::kF64:
        out() << v.value.f64;
        break;
      case ast::LitExpr::Value::Type::kU32:
        out() << v.value.u64;
        break;
      case ast::LitExpr::Value::Type::kU64:
        out() << v.value.u64;
        break;
      default:
        assert(false);
    }
  }

  void GLSLPrinter::print(ast::BinaryExpr* bexpr)
  {
    print(bexpr->lhs().get());

    switch (bexpr->type()) {
      case ast::BinaryExpr::Type::kAdd:
        out() << " + ";
        break;
      case ast::BinaryExpr::Type::KSub:
        out() << " - ";
        break;
      case ast::BinaryExpr::Type::kDiv:
        out() << " / ";
        break;
      case ast::BinaryExpr::Type::kMul:
        out() << " * ";
        break;
      case ast::BinaryExpr::Type::kMod:
        out() << " % ";
        break;
      case ast::BinaryExpr::Type::kMemberAccess:
        out() << ".";
        break;
      case ast::BinaryExpr::Type::kSwizzle:
        out() << ".";
        break;
      case ast::BinaryExpr::Type::kCompoundAdd:
        out() << " += ";
        break;
      case ast::BinaryExpr::Type::kCompoundSub:
        out() << " -= ";
        break;
      case ast::BinaryExpr::Type::kCompoundDiv:
        out() << " /= ";
        break;
      case ast::BinaryExpr::Type::kCompoundMul:
        out() << " *= ";
        break;
      case ast::BinaryExpr::Type::kCompoundMod:
        out() << " %= ";
        break;
      case ast::BinaryExpr::Type::kComma:
        out() << ", ";
        break;
      case ast::BinaryExpr::Type::kOrEqual:
        out() << " |= ";
        break;
      case ast::BinaryExpr::Type::kXorEqual:
        out() << " ^= ";
        break;
      case ast::BinaryExpr::Type::kAndEqual:
        out() << " &= ";
        break;
      case ast::BinaryExpr::Type::kRightShiftEqual:
        out() << " >>= ";
        break;
      case ast::BinaryExpr::Type::kLeftShiftEqual:
        out() << " <<= ";
        break;
      case ast::BinaryExpr::Type::kModulusEqual:
        out() << " %= ";
        break;
      case ast::BinaryExpr::Type::kDivideEqual:
        out() << " /= ";
        break;
      case ast::BinaryExpr::Type::kMultiplyEqual:
        out() << " *= ";
        break;
      case ast::BinaryExpr::Type::kSubtractEqual:
        out() << " -= ";
        break;
      case ast::BinaryExpr::Type::kAddEqual:
        out() << " += ";
        break;
      case ast::BinaryExpr::Type::kEqual:
        out() << " = ";
        break;
      case ast::BinaryExpr::Type::kOrOr:
        out() << " || ";
        break;
      case ast::BinaryExpr::Type::kAndAnd:
        out() << " && ";
        break;
      case ast::BinaryExpr::Type::kBitOr:
        out() << " | ";
        break;
      case ast::BinaryExpr::Type::kBitXor:
        out() << " ^ ";
        break;
      case ast::BinaryExpr::Type::kBitAnd:
        out() << " & ";
        break;
      case ast::BinaryExpr::Type::kEqualEqual:
        out() << " == ";
        break;
      case ast::BinaryExpr::Type::kNotEqual:
        out() << " != ";
        break;
      case ast::BinaryExpr::Type::kGreaterThan:
        out() << " > ";
        break;
      case ast::BinaryExpr::Type::kGreaterThanEqual:
        out() << " >= ";
        break;
      case ast::BinaryExpr::Type::kLessThan:
        out() << " < ";
        break;
      case ast::BinaryExpr::Type::kLessThanEqual:
        out() << " <= ";
        break;
      case ast::BinaryExpr::Type::kLeftShift:
        out() << " << ";
        break;
      case ast::BinaryExpr::Type::kRightShift:
        out() << " >> ";
        break;
      case ast::BinaryExpr::Type::kSubtract:
        out() << " - ";
        break;
      case ast::BinaryExpr::Type::kMultiply:
        out() << " * ";
        break;
      case ast::BinaryExpr::Type::kDivide:
        out() << " / ";
        break;
      case ast::BinaryExpr::Type::kModulus:
        out() << " % ";
        break;
      case ast::BinaryExpr::Type::kIncrement:
        out() << "++";
        break;
      case ast::BinaryExpr::Type::kDecrement:
        out() << "--";
        break;
      case ast::BinaryExpr::Type::kIndexAccessor:
        out() << "[";
        break;
      default:
        assert(false);
    }

    print(bexpr->rhs().get());

    if (bexpr->type() == ast::BinaryExpr::Type::kIndexAccessor)
      out() << "]";
  }

  void GLSLPrinter::print(ast::UnaryExpr* uexpr)
  {
    switch (uexpr->type()) {
      case ast::UnaryExpr::Type::kFlip:
        out() << "~";
        break;
      case ast::UnaryExpr::Type::kMinus:
        out() << "-";
        break;
      case ast::UnaryExpr::Type::kNot:
        out() << "!";
        break;
      case ast::UnaryExpr::Type::kPlus:
        out() << "+";
        break;
    }

    print(uexpr->operand().get());
  }

  void GLSLPrinter::print(ast::IdExpr* idexpr)
  {
    out() << idexpr->ident();
  }

  void GLSLPrinter::print(ast::CallExpr* callexpr)
  {
    out() << callexpr->id()->ident() << "(";

    auto& args = callexpr->args();
    
    for (auto i = 0; i < args.size(); i++) {
      if (i > 0) out() << ", ";

      auto& arg = args[i];

      print(arg.get());
    }

    out() << ")";
  }

  void GLSLPrinter::print(ast::ArrayType* array_type)
  {
    print(array_type->type().get());
    out() << "[";
    if (auto& size_expr = array_type->arraySizeExpr(); size_expr)
      print(size_expr.get());
    out() << "]";
  }

  void GLSLPrinter::print(ast::TypeId* type_id)
  {
    out() << type_id->id();
  }
}