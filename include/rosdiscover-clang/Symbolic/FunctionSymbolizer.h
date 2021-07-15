#pragma once

#include <vector>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>

#include "Context.h"
#include "Function.h"

namespace rosdiscover {
namespace symbolic {

class FunctionSymbolizer {
public:
  static void symbolize(
      clang::ASTContext &astContext,
      SymbolicContext &symContext,
      clang::FunctionDecl const *function,
      std::vector<api_call::RosApiCall *> &apiCalls,
      std::vector<clang::Expr *> &functionCalls
  ) {
    FunctionSymbolizer(
        astContext,
        symContext,
        function,
        apiCalls,
        functionCalls
    ).run();
  }

private:
  FunctionSymbolizer(
      clang::ASTContext &astContext,
      SymbolicContext &symContext,
      clang::FunctionDecl const *function,
      std::vector<api_call::RosApiCall *> &apiCalls,
      std::vector<clang::Expr *> &functionCalls
  ) : astContext(astContext),
      symContext(symContext),
      function(function),
      apiCalls(apiCalls),
      functionCalls(functionCalls)
  {}

  clang::ASTContext &astContext;
  SymbolicContext &symContext;
  clang::FunctionDecl const *function;
  std::vector<api_call::RosApiCall *> &apiCalls;
  std::vector<clang::Expr *> &functionCalls;

  void symbolizeApiCall(api_call::RosApiCall *apiCall) {

  }

  clang::FunctionDecl const * getCallee(clang::Expr *expr) const {
    if (auto *callExpr = dyn_cast<clang::CallExpr>(expr)) {
      return getCallee(callExpr);
    } else if (auto *constructExpr = dyn_cast<clang::CXXConstructExpr>(expr)) {
      return getCallee(constructExpr);
    } else {
      llvm::errs() << "FATAL ERROR: cannot determine callee from expr:\n";
      expr->dumpColor();
      abort();
    }
  }

  clang::FunctionDecl const * getCallee(clang::CallExpr *expr) const {
    auto *decl = expr->getDirectCallee();
    if (decl == nullptr) {
      llvm::errs() << "FATAL ERROR: failed to obtain direct callee from call expr:\n";
      expr->dumpColor();
      abort();
    }
    return decl->getCanonicalDecl();
  }

  clang::FunctionDecl const * getCallee(clang::CXXConstructExpr *expr) const {
    return expr->getConstructor()->getCanonicalDecl();
  }

  SymbolicFunctionCall * symbolizeFunctionCall(clang::Expr *callExpr) {
    auto *function = symContext.getDefinition(getCallee(callExpr));
    return new SymbolicFunctionCall(function);
  }

  void run() {
    // TODO determine correct order for API and function calls
    auto compound = SymbolicCompound();

    for (auto *apiCall : apiCalls) {
      symbolizeApiCall(apiCall);
    }

    for (auto *functionCall : functionCalls) {
      compound.append(symbolizeFunctionCall(functionCall));
    }

    symContext.define(function, compound);
  }
};

} // rosdiscover::symbolic
} // rosdiscover
