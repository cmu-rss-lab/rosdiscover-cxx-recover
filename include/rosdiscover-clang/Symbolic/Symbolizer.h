#pragma once

#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/Analysis/CallGraph.h>

#include <llvm/Support/raw_ostream.h>

#include "../ApiCall/Finder.h"
#include "../ApiCall/RosApiCall.h"
#include "../Helper/utils.h"
#include "../Callback/Callback.h"
#include "Context.h"
#include "Function.h"
#include "FunctionSymbolizer.h"

namespace rosdiscover {
namespace symbolic {

class Symbolizer {
public:
  static void symbolize(
    clang::ASTContext &astContext,
    SymbolicContext &symContext,
    std::vector<std::string> &restrictAnalysisToPaths
  ) {
    Symbolizer(astContext, symContext, restrictAnalysisToPaths).run();
  }

private:
  Symbolizer(
    clang::ASTContext &astContext,
    SymbolicContext &symContext,
    std::vector<std::string> &restrictAnalysisToPaths
  )
    : symContext(symContext),
      astContext(astContext),
      restrictAnalysisToPaths(restrictAnalysisToPaths),
      callGraph(),
      apiCalls(),
      callbacks(),
      functionToCallbacks(),
      relevantCallbacks(),
      functionToApiCalls(),
      relevantFunctions(),
      relevantFunctionCalls(),
      relevantFunctionNames(),
      astFunctionToSymbolic()
  {}

  SymbolicContext &symContext;
  clang::ASTContext &astContext;
  std::vector<std::string> &restrictAnalysisToPaths;
  clang::CallGraph callGraph;
  std::vector<api_call::RosApiCall *> apiCalls;
  std::vector<Callback*> callbacks;
  std::unordered_map<clang::FunctionDecl const *, std::vector<Callback*>> functionToCallbacks;
  std::unordered_map<clang::FunctionDecl const *, std::vector<Callback*>> relevantCallbacks;
  std::unordered_map<clang::FunctionDecl const *, std::vector<api_call::RosApiCall *>> functionToApiCalls;
  std::unordered_set<clang::FunctionDecl const *> relevantFunctions;
  std::unordered_map<clang::FunctionDecl const *, std::vector<clang::Expr *>> relevantFunctionCalls;
  [[maybe_unused]] std::unordered_set<std::string> relevantFunctionNames;

  // TODO instead use AnnotatedFunctionDecl and AnnotatedContext
  std::unordered_map<clang::FunctionDecl const*, SymbolicFunction*> astFunctionToSymbolic;

  /** Constructs the call graph */
  void buildCallGraph() {
    callGraph.addToCallGraph(astContext.getTranslationUnitDecl());
  }

  /** Finds all callbacks from ROS API calls that can be statically resolved */
  void findCallbacks() {
    for (auto *call : apiCalls) {
      auto *callback = call->getCallback(astContext);
      if (callback != nullptr) {
        llvm::outs() << "DEBUG: registering callback: ";
        callback->print(llvm::outs());
        llvm::outs() << "\n";
        callbacks.push_back(callback);
      }
    }
  }

  /** Finds all direct ROS API calls */
  void findRosApiCalls() {
    llvm::outs() << "DEBUG: finding ROS API calls...\n";
    apiCalls = api_call::RosApiCallFinder::find(astContext);
    llvm::outs() << "DEBUG: found ROS API calls\n";

    // group API calls by parent function
    llvm::outs() << "DEBUG: grouping ROS API calls by parent function...\n";
    for (auto *call : apiCalls) {
      auto *expr = call->getExpr();
      llvm::outs() << "DEBUG: examining API call...\n";
      auto *functionDecl = getParentFunctionDecl(astContext, expr);
      // llvm::outs() << "DEBUG: parent function for API call: ";
      // functionDecl->dump();
      // llvm::outs() << "\n";
      // functionDecl = functionDecl->getCanonicalDecl();
      llvm::outs() << "DEBUG: found canonical definition for parent function\n";
      if (functionDecl == nullptr) {
        llvm::errs() << "failed to determine parent function for ROS API call\n";
        continue;
      }

      // check to see whether this API call takes place in a file
      // that we're allowed to analyze
      if (!restrictAnalysisToPaths.empty()) {
        llvm::outs() << "DEBUG: finding name of file that API calls appears in...\n";
        auto filename = clang::FullSourceLoc(
          expr->getBeginLoc(),
          astContext.getSourceManager()
        ).getFileEntry()->getName().str();
        llvm::outs() << "DEBUG: API call belongs to file: " << filename << "\n";

        bool ignoreFile = true;
        for (auto const &allowedPath : restrictAnalysisToPaths) {
          if (starts_with(filename, allowedPath)) {
            ignoreFile = false;
            break;
          }
        }

        if (ignoreFile) {
          llvm::outs() << "ignoring API call in: " << filename << "\n";
          continue;
        }
      }

      if (functionToApiCalls.find(functionDecl) == functionToApiCalls.end()) {
        functionToApiCalls.emplace(functionDecl, std::vector<api_call::RosApiCall *>());
      }
      functionToApiCalls[functionDecl].push_back(call);
    }
    llvm::outs() << "grouped ROS API calls by parent function\n";

    for (auto const &entry : functionToApiCalls) {
      llvm::outs()
        << "ROS API calls found in function: "
        << entry.first->getQualifiedNameAsString()
        << " ["
        << &(*(entry.first))
        << "]\n";
    }
  }

