#pragma once

#include <string>

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <llvm/ADT/APInt.h>

#include "String.h"

namespace rosdiscover {
namespace symbolic {

class StringSymbolizer {
public:
  StringSymbolizer(clang::ASTContext &astContext) : astContext(astContext) {}

  SymbolicString* symbolize(clang::Expr *expr) {
    llvm::outs() << "symbolizing: ";
    expr->dumpColor();
    llvm::outs() << "\n";

    if (clang::DeclRefExpr *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(expr)) {
      return symbolize(declRefExpr);
    } else if (clang::StringLiteral *literal = dyn_cast<clang::StringLiteral>(expr)) {
      return symbolize(literal);
    } else if (clang::ImplicitCastExpr *implicitCastExpr = dyn_cast<clang::ImplicitCastExpr>(expr)) {
      return symbolize(implicitCastExpr);
    } else if (clang::CXXConstructExpr *constructExpr = dyn_cast<clang::CXXConstructExpr>(expr)) {
      return symbolize(constructExpr);
    }

    llvm::outs() << "unable to symbolize expression: treating as unknown\n";
    return SymbolicUnknown::create();
  }

private:
  clang::ASTContext &astContext;

  SymbolicString* symbolize(clang::StringLiteral *literal) {
    return StringLiteral::create(literal->getString().str());
  }

  SymbolicString* symbolize(clang::CXXConstructExpr *constructExpr) {
    // does this call the std::string constructor?
    // FIXME this is a bit hacky and may break when other libc++ versions are used
    if (constructExpr->getConstructor()->getParent()->getQualifiedNameAsString() != "std::__cxx11::basic_string") {
      return symbolize(constructExpr->getArg(0));
    }

    return SymbolicUnknown::create();
  }

  SymbolicString* symbolize(clang::ImplicitCastExpr *castExpr) {
    // TODO check that we're dealing with strings or char[]
    return symbolize(castExpr->getSubExpr());
  }

  SymbolicString* symbolize(clang::DeclRefExpr *nameExpr) {
    return SymbolicUnknown::create();
  }
};

} // rosdiscover::symbolic
} // rosdiscover
