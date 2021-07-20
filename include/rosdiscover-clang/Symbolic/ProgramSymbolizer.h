#pragma once

#include <assert.h>
#include <vector>

#include <clang/Tooling/Tooling.h>

#include "Program.h"
#include "Symbolizer.h"

namespace rosdiscover {
namespace symbolic {

class ProgramSymbolizer {
public:
  static std::unique_ptr<SymbolicProgram> symbolize(
      clang::tooling::CompilationDatabase const &compilationDatabase,
      llvm::ArrayRef<std::string> sourcePaths
  ) {
    auto symbolizer = ProgramSymbolizer(compilationDatabase, sourcePaths);
    return std::move(symbolizer.program);
  }

private:
  clang::tooling::ClangTool tool;
  std::unique_ptr<SymbolicProgram> program;
  std::unique_ptr<clang::ASTUnit> mergedAst;

  ProgramSymbolizer(
      clang::tooling::CompilationDatabase const &compilationDatabase,
      llvm::ArrayRef<std::string> sourcePaths
  ) : tool(compilationDatabase, sourcePaths),
      program(std::make_unique<SymbolicProgram>())
  {}

  void buildAST() {
    // build the AST for each translation unit
    std::vector<std::unique_ptr<clang::ASTUnit>> asts;
    tool.buildASTs(asts);
    size_t numAsts = asts.size();
    assert(numAsts > 0);

    // we're going to merge everything into the first translation unit in our list
    // clang::ASTUnit toUnit = asts[0].get();

    // TODO merge the ASTs into a single translation unit
    // https://clang.llvm.org/docs/LibASTImporter.html
    // https://clang.llvm.org/docs/InternalsManual.html#the-astimporter
    // https://github.com/correctcomputation/checkedc-clang/issues/551

    mergedAst = std::move(asts[0]);
  }

  void run() {
    buildAST();
    Symbolizer::symbolize(mergedAst->getASTContext(), program->getContext());
  }
};

} // rosdiscover::symbolic
} // rosdiscover
