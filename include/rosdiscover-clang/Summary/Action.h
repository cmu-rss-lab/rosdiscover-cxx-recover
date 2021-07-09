#pragma once

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

namespace rosdiscover {
namespace summary {


class SummaryBuilderASTConsumer : public clang::ASTConsumer {
public:
  void HandleTranslationUnit(clang::ASTContext &context) override {
    // build a summary for each (unseen) function definition
    // NOTE: we may want to limit our attention to specific files
    llvm::outs() << "building summaries\n";
    auto *tu_decl = context.getTranslationUnitDecl();
    for (auto *decl : tu_decl->decls()) {
      if (auto *func_decl = dyn_cast<clang::FunctionDecl>(decl)) {
        llvm::outs() << "checking function: " << func_decl->getNameInfo().getAsString() << "\n";
      }
    }

    // find each ROS API call
    // - possibly use ASTMatcher
    // - but then use a silly visitor to get a mutable version of the same API call?

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
