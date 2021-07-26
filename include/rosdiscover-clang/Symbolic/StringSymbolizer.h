#pragma once

#include <string>
#include <unordered_map>

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <llvm/ADT/APInt.h>

#include "../Builder/ValueBuilder.h"
#include "../Value/String.h"
#include "../Helper/FindDefVisitor.h"

namespace rosdiscover {
namespace symbolic {

class StringSymbolizer {
public:
  StringSymbolizer(
    clang::ASTContext &astContext,
    std::unordered_map<clang::Expr const *, SymbolicVariable *> &apiCallToVar
  )
  : astContext(astContext), apiCallToVar(apiCallToVar), valueBuilder() {}

  std::unique_ptr<SymbolicString> symbolize(clang::Expr *expr) {
    llvm::outs() << "symbolizing: ";
    expr->dumpColor();
    llvm::outs() << "\n";

    // is this expression mapped to a ROS API call?
    if (clang::CallExpr const *callExpr = clang::dyn_cast<clang::CallExpr>(expr)) {
      if (apiCallToVar.find(callExpr) != apiCallToVar.end())
        return valueBuilder.varRef(apiCallToVar[callExpr]);
    }

    if (clang::DeclRefExpr *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(expr)) {
      return symbolize(declRefExpr);
    } else if (clang::StringLiteral *literal = clang::dyn_cast<clang::StringLiteral>(expr)) {
      return symbolize(literal);
    } else if (clang::ImplicitCastExpr *implicitCastExpr = clang::dyn_cast<clang::ImplicitCastExpr>(expr)) {
      return symbolize(implicitCastExpr);
    } else if (clang::CXXConstructExpr *constructExpr = clang::dyn_cast<clang::CXXConstructExpr>(expr)) {
      return symbolize(constructExpr);
    } else if (clang::CXXBindTemporaryExpr *bindTempExpr = clang::dyn_cast<clang::CXXBindTemporaryExpr>(expr)) {
      return symbolize(bindTempExpr);
    } else if (clang::MaterializeTemporaryExpr *materializeTempExpr = clang::dyn_cast<clang::MaterializeTemporaryExpr>(expr)) {
      return symbolize(materializeTempExpr);
    }

    llvm::outs() << "unable to symbolize expression: treating as unknown\n";
    return valueBuilder.unknown();
  }

private:
  [[maybe_unused]] clang::ASTContext &astContext;
  std::unordered_map<clang::Expr const *, SymbolicVariable *> &apiCallToVar;
  ValueBuilder valueBuilder;

  std::unique_ptr<SymbolicString> symbolize(clang::StringLiteral *literal) {
    return valueBuilder.stringLiteral(literal->getString().str());
  }

  std::unique_ptr<SymbolicString> symbolize(clang::CXXConstructExpr *expr) {
    // does this call the std::string constructor?
    // FIXME this is a bit hacky and may break when other libc++ versions are used
    //
    auto constructorName = expr->getConstructor()->getParent()->getQualifiedNameAsString();
    llvm::outs() << "calling constructor: " << constructorName << "\n";
    if (constructorName == "std::__cxx11::basic_string") {
      return symbolize(expr->getArg(0));
    }

    return valueBuilder.unknown();
  }

  std::unique_ptr<SymbolicString> symbolize(clang::ImplicitCastExpr *expr) {
    // TODO check that we're dealing with strings or char[]
    return symbolize(expr->getSubExpr());
  }

  std::unique_ptr<SymbolicString> symbolize(clang::DeclRefExpr *nameExpr) {
    // TODO does this refer to a parameter?

    // find the corresponding definition
    auto *decl = nameExpr->getDecl();

    llvm::outs() << "reference to decl: ";
    decl->dumpColor();

    if (auto *varDecl = clang::dyn_cast<clang::VarDecl>(decl)) {
      llvm::outs() << "this is a vardecl\n";
      auto *def = varDecl->getDefinition(astContext);
      def->dumpColor();

      if (def->hasInit()) {
        llvm::outs() << "\nwe have an initializer:\n";
        def->getInit()->dumpColor();
      }
    }

    llvm::outs() << "\n";

    return valueBuilder.unknown();
  }

  std::unique_ptr<SymbolicString> symbolize(clang::CXXBindTemporaryExpr *expr) {
    return symbolize(expr->getSubExpr());
  }

  std::unique_ptr<SymbolicString> symbolize(clang::MaterializeTemporaryExpr *expr) {
    return symbolize(expr->getSubExpr());
  }
};

} // rosdiscover::symbolic
} // rosdiscover
