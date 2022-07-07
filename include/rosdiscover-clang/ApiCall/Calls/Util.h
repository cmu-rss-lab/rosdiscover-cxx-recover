#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/AST/APValue.h>
#include <clang/AST/Expr.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Lex/Lexer.h>

#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/APFloat.h>

#include "../RosApiCall.h"

namespace rosdiscover {

  static inline bool stmtContainsStmt(const clang::Stmt* parent, const clang::Stmt* child) {
    if (parent == nullptr || child == nullptr) {
      return false;
    }
    for (auto c: parent->children()) {
      if (c == child) {
        return true;
      } else if (stmtContainsStmt(c, child)) {
        return true;
      }
    }
    return false;
  }

  static std::string prettyPrint(clang::Stmt* const statement, clang::ASTContext const &context) {
    auto range = clang::CharSourceRange::getTokenRange(statement->getSourceRange());
    llvm::StringRef ref = clang::Lexer::getSourceText(
      range, 
      context.getSourceManager(), 
      clang::LangOptions()
    );
    return ref.str();
  }

  static std::vector<clang::Stmt*> getTransitiveChildenByType(clang::Stmt* const parent, bool const includeDeclRefExpr) {
    std::vector<clang::Stmt*> result;
    for (auto c: parent->children()) {
      if (includeDeclRefExpr) {
        if (auto *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(c)) {
          result.push_back(declRefExpr);
        } 
      }
      auto childResult = getTransitiveChildenByType(c,  includeDeclRefExpr);
      result.insert(result.end(), childResult.begin(), childResult.end());
    }
    return result;
  }
  
namespace api_call {

  static clang::APValue const * evaluateNumber(
    const std::string debugTag, 
    const clang::Expr *expr,
    const clang::ASTContext &Ctx
  ) {
    //Try evaluating the frequency as integer.
    clang::Expr::EvalResult resultInt;
    if (expr->EvaluateAsInt(resultInt, Ctx)) {
      llvm::outs() << "DEBUG [" << debugTag << "]: evaluated INT: (" << resultInt.Val.getInt().getSExtValue() << ")\n";
      return new clang::APValue(resultInt.Val);
    }

    //Try evaluating the frequency as float.
    llvm::APFloat resultFloat(0.0);
    if (expr->EvaluateAsFloat(resultFloat, Ctx)) {
      llvm::outs() << "DEBUG [" << debugTag << "]: evaluated Float: (" << resultFloat.convertToDouble() << ")\n";
      return new clang::APValue(resultFloat);
    }

    //Try evaluating the frequency as fixed point.
    clang::Expr::EvalResult resultFixed;
    if (expr->EvaluateAsFixedPoint(resultFixed, Ctx)) {
      llvm::outs() << "DEBUG [" << debugTag << "]: evaluated Fixed: (" << resultFixed.Val.getFixedPoint().toString() << ")\n";
      return new clang::APValue(resultFixed.Val.getFixedPoint());
    } 
  
    //All evaluation attempts have failed.
    llvm::outs() << "DEBUG [" << debugTag << "]: has unsupported type: "; 
    expr->dump();
    llvm::outs() << "\n";

    return nullptr;
  }

  static const clang::ValueDecl *getCallerDecl(const std::string debugTag, const clang::CXXMemberCallExpr * memberCallExpr) {
    const auto *caller = memberCallExpr->getImplicitObjectArgument()->IgnoreImpCasts();

    const auto *member = clang::dyn_cast<clang::MemberExpr>(caller);
    if (member == nullptr)
    {
      const auto *declRef = clang::dyn_cast<clang::DeclRefExpr>(caller);
      if (declRef == nullptr || !declRef->getDecl()) {
        llvm::outs() << "ERROR [" << debugTag << "] Can't find declaration of CXXMemberCallExpr: ";
        memberCallExpr->dump();
        llvm::outs() << "\n";
        return nullptr;
      }
      return declRef->getDecl();
    }
    return member->getMemberDecl();
  }
} // rosdiscover::api_call
} // rosdiscover
