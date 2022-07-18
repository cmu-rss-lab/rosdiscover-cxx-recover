#pragma once

#include <string>
#include <unordered_map>

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <llvm/ADT/APInt.h>

#include "../Builder/ValueBuilder.h"
#include "../Value/String.h"
#include "../Helper/FindDefVisitor.h"

#include "../Ast/Stmt/Exprs.h"
#include "IntSymbolizer.h"
#include "BoolSymbolizer.h"
#include "FloatSymbolizer.h"

namespace rosdiscover {

class ExprSymbolizer {
public:
  ExprSymbolizer(
    clang::ASTContext &astContext
  ): 
    astContext(astContext), 
    valueBuilder(), 
    intSymbolizer(),
    boolSymbolizer(astContext),
    floatSymbolizer()
  {}

  std::unique_ptr<SymbolicExpr> symbolize(clang::Expr *expr) {
    if (expr == nullptr) {
      llvm::outs() << "ERROR! Symbolizing (expr): NULLPTR";
      return valueBuilder.unknown();
    }

    expr = expr->IgnoreParenCasts()->IgnoreImpCasts()->IgnoreCasts();

    if (auto *binaryOp = clang::dyn_cast<clang::BinaryOperator>(expr)) {
      return symbolizeBinaryOp(binaryOp);
    } 


    llvm::outs() << "symbolizing (expr): ";
    expr->dump();
    llvm::outs() << "\n";

    llvm::outs() << "unable to symbolize expression (expr): treating as unknown\n";
    return valueBuilder.unknown();
  }

  std::unique_ptr<SymbolicExpr> symbolizeBinaryOp(clang::BinaryOperator *binOp) {
    llvm::outs() << "unable to symbolize expression (expr): treating as unknown\n";
    switch (binOp->getOpcode()) {
      case clang::BinaryOperator::Opcode::BO_LAnd: 
        return std::make_unique<AndExpr>(symbolize(binOp->getLHS()), symbolize(binOp->getRHS()));
      case clang::BinaryOperator::Opcode::BO_LOr: 
        return std::make_unique<OrExpr>(symbolize(binOp->getLHS()), symbolize(binOp->getRHS()));        
      default: 
        llvm::outs() << "Unsupported boolean operator: " << binOp->getOpcode() << " in expr: " << prettyPrint(binOp, astContext);
        return valueBuilder.unknown();
    }
  }

private:
  clang::ASTContext &astContext;
  ValueBuilder valueBuilder;
  IntSymbolizer intSymbolizer;
  BoolSymbolizer boolSymbolizer;
  FloatSymbolizer floatSymbolizer;
};

} // rosdiscover
