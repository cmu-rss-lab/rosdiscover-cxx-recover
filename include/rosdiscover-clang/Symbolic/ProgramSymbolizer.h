#pragma once

#include <clang/Tooling/Tooling.h>

#include "Program.h"

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

  ProgramSymbolizer(
      clang::tooling::CompilationDatabase const &compilationDatabase,
      llvm::ArrayRef<std::string> sourcePaths
  ) : tool(compilationDatabase, sourcePaths),
      program(std::make_unique<SymbolicProgram>())
  {}
};

} // rosdiscover::symbolic
} // rosdiscover
