#pragma once

#include <queue>
#include <unordered_set>

#include <clang/Analysis/CallGraph.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#include <rosdiscover-clang/ApiCall/RosApiCall.h>
#include <rosdiscover-clang/ApiCall/Finder.h>
#include <rosdiscover-clang/Name/Symbolizer.h>
#include <rosdiscover-clang/Symbolic/Symbolizer.h>

namespace rosdiscover {
namespace summary {


//class FunctionSymbolizer {
//
//  // find all of the ROS API calls within this function
//
//};


/**
 * LIMITATION: this does not CURRENTLY provide CTU results
 *
 * NOTES:
 * * we may want to limit our attention to specific files
 */

class SummaryBuilderASTConsumer : public clang::ASTConsumer {
public:

  void HandleTranslationUnit(clang::ASTContext &context) override {
    symbolic::Symbolizer::symbolize(context);

    // UnsymbolizedFunction: API calls + relevant function calls
    // SymbolizedFunction: API calls [SymbolicRosApiCall] + relevant function calls
    // FunctionSummary: conditional API calls [SymbolicConditionalRosApiCall] + conditional function calls

    // build a symbolizer
    // - turn this into a FunctionSymbolizer
    // - first step: find the ROS API calls within this function
    // - second step: symbolize those ROS API calls
    // - third step: compute the path condition for each ROS API call
    // auto symbolizer = name::NameSymbolizer(context);


    // BasicFunctionSummary
    // --------------------
    // - ApiCall -> ConditionalApiCall
    // --------------------
    // - API calls [path condition]
    // - non-API calls


    // - parameter read [w/ default]

    // - group ROS API calls by their parent function decl
    // - we need to symbolize the entire group at once

    // - write a path condition builder
    //    - accepts a Clang stmt
    //    - produces a symbolic path condition in terms of formals

  }
};


class SummaryBuilderAction : public clang::ASTFrontendAction {
public:
  // TODO use pass-by-reference to store summaries
  static std::unique_ptr<clang::tooling::FrontendActionFactory> factory() {
    class Factory : public clang::tooling::FrontendActionFactory {
    public:
      std::unique_ptr<clang::FrontendAction> create() override {
        llvm::outs() << "building action...\n";
        return std::unique_ptr<clang::FrontendAction>(new SummaryBuilderAction());
      }
    };

    return std::unique_ptr<clang::tooling::FrontendActionFactory>(new Factory());
  }

  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer (
    clang::CompilerInstance &compiler,
    llvm::StringRef filename
  ) override {
    llvm::outs() << "building consumer...\n";
    return std::unique_ptr<clang::ASTConsumer>(
      new SummaryBuilderASTConsumer()
    );
  }
};


} // rosdiscover::sumary
} // rosdiscover
