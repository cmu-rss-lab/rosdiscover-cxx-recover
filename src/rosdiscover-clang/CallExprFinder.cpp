#include <rosdiscover-clang/CallExprFinder.h>

#include <memory>
#include <unordered_map>

#include <clang/AST/ASTConsumer.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Analysis/CallGraph.h>

namespace rosdiscover {

class BuildCallGraphConsumer : public clang::ASTConsumer {
public:
  virtual void HandleTranslationUnit(clang::ASTContext &context) {
    visitor.TraverseDecl(context.getTranslationUnitDecl());
  }
private:
  clang::CallGraph visitor;
};

class BuildCallGraphAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &compiler,
    llvm::StringRef filename
  ) {
    return std::make_unique<BuildCallGraphConsumer>();
  }
};

std::unique_ptr<clang::tooling::FrontendActionFactory> buildCallGraphActionFactory() {
  class FrontendActionFactory : public clang::tooling::FrontendActionFactory {
  public:
    std::unique_ptr<clang::FrontendAction> create() override {
      return std::make_unique<BuildCallGraphAction>();
    }
  };

  return std::unique_ptr<clang::tooling::FrontendActionFactory>(
    new FrontendActionFactory()
  );
};

// std::unordered_map<llvm::FunctionDecl*, std::vector<llvm::CallExpr*>> CallExprFinder::find(clang::tooling::ClangTool &tool) {
void CallExprFinder::find(clang::tooling::ClangTool &tool) {
  std::unordered_map<clang::ASTContext*, clang::CallGraph*> translationUnitToCallGraph;
  tool.run(buildCallGraphActionFactory().get());
}

// class CallExprFinder::Finder : public clang::ast_matchers::MatchFinder::MatchCallback {
// public:
//   void run(const clang::ast_matchers::MatchFinder::MatchResult &result) {
//     if (auto *api_call = build(result)) {
//       found.push_back(api_call);
//     }
//   }
//
// protected:
//   Finder(std::unordered_map<llvm::FunctionDecl*, std::vector<llvm::CallExpr*>> &results) : results(results) {}
//
// private:
//   std::unordered_map<llvm::FunctionDecl*, std::vector<llvm::CallExpr*>> &results;
// };


} // rosdiscover
