#include <iostream>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Rewrite/Core/Rewriter.h>

#include <rosdiscover-clang/Symbolic/ProgramSymbolizer.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace rosdiscover::symbolic;

static llvm::cl::OptionCategory MyToolCategory("rosdiscover options");
static llvm::cl::extrahelp CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);

int main(int argc, const char **argv) {
  CommonOptionsParser optionsParser(argc, argv, MyToolCategory);

  auto program = ProgramSymbolizer::symbolize(optionsParser.getCompilations(), optionsParser.getSourcePathList());
  auto json = program->toJson();
  std::cout << std::setw(2) << json;

  // TODO avoid the need to run things via ClangTool
  // https://eli.thegreenplace.net/2012/06/08/basic-source-to-source-transformation-with-clang
  // ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());

  // TODO build and merge ASTs
  // - raise an error if building or merging failed
  // - buildASTs(&asts)

  // build some function summaries
  // auto action = rosdiscover::summary::SummaryBuilderAction::factory();
  // tool.run(action.get());
  return 0;
}
