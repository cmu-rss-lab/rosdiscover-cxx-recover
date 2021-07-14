#pragma once

#include <vector>

#include <clang/AST/ASTContext.h>

namespace rosdiscover {
namespace symbolic {

class FunctionSymbolizer {
public:
  static SymbolicFunction symbolize(
      clang::ASTContext &astContext,
      std::vector<api_call::RosApiCall *> apiCalls,
      std::vector<clang::Expr *> relevantFunctionCalls
  ) {
    FunctionSymbolizer(astContext, apiCalls, relevantFuntionCalls).run();
  }

private:
  FunctionSymbolizer(
      clang::ASTContext &astContext,
      std::vector<api_call::RosApiCall *> apiCalls,
      std::vector<clang::Expr *> relevantFunctionCalls
  ) : astContext(astContext),
      apiCalls(apiCalls),
      relevantFunctionCalls(relevantFunctionCalls)
  {}

  clang::ASTContext &astContext;
  std::vector<api_call::RosApiCall *> &apiCalls;
  std::vector<clang::Expr *> &relevantFunctionCalls;

  void symbolizeApiCall(api_call::RosApiCall *apiCall) {

  }

  SymbolicFunctionCall* symbolizeFunctionCall(clang::Expr *callExpr) {
    // get name of target?
    // or: use context to get SymbolicFunction?
    auto *function = symContext.getDefinitionForCall(callExpr);
    return SymbolicFunctionCall::create(function);
  }

  void run() {
    // TODO determine correct order for API and function calls
    for (auto *apiCall : apiCalls)
      symbolizeApiCall(apiCall);

    for (auto *functionCall : relevantFunctionCalls)
      symbolizeFunctionCall(functionCall);
  }
};

} // rosdiscover::symbolic
} // rosdiscover
