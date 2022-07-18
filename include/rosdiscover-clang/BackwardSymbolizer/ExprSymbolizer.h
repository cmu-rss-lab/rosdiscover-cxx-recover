#pragma once

#include <string>
#include <unordered_map>

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <llvm/ADT/APInt.h>

#include "../Builder/ValueBuilder.h"
#include "../Value/String.h"
#include "../Helper/FindDefVisitor.h"
#include "../ApiCall/Calls/Util.h"

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

    auto *constNum = api_call::evaluateNumber("ExprSymbolizer", expr, astContext, false);
    if (constNum != nullptr) {
      return std::make_unique<SymbolicConstant>(*constNum);
    } 

    llvm::outs() << "symbolizing (expr): ";
    expr->dump();
    llvm::outs() << "\n";

    if (auto *binOpExpr = clang::dyn_cast<clang::BinaryOperator>(expr)) {
      return symbolizeBinaryOp(binOpExpr);
    } else if (auto *unaryOpExpr = clang::dyn_cast<clang::UnaryOperator>(expr)) {
      return symbolizeUnaryOp(unaryOpExpr);
    } else if (auto *operatorCallExpr = clang::dyn_cast<clang::CXXOperatorCallExpr>(expr)) {
      return symbolizeOperatorCallExpr(operatorCallExpr);
    } else if (auto *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(expr)) {
      return symbolizeDeclRef(declRefExpr);
    } else if (auto *memberExpr = clang::dyn_cast<clang::MemberExpr>(expr)) {
      return symbolize(memberExpr->getBase());
    } else if (auto *literal = clang::dyn_cast<clang::StringLiteral>(expr)) {
      return std::make_unique<SymbolicStringConstant>(literal->getString().str());
    } else if (auto *callExpr = clang::dyn_cast<clang::CallExpr>(expr)) {
      return symbolizeCallExpr(callExpr);
    }

    llvm::outs() << "unable to symbolize expression (expr): Unsupported expression type " << expr->getStmtClassName() << ". treating as unknown\n";
    return valueBuilder.unknown();
  }
  
  std::unique_ptr<SymbolicExpr> symbolizeOperatorCallExpr(const clang::CXXOperatorCallExpr *operatorCallExpr) {
    const std::string op = prettyPrint(operatorCallExpr->getCallee(), astContext);
    if (operatorCallExpr->isComparisonOp()) {
      return std::make_unique<CompareExpr>(symbolize(operatorCallExpr->getArg(0)), symbolize(operatorCallExpr->getArg(1)), op);
    } else if (op == "!") {
      return std::make_unique<NegateExpr>(symbolize(operatorCallExpr->getArg(0)));
    }
    llvm::outs() << "unable to symbolize expression (expr): treating as unknown\n";
    return valueBuilder.unknown();
  }

  std::unique_ptr<SymbolicExpr> symbolizeDeclRef(const clang::DeclRefExpr *declRefExpr) {
    if (declRefExpr->getDecl() == nullptr)  {
      llvm::outs() << "unable to symbolize expression (expr) since decl wasn't found: treating as unknown\n";
      declRefExpr->dump();
      return valueBuilder.unknown();    
    }

    auto decl = declRefExpr->getDecl();
    if (auto *varDecl = clang::dyn_cast<clang::VarDecl>(decl)) {
      return std::make_unique<SymbolicVariableReference>(declRefExpr, varDecl);
    } else if (auto *funcDecl = clang::dyn_cast<clang::FunctionDecl>(decl)) {
      return std::make_unique<SymbolicCall>(declRefExpr);
    }

    llvm::outs() << "unable to symbolize expression (expr) due to unsupported decl type: treating as unknown\n";
    declRefExpr->dump();
    return valueBuilder.unknown();
  }

  std::unique_ptr<SymbolicExpr> symbolizeCallExpr(const clang::CallExpr *callExpr) {
    auto funcDecl = callExpr->getDirectCallee();
    if (funcDecl == nullptr) {
      llvm::outs() << "unable to symbolize expression (expr) since func decl wasn't found: treating as unknown\n";
      callExpr->dump();
      return valueBuilder.unknown();
    }
    if (funcDecl->getQualifiedNameAsString() == "ros::ok") {
      return std::make_unique<TrueExpr>();
    }
    llvm::outs() << "unable to symbolize expression (expr) due to unknown call name: treating as unknown\n";
    declRefExpr->dump();
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

      case clang::BinaryOperator::Opcode::BO_Add: 
        return std::make_unique<BinaryMathExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()), BinaryMathOperator::Add);
      case clang::BinaryOperator::Opcode::BO_Sub: 
        return std::make_unique<BinaryMathExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()), BinaryMathOperator::Sub);
      case clang::BinaryOperator::Opcode::BO_Mul: 
        return std::make_unique<BinaryMathExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()), BinaryMathOperator::Mul);
      case clang::BinaryOperator::Opcode::BO_Div: 
        return std::make_unique<BinaryMathExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()), BinaryMathOperator::Div);
      case clang::BinaryOperator::Opcode::BO_Rem: 
        return std::make_unique<BinaryMathExpr>(symbolize(binOpExpr->getLHS()), symbolize(binOpExpr->getRHS()), BinaryMathOperator::Rem);

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
