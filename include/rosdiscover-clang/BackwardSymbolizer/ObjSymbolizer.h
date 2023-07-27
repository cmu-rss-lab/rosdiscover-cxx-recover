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

class ObjSymbolizer {
public:
  ObjSymbolizer(clang::ASTContext &astContext)
  : valueBuilder(), astContext(astContext) {}

  std::unique_ptr<SymbolicObject> symbolize(const clang::Expr *expr) {

    if (expr == nullptr) {
      llvm::outs() << "ERROR! Symbolizing (obj): NULLPTR";
      return valueBuilder.unknown();
    }

    expr = expr->IgnoreParenCasts();

    llvm::outs() << "symbolizing (obj): ";
    expr->dump();
    llvm::outs() << "\n";

    if (auto *literal = clang::dyn_cast<clang::IntegerLiteral>(expr)) {
      return symbolize(literal);
    }

    //Try evaluating the frequency as integer.
    clang::Expr::EvalResult resultInt;
    if (!expr->isValueDependent() && expr->EvaluateAsInt(resultInt, astContext)) {
      llvm::outs() << "DEBUG [ObjSymbolizer]: evaluated INT: (" << resultInt.Val.getInt().getSExtValue() << ")\n";
      return valueBuilder.integerLiteral(resultInt.Val.getInt().getSExtValue());
    }

    llvm::outs() << "unable to symbolize expression (obj): treating as unknown\n";
    return valueBuilder.unknown();
  }
  
  std::unique_ptr<SymbolicInteger> symbolize(const clang::APValue *literal) {
    if (literal == nullptr) {
      llvm::outs() << "unable to symbolize value (obj): treating as unknown\n";
      return valueBuilder.unknown();
    }

    return valueBuilder.integerLiteral(literal->getInt().getSExtValue());
  }
private:
  ValueBuilder valueBuilder;
  clang::ASTContext &astContext;

  std::unique_ptr<SymbolicInteger> symbolize(const clang::IntegerLiteral *literal) {
    if (literal == nullptr) {
      llvm::outs() << "unable to symbolize value (obj): treating as unknown\n";
      return valueBuilder.unknown();
    }

    return valueBuilder.integerLiteral(literal->getValue().getSExtValue());
  }


};
} // rosdiscover