  /** Computes the set of architecturally-relevant functions */
  void findRelevantFunctions() {
    llvm::outs() << "computing relevant functions...\n";
    llvm::outs() << "determining function callers\n";
    auto functionToCallers = findCallers(callGraph);
    llvm::outs() << "determined function callers\n";

    // handle callbacks
    for (auto *callback : callbacks) {
      auto *parentFunction = callback->getParentFunction();
      auto *targetFunction = callback->getTargetFunction();
      // FIXME the target function MAY be different
      functionToCallers[parentFunction].insert(targetFunction);
    }

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
      auto name = function->getQualifiedNameAsString();
      relevantFunctionNames.insert(name);
      llvm::outs() << "found relevant function: " << name << "\n";
    }

    llvm::outs() << "finished finding all relevant functions\n";
  }

  void findRelevantCallbacks() {
    for (auto *callback : callbacks) {
      auto *parentFunction = callback->getParentFunction();
      auto *targetFunction = callback->getTargetFunction();
      if (relevantFunctions.find(targetFunction) != relevantFunctions.end()) {
        llvm::outs() << "DEBUG: callback is relevant: ";
        callback->print(llvm::outs());
        llvm::outs() << "\n";
        if (relevantCallbacks.find(parentFunction) == relevantCallbacks.end()) {
          relevantCallbacks[parentFunction] = {};
        }
        relevantCallbacks[parentFunction].push_back(callback);
      } else {
        llvm::outs()
          << "DEBUG: callback deemed irrelevant: "
          << targetFunction->getQualifiedNameAsString()
          << "\n";
      }
    }
  }

  void findRelevantFunctionCalls() {
    // look at all function calls within the set of relevant functions
    for (auto const *caller : relevantFunctions) {
      llvm::outs()
        << "finding all calls to relevant function: "
        << caller->getQualifiedNameAsString()
        << "\n";
      relevantFunctionCalls.emplace(caller, std::vector<clang::Expr *>());
      auto *callerNode = callGraph.getNode(caller);

      if (callerNode == nullptr) {
        llvm::errs()
          << "Call graph node is missing for relevant function ["
          << caller->getQualifiedNameAsString()
          << "]. Trying to use canonical declaration as a workaround. \n";

        callerNode = callGraph.getNode(caller->getCanonicalDecl());
        if (callerNode == nullptr) {
          llvm::errs() << "Canonical decl workaround didn't work\n";
          continue;
        } else {
          llvm::outs() << "DEBUG: canonical workaround worked!\n";
        }
      }
      llvm::outs() << "-> fetched call graph node\n";

      for (clang::CallGraphNode::CallRecord const &callRecord : *callerNode) {
        // is this a call to another relevant function?
        auto const *callee = clang::dyn_cast<clang::FunctionDecl>(callRecord.Callee->getDecl());
        // auto const *canonicalCallee = callee->getCanonicalDecl();
        auto calleeName = callee->getQualifiedNameAsString();
        llvm::outs()
          << "DEBUG: CHECKING IF CALL IS RELEVANT [dest: "
          << calleeName
          << "\n";

        if (relevantFunctionNames.find(calleeName) != relevantFunctionNames.end()) {
          llvm::outs() << "DEBUG: MATCH\n";
          relevantFunctionCalls[caller].push_back(callRecord.CallExpr);
        } else {
          llvm::outs() << "DEBUG: NO MATCH\n";
        }

        // if (relevantFunctions.find(callee) != relevantFunctions.end()) {
        //   llvm::outs() << "DEBUG: FOUND NON-CANONICAL MATCH\n";
        //   relevantFunctionCalls[caller].push_back(callRecord.CallExpr);
        // } else if (relevantFunctions.find(canonicalCallee) != relevantFunctions.end()) {
        //   llvm::outs() << "DEBUG: FOUND CANONICAL MATCH\n";
        //   relevantFunctionCalls[caller].push_back(callRecord.CallExpr);
        // } else {
        //   llvm::outs() << "DEBUG: NO MATCH\n";
        // }
      }

      llvm::outs()
        << caller->getQualifiedNameAsString()
        << ": found "
        << relevantFunctionCalls[caller].size()
        << " relevant function calls\n";

      // attempting to dump certain function calls sometimes leads to a crash?
      // for (auto call : relevantFunctionCalls[caller]) {
      //   call->dumpColor();
      //   llvm::outs() << "\n";
      // }
    }

    llvm::outs() << "finished finding all relevant functions calls\n";
  }

  void symbolize(clang::FunctionDecl const *function) {
    llvm::outs()
      << "symbolizing function: "
      << function->getQualifiedNameAsString()
      << "\n";
    auto *symFunction = astFunctionToSymbolic[function];
    auto &apiCalls = functionToApiCalls[function];
    auto &functionCalls = relevantFunctionCalls[function];
    auto &callbacks = relevantCallbacks[function];

    llvm::outs()
      << "using " << callbacks.size() << " relevant callbacks during symbolization\n";

    FunctionSymbolizer::symbolize(
        astContext,
        symContext,
        *symFunction,
        function,
        apiCalls,
        functionCalls,
        callbacks
    );
    llvm::outs()
      << "symbolized function: "
      << function->getQualifiedNameAsString()
      << "\n";
  }

  void run() {
    buildCallGraph();
    findRosApiCalls();
    findCallbacks();
    findRelevantFunctions();
    findRelevantFunctionCalls();
    findRelevantCallbacks();

    // declare all of the relevant functions
    llvm::outs() << "declaring symbolic functions\n";
    for (auto const *function : relevantFunctions) {
      astFunctionToSymbolic.emplace(function, symContext.declare(astContext, function));
    }
    llvm::outs() << "declared symbolic functions\n";

    // produce initial definitions for each function
    llvm::outs() << "obtaining symbolic function definitions...\n";
    for (auto const *function : relevantFunctions)
      symbolize(function);
    llvm::outs() << "obtained symbolic function definitions...\n";

    symContext.print(llvm::outs());
    llvm::outs() << "\n";
  }
};

} // rosdiscover::symbolic
} // rosdiscover
