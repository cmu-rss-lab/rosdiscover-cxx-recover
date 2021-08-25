#pragma once

#include <unordered_set>
#include <vector>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>

#include <fmt/core.h>

#include "../Helper/StmtOrderingVisitor.h"
#include "../RawStatement.h"
#include "../Stmt/Stmts.h"
#include "../Value/String.h"
#include "../Value/Value.h"
#include "../Variable/Variable.h"
#include "Context.h"
#include "Function.h"
#include "StringSymbolizer.h"

namespace rosdiscover {
namespace symbolic {

class FunctionSymbolizer {
public:
  static void symbolize(
      clang::ASTContext &astContext,
      SymbolicContext &symContext,
      SymbolicFunction &symFunction,
      clang::FunctionDecl const *function,
      std::vector<api_call::RosApiCall *> &apiCalls,
      std::vector<clang::Expr *> &functionCalls
  ) {
    /*
    std::unordered_map<const clang::ParmVarDecl *, std::string> declToArgName;
    llvm::outs() << "DEBUG: building parameter map\n";
    for (auto it = symFunction.params_begin(); it != symFunction.params_end(); it++) {
      const clang::ParmVarDecl *parmVarDecl = clang::dyn_cast<clang::ParmVarDecl>(
        function->getParamDecl(it->second.getIndex())->getCanonicalDecl()
      );
      llvm::outs() << "DEBUG: Added ParmVarDecl mapping [" << it->second.getName() << "]: ";
      parmVarDecl->dumpColor();
      llvm::outs() << "\n";
      declToArgName.emplace(parmVarDecl, it->second.getName());
    }
    */
    std::unordered_set<std::string> symbolicArgNames;
    for (auto it = symFunction.params_begin(); it != symFunction.params_end(); it++) {
      symbolicArgNames.emplace(it->second.getName());
    }

    FunctionSymbolizer(
        astContext,
        symContext,
        symFunction,
        function,
        apiCalls,
        functionCalls,
        symbolicArgNames
        // declToArgName
    ).run();
  }

private:
  FunctionSymbolizer(
      clang::ASTContext &astContext,
      SymbolicContext &symContext,
      SymbolicFunction &symFunction,
      clang::FunctionDecl const *function,
      std::vector<api_call::RosApiCall *> &apiCalls,
      std::vector<clang::Expr *> &functionCalls,
      std::unordered_set<std::string> &symbolicArgNames
//      std::unordered_map<const clang::ParmVarDecl *, std::string> &declToArgName
  ) : astContext(astContext),
      symContext(symContext),
      symFunction(symFunction),
      function(function),
      apiCalls(apiCalls),
      functionCalls(functionCalls),
      apiCallToVar(),
      stringSymbolizer(astContext, apiCallToVar),
      valueBuilder(),
      symbolicArgNames(symbolicArgNames)
//      declToArgName(declToArgName)
  {}

  clang::ASTContext &astContext;
  SymbolicContext &symContext;
  SymbolicFunction &symFunction;
  clang::FunctionDecl const *function;
  std::vector<api_call::RosApiCall *> &apiCalls;
  std::vector<clang::Expr *> &functionCalls;
  std::unordered_map<clang::Expr const *, SymbolicVariable *> apiCallToVar;
  StringSymbolizer stringSymbolizer;
  ValueBuilder valueBuilder;
  std::unordered_set<std::string> symbolicArgNames;
//  std::unordered_map<const clang::ParmVarDecl *, std::string> declToArgName;

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::RosApiCall *apiCall) {
    using namespace rosdiscover::api_call;
    llvm::outs() << "symbolizing ROS API call: ";
    apiCall->print(llvm::outs());
    llvm::outs() << "\n";

    if (apiCall->hasNodeHandle()) {
      return symbolizeNodeHandleApiCall((api_call::NodeHandleRosApiCall*) apiCall);
    } else {
      return symbolizeBareApiCall((api_call::BareRosApiCall*) apiCall);
    }
  }

