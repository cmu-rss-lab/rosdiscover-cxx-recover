#pragma once

#include <vector>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>

#include "ApiCall.h"
#include "Context.h"
#include "Function.h"
#include "String.h"
#include "Value.h"

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

  SymbolicRosApiCall * symbolizeApiCall(api_call::RosApiCall *apiCall) {
    using namespace rosdiscover::api_call;
    llvm::outs() << "symbolizing ROS API call: ";
    apiCall->print(llvm::outs());
    llvm::outs() << "\n";

    switch (apiCall->getKind()) {
      case RosApiCallKind::AdvertiseServiceCall:
        return symbolizeApiCall((AdvertiseServiceCall*) apiCall);
      case RosApiCallKind::AdvertiseTopicCall:
        return symbolizeApiCall((AdvertiseTopicCall*) apiCall);
      case RosApiCallKind::BareDeleteParamCall:
        return symbolizeApiCall((BareDeleteParamCall*) apiCall);
      case RosApiCallKind::BareGetParamCachedCall:
        return symbolizeApiCall((BareGetParamCachedCall*) apiCall);
      case RosApiCallKind::BareGetParamCall:
        return symbolizeApiCall((BareGetParamCall*) apiCall);
      case RosApiCallKind::BareGetParamWithDefaultCall:
        return symbolizeApiCall((BareGetParamWithDefaultCall*) apiCall);
      case RosApiCallKind::BareHasParamCall:
        return symbolizeApiCall((BareHasParamCall*) apiCall);
      case RosApiCallKind::BareServiceCall:
        return symbolizeApiCall((BareServiceCall*) apiCall);
      case RosApiCallKind::BareSetParamCall:
        return symbolizeApiCall((BareSetParamCall*) apiCall);
      case RosApiCallKind::DeleteParamCall:
        return symbolizeApiCall((DeleteParamCall*) apiCall);
      case RosApiCallKind::GetParamCachedCall:
        return symbolizeApiCall((GetParamCachedCall*) apiCall);
      case RosApiCallKind::GetParamCall:
        return symbolizeApiCall((GetParamCall*) apiCall);
      case RosApiCallKind::GetParamWithDefaultCall:
        return symbolizeApiCall((GetParamWithDefaultCall*) apiCall);
      case RosApiCallKind::HasParamCall:
        return symbolizeApiCall((HasParamCall*) apiCall);
      case RosApiCallKind::RosInitCall:
        return symbolizeApiCall((RosInitCall*) apiCall);
      case RosApiCallKind::ServiceClientCall:
        return symbolizeApiCall((ServiceClientCall*) apiCall);
      case RosApiCallKind::SetParamCall:
        return symbolizeApiCall((SetParamCall*) apiCall);
      case RosApiCallKind::SubscribeTopicCall:
        return symbolizeApiCall((SubscribeTopicCall*) apiCall);
    }
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::AdvertiseServiceCall *apiCall) {
    return new ServiceProvider(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::AdvertiseTopicCall *apiCall) {
    return new Publisher(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareDeleteParamCall *apiCall) {
    return new DeleteParam(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareGetParamCachedCall *apiCall) {
    return new ReadParam(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareGetParamCall *apiCall) {
    return new ReadParam(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareGetParamWithDefaultCall *apiCall) {
    return new ReadParamWithDefault(StringLiteral::create("foobar"), StringLiteral::create("DEFAULT"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareHasParamCall *apiCall) {
    return new HasParam(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareServiceCall *apiCall) {
    return new ServiceCaller(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareSetParamCall *apiCall) {
    return new WriteParam(StringLiteral::create("foobar"), StringLiteral::create("VALUE"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::DeleteParamCall *apiCall) {
    return new DeleteParam(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::GetParamCachedCall *apiCall) {
    return new ReadParam(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::GetParamCall *apiCall) {
    return new ReadParam(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::GetParamWithDefaultCall *apiCall) {
    return new ReadParamWithDefault(StringLiteral::create("foobar"), StringLiteral::create("DEFAULT"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::HasParamCall *apiCall) {
    return new HasParam(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::RosInitCall *apiCall) {
    return new RosInit(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::ServiceClientCall *apiCall) {
    return new ServiceCaller(StringLiteral::create("foobar"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::SetParamCall *apiCall) {
    return new WriteParam(StringLiteral::create("foobar"), StringLiteral::create("VALUE"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::SubscribeTopicCall *apiCall) {
    return new Subscriber(StringLiteral::create("foobar"));
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
      compound.append(symbolizeApiCall(apiCall));
    }

    for (auto *functionCall : functionCalls) {
      compound.append(symbolizeFunctionCall(functionCall));
    }

    symContext.define(function, compound);
  }
};

} // rosdiscover::symbolic
} // rosdiscover
