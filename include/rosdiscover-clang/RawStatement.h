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
  Assignment,
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
  virtual bool contains(RawStatement* stmt) {
      return stmt == this;    
  }
  std::string getKindAsString() {
    switch (getKind()){
      case RawStatementKind::RosApiCall: 
        return "RosApiCall";
      case RawStatementKind::FunctionCall: 
        return "FunctionCall";
      case RawStatementKind::Assignment: 
        return "Assignment";
      case RawStatementKind::Callback: 
        return "Callback";
      case RawStatementKind::If:
        return "If";
      case RawStatementKind::While:
        return "While";
      case RawStatementKind::Compound: 
        return "Compound";
      default:
        return "UnknownKind";
    }

  }
};

class RawCompound : public RawStatement {
public:
  RawCompound(clang::Stmt* underlyingStmt) : underlyingStmt(underlyingStmt), statements() {}
  ~RawCompound(){}

  RawCompound(RawCompound&& other)
  : underlyingStmt(other.underlyingStmt), statements(other.statements)
  {}

  bool contains(RawStatement* statement) override {
    if (statement == this) 
      return true;

    for(auto stmt: statements) {
      if (stmt->contains(statement)) {
        return true;
      }
    }
    return false;
  }

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
    falseBody(new RawCompound(ifStmt->getElse())) {}
  ~RawIfStatement(){}

  clang::Stmt* getUnderlyingStmt() override {
    return ifStmt;
  }

  bool contains(RawStatement* statement) override {
    return statement == this || 
           getTrueBody()->contains(statement) ||
           getFalseBody()->contains(statement)  ;
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

  bool contains(RawStatement* statement) override {
    return statement == this || 
           getBody()->contains(statement);
  }

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

class RawAssignment : public RawStatement { 
public:
  RawAssignment(RawAssignment *other) : assign(other->assign) {}
  
  RawAssignment(const clang::Expr *assign) : assign(assign) {}
  ~RawAssignment(){}
  
  const clang::Expr* getBinaryOperator() {
    return assign;
  }

  clang::Stmt* getUnderlyingStmt() override {
    return const_cast<clang::Expr *>(assign);
  }

  RawStatementKind getKind() override {
    return RawStatementKind::Assignment;
  }

private:
  const clang::Expr *assign;
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
