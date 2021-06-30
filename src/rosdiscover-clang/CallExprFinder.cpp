#include <rosdiscover-clang/CallExprFinder>

namespace rosdiscover {

std::unordered_map<llvm::FunctionDecl*, std::vector<llvm::CallExpr*>> CallExprFinder::find(clang::tooling::ClangTool &tool) {
  // TODO!
}

class CallExprFinder::Finder : public clang::ast_matchers::MatchFinder::MatchCallback {
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


} // rosdiscover
