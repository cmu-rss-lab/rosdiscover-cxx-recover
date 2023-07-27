#pragma once

#include <string>
#include <unordered_map>

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/APValue.h>

#include"../ApiCall/Calls/Util.h"

namespace rosdiscover {

class RateSymbolizer {
private:

  static std::vector<const clang::CXXConstructExpr*> getTransitiveChilden(const clang::Stmt* parent) {
    std::vector<const clang::CXXConstructExpr*> result;
    for (auto c: parent->children()) {
      if (auto *expr = clang::dyn_cast<clang::CXXConstructExpr>(c)) {
        result.push_back(expr);
      }
      auto childResult = getTransitiveChilden(c);
      result.insert(result.end(), childResult.begin(), childResult.end());
    }
    return result;
  }

  // Used for unwrapping bind calls
  static const clang::APValue * unwrapMaterializeTemporaryExprConst(
    const clang::Expr* tempExpr,
    const clang::ASTContext &ctx
  ) {
      std::vector<const clang::CXXConstructExpr*> calls = getTransitiveChilden(tempExpr);
      for (auto* c: calls) {
        auto result = symbolizeRateConstructor(c, ctx);
        if (result != nullptr) {
          return result;
        }        
      }
      return nullptr;
  }

public:
  static const clang::APValue* symbolizeRateConstructor(const clang::CXXConstructExpr *rateConstructor, const clang::ASTContext &ctx) {
    if (rateConstructor == nullptr) {
      llvm::outs() << "ERROR [RateSymbolizer]: Didn't fine Rate constructor\n";
      return nullptr;         
    }

    //Get the frequency argument of the rate constructor
    const auto *frequencyArg = rateConstructor->getArg(0)->IgnoreImpCasts();
    llvm::outs() << "DEBUG [RateSymbolizer]: Rate found (" << frequencyArg->getStmtClassName() << ")\n";

    const auto *rateResult = api_call::evaluateNumber("RateSymbolizer", frequencyArg, ctx);
    if (rateResult != nullptr)
      return rateResult;

    const auto *frequencyArgMemberExpr = clang::dyn_cast<clang::MemberExpr>(frequencyArg);
    if (frequencyArgMemberExpr != nullptr) {
      llvm::outs() << "Debug [RateSymbolizer]: evaluateNumberMemberExpr \n";
      return api_call::evaluateNumberMemberExpr("RateSymbolizer", frequencyArgMemberExpr, ctx);     
    }

    return nullptr;
  }

  static const clang::APValue* symbolizeRate(const clang::Expr *expr, const clang::ASTContext &ctx) {

    if (expr == nullptr) {
      llvm::outs() << "ERROR! Symbolizing (rate): NULLPTR";
      return nullptr;
    }

    llvm::outs() << "Debug [RateSymbolizer]: Attempt symbolizeRate: \n";
    expr->dump();
    llvm::outs() << "\n";

    //auto rateConstructor = (getTransitiveCXXBindTemporaryExprChildenByType(expr->IgnoreCasts()));
    if (const auto *constructExpr = clang::dyn_cast<clang::CXXConstructExpr>(expr)) {
      llvm::outs() << "Debug [RateSymbolizer]: Attempt to find Constructor ";
      auto result = symbolizeRateConstructor(constructExpr, ctx);
      if (result != nullptr) {
        llvm::outs() << "Debug [RateSymbolizer]: Found Constructor ";
        return result;
      }

      llvm::outs() << "Debug [RateSymbolizer]: Finding Constructor ";
      constructExpr->getArg(0)->IgnoreCasts()->dump();
      llvm::outs() << "\n";
      if (const auto *tempExr = clang::dyn_cast<clang::MaterializeTemporaryExpr>(constructExpr->getArg(0)->IgnoreCasts())) {
        auto result = unwrapMaterializeTemporaryExprConst(tempExr, ctx);
        if (result != nullptr) {
          llvm::outs() << "Debug [RateSymbolizer]: Found result ";
          return result;
        }
      }   
    } else if (const auto *constructExpr = clang::dyn_cast<clang::CXXBindTemporaryExpr>(expr)) {
      llvm::outs() << "Debug [RateSymbolizer]: Found CXXBindTemporaryExpr ";
      auto result = unwrapMaterializeTemporaryExprConst(constructExpr, ctx);
      if (result != nullptr) {
        llvm::outs() << "Debug [RateSymbolizer]: Found result ";
        return result;
      }
    }
    
    return nullptr;
    
  }
  /*
  clang::APValue symbolizeMaterializeTemporaryExpr(const clang::MaterializeTemporaryExpr *expr) {

    if (expr == nullptr) {
      llvm::outs() << "ERROR! Symbolizing (int): NULLPTR";
      return nullptr;
    }

    expr = expr->IgnoreCasts();

    if (auto *tempExpr = clang::dyn_cast<clang::MaterializeTemporaryExpr>(expr)) {
      return symbolizeMaterializeTemporaryExpr(tempExpr);
    }
    if (auto *bindTempExpr = clang::dyn_cast<clang::CXXBindTemporaryExpr>(expr)) {
      return symbolizeCXXBindTemporaryExpr(bindTempExpr);
    }
    if (auto *constructExpr = clang::dyn_cast<clang::CXXConstructExpr>(expr)) {
      return symbolizeNodeHandle(constructExpr);
    }

    llvm::outs() << "symbolizing (rate): ";
    expr->dump();
    llvm::outs() << "\n";
    
    return nullptr;
  }

    const clang::APValue* symbolize(const clang::Expr *expr) {

    if (expr == nullptr) {
      llvm::outs() << "ERROR! Symbolizing (int): NULLPTR";
      return nullptr;
    }

    expr = expr->IgnoreCasts();

    if (auto *tempExpr = clang::dyn_cast<clang::MaterializeTemporaryExpr>(expr)) {
      return symbolizeMaterializeTemporaryExpr(tempExpr);
    }
    if (auto *bindTempExpr = clang::dyn_cast<clang::CXXBindTemporaryExpr>(expr)) {
      return symbolizeCXXBindTemporaryExpr(bindTempExpr);
    }
    if (auto *constructExpr = clang::dyn_cast<clang::CXXConstructExpr>(expr)) {
      return symbolizeNodeHandle(constructExpr);
    }

    llvm::outs() << "symbolizing (rate): ";
    expr->dump();
    llvm::outs() << "\n";
    
    return nullptr;
  }*/

};
} // rosdiscover
