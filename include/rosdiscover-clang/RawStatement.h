#pragma once

#include <clang/AST/Stmt.h>

#include "ApiCall/RosApiCall.h"

namespace rosdiscover {

enum class RawStatementKind {
  RosApiCall,
  FunctionCall
};

class RawStatement {
public:
  virtual ~RawStatement(){}
  virtual clang::Stmt* getUnderlyingStmt() = 0;
  virtual RawStatementKind getKind() = 0;
};

class RawRosApiCallStatement : public RawStatement {
public:
  RawRosApiCallStatement(api_call::RosApiCall *apiCall) : apiCall(apiCall) {}
  ~RawRosApiCallStatement(){}

  api_call::RosApiCall* getApiCall() {
    return apiCall;
  }

  clang::Stmt* getUnderlyingStmt() override {
    return const_cast<clang::CallExpr*>(apiCall->getCallExpr());
  }

  RawStatementKind getKind() override {
    return RawStatementKind::RosApiCall;
  }

private:
  api_call::RosApiCall *apiCall;
};

class RawFunctionCallStatement : public RawStatement {
public:
  RawFunctionCallStatement(clang::Expr *expr) : expr(expr) {}
  ~RawFunctionCallStatement(){}

  clang::Expr* getCall() {
    return expr;
  }

  clang::Stmt* getUnderlyingStmt() override {
    return expr;
  }

  RawStatementKind getKind() override {
    return RawStatementKind::FunctionCall;
  }

private:
  clang::Expr *expr;
};

} // rosdiscover