  std::unique_ptr<SymbolicNodeHandle> symbolizeNodeHandle(
    clang::ValueDecl const *decl,
    clang::Expr *atExpr
  ) {
    if (auto const *fieldDecl = clang::dyn_cast<clang::FieldDecl>(decl)) {
      return symbolizeNodeHandle(fieldDecl);
    } else if (auto const *parmVarDecl = clang::dyn_cast<clang::ParmVarDecl>(decl)) {
      return symbolizeNodeHandle(parmVarDecl);
    } else if (auto const *varDecl = clang::dyn_cast<clang::VarDecl>(decl)) {
      return symbolizeNodeHandle(varDecl, atExpr);
    } else {
      llvm::errs() << "ERROR: failed to symbolize node handle: ";
      decl->dumpColor();
      llvm::errs() << "\n";
      abort();
    }
  }

  std::unique_ptr<SymbolicNodeHandle> symbolizeNodeHandle(
    clang::Expr *expr
  ) {
    llvm::outs() << "symbolizing node handle expr: ";
    expr->dumpColor();
    llvm::outs() << "\n";

    if (auto *bindTempExpr = clang::dyn_cast<clang::CXXBindTemporaryExpr>(expr)) {
      return symbolizeNodeHandle(bindTempExpr->getSubExpr());
    }
    if (auto *cleanupsExpr = clang::dyn_cast<clang::ExprWithCleanups>(expr)) {
      return symbolizeNodeHandle(cleanupsExpr->getSubExpr());
    }

    if (auto *constructExpr = clang::dyn_cast<clang::CXXConstructExpr>(expr)) {
      auto *constructorDecl = constructExpr->getConstructor();
      if (constructorDecl->isCopyOrMoveConstructor()) {
        return symbolizeNodeHandle(constructExpr->getArg(0)->IgnoreParenCasts());
      } else {
        auto *nameExpr = constructExpr->getArg(0)->IgnoreParenCasts();
        auto name = stringSymbolizer.symbolize(nameExpr);
        return valueBuilder.nodeHandle(std::move(name));
      }
    }

    if (auto *memberCallExpr = clang::dyn_cast<clang::CXXMemberCallExpr>(expr)) {
      auto *methodDecl = memberCallExpr->getMethodDecl();
      auto methodName = methodDecl->getQualifiedNameAsString();
      if (methodName == "nodelet::Nodelet::getNodeHandle") {
        return valueBuilder.publicNodeHandle();
      } else if (methodName == "nodelet::Nodelet::getPrivateNodeHandle") {
        return valueBuilder.privateNodeHandle();
      }
    }

    if (auto *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(expr)) {
      llvm::outs() << "DEBUG: attempting to symbolize node handle DeclRefExpr\n";
      return symbolizeNodeHandle(declRefExpr->getDecl(), declRefExpr);
    }

    llvm::outs() << "WARNING: unable to symbolize node handle expression: ";
    expr->dumpColor();
    llvm::outs() << "\n";
    return valueBuilder.unknownNodeHandle();
  }

  std::unique_ptr<SymbolicNodeHandle> symbolizeNodeHandle(
    clang::VarDecl const *decl,
    clang::Expr *atExpr
  ) {
    llvm::outs() << "symbolizing node handle in var decl: ";
    decl->dumpColor();
    llvm::outs() << "\n";
    auto *def = FindDefVisitor::find(astContext, decl, atExpr);
    return symbolizeNodeHandle(def);
  }

