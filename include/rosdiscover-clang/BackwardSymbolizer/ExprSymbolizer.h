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

  std::unique_ptr<SymbolicExpr> symbolize(const clang::Expr *expr) {
    if (expr == nullptr) {
      llvm::outs() << "ERROR! Symbolizing (expr): NULLPTR";
      return valueBuilder.unknown();
    }

    expr = expr->IgnoreParenCasts()->IgnoreImpCasts()->IgnoreCasts();

    if (auto *binOpExpr = clang::dyn_cast<clang::BinaryOperator>(expr)) {
      return symbolizeBinaryOp(binOpExpr);
    } else if (auto *unaryOpExpr = clang::dyn_cast<clang::UnaryOperator>(expr)) {
      return symbolizeUnaryOp(unaryOpExpr);
    } 

    llvm::outs() << "symbolizing (expr): ";
    expr->dump();
    llvm::outs() << "\n";

    llvm::outs() << "unable to symbolize expression (expr): treating as unknown\n";
    return valueBuilder.unknown();
  }

  std::unique_ptr<SymbolicExpr> symbolizeBinaryOp(const clang::BinaryOperator *binOpExpr) {
    switch (binOpExpr->getOpcode()) {
      case clang::BinaryOperator::Opcode::BO_LAnd: 
        return std::make_unique<AndExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()));
      case clang::BinaryOperator::Opcode::BO_LOr: 
        return std::make_unique<OrExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()));
      case clang::BinaryOperator::Opcode::BO_EQ: 
        return std::make_unique<CompareExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()), CompareOperator::EQ);
      case clang::BinaryOperator::Opcode::BO_NE: 
        return std::make_unique<CompareExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()), CompareOperator::NE);
      case clang::BinaryOperator::Opcode::BO_LT: 
        return std::make_unique<CompareExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()), CompareOperator::LT);
      case clang::BinaryOperator::Opcode::BO_LE: 
        return std::make_unique<CompareExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()), CompareOperator::LE);
      case clang::BinaryOperator::Opcode::BO_GT:
        return std::make_unique<CompareExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()), CompareOperator::GT);
      case clang::BinaryOperator::Opcode::BO_GE: 
        return std::make_unique<CompareExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()), CompareOperator::GE);
      default: 
        llvm::outs() << "Unsupported binar operator: " << binOpExpr->getOpcode() << " in expr: " << prettyPrint(binOpExpr, astContext);
        return valueBuilder.unknown();
    }
  }

  std::unique_ptr<SymbolicExpr> symbolizeUnaryOp(const clang::UnaryOperator *unaryOpExr) {
    switch (unaryOpExr->getOpcode()) {
      case clang::UnaryOperator::Opcode::UO_LNot:
        return std::make_unique<NegateExpr>(symbolize(unaryOpExr->getSubExpr()));
      default: 
        llvm::outs() << "Unsupported unary operator: " << unaryOpExr->getOpcode() << " in expr: " << prettyPrint(unaryOpExr, astContext);
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
