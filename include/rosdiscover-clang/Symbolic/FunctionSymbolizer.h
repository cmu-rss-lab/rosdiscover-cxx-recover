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

  }

private:
  SymbolicFunction symbolize(
      clang::ASTContext &astContext,
      std::vector<api_call::RosApiCall *> apiCalls,
      std::vector<clang::Expr *> relevantFunctionCalls
  ) : astContext(astContext),
      apiCalls(apiCalls),
      relevantFunctionCalls(relevantFunctionCalls)
  {}

  clang::ASTContext &astContext;
  std::vector<api_call::RosApiCall *> apiCalls;
  std::vector<clang::Expr *> relevantFunctionCalls;
};

} // rosdiscover::symbolic
} // rosdiscover
