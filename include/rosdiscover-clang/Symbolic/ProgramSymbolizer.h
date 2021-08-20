#pragma once

#include <assert.h>
#include <vector>

#include <clang/AST/ASTImporter.h>
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
    symbolizer.run();
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
    llvm::outs() << "built " << numAsts << " ASTs\n";
    assert(numAsts > 0);

    // we merge all top-level decls into the first translation unit in our list
    // - we could check which TU is the main file AST (via isMainFileAST),
    //    but I don't think this makes a difference?
    // - https://clang.llvm.org/docs/LibASTImporter.html
    // - https://clang.llvm.org/docs/InternalsManual.html#the-astimporter
    // - https://github.com/correctcomputation/checkedc-clang/issues/551
    clang::ASTUnit *toUnit = asts[0].get();
    for (auto i = 1; i < numAsts; ++i) {
      llvm::outs() << "importing decls from translation unit [" << i << "/" << numAsts - 1 << "]\n";
      clang::ASTUnit *fromUnit = asts[i].get();
      clang::ASTImporter importer(
        toUnit->getASTContext(),
        toUnit->getFileManager(),
        fromUnit->getASTContext(),
        fromUnit->getFileManager(),
        /*MinimalImport=*/false
      );
      for (
        auto top_level_iterator = fromUnit->top_level_begin(), top_level_end = fromUnit->top_level_end();
        top_level_iterator != top_level_end;
        top_level_iterator++
      ) {
        clang::Decl *fromDecl = *top_level_iterator;
        llvm::Expected<clang::Decl*> importedOrError = importer.Import(fromDecl);
      }
    }

    llvm::outs() << "successfully merged " << numAsts << " ASTs into a single AST for analysis\n";
    mergedAst = std::move(asts[0]);
  }

  void run() {
    buildAST();
    Symbolizer::symbolize(mergedAst->getASTContext(), program->getContext());
  }
};

} // rosdiscover::symbolic
} // rosdiscover
