#pragma once

#include <vector>

#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace rosdiscover {
namespace api_call {


class RosApiCallFinder {
public:

  static std::vector<RosApiCall*> find(clang::ClangTool &tool) {
    auto finder = RosApiCallFinder(tool);
  }

private:
  RosApiCallFinder() : calls() {}

  void build() {
    RosInit::addToFinder(this);
    // matchFinder.addMatcher(RosInit::PATTERN, RosInit::Finder(calls));
  }

  /** Finds all API calls within a program described by a clang tool. */
  void run(clang::ClangTool &tool) {

  }

  /** Finds all API calls within a given AST context. */
  void run(clang::ASTContext &context) {
    clang::ast_matchers::MatchFinder finder;

    // TODO use an anonymous function?
    auto init_matcher = ApiCallFinder();
    finder.addMatcher(InitMatcher, &init_matcher);

    // TODO use MatchFinder::matchAST(ASTContext &)

    llvm::outs() << "finding ROS API calls...\n";
    auto res = tool.run(newFrontendActionFactory(&finder).get());

  }

private:
  clang::ast_matchers::MatchFinder matchFinder;
  std::vector<RosApiCall*> calls;
};


} // rosdiscover::api_call
} // rosdiscover
