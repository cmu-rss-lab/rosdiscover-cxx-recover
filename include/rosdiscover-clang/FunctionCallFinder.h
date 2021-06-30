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
}; // rosdiscover::CallExprFinder


} // rosdiscover
