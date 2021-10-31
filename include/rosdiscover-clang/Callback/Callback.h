#pragma once

#include <memory>

#include <clang/AST/Stmt.h>

#include "../ApiCall/RosApiCall.h"
#include "../Helper/utils.h"

namespace rosdiscover {

class Callback {
public:
  static Callback create(
    clang::ASTContext &context,
    api_call::RosApiCall *apiCall,
    clang::FunctionDecl const *target
  ) {
    auto *callExpr = apiCall->getCallExpr();
    auto *parent = getParentFunctionDecl(context, callExpr);
    return Callback(apiCall, target, parent);
  }

  api_call::RosApiCall* getApiCall() const {
    return apiCall;
  }

  clang::FunctionDecl const * getParentFunction() const {
    return parent;
  }

  clang::FunctionDecl const * getTargetFunction() const {
    return target;
  }

private:
  Callback(
    api_call::RosApiCall *apiCall,
    clang::FunctionDecl const *parent,
    clang::FunctionDecl const *target
  ) : apiCall(apiCall), parent(parent), target(target) {}

  api_call::RosApiCall *apiCall;
  clang::FunctionDecl const *parent;
  clang::FunctionDecl const *target;
};

}