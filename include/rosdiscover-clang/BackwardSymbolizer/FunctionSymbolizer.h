#pragma once

#include <unordered_set>
#include <string> 
#include <vector>
#include <unordered_map>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/Analysis/CFG.h>
#include <clang/Analysis/Analyses/Dominators.h>
#include <clang/Analysis/Analyses/CFGReachabilityAnalysis.h>
#include <clang/Analysis/CFGStmtMap.h>
#include <clang/AST/ParentMap.h>
#include <llvm/ADT/STLExtras.h>
#include <fmt/core.h>

#include "../ApiCall/Calls/Util.h"
#include "../Ast/Ast.h"
#include "../Ast/Stmt/ControlDependency.h"
#include "../Helper/StmtOrderingVisitor.h"
#include "../RawStatement.h"
#include "../Value/String.h"
#include "../Value/Value.h"
#include "../Cfg/CFGBlock.h"
#include "StringSymbolizer.h"
#include "IntSymbolizer.h"
#include "BoolSymbolizer.h"
#include "FloatSymbolizer.h"

namespace rosdiscover {

class FunctionSymbolizer {
public:
  static void symbolize(
      clang::ASTContext &astContext,
      SymbolicContext &symContext,
      SymbolicFunction &symFunction,
      clang::FunctionDecl const *function,
      std::vector<api_call::RosApiCall *> &apiCalls,
      std::vector<clang::Expr *> &functionCalls,
      std::vector<Callback*> &callbacks
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
        symbolicArgNames,
        callbacks
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
      std::unordered_set<std::string> &symbolicArgNames,
      std::vector<Callback*> &callbacks
//      std::unordered_map<const clang::ParmVarDecl *, std::string> &declToArgName
  ) : astContext(astContext),
      symContext(symContext),
      symFunction(symFunction),
      function(function),
      apiCalls(apiCalls),
      functionCalls(functionCalls),
      apiCallToVar(),
      stringSymbolizer(astContext, apiCallToVar),
      intSymbolizer(),
      floatSymbolizer(),
      boolSymbolizer(astContext),
      ifMap(),
      whileMap(),
      compoundMap(),
      valueBuilder(),
      symbolicArgNames(symbolicArgNames),
      callbacks(callbacks)
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
  IntSymbolizer intSymbolizer;
  FloatSymbolizer floatSymbolizer;
  BoolSymbolizer boolSymbolizer;
  std::unordered_map<long, RawIfStatement*> ifMap; //keys are the IDs of the corresponding clang stmts.
  std::unordered_map<long, RawWhileStatement*> whileMap; //keys are the IDs of the corresponding clang stmts.
  std::unordered_map<long, RawCompound*> compoundMap; //keys are the IDs of the corresponding clang stmts.
  ValueBuilder valueBuilder;
  std::unordered_set<std::string> symbolicArgNames;
  [[maybe_unused]] std::vector<Callback*> &callbacks;
//  std::unordered_map<const clang::ParmVarDecl *, std::string> declToArgName;

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::RosApiCall *apiCall) {
    using namespace rosdiscover::api_call;
    llvm::outs() << "symbolizing ROS API call: ";
    apiCall->print(llvm::outs());
    llvm::outs() << "\n";

    if (apiCall->hasNodeHandle()) {
      llvm::outs() << "DEBUG: symbolizing ROS API call with associated node handle...\n";
      return symbolizeApiCallWithNodeHandle((api_call::NodeHandleRosApiCall*) apiCall);
    } else {
      llvm::outs() << "DEBUG: symbolizing bare ROS API call\n";
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
      return symbolizeNodeHandle(constructExpr);
    }
    if (auto *memberCallExpr = clang::dyn_cast<clang::CXXMemberCallExpr>(expr)) {
      auto *methodDecl = memberCallExpr->getMethodDecl();
      auto methodName = methodDecl->getQualifiedNameAsString();
      if (methodName == "nodelet::Nodelet::getNodeHandle"
       || methodName == "nodelet::Nodelet::getMTNodeHandle") {
        return valueBuilder.publicNodeHandle();
      } else if (methodName == "nodelet::Nodelet::getPrivateNodeHandle"
       || methodName == "nodelet::Nodelet::getMTPrivateNodeHandle") {
        return valueBuilder.privateNodeHandle();
      }
    }
    if (auto *unaryOp = clang::dyn_cast<clang::UnaryOperator>(expr)) {
      if (clang::UnaryOperator::getOpcodeStr(unaryOp->getOpcode()) == "&") {
        return symbolizeNodeHandle(unaryOp->getSubExpr());
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
    clang::CXXConstructExpr *expr
  ) {
    auto *constructorDecl = expr->getConstructor();

    // ros::NodeHandle::NodeHandle(const NodeHandle &rhs)
    if (constructorDecl->isCopyOrMoveConstructor()) {
      return symbolizeNodeHandle(expr->getArg(0)->IgnoreParenCasts());
    }

    // ros::NodeHandle::NodeHandle(const std::string &ns = std::string(), const M_string &remappings = M_string())
    if (constructorDecl->getParamDecl(0)->getOriginalType().getAsString() == "const std::string &") {
      llvm::outs()
        << "DEBUG: symbolizing node handle constructor "
        << "[ros::NodeHandle::NodeHandle(const std::string &ns = std::string(), const M_string &remappings = M_string())]\n";
      auto *nameExpr = expr->getArg(0)->IgnoreParenCasts();

      // default constructor
      if (clang::isa<clang::CXXDefaultArgExpr>(nameExpr)) {
        llvm::outs() << "DEBUG: symbolizing default constructor argument\n";
        return valueBuilder.publicNodeHandle();
      }

      llvm::outs() << "DEBUG: symbolizing non-default argument: ";
      nameExpr->dumpColor();
      llvm::outs() << "\n";

      auto name = stringSymbolizer.symbolize(nameExpr);
      return valueBuilder.nodeHandle(std::move(name));
    }

    // ros::NodeHandle::NodeHandle(const NodeHandle &parent, const std::string &ns)
    // ros::NodeHandle::NodeHandle(const NodeHandle &parent, const std::string &ns, const M_string &remappings)
    llvm::outs()
      << "WARNING: parent node handle constructors are not currently supported\n";
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

  std::unique_ptr<SymbolicStmt> symbolizeApiCallWithNodeHandle(
    api_call::RosApiCallWithNodeHandle *apiCall
  ) {
    using namespace rosdiscover::api_call;

    // resolved the associated node handle
    // TODO: this can be cached for each node handle
    clang::Expr *atExpr = const_cast<clang::Expr*>(apiCall->getExpr());
    auto nodeHandle = symbolizeNodeHandle(apiCall->getNodeHandleDecl(), atExpr);
    llvm::outs() << "DEBUG: found symbolic node handle: ";
    nodeHandle->print(llvm::outs());
    llvm::outs() << "\n";

    llvm::outs() << "DEBUG: symbolizing API call based on kind...\n";
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
      case RosApiCallKind::MessageFiltersSubscriberCall:
        return symbolizeApiCall(std::move(nodeHandle), (MessageFiltersSubscriberCall*) apiCall);
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
      case RosApiCallKind::PublishCall:
        return symbolizeApiCall((PublishCall*) apiCall);
      case RosApiCallKind::RateSleepCall:
        return symbolizeApiCall((RateSleepCall*) apiCall);        
      default:
        llvm::errs() << "unrecognized bare ROS API call: ";
        apiCall->print(llvm::outs());
        llvm::outs() << "\n";
        abort();
    }
  }

  std::unique_ptr<SymbolicString> symbolizeApiCallName(api_call::NamedRosApiCall *apiCall) {
    return stringSymbolizer.symbolize(const_cast<clang::Expr*>(apiCall->getNameExpr()));
  }

  std::unique_ptr<SymbolicString> symbolizeNodeHandleApiCallName(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::NamedRosApiCall *apiCall
  ) {
    llvm::outs() << "DEBUG: symbolizing NodeHandleApiCallName\n";
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
    llvm::outs() << "DEBUG: symbolizing AdvertiseServiceCall\n";
    auto name = symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall);
    auto requestResponseFormatNames = apiCall->getRequestResponseFormatNames();
    auto requestFormatName = std::get<0>(requestResponseFormatNames);
    auto responseFormatName = std::get<1>(requestResponseFormatNames);
    return std::make_unique<ServiceProvider>(
      std::move(name),
      requestFormatName,
      responseFormatName
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::AdvertiseTopicCall *apiCall
  ) {
    llvm::outs() << "DEBUG: symbolizing AdvertiseTopicCall\n";
    return std::make_unique<Publisher>(
      symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall),
      apiCall->getFormatName()
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::MessageFiltersSubscriberCall *apiCall
  ) {
    llvm::outs() << "DEBUG: symbolizing call to message_filters::Subscriber\n";
    auto formatName = apiCall->getFormatName();
    llvm::outs() << "DEBUG [message_filters::Subscriber]: uses format: " << formatName << "\n";

    auto* callback = apiCall->getCallback(astContext);
    std::unique_ptr<SymbolicFunctionCall> symbolicCallBack;
    if (callback == nullptr) {
      symbolicCallBack = UnknownSymbolicFunctionCall::create();
    } else {
      symbolicCallBack = symbolizeCallback(new RawCallbackStatement(callback));
    }
    return std::make_unique<Subscriber>(
      symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall),
      apiCall->getFormatName(),
      std::move(symbolicCallBack)
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareDeleteParamCall *apiCall) {
    llvm::outs() << "DEBUG: symbolizing BareDeleteParamCall\n";
    return std::make_unique<DeleteParam>(symbolizeApiCallName(apiCall));
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareGetParamCachedCall *apiCall) {
    llvm::outs() << "DEBUG: symbolizing BareGetParamCachedCall\n";
    return createAssignment(
      std::make_unique<ReadParam>(symbolizeApiCallName(apiCall)),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareGetParamCall *apiCall) {
    llvm::outs() << "DEBUG: symbolizing BareGetParamCall\n";
    return createAssignment(
      std::make_unique<ReadParam>(symbolizeApiCallName(apiCall)),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareGetParamWithDefaultCall *apiCall) {
    llvm::outs() << "DEBUG: symbolizing BareGetParamWithDefaultCall\n";
    return createAssignment(
      std::make_unique<ReadParamWithDefault>(symbolizeApiCallName(apiCall), valueBuilder.unknown()),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareHasParamCall *apiCall) {
    // TODO we know that this is a bool!
    llvm::outs() << "DEBUG: symbolizing BareHasParamCall\n";
    return createAssignment(
      std::make_unique<HasParam>(symbolizeApiCallName(apiCall)),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareServiceCall *apiCall) {
    llvm::outs() << "DEBUG: symbolizing BareServiceCall\n";
    return std::make_unique<ServiceCaller>(
      symbolizeApiCallName(apiCall),
      apiCall->getFormatName()
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::BareSetParamCall *apiCall) {
    llvm::outs() << "DEBUG: symbolizing BareSetParamCall\n";
    return std::make_unique<WriteParam>(symbolizeApiCallName(apiCall), valueBuilder.unknown());
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::DeleteParamCall *apiCall
  ) {
    llvm::outs() << "DEBUG: symbolizing DeleteParamCall\n";
    return std::make_unique<DeleteParam>(
      symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall)
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::GetParamCachedCall *apiCall
  ) {
    llvm::outs() << "DEBUG: symbolizing GetParamCachedCall\n";
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
    llvm::outs() << "DEBUG: symbolizing GetParamCall\n";
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
    llvm::outs() << "DEBUG: symbolizing GetParamWithDefaultCall\n";
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
    llvm::outs() << "DEBUG: symbolizing HasParamCall\n";
    auto name = symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall);
    return createAssignment(
      std::make_unique<HasParam>(std::move(name)),
      apiCall
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(api_call::RosInitCall *apiCall) {
    llvm::outs() << "DEBUG: symbolizing RosInitCall\n";
    return std::make_unique<RosInit>(symbolizeApiCallName(apiCall));
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::ServiceClientCall *apiCall
  ) {
    llvm::outs() << "DEBUG: symbolizing ServiceClientCall\n";
    return std::make_unique<ServiceCaller>(
      symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall),
      apiCall->getFormatName()
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::SetParamCall *apiCall
  ) {
    llvm::outs() << "DEBUG: symbolizing SetParamCall\n";
    return std::make_unique<WriteParam>(
      symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall),
      valueBuilder.unknown()
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    std::unique_ptr<SymbolicNodeHandle> nodeHandle,
    api_call::SubscribeTopicCall *apiCall
  ) {
    llvm::outs() << "DEBUG: symbolizing SubscribeTopicCall\n";
    auto* callback = apiCall->getCallback(astContext);
    std::unique_ptr<SymbolicFunctionCall> symbolicCallBack;
    if (callback == nullptr) {
      symbolicCallBack = UnknownSymbolicFunctionCall::create();
    } else {
      symbolicCallBack = symbolizeCallback(new RawCallbackStatement(callback));
    }
    return std::make_unique<Subscriber>(
      symbolizeNodeHandleApiCallName(std::move(nodeHandle), apiCall),
      apiCall->getFormatName(),
      std::move(symbolicCallBack)
    );
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    api_call::RateSleepCall *apiCall
  ) {
    llvm::outs() << "DEBUG: symbolizing RateSleepCall\n";
    return std::make_unique<RateSleep>(
        floatSymbolizer.symbolize(apiCall->getRate(astContext))
    );    
  }

  std::unique_ptr<SymbolicStmt> symbolizeApiCall(
    api_call::PublishCall *apiCall
  ) {
    llvm::outs() << "DEBUG: symbolizing PublishCall\n";

    return std::make_unique<Publish>(
        apiCall->getPublisherName(astContext),
        getControlDependenciesObjects(apiCall->getCallExpr())
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

  // Recursively builds a graph of control dependencies starting from the last block.
  std::vector<CFGBlock*> buildGraph(
    bool first,
    const clang::CFGBlock* block,
    const llvm::SmallVector<clang::CFGBlock *, 4> &deps,
    clang::CFGDominatorTreeImpl<true> &postdominatorAnalysis,
    clang::CFGDominatorTreeImpl<false> &dominatorAnalysis,
    std::vector<const clang::CFGBlock*> &analyzed,
    std::vector<CFGBlock*> &controlDependencyGraphNodes,
    std::unordered_map<long, std::unique_ptr<CFGBlock>> &blockMap
  ) {
    std::vector<CFGBlock*> predecessors;
    if (block == nullptr || block->pred_empty() || llvm::is_contained(analyzed, block)) {
      return predecessors;
    }
    analyzed.push_back(block);
    
    llvm::outs() << "buildGraph for: ";
    block->dump();
    llvm::outs() << "\n";

    // Recursively build the graph for the block's predecessors
    for (const clang::CFGBlock::AdjacentBlock predecessorBlock: block->preds()) {
      auto indirectPredecessors = buildGraph(
        false,
        predecessorBlock.getReachableBlock(),
        deps,
        postdominatorAnalysis,
        dominatorAnalysis,
        analyzed,
        controlDependencyGraphNodes,
        blockMap
      );
      predecessors.insert(predecessors.end(), indirectPredecessors.begin(), indirectPredecessors.end()); //merge results
    }

    // Create a node for control dependencies and the first block
    if (first || llvm::is_contained(deps, block)) {
      if (!blockMap.count(block->getBlockID())) //lazy creation of CFG blocks
        blockMap.emplace(block->getBlockID(), std::make_unique<CFGBlock>(block));

      auto newControlDependencyNode = blockMap.at(block->getBlockID()).get();

      // Create edges for predecessors
      for (auto *predecessor: predecessors) {

        // Check all nodes of the control dependency graph for potential edges to be created and their type
        for(auto *depBlock : controlDependencyGraphNodes) {
          bool trueBranchDominates = false;
          bool falseBranchDominates = false;

          int i = 0;
          for (const clang::CFGBlock *sBlock: predecessor->getClangBlock()->succs()) {
            // The only post-dominating control dependency is the directly following control dependency,
            // The exception to this is the head of a loop, which is the only control dependency which then also pre-dominiates the 
            // inner statement. Hence those edges need to be ignored to avoid circles in the control dependcy graph. 
            if ((postdominatorAnalysis.dominates(depBlock->getClangBlock(), sBlock) 
              && !dominatorAnalysis.dominates(depBlock->getClangBlock(), sBlock)) 
              || depBlock->getClangBlock()->getBlockID() == sBlock->getBlockID()) {
              if (i == 0) { //true branch, as defined by clang's order of successors                
                  llvm::outs() << "true branch dominates stmt\n";
                  trueBranchDominates = true;
              } else if (i == 1) { //false branch
                  llvm::outs() << "false branch dominates stmt\n";
                  falseBranchDominates = true;
              } 
            } 
            if (i > 1) {
              //TODO: Handle switch-case here.
              llvm::outs() << "Too many branches. Swich not yet supported\n";
              abort();
            }
            i++;
          }
          if (!trueBranchDominates && !falseBranchDominates) {
            continue; // No edge needed
          }
          if (trueBranchDominates && falseBranchDominates){
            llvm::outs() << "ERROR: falseBranchDominates: " << falseBranchDominates << " trueBranchDominates: " << trueBranchDominates << "\n";
            llvm::outs() << "depBlock->getClangBlock(): ";
            depBlock->getClangBlock()->dump();
            llvm::outs() << "\npredecessor->getClangBlock(): ";
            predecessor->getClangBlock()->dump();
            abort();
          }
          
          // Creating edge
          CFGEdge::EdgeType type;
          if (i == 1) {
            type = CFGEdge::EdgeType::Normal;
          } else if (i == 2) {
            type = falseBranchDominates ? CFGEdge::EdgeType::False : CFGEdge::EdgeType::True;
          } else {
            type = CFGEdge::EdgeType::Unknown;
            llvm::outs() << "ERROR: Unknown edge type\n";
          }

          if (predecessor->createEdge(depBlock, type)) {
            llvm::outs() << "created edge between " << predecessor->getConditionStr(astContext) << " and " << depBlock->getConditionStr(astContext) << " of type " << CFGEdge::getEdgeTypeName(type) << "\n";
          }
        }
      }
      return {newControlDependencyNode}; // Return newly created control dependency node
    } else { 
      return predecessors; // For non-control dependency nodes forward the predecessors to the next dependency node
    }
  }

  std::unique_ptr<CFGBlock> buildGraph(const clang::CFGBlock* clangBlockOfInterest, const llvm::SmallVector<clang::CFGBlock *, 4> &deps, clang::CFGDominatorTreeImpl<true> &postdominatorAnalysis, clang::CFGDominatorTreeImpl<false> &dominatorAnalysis) {
    std::vector<const clang::CFGBlock*> analyzed;
    std::unordered_map<long, std::unique_ptr<CFGBlock>> blockMap; //maps BlockID to CFGBlockObject
    auto cfgBlockOfInterest = std::make_unique<CFGBlock>(clangBlockOfInterest);
    std::vector<CFGBlock*> controlDependencyGraphNodes = {cfgBlockOfInterest.get()};
    blockMap.emplace(clangBlockOfInterest->getBlockID(), std::move(cfgBlockOfInterest));
    for (auto depsBlock: deps) {
      if (postdominatorAnalysis.dominates(depsBlock, clangBlockOfInterest) && !dominatorAnalysis.dominates(depsBlock, clangBlockOfInterest))
        continue; //ignore CFG blocks that come after the block of interest.
      auto depsCfgBlock = std::make_unique<CFGBlock>(depsBlock);
      controlDependencyGraphNodes.push_back(depsCfgBlock.get());
      blockMap.emplace(depsBlock->getBlockID(), std::move(depsCfgBlock));
    }
    llvm::outs() << "#### buildGraph ####\n";
    buildGraph(true, clangBlockOfInterest, deps, postdominatorAnalysis, dominatorAnalysis, analyzed, controlDependencyGraphNodes, blockMap);
    llvm::outs() << "#### graph built ####\n";
    return std::move(blockMap.at(clangBlockOfInterest->getBlockID()));
  }

  std::vector<std::unique_ptr<SymbolicControlDependency>> getControlDependenciesObjects(const clang::Stmt* stmt) {
    const std::unique_ptr<clang::CFG> sourceCFG = clang::CFG::buildCFG(
          function, function->getBody(), &astContext, clang::CFG::BuildOptions());
    clang::ControlDependencyCalculator cdc(sourceCFG.get());
    stmt->dump();
    llvm::outs() << "getControlDependencies: ";
    std::unique_ptr<clang::ParentMap> PM = std::make_unique<clang::ParentMap>(function->getBody());
    auto CM = std::unique_ptr<clang::CFGStmtMap>(clang::CFGStmtMap::Build(sourceCFG.get(), PM.get()));
    auto stmt_block = CM->getBlock(stmt); 
    stmt_block->dump();
    auto deps = cdc.getControlDependencies(const_cast<clang::CFGBlock *>(stmt_block));
    
    std::vector<std::unique_ptr<SymbolicControlDependency>> results;

    auto prevBlock = stmt_block;
    auto analysis = std::make_unique<clang::CFGReverseBlockReachabilityAnalysis>(*(sourceCFG.get()));
    clang::CFGDominatorTreeImpl<true> postDominatorAnalysis(sourceCFG.get());
    clang::CFGDominatorTreeImpl<false> dominatorAnalysis(sourceCFG.get());

    for (clang::CFGBlock *block: deps) {
      block->dump();
    }
    auto graph = buildGraph(stmt_block, deps, postDominatorAnalysis, dominatorAnalysis);
    graph->getClangBlock()->dump();
    auto condStr = graph->getFullConditionStr(astContext);
    llvm::outs() << "\nFullControlCondition: " << condStr << "\n";

    for (clang::CFGBlock *block: deps) {

      //To identify whether the control condition needs to be negated, we need to see if the true or false branch is 
      //leading to the statement. To do this, we need to see if within the sub-graph of the CFG starting from the control-
      //dependent the true or false branch is dominating the lower-level control-dependent block. Since one of the paths
      //could be a sink in the CFG, there can be multiple paths that lead to the statement. Hence, the CFG entry point 
      //needs to be the control-depdent block.
      auto entry = sourceCFG->createBlock();
      clang::CFGBlock::AdjacentBlock aBlock(block, true);
      entry->addSuccessor(aBlock, sourceCFG->getBumpVectorContext());
      sourceCFG->setEntry(entry);
      clang::CFGDominatorTreeImpl<false> dominatorAnalysis(sourceCFG.get());

      try {
        if (block == nullptr || block->empty() || block->size() < 1 || block->size() > 1000 || block->getTerminatorStmt() == nullptr )   {
          continue;
        }
        llvm::outs() << "size " << block->size() << "\n";
        llvm::outs() << "looking for terminator condition in " << block->getTerminatorStmt()->getStmtClassName() << "\n";

        const auto *condition = block->getTerminatorCondition();
        if (condition == nullptr) {
          llvm::outs() << "no terminator condition\n";
          continue;
        }
        auto conditionStr = rosdiscover::prettyPrint(condition, astContext);
        if (block->getTerminatorStmt()->getStmtClass() == clang::Stmt::SwitchStmtClass) {
          conditionStr = "switch (" + conditionStr + ")";
          llvm::outs() << "ERROR: Encountered switch: " << conditionStr << "\n";
          abort();
        }
        
        llvm::outs() << "terminator condition found: " << conditionStr << "\n";

        std::vector<std::unique_ptr<SymbolicCall>> functionCalls;
        std::vector<std::unique_ptr<SymbolicVariableReference>> variableReferences;
        for (auto delcRef : getTransitiveChildenByType(condition, true, false)) {
          auto *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(delcRef);
          if (declRefExpr != nullptr && declRefExpr->getDecl() != nullptr)  {
            auto decl = declRefExpr->getDecl();
            if (auto *varDecl = clang::dyn_cast<clang::VarDecl>(decl)) {
              variableReferences.push_back(std::make_unique<SymbolicVariableReference>(declRefExpr, varDecl));
            } else if (auto *funcDecl = clang::dyn_cast<clang::FunctionDecl>(decl)) {
              functionCalls.push_back(std::make_unique<SymbolicCall>(declRefExpr));
            }
          }
        }

        llvm::outs() << "variableReferences and functionCalls created\n";
        
        results.push_back(
          std::make_unique<SymbolicControlDependency>(
            std::move(functionCalls), 
            std::move(variableReferences),
            condition->getSourceRange().printToString(astContext.getSourceManager()),
            conditionStr
          )
        );

        llvm::outs() << "SymbolicControlDependency created\n";
      } catch (...) {
        llvm::outs() << "[Error] Failed to create SymbolicControlDependency";
      }
      prevBlock = block;
    }

    llvm::outs() << "getControlDependenciesObjects end\n";

    return results;
  }

  std::unique_ptr<SymbolicStmt> symbolizeFunctionCall(clang::Expr *callExpr) {
    auto *calledFunction = symContext.getDefinition(getCallee(callExpr));
    llvm::outs() << "DEBUG: symbolizing call to function: " << calledFunction->getName() << "\n";

    std::unordered_map<std::string, std::unique_ptr<SymbolicValue>> args;
    for (
      auto it = calledFunction->params_begin();
      it != calledFunction->params_end();
      it++
    ) {
      auto &param = it->second;
      llvm::outs() << "DEBUG: symbolizing function call parameter: ";
      param.print(llvm::outs());
      llvm::outs() << "\n";

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
        case SymbolicValueType::Float:
          llvm::errs() << "WARNING: float symbolization is currently unsupported\n";
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

    return SymbolicFunctionCall::create(calledFunction, args, getControlDependenciesObjects(callExpr));
  }


  std::unique_ptr<SymbolicIfStmt> symbolizeIf(RawIfStatement* rawIf) {
    clang::IfStmt *stmt = rawIf->getIfStmt();

    llvm::outs() << "DEBUG: symbolizing if: ";
    stmt->dump();
    llvm::outs() << "\n";

    auto value = boolSymbolizer.symbolize(stmt->getCond());
    auto trueBranch = symbolizeCompound(rawIf->getTrueBody());
    auto falseBranch = symbolizeCompound(rawIf->getFalseBody());

    return std::make_unique<SymbolicIfStmt>(stmt, std::move(value), std::move(trueBranch), std::move(falseBranch));
  }

  std::unique_ptr<SymbolicCompound> symbolizeCompound(RawCompound *stmt) {
    auto result = std::make_unique<SymbolicCompound>();
    for (auto &s: stmt->getStmts()) {
      result->append(symbolizeStatement(s));
    }

    return result;
  }

  std::unique_ptr<SymbolicWhileStmt> symbolizeWhile(RawWhileStatement* rawWhile) {

    clang::WhileStmt *stmt = rawWhile->getWhileStmt();
    llvm::outs() << "DEBUG: symbolizing while: ";
    stmt->dump();
    llvm::outs() << "\n";

    auto symbolicWhile = std::make_unique<SymbolicWhileStmt>(stmt, boolSymbolizer.symbolize(stmt->getCond()), symbolizeCompound(rawWhile->getBody()));
    llvm::outs() << "SymbolizedWhile: ";
    symbolicWhile->print(llvm::outs());
    return symbolicWhile;
  }
  
  /* 
  Recursively adds the given statement to the control flow nodes of its parents and returns the highest level control flow parent.
  */
  RawStatement* constructParentControlFlow(clang::DynTypedNode node, RawStatement* raw) {

    // Stop the recursion if the function level has been reached.
    clang::FunctionDecl const *functionDecl = node.get<clang::FunctionDecl>();
    if (functionDecl != nullptr) {
      return raw;
    }
    
    clang::WhileStmt const *whileStmt = node.get<clang::WhileStmt>();
    if (whileStmt != nullptr) {
      llvm::outs() << "DEBUG FOUND WHILE!!!!";

      //construct RawWhile if not already built
      long whileID = whileStmt->getID(astContext);
      if (!whileMap.count(whileID)) {
        std::unique_ptr<RawWhileStatement> rs = std::unique_ptr<RawWhileStatement>(new RawWhileStatement(const_cast<clang::WhileStmt*>(whileStmt)));
        whileMap.emplace(whileID, new RawWhileStatement(const_cast<clang::WhileStmt*>(whileStmt)));
      }

      //Add to Body
      whileMap.at(whileID)->getBody()->append(raw);
      raw = whileMap[whileID]; // continue recursion with the parents of the while statement.
    }

    clang::IfStmt const *ifStmt = node.get<clang::IfStmt>();
    if (ifStmt != nullptr) {
      llvm::outs() << "DEBUG FOUND IF!!!!";

      //construct RawIf if not already built
      long ifID = ifStmt->getID(astContext);
      if (!ifMap.count(ifID)) {
        ifMap.emplace(ifID, new RawIfStatement(const_cast<clang::IfStmt*>(ifStmt)));
      }

      //Add to if or else branch
      if (ifStmt->getThen() == raw->getUnderlyingStmt() || stmtContainsStmt(ifStmt->getThen(), raw->getUnderlyingStmt())) { 
        llvm::outs() << "Debug: Add to then";
        ifMap.at(ifID)->getTrueBody()->append(raw);
        raw = ifMap[ifID];
      } else if (ifStmt->getElse() == raw->getUnderlyingStmt() || stmtContainsStmt(ifStmt->getElse(), raw->getUnderlyingStmt())) { 
        llvm::outs() << "Debug: Add to else";
        ifMap.at(ifID)->getFalseBody()->append(raw);
        raw = ifMap[ifID];
      } else if (ifStmt->getCond() == raw->getUnderlyingStmt() || stmtContainsStmt(ifStmt->getCond(), raw->getUnderlyingStmt())) { 
        llvm::outs() << "Debug: In condition, treat as outside of if";
      } else {
        llvm::outs() << "ERROR: raw is neither in then nor else! Raw: ";
        raw->getUnderlyingStmt()->dump();
        llvm::outs() << "\n IfStmt: ";
        ifStmt->dump();
        llvm::outs() << "\n";
        abort();
      }
      raw = ifMap[ifID];
    }
    RawStatement* result = raw;
    for (clang::DynTypedNode const parent : astContext.getParents(node)) {
      result = constructParentControlFlow(parent, raw);
    }

    return result;
  }

  RawStatement* constructParentControlFlow(RawStatement* raw) {
    auto node = clang::DynTypedNode::create(*(raw->getUnderlyingStmt()));
    return constructParentControlFlow(node, raw);
  }

  std::vector<RawStatement*> computeStatementOrder() {
    // unify all of the statements in this function
    std::vector<RawStatement*> unordered;
    for (auto *apiCall : apiCalls) {
      unordered.push_back(new RawRosApiCallStatement(apiCall));
    }
    for (auto *functionCall : functionCalls) {
      unordered.push_back(new RawFunctionCallStatement(functionCall));
    }
    for (auto *callback : callbacks) {
      unordered.push_back(new RawCallbackStatement(callback));
    }

    // create a mapping from underlying stmts
    std::vector<clang::Stmt*> unorderedClangStmts;
    std::unordered_map<clang::Stmt*, std::vector<RawStatement*>> clangToRawStmts;
    for (auto *rawStatement : unordered) {
      auto clangStmt = rawStatement->getUnderlyingStmt();
      if (clangToRawStmts.find(clangStmt) == clangToRawStmts.end()) {
        clangToRawStmts[clangStmt] = {};
      }
      unorderedClangStmts.push_back(clangStmt);
      clangToRawStmts[clangStmt].push_back(rawStatement);
    }

    // find the ordering of underlying stmts
    std::vector<RawStatement*> ordered;
    auto orderedClangStmts = StmtOrderingVisitor::computeOrder(astContext, function, unorderedClangStmts);
    for (auto *clangStmt : orderedClangStmts) {
      for (auto *rawStatement : clangToRawStmts[clangStmt]) {
        ordered.push_back(rawStatement);
      }
    }
 
    std::vector<RawStatement*> result;
    for (auto &rawStmt : ordered) { //In lexical order look for control flow parents and add them to result
      auto highestLevelParent = constructParentControlFlow(rawStmt);

      //avoid duplication in the result
      if (std::find(result.begin(), result.end(), highestLevelParent) == result.end()) {
        result.push_back(highestLevelParent); 
      }
    }
    
    //For compatibiliy, include the leaves for now. TODO: Fix rosdisover python to find leaves in control flow.
    ordered.insert( ordered.end(), result.begin(), result.end() );
    return ordered;
  }

  std::unique_ptr<SymbolicFunctionCall> symbolizeCallback(RawCallbackStatement *statement) {
    llvm::outs() << "DEBUG: symbolizing callback\n";
    if (statement == nullptr) {
      llvm::outs() << "ERROR: callback statement\n";
    }
    if (statement->getTargetFunction() == nullptr) {
      llvm::outs() << "ERROR: no target function\n";
    }
    llvm::outs() << "DEBUG: getting definition\n";
    auto *function = symContext.getDefinition(statement->getTargetFunction());
    if (function == nullptr) {
      llvm::outs() << "ERROR: target function definition not found\n";
    }
    llvm::outs() << "DEBUG: target function definition found\n";
    auto result = SymbolicFunctionCall::create(function);
    llvm::outs() << "DEBUG: symbolized callback\n";
    return result;
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
      case RawStatementKind::Callback:
        symbolic = symbolizeCallback((RawCallbackStatement*) statement);
        break;
      case RawStatementKind::If:
        symbolic = symbolizeIf(((RawIfStatement*) statement));
        break;
      case RawStatementKind::While:
        symbolic = symbolizeWhile(((RawWhileStatement*) statement));
        break;
      case RawStatementKind::Compound:
        symbolic = symbolizeCompound((RawCompound*) statement);
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
      compound->append(symbolizeStatement(rawStmt));
    }

    llvm::outs() << "Symbolized Function\n";

    symContext.define(function, std::move(compound));
  }
};

} // rosdiscover
