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

  SymbolicString* symbolize(clang::Expr *nameExpr) {
    return StringLiteral::create("PLACEHOLDER");
  }

  SymbolicString* symbolize(clang::StringLiteral *literal) {
    return StringLiteral::create(literal->getString().str());
  }

private:
  clang::ASTContext &astContext;
};

} // rosdiscover::symbolic
} // rosdiscover
