#pragma once

#include <string>
#include <unordered_map>

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <llvm/ADT/APInt.h>

#include "../Builder/ValueBuilder.h"
#include "../Value/String.h"
#include "../Value/Value.h"
#include "../Helper/FindDefVisitor.h"
#include "../ApiCall/Calls/Util.h"

#include "../Ast/Stmt/Exprs.h"
#include "IntSymbolizer.h"
#include "BoolSymbolizer.h"
#include "FloatSymbolizer.h"
#include "StringSymbolizer.h"

namespace rosdiscover {

class ExprSymbolizer {
public:
  ExprSymbolizer(
    clang::ASTContext &astContext,
    std::unordered_map<clang::Expr const *, SymbolicVariable *> &apiCallToVar
  ): 
    astContext(astContext), 
    valueBuilder(), 
    intSymbolizer(),
    boolSymbolizer(astContext),
    floatSymbolizer(),
    stringSymbolizer(astContext, apiCallToVar)
  {}

  std::unique_ptr<SymbolicExpr> symbolize(const clang::Expr *expr) {
    if (expr == nullptr) {
      llvm::outs() << "ERROR! Symbolizing (expr): NULLPTR";
      return valueBuilder.unknown();
    }

    expr = expr->IgnoreParenCasts()->IgnoreImpCasts()->IgnoreCasts();

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
      return symbolizeMemberExpr(memberExpr);
    } else if (auto *callExpr = clang::dyn_cast<clang::CallExpr>(expr)) {
      return symbolizeCallExpr(callExpr);
    } else if (auto *thisExpr = clang::dyn_cast<clang::CXXThisExpr>(expr)) {
      return std::make_unique<ThisExpr>();
    } else if (auto *nullExpr = clang::dyn_cast<clang::GNUNullExpr>(expr)) {
      return std::make_unique<NullExpr>();
    } 
    
    return symbolizeConstant(expr);
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
  
  std::unique_ptr<SymbolicExpr> symbolizeMemberExpr(const clang::MemberExpr *memberExpr) {
    return std::make_unique<SymbolicMemberVariableReference>(
      true, 
      false, 
      memberExpr->getType().getAsString(), 
      memberExpr->getMemberNameInfo().getAsString(), 
      false, 
      false, 
      false,
      symbolize(memberExpr->getBase())
      );    
  }

  std::unique_ptr<SymbolicExpr> symbolizeConstant(const clang::Expr *expr) {
    expr = expr->IgnoreParenCasts()->IgnoreImpCasts()->IgnoreCasts();
    switch (SymbolicValue::getSymbolicType(expr->getType())) {
      case SymbolicValueType::String: 
        return stringSymbolizer.symbolize(expr);
      case SymbolicValueType::Bool: 
        return boolSymbolizer.symbolize(expr);
      case SymbolicValueType::Float:
        return floatSymbolizer.symbolize(expr);
      case SymbolicValueType::Integer:
        return intSymbolizer.symbolize(expr);
      case SymbolicValueType::NodeHandle: 
        llvm::outs() << "unable to symbolize expression (expr) Node Handle not supported: treating as unknown\n";
        expr->dump();
        return valueBuilder.unknown();
      case SymbolicValueType::Unsupported:
        auto *constNum = api_call::evaluateNumber("ExprSymbolizer", expr, astContext, false);
        if (constNum != nullptr) {
          return std::make_unique<SymbolicConstant>(*constNum);
        }
        llvm::outs() << "unable to symbolize expression (expr) not supported: treating as unknown\n";
        expr->dump();
        return valueBuilder.unknown();
    }
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
    } else if (auto *enumDecl = clang::dyn_cast<clang::EnumConstantDecl>(decl)) {
      clang::Expr::EvalResult resultInt;
      long enumValue = -1;
      if (declRefExpr->EvaluateAsInt(resultInt, astContext)) {
        enumValue = resultInt.Val.getInt().getSExtValue();
      }
      return std::make_unique<SymbolicEnumReference>(enumDecl->getType().getAsString(), enumDecl->getNameAsString(), enumValue);
    }
    
    return symbolizeConstant(declRefExpr);
  }

  std::unique_ptr<SymbolicExpr> symbolizeCallExpr(const clang::CallExpr *callExpr) {
    auto funcDecl = callExpr->getDirectCallee();
    if (funcDecl == nullptr) {
      llvm::outs() << "unable to symbolize expression (expr) since func decl wasn't found: treating as unknown\n";
      callExpr->dump();
      return valueBuilder.unknown();
    }
    if (funcDecl->getQualifiedNameAsString() == "ros::ok") {
      return std::make_unique<BoolLiteral>(true);
    }
    llvm::outs() << "unable to symbolize expression (expr) due to unknown call name: treating as unknown\n";
    callExpr->dump();
    return valueBuilder.unknown();
  }

  std::unique_ptr<SymbolicExpr> symbolizeBinaryOp(const clang::BinaryOperator *binOpExpr) {
    switch (binOpExpr->getOpcode()) {
      case clang::BinaryOperator::Opcode::BO_LAnd: 
        return std::make_unique<AndExpr>(
          symbolize(binOpExpr->getLHS()),
          symbolize(binOpExpr->getRHS())
        );
      case clang::BinaryOperator::Opcode::BO_LOr: 
        return std::make_unique<OrExpr>(
          symbolize(binOpExpr->getLHS()),
          symbolize(binOpExpr->getRHS())
        );

      case clang::BinaryOperator::Opcode::BO_EQ:
      case clang::BinaryOperator::Opcode::BO_NE:
      case clang::BinaryOperator::Opcode::BO_LT:
      case clang::BinaryOperator::Opcode::BO_LE:
      case clang::BinaryOperator::Opcode::BO_GT:
      case clang::BinaryOperator::Opcode::BO_GE:
      case clang::BinaryOperator::Opcode::BO_Cmp:
        return std::make_unique<CompareExpr>(
          symbolize(binOpExpr->getLHS()),
          symbolize(binOpExpr->getRHS()),
          CompareExpr::compareOperatorFromOpCode(binOpExpr->getOpcode())
        );

      case clang::BinaryOperator::Opcode::BO_Add:
      case clang::BinaryOperator::Opcode::BO_Sub:
      case clang::BinaryOperator::Opcode::BO_Mul:
      case clang::BinaryOperator::Opcode::BO_Div:
      case clang::BinaryOperator::Opcode::BO_Rem:
        return std::make_unique<BinaryMathExpr>(
          symbolize(binOpExpr->getLHS()),
          symbolize(binOpExpr->getRHS()),
          BinaryMathExpr::binaryMathOperatorFromOpCode(binOpExpr->getOpcode())
        );

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
  StringSymbolizer stringSymbolizer;
};

} // rosdiscover
