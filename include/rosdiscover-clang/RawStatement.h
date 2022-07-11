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
  While,
  Compound
};

class RawStatement {
public:
  virtual ~RawStatement(){}
  virtual clang::Stmt* getUnderlyingStmt() = 0;
  virtual RawStatementKind getKind() = 0;
};

class RawCompound : public RawStatement {
public:
  RawCompound(clang::Stmt* underlyingStmt) : underlyingStmt(underlyingStmt), statements() {}
  ~RawCompound(){}

  RawCompound(RawCompound&& other)
  : underlyingStmt(other.underlyingStmt), statements(other.statements)
  {}

  void append(RawStatement* statement) {
    statements.push_back(statement);
  }

  clang::Stmt* getUnderlyingStmt() override {
    return underlyingStmt;
  }

  std::vector<RawStatement*> getStmts() {
    return statements;
  }

  RawStatementKind getKind() override {
    return RawStatementKind::Compound;
  }

private:
  clang::Stmt* underlyingStmt; 
  std::vector<RawStatement*> statements;

}; //RawCompound

class RawIfStatement : public RawStatement {
public:
  RawIfStatement(clang::IfStmt *ifStmt) : 
    ifStmt(ifStmt), 
    trueBody(new RawCompound(ifStmt->getThen())), 
    falseBody(new RawCompound(ifStmt->getThen())) {}
  ~RawIfStatement(){}

  clang::Stmt* getUnderlyingStmt() override {
    return ifStmt;
  }

  clang::IfStmt* getIfStmt() {
    return ifStmt;
  }

  RawCompound* getTrueBody() {
    return trueBody;
  }

  RawCompound* getFalseBody() {
    return falseBody;
  }

  RawStatementKind getKind() override {
    return RawStatementKind::If;
  }

private:
  clang::IfStmt *ifStmt;
  RawCompound* trueBody;
  RawCompound* falseBody;
};

class RawWhileStatement : public RawStatement {
public:
  RawWhileStatement(clang::WhileStmt *whileStmt) : 
    whileStmt(whileStmt), 
    body(new RawCompound(whileStmt->getBody())) {}
  ~RawWhileStatement(){}

  clang::Stmt* getUnderlyingStmt() override {
    return whileStmt;
  }

  clang::WhileStmt* getWhileStmt() {
    return whileStmt;
  }

  RawCompound* getBody() {
    return body;
  }
  
  RawStatementKind getKind() override {
    return RawStatementKind::While;
  }

private:
  clang::WhileStmt *whileStmt;
  RawCompound* body;
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
    if (callback == nullptr) {
      llvm::outs() << "ERROR: No callback";
    }
    return callback->getTargetFunction();
  }

private:
  Callback *callback;
};

} // rosdiscover
