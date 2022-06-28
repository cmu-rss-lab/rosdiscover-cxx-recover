#pragma once

#include <clang/AST/Stmt.h>

#include "ApiCall/RosApiCall.h"
#include "Ast/Stmt/If.h"
#include "Ast/Stmt/While.h"
#include "Callback/Callback.h"

namespace rosdiscover {

enum class RawStatementKind {
  RosApiCall,
  FunctionCall,
  Callback,
  If,
  While
};

class RawStatement {
public:
  virtual ~RawStatement(){}
  virtual clang::Stmt* getUnderlyingStmt() = 0;
  virtual RawStatementKind getKind() = 0;
};

class RawIfStatement : public RawStatement {
public:
  RawIfStatement(clang::IfStmt *ifStmt) : ifStmt(ifStmt) {}
  ~RawIfStatement(){}

  clang::Stmt* getUnderlyingStmt() override {
    return ifStmt;
  }

  clang::IfStmt* getIfStmt() {
    return ifStmt;
  }

  RawStatementKind getKind() override {
    return RawStatementKind::If;
  }

private:
  clang::IfStmt *ifStmt;
};

class RawWhileStatement : public RawStatement {
public:
  RawWhileStatement(clang::WhileStmt *whileStmt) : whileStmt(whileStmt) {}
  ~RawWhileStatement(){}

  clang::Stmt* getUnderlyingStmt() override {
    return whileStmt;
  }

  clang::WhileStmt* getWhileStmt() {
    return whileStmt;
  }
  
  RawStatementKind getKind() override {
    return RawStatementKind::While;
  }

private:
  clang::WhileStmt *whileStmt;
};

class RawRosApiCallStatement : public RawStatement {
public:
  RawRosApiCallStatement(api_call::RosApiCall *apiCall) : apiCall(apiCall) {}
  ~RawRosApiCallStatement(){}

  api_call::RosApiCall* getApiCall() {
    return apiCall;
  }

  clang::Stmt* getUnderlyingStmt() override {
    return const_cast<clang::Expr*>(apiCall->getExpr());
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

class RawCallbackStatement : public RawStatement {
public:
  RawCallbackStatement(Callback *callback) : callback(callback) {}
  ~RawCallbackStatement(){}

  clang::Stmt* getUnderlyingStmt() override {
    return const_cast<clang::Expr*>(callback->getApiCall()->getExpr());
  }

  RawStatementKind getKind() override {
    return RawStatementKind::Callback;
  }

  clang::FunctionDecl const * getTargetFunction() const {
    return callback->getTargetFunction();
  }

private:
  Callback *callback;
};

} // rosdiscover
