#pragma once

#include <assert.h>
#include <string>
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
      llvm::ArrayRef<std::string> sourcePaths,
      std::vector<std::string> &restrictAnalysisToPaths
  ) {
    auto symbolizer = ProgramSymbolizer(
      compilationDatabase,
      sourcePaths,
      restrictAnalysisToPaths
    );
    symbolizer.run();
    return std::move(symbolizer.program);
  }

private:
  clang::tooling::ClangTool tool;
  std::unique_ptr<SymbolicProgram> program;
  std::unique_ptr<clang::ASTUnit> mergedAst;
  std::vector<std::string> &restrictAnalysisToPaths;

  ProgramSymbolizer(
      clang::tooling::CompilationDatabase const &compilationDatabase,
      llvm::ArrayRef<std::string> sourcePaths,
      std::vector<std::string> &restrictAnalysisToPaths
  ) : tool(compilationDatabase, sourcePaths),
      program(std::make_unique<SymbolicProgram>()),
      restrictAnalysisToPaths(restrictAnalysisToPaths)
  {
    tool.setDiagnosticConsumer(new clang::IgnoringDiagConsumer());
  }

  void buildAST() {
    // build the AST for each translation unit
    llvm::outs() << "building ASTs..\n";
    std::vector<std::unique_ptr<clang::ASTUnit>> asts;
    tool.buildASTs(asts);
    size_t numAsts = asts.size();
    llvm::outs() << "built " << numAsts << " ASTs\n";
    assert(numAsts > 0);

    // the same source file may be compiled multiple times for different CMake build targets
    // Clang doesn't know that, so, by default, it will include multiple copies of the same
    // AST, which results in a failed merging process
    std::set<std::string> representedSourceFiles;
    std::vector<std::unique_ptr<clang::ASTUnit>> deduplicatedAsts;
    for (auto i = 0; i < numAsts; i++) {
      auto sourceFileName = asts[i].get()->getOriginalSourceFileName().str();
      llvm::outs() << "AST[" << i << "]: " << sourceFileName << "\n";

      if (representedSourceFiles.find(sourceFileName) != representedSourceFiles.end()) {
        llvm::outs() << "ignoring duplicate AST: " << sourceFileName << "\n";
      } else {
        representedSourceFiles.insert(sourceFileName);
        deduplicatedAsts.push_back(std::move(asts[i]));
      }
    }

    llvm::outs() << "obtained " << deduplicatedAsts.size() << " deduplicated ASTs\n";
    for (auto i = 0; i < deduplicatedAsts.size(); i++) {
      auto sourceFileName = deduplicatedAsts[i].get()->getOriginalSourceFileName().str();
      llvm::outs() << "deduplicated AST[" << i << "]: " << sourceFileName << "\n";
    }
    assert(deduplicatedAsts.size());
    asts = std::move(deduplicatedAsts);
    numAsts = asts.size();

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
      llvm::outs() << "DEBUG: constructed AST importer\n";
      for (
        auto top_level_iterator = fromUnit->top_level_begin(), top_level_end = fromUnit->top_level_end();
        top_level_iterator != top_level_end;
        top_level_iterator++
      ) {
        // ISSUE: we sometimes try to re-import existing definitions;
        // we just want to skip those!
        clang::Decl *fromDecl = *top_level_iterator;
        llvm::Expected<clang::Decl*> importedOrError = importer.Import(fromDecl);
        if (!importedOrError) {
          llvm::Error error = importedOrError.takeError();
          llvm::errs()
            << "WARNING: error when attemping to merge decl ["
            << toString(std::move(error))
            << "]\n";
          // fromDecl->dump();
          // llvm::errs() << "\n";
          // abort();
        }
      }
    }

    llvm::outs() << "successfully merged " << numAsts << " ASTs into a single AST for analysis\n";
    mergedAst = std::move(asts[0]);
  }

  void run() {
    buildAST();
    Symbolizer::symbolize(
      mergedAst->getASTContext(),
      program->getContext(),
      restrictAnalysisToPaths
    );
  }
};

} // rosdiscover::symbolic
} // rosdiscover
