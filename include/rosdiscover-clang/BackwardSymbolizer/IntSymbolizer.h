#pragma once

#include <string>
#include <unordered_map>

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/APValue.h>

#include "../Builder/ValueBuilder.h"
#include "../Value/String.h"
#include "../Helper/FindDefVisitor.h"

namespace rosdiscover {

class IntSymbolizer {
public:
  IntSymbolizer(
  )
  : valueBuilder() {}

  std::unique_ptr<SymbolicInteger> symbolize(clang::Expr *expr) {
    expr = expr->IgnoreParenCasts();

    llvm::outs() << "symbolizing: ";
    expr->dump();
    llvm::outs() << "\n";

    if (clang::IntegerLiteral *literal = clang::dyn_cast<clang::IntegerLiteral>(expr)) {
      return symbolize(literal);
    } 

    llvm::outs() << "unable to symbolize expression: treating as unknown\n";
    return valueBuilder.unknown();
  }
  
  std::unique_ptr<SymbolicInteger> symbolize(clang::APValue literal) {
    return valueBuilder.integerLiteral(literal.getInt().getSExtValue());
  }
private:
  ValueBuilder valueBuilder;

  std::unique_ptr<SymbolicInteger> symbolize(clang::IntegerLiteral *literal) {
    return valueBuilder.integerLiteral(literal->getValue().getSExtValue());
  }


};
} // rosdiscover