  std::unique_ptr<SymbolicNodeHandle> symbolizeNodeHandle(clang::FieldDecl const *decl) {
    llvm::outs() << "symbolizing node handle in CXX record field: ";
    decl->dumpColor();
    llvm::outs() << "\n";

    auto const *recordDecl = clang::dyn_cast<clang::CXXRecordDecl>(decl->getParent());
    if (recordDecl == nullptr) {
      llvm::errs() << "failed to retrieve associated CXX record\n";
      abort();
    }

    // for now, we assume that the node handle is initialized in the constructor's
    // initializer list
    auto symbolic = std::unique_ptr<SymbolicNodeHandle>();

    for (auto const *constructorDecl : recordDecl->ctors()) {
      if (constructorDecl->isCopyOrMoveConstructor())
        continue;

      auto const *constructorDef = clang::dyn_cast<clang::CXXConstructorDecl>(
        constructorDecl->getDefinition()
      );

      if (constructorDef == nullptr) {
        llvm::errs() << "WARNING: unable to retrieve definition for constructor: ";
        constructorDef->dumpColor();
        llvm::errs() << "\n";
        continue;
      }

      for (auto const *initDecl : constructorDef->inits()) {
        if (auto const *initMemberDecl = initDecl->getMember()) {
          if (initMemberDecl != decl)
            continue;

          // FIXME if this doesn't call the NodeHandle constructor, skip to unknown
          auto *nameExpr = initDecl->getInit()->IgnoreParenCasts();

          /**
          // FIXME MOVE THIS!
          if (auto *fieldConstructExpr = clang::dyn_cast<clang::CXXConstructExpr>(nameExpr)) {
            auto fieldConstructorName = fieldConstructExpr->getConstructor()->getQualifiedNameAsString();
            if (fieldConstructorName == "ros::NodeHandle::NodeHandle") {
              nameExpr = fieldConstructExpr->getArg(0)->IgnoreParenCasts();
            }
          }

          auto name = stringSymbolizer.symbolize(nameExpr);
          auto newSymbolic = valueBuilder.nodeHandle(std::move(name));
          */
          auto newSymbolic = symbolizeNodeHandle(nameExpr);

          // FIXME check for ambiguous definition!
          // if (symbolic.get() != nullptr && !symbolic.equals(newSymbolic)) {
          //   llvm::outs() << "WARNING: node handle has ambiguous definition; treating as unknown\n";
          //   return SymbolicNodeHandle::unknown();
          // }

          symbolic = std::move(newSymbolic);
        }
      }
    }

    // FIXME verbose?
    if (symbolic == nullptr) {
      return valueBuilder.unknownNodeHandle();
    } else {
      return symbolic;
    }
  }

  std::unique_ptr<SymbolicNodeHandle> symbolizeNodeHandle(clang::ParmVarDecl const *decl) {
    auto argName = decl->getNameAsString();
    llvm::outs() << "DEBUG: symbolizing node handle ParmVarDecl [name: " << argName << "]: ";
    decl->dumpColor();
    llvm::outs() << "\n";

    if (symbolicArgNames.find(argName) != symbolicArgNames.end()) {
      return valueBuilder.arg(argName);
    }
    return valueBuilder.unknownNodeHandle();
  }

  std::unique_ptr<SymbolicStmt> symbolizeNodeHandleApiCall(api_call::NodeHandleRosApiCall *apiCall) {
    using namespace rosdiscover::api_call;

    // resolved the associated node handle
    // TODO: this can be cached for each node handle
    clang::Expr *atExpr = const_cast<clang::Expr*>(static_cast<const clang::Expr*>(apiCall->getCallExpr()));
    auto nodeHandle = symbolizeNodeHandle(apiCall->getNodeHandleDecl(), atExpr);
    llvm::outs() << "symbolic node handle: ";
    nodeHandle->print(llvm::outs());
    llvm::outs() << "\n";

    switch (apiCall->getKind()) {
      case RosApiCallKind::AdvertiseServiceCall:
        return symbolizeApiCall(std::move(nodeHandle), (AdvertiseServiceCall*) apiCall);
      case RosApiCallKind::AdvertiseTopicCall:
        return symbolizeApiCall(std::move(nodeHandle), (AdvertiseTopicCall*) apiCall);
      case RosApiCallKind::DeleteParamCall:
        return symbolizeApiCall(std::move(nodeHandle), (DeleteParamCall*) apiCall);
      case RosApiCallKind::GetParamCachedCall:
        return symbolizeApiCall(std::move(nodeHandle), (GetParamCachedCall*) apiCall);
      case RosApiCallKind::GetParamCall:
        return symbolizeApiCall(std::move(nodeHandle), (GetParamCall*) apiCall);
      case RosApiCallKind::GetParamWithDefaultCall:
        return symbolizeApiCall(std::move(nodeHandle), (GetParamWithDefaultCall*) apiCall);
      case RosApiCallKind::HasParamCall:
        return symbolizeApiCall(std::move(nodeHandle), (HasParamCall*) apiCall);
      case RosApiCallKind::ServiceClientCall:
        return symbolizeApiCall(std::move(nodeHandle), (ServiceClientCall*) apiCall);
      case RosApiCallKind::SetParamCall:
        return symbolizeApiCall(std::move(nodeHandle), (SetParamCall*) apiCall);
      case RosApiCallKind::SubscribeTopicCall:
        return symbolizeApiCall(std::move(nodeHandle), (SubscribeTopicCall*) apiCall);
      default:
        llvm::errs() << "unrecognized ROS API call with node handle: ";
        apiCall->print(llvm::outs());
        llvm::outs() << "\n";
        abort();
    }
  }


