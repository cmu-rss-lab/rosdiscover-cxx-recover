#pragma once

#include <vector>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>

#include "../StmtOrderingVisitor.h"
#include "../RawStatement.h"
#include "ApiCall.h"
#include "Context.h"
#include "Function.h"
#include "String.h"
#include "StringSymbolizer.h"
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
      functionCalls(functionCalls),
      stringSymbolizer(astContext)
  {}

  clang::ASTContext &astContext;
  SymbolicContext &symContext;
  clang::FunctionDecl const *function;
  std::vector<api_call::RosApiCall *> &apiCalls;
  std::vector<clang::Expr *> &functionCalls;
  StringSymbolizer stringSymbolizer;

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

  SymbolicString * symbolizeApiCallName(api_call::RosApiCall *apiCall) {
    return stringSymbolizer.symbolize(const_cast<clang::Expr*>(apiCall->getNameExpr()));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::AdvertiseServiceCall *apiCall) {
    return new ServiceProvider(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::AdvertiseTopicCall *apiCall) {
    return new Publisher(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareDeleteParamCall *apiCall) {
    return new DeleteParam(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareGetParamCachedCall *apiCall) {
    return new ReadParam(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareGetParamCall *apiCall) {
    return new ReadParam(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareGetParamWithDefaultCall *apiCall) {
    return new ReadParamWithDefault(symbolizeApiCallName(apiCall), StringLiteral::create("DEFAULT"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareHasParamCall *apiCall) {
    return new HasParam(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareServiceCall *apiCall) {
    return new ServiceCaller(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::BareSetParamCall *apiCall) {
    return new WriteParam(symbolizeApiCallName(apiCall), StringLiteral::create("VALUE"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::DeleteParamCall *apiCall) {
    return new DeleteParam(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::GetParamCachedCall *apiCall) {
    return new ReadParam(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::GetParamCall *apiCall) {
    return new ReadParam(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::GetParamWithDefaultCall *apiCall) {
    return new ReadParamWithDefault(symbolizeApiCallName(apiCall), StringLiteral::create("DEFAULT"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::HasParamCall *apiCall) {
    return new HasParam(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::RosInitCall *apiCall) {
    return new RosInit(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::ServiceClientCall *apiCall) {
    return new ServiceCaller(symbolizeApiCallName(apiCall));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::SetParamCall *apiCall) {
    return new WriteParam(symbolizeApiCallName(apiCall), StringLiteral::create("VALUE"));
  }

  SymbolicRosApiCall * symbolizeApiCall(api_call::SubscribeTopicCall *apiCall) {
    return new Subscriber(symbolizeApiCallName(apiCall));
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

  std::vector<std::unique_ptr<RawStatement>> computeStatementOrder() {
    // unify all of the statements in this function
    std::vector<RawStatement*> unordered;
    for (auto *apiCall : apiCalls) {
      unordered.push_back(new RawRosApiCallStatement(apiCall));
    }
    for (auto *functionCall : functionCalls) {
      unordered.push_back(new RawFunctionCallStatement(functionCall));
    }

    // create a mapping from underlying stmts
    std::vector<clang::Stmt*> unorderedClangStmts;
    std::unordered_map<clang::Stmt*, RawStatement*> clangToRawStmt;
    for (auto *rawStatement : unordered) {
      auto clangStmt = rawStatement->getUnderlyingStmt();
      unorderedClangStmts.push_back(clangStmt);
      clangToRawStmt.emplace(clangStmt, rawStatement);
    }

    // find the ordering of underlying stmts
    std::vector<std::unique_ptr<RawStatement>> ordered;
    auto orderedClangStmts = StmtOrderingVisitor::computeOrder(astContext, function, unorderedClangStmts);
    for (auto *clangStmt : orderedClangStmts) {
      ordered.push_back(std::unique_ptr<RawStatement>(clangToRawStmt[clangStmt]));
    }

    return ordered;
  }

  SymbolicStmt* symbolizeStatement(RawStatement *statement) {
    SymbolicStmt *symbolic;
    switch (statement->getKind()) {
      case RawStatementKind::RosApiCall:
        symbolic = symbolizeApiCall(((RawRosApiCallStatement*) statement)->getApiCall());
        break;
      case RawStatementKind::FunctionCall:
        symbolic = symbolizeFunctionCall(((RawFunctionCallStatement*) statement)->getCall());
        break;
    }
    return AnnotatedSymbolicStmt::create(astContext, symbolic, statement);
  }

  void run() {
    auto compound = SymbolicCompound();

    for (auto &statement : computeStatementOrder()) {
      compound.append(symbolizeStatement(statement.get()));
    }

    symContext.define(function, compound);
  }
};

} // rosdiscover::symbolic
} // rosdiscover