#pragma once

#include <string>
#include <unordered_map>

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/APValue.h>

#include "../Builder/ValueBuilder.h"
#include "../Value/Float.h"
#include "../Helper/FindDefVisitor.h"

namespace rosdiscover {

class FloatSymbolizer {
public:
  FloatSymbolizer(clang::ASTContext &astContext)
  : valueBuilder(), astContext(astContext) {}

  std::unique_ptr<SymbolicFloat> symbolize(const clang::Expr *expr) {

    if (expr == nullptr) {
      llvm::outs() << "ERROR! Symbolizing (float): NULLPTR";
      return valueBuilder.unknown();
    }

    expr = expr->IgnoreParenCasts();

    llvm::outs() << "symbolizing (float): ";
    expr->dump();
    llvm::outs() << "\n";

    if (auto *literal = clang::dyn_cast<clang::FloatingLiteral>(expr)) {
      return symbolize(literal);
    } 

    //Try evaluating the frequency as float.
    llvm::APFloat resultFloat(0.0);
    if (expr->EvaluateAsFloat(resultFloat, astContext)) {
      llvm::outs() << "DEBUG [FloatSymbolizer]: evaluated Float: (" << resultFloat.convertToDouble() << ")\n";
      return valueBuilder.floatingLiteral(resultFloat.convertToDouble());
    }

    llvm::outs() << "unable to symbolize expression (float): " << prettyPrint(expr, astContext) << ". treating as unknown\n";
    return valueBuilder.unknown();
  }
  
  std::unique_ptr<SymbolicFloat> symbolize(const clang::APValue *literal) {
    if (literal == nullptr) {
      llvm::outs() << "unable to symbolize value: treating as unknown\n";
      return valueBuilder.unknown();
    }


    if (literal->isFloat()) {
      return valueBuilder.floatingLiteral(literal->getFloat().convertToDouble());
    } else if (literal->isInt()) {
      return valueBuilder.floatingLiteral(literal->getInt().getSExtValue());
    } else {
      llvm::outs() << "unable to symbolize value: treating as unknown\n";
      return valueBuilder.unknown();
    }
  }
private:
  ValueBuilder valueBuilder;
  clang::ASTContext &astContext;

  std::unique_ptr<SymbolicFloat> symbolize(const clang::FloatingLiteral *literal) {
    return valueBuilder.floatingLiteral(literal->getValue().convertToDouble());
  }

  std::unique_ptr<SymbolicFloat> symbolize(const clang::IntegerLiteral *literal) {
    return valueBuilder.floatingLiteral(literal->getValue().getSExtValue());
  }


};
} // rosdiscover