  std::unique_ptr<SymbolicStmt> symbolizeBareApiCall(api_call::BareRosApiCall *apiCall) {
    using namespace rosdiscover::api_call;
    llvm::outs() << "symbolizing bare ROS API call: ";
    apiCall->print(llvm::outs());
    llvm::outs() << "\n";

    switch (apiCall->getKind()) {
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
      case RosApiCallKind::RosInitCall:
        return symbolizeApiCall((RosInitCall*) apiCall);
      default:
        llvm::errs() << "unrecognized bare ROS API call: ";
        apiCall->print(llvm::outs());
        llvm::outs() << "\n";
        abort();
    }
  }

  std::unique_ptr<SymbolicString> symbolizeApiCallName(api_call::RosApiCall *apiCall) {
    return stringSymbolizer.symbolize(const_cast<clang::Expr*>(apiCall->getNameExpr()));
  }

  std::unique_ptr<SymbolicString> symbolizeNodeHandleApiCallName(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::RosApiCall *apiCall
  ) {
    auto name = symbolizeApiCallName(apiCall);

    // FIXME: since we use unique_ptr, we need to use a separate node handle
    // for each concatenate command. the solution is probably to switch to using
    // shared_ptrs in more places.
    return valueBuilder.concatenate(std::move(nodeHandle), std::move(name));
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::AdvertiseServiceCall *apiCall
  ) {
    auto name = symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall);
    return std::make_unique<ServiceProvider>(std::move(name));
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::AdvertiseTopicCall *apiCall
  ) {
    return std::make_unique<Publisher>(
      symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall),
      apiCall->getFormatName()
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareDeleteParamCall *apiCall) {
    return std::make_unique<DeleteParam>(symbolizeApiCallName(apiCall));
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareGetParamCachedCall *apiCall) {
    return createAssignment(
      std::make_unique<ReadParam>(symbolizeApiCallName(apiCall)),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareGetParamCall *apiCall) {
    return createAssignment(
      std::make_unique<ReadParam>(symbolizeApiCallName(apiCall)),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareGetParamWithDefaultCall *apiCall) {
    return createAssignment(
      std::make_unique<ReadParamWithDefault>(symbolizeApiCallName(apiCall), valueBuilder.unknown()),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareHasParamCall *apiCall) {
    // TODO we know that this is a bool!
    return createAssignment(
      std::make_unique<HasParam>(symbolizeApiCallName(apiCall)),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareServiceCall *apiCall) {
    return std::make_unique<ServiceCaller>(
      symbolizeApiCallName(apiCall),
      apiCall->getFormatName()
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareSetParamCall *apiCall) {
    return std::make_unique<WriteParam>(symbolizeApiCallName(apiCall), valueBuilder.unknown());
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::DeleteParamCall *apiCall
  ) {
    return std::make_unique<DeleteParam>(
      symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall)
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::GetParamCachedCall *apiCall
  ) {
    auto name = symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall);
    return createAssignment(
      std::make_unique<ReadParam>(std::move(name)),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::GetParamCall *apiCall
  ) {
    auto name = symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall);
    return createAssignment(
      std::make_unique<ReadParam>(std::move(name)),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::GetParamWithDefaultCall *apiCall
  ) {
    auto name = symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall);
    return createAssignment(
      std::make_unique<ReadParamWithDefault>(std::move(name), valueBuilder.unknown()),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::HasParamCall *apiCall
  ) {
    // TODO we know that this is a bool!
    auto name = symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall);
    return createAssignment(
      std::make_unique<HasParam>(std::move(name)),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::RosInitCall *apiCall) {
    return std::make_unique<RosInit>(symbolizeApiCallName(apiCall));
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::ServiceClientCall *apiCall
  ) {
    return std::make_unique<ServiceCaller>(
      symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall),
      apiCall->getFormatName()
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::SetParamCall *apiCall
  ) {
    return std::make_unique<WriteParam>(
      symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall),
      valueBuilder.unknown()
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::SubscribeTopicCall *apiCall
  ) {
    return std::make_unique<Subscriber>(
      symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall),
      apiCall->getFormatName()
    );
  }

  // TODO a unique_ptr should be passed in here!
  std::unique_ptr<SymbolicStmt> createAssignment(
    std::unique_ptr<SymbolicValue> value,
    api_call::RosApiCall *apiCall
  ) {
    // TODO determine type
    auto *local = symFunction.createLocal(SymbolicValueType::Unsupported);
    auto stmt = std::make_unique<AssignmentStmt>(local, std::move(value));

    // maintain a mapping from the ROS API call expression to the corresponding
    // symbolic variable
    auto *resultExpr = apiCall->getResultExpr();
    assert(resultExpr);
    apiCallToVar.emplace(resultExpr, local);

    llvm::outs() << "added expr->result mapping [" << local->getName() << "]:\n";
    resultExpr->dumpColor();
    llvm::outs() << "\n";

    return std::move(stmt);
  }

  clang::FunctionDecl const * getCallee(clang::Expr *expr) const {
    if (auto *callExpr = clang::dyn_cast<clang::CallExpr>(expr)) {
      return getCallee(callExpr);
    } else if (auto *constructExpr = clang::dyn_cast<clang::CXXConstructExpr>(expr)) {
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

  std::unique_ptr<SymbolicStmt> symbolizeFunctionCall(clang::Expr *callExpr) {
    auto *function = symContext.getDefinition(getCallee(callExpr));

    std::unordered_map<std::string, std::unique_ptr<SymbolicValue>> args;
    for (
      auto it = function->params_begin();
      it != function->params_end();
      it++
    ) {
      auto &param = it->second;

      // fetch the expression for the associated parameter
      clang::Expr *paramExpr;
      if (auto *constructExpr = clang::dyn_cast<clang::CXXConstructExpr>(callExpr)) {
        paramExpr = constructExpr->getArg(param.getIndex());
      } else if (auto *functionCallExpr = clang::dyn_cast<clang::CallExpr>(callExpr)) {
        paramExpr = functionCallExpr->getArg(param.getIndex());
      } else {
        llvm::errs() << "ERROR: unrecognized function call type: ";
        callExpr->dumpColor();
        llvm::errs() << "\n";
        abort();
      }

      // symbolize the parameter expression
      std::unique_ptr<SymbolicValue> symbolicParam = valueBuilder.unknown();
      switch (param.getType()) {
        case SymbolicValueType::String:
          llvm::outs() << "DEBUG: attempting to symbolize string param\n";
          symbolicParam = stringSymbolizer.symbolize(paramExpr);
          break;
        // where was the node handle defined?
        case SymbolicValueType::NodeHandle:
          llvm::outs() << "DEBUG: attempting to symbolize node handle param\n";
          symbolicParam = symbolizeNodeHandle(paramExpr->IgnoreParenCasts());
          break;
        case SymbolicValueType::Bool:
          llvm::errs() << "WARNING: boolean symbolization is currently unsupported\n";
          continue;
        case SymbolicValueType::Integer:
          llvm::errs() << "WARNING: integer symbolization is currently unsupported\n";
          continue;
        case SymbolicValueType::Unsupported:
          llvm::errs() << "ERROR: attempted to symbolize an unsupported type\n";
          abort();
      }

      llvm::outs() << "DEBUG: symbolic parameter [" << param.getName() << "]: ";
      symbolicParam->print(llvm::outs());
      llvm::outs() << "\n";

      // store the symbolic parameter
      args.emplace(param.getName(), std::move(symbolicParam));
    }

    return SymbolicFunctionCall::create(function, args);
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

  std::unique_ptr<SymbolicStmt> symbolizeStatement(RawStatement *statement) {
    std::unique_ptr<SymbolicStmt> symbolic;
    switch (statement->getKind()) {
      case RawStatementKind::RosApiCall:
        symbolic = symbolizeApiCall(((RawRosApiCallStatement*) statement)->getApiCall());
        break;
      case RawStatementKind::FunctionCall:
        symbolic = symbolizeFunctionCall(((RawFunctionCallStatement*) statement)->getCall());
        break;
    }
    return AnnotatedSymbolicStmt::create(
      astContext,
      std::move(symbolic),
      statement
    );
  }

  void run() {
    // TODO this should operate on a reference instead!
    auto compound = std::make_unique<SymbolicCompound>();

    for (auto &rawStmt : computeStatementOrder()) {
      compound->append(symbolizeStatement(rawStmt.get()));
    }

    symContext.define(function, std::move(compound));
  }
};

} // rosdiscover::symbolic
} // rosdiscover
