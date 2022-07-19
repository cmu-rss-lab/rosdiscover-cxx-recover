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

class BoolSymbolizer {
public:
  BoolSymbolizer(
    const clang::ASTContext &astContext
  )
  : astContext(astContext), valueBuilder() {}

  std::unique_ptr<SymbolicBool> symbolize(const clang::Expr *expr) {
    if (expr == nullptr) {
      llvm::outs() << "ERROR! Symbolizing (bool): NULLPTR";
      return valueBuilder.unknown();
    }

    expr = expr->IgnoreParenCasts()->IgnoreImpCasts()->IgnoreCasts();

    llvm::outs() << "symbolizing (bool): ";
    expr->dump();
    llvm::outs() << "\n";

    if (expr->isKnownToHaveBooleanValue()) {
      bool result;
      expr->EvaluateAsBooleanCondition(result, astContext);
      valueBuilder.boolLiteral(result);
    }

    llvm::outs() << "unable to symbolize expression (bool): treating as unknown\n";
    return valueBuilder.unknown();
  }

private:
  const clang::ASTContext &astContext;
  ValueBuilder valueBuilder;
};

} // rosdiscover
