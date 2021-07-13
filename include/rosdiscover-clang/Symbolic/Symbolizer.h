#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/Analysis/CallGraph.h>

#include <llvm/Support/raw_ostream.h>

#include <rosdiscover-clang/ApiCall/RosApiCall.h>
#include <rosdiscover-clang/ApiCall/Finder.h>
#include <rosdiscover-clang/utils.h>

namespace rosdiscover {
namespace symbolic {

/** limitation: operates over a single translation unit for now */
class Symbolizer {
public:
  // todo: return SymbolicContext
  static void symbolize(clang::ASTContext &astContext) {
    Symbolizer(astContext).run();
  }

private:
  Symbolizer(clang::ASTContext &astContext)
    : astContext(astContext),
      callGraph(),
      apiCalls(),
      functionToApiCalls(),
      relevantFunctions(),
      relevantFunctionCalls()
  {}

  clang::ASTContext &astContext;
  clang::CallGraph callGraph;
  std::vector<api_call::RosApiCall *> apiCalls;
  std::unordered_map<clang::FunctionDecl const *, std::vector<api_call::RosApiCall *>> functionToApiCalls;
  std::unordered_set<clang::FunctionDecl const *> relevantFunctions;
  std::unordered_map<clang::FunctionDecl const *, std::vector<clang::Expr *>> relevantFunctionCalls;

  /** Constructs the call graph */
  void buildCallGraph() {
    callGraph.addToCallGraph(astContext.getTranslationUnitDecl());
  }

  /** Finds all direct ROS API calls */
  void findRosApiCalls() {
    apiCalls = api_call::RosApiCallFinder::find(astContext);

    // group API calls by parent function
    for (auto *call : apiCalls) {
      auto *functionDecl = getParentFunctionDecl(astContext, call->getCallExpr());
      functionDecl = functionDecl->getCanonicalDecl();
      if (functionDecl == nullptr) {
        llvm::errs() << "failed to determine parent function for ROS API call\n";
      }

      if (functionToApiCalls.find(functionDecl) == functionToApiCalls.end()) {
        functionToApiCalls.emplace(functionDecl, std::vector<api_call::RosApiCall *>());
      }
      functionToApiCalls[functionDecl].push_back(call);
    }

    for (auto const &entry : functionToApiCalls) {
      llvm::outs() << "ROS API calls found in function: " << entry.first->getQualifiedNameAsString() << "\n";
    }
  }

  /** Computes the set of architecturally-relevant functions */
  void findRelevantFunctions() {
    llvm::outs() << "computing relevant functions...\n";
    llvm::outs() << "determining function callers\n";
    auto functionToCallers = findCallers(callGraph);
    llvm::outs() << "determined function callers\n";

    std::queue<clang::FunctionDecl const *> queue;
    for (auto const &entry : functionToApiCalls) {
      queue.push(entry.first);
    }

    while (!queue.empty()) {
      auto const *function = queue.front();
      relevantFunctions.insert(function);
      queue.pop();

      for (auto caller : functionToCallers[function]) {
        if (relevantFunctions.find(caller) == relevantFunctions.end())
          queue.push(caller);
      }
    }

    for (auto const *function : relevantFunctions) {
      llvm::outs() << "found relevant function: " << function->getQualifiedNameAsString() << "\n";
    }
  }

  void findRelevantFunctionCalls() {
    // look at all function calls within the set of relevant functions
    for (auto const *caller : relevantFunctions) {
      relevantFunctionCalls.emplace(caller, std::vector<clang::Expr *>());
      auto *callerNode = callGraph.getNode(caller);
      for (clang::CallGraphNode::CallRecord const &callRecord : *callerNode) {
        // is this a call to another relevant function?
        auto const *callee = dyn_cast<clang::FunctionDecl>(callRecord.Callee->getDecl())->getCanonicalDecl();
        if (relevantFunctions.find(callee) != relevantFunctions.end())
          relevantFunctionCalls[caller].push_back(callRecord.CallExpr);
      }

      llvm::outs()
        << caller->getQualifiedNameAsString()
        << ": found "
        << relevantFunctionCalls[caller].size()
        << " relevant function calls\n";

      for (auto call : relevantFunctionCalls[caller]) {
        call->dumpColor();
        llvm::outs() << "\n";
      }
    }
  }

  // void symbolize(clang::FunctionDecl const *function, SymbolicContext &symContext);

  void run() {
    buildCallGraph();
    findRosApiCalls();
    findRelevantFunctions();
    findRelevantFunctionCalls();

    // symbolize each relevant function
    // for (auto const *function : relevantFunctions) {

    // }
  }
};

// find all calls to the relevant functions
//
// - list of ROS API calls
// - list of function calls
// - convert each to a SymbolicStmt
// - fold into a SymbolicCompound
// - emit a SymbolicFunction


} // rosdiscover::symbolic
} // rosdiscover