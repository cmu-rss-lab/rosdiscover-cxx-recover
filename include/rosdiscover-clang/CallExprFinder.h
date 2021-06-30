#pragma once

#include <unordered_map>
#include <vector>

namespace rosdiscover {

/**
 * Produces a mapping from each function within a given program to the call
 * expressions contained within that function.
 */

class CallExprFinder {
public:
  static std::unordered_map<llvm::FunctionDecl*, std::vector<llvm::CallExpr*>> find(clang::tooling::ClangTool &tool) {
    return RosApiCallFinder(tool).run();
  }

private:

  class Finder : public clang::ast_matchers::MatchFinder::MatchCallback {
  public:
    void run(const clang::ast_matchers::MatchFinder::MatchResult &result) {
      if (auto *api_call = build(result)) {
        found.push_back(api_call);
      }
    }

  protected:
    Finder(std::unordered_map<llvm::FunctionDecl*, std::vector<llvm::CallExpr*>> &results) : results(results) {}

  private:
    std::unordered_map<llvm::FunctionDecl*, std::vector<llvm::CallExpr*>> &results;
  };

}; // rosdiscover::CallExprFinder


} // rosdiscover
