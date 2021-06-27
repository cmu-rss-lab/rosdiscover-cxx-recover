#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>

#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

static llvm::cl::OptionCategory MyToolCategory("rosdiscover options");
static llvm::cl::extrahelp CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);

StatementMatcher InitMatcher = callExpr(
  isExpansionInMainFile(),
  callee(cxxMethodDecl(hasName("init")))
).bind("init");


class ApiCallFinder : public MatchFinder::MatchCallback
{
public:

  virtual void run(const MatchFinder::MatchResult &result) override {
    auto *call = result.Nodes.getNodeAs<clang::CallExpr>("init");
    llvm::outs() << "found ros::init call: ";
    call->dumpColor();
    llvm::outs() << "\n";
  }

}; // ApiCallFinder


int main(int argc, const char **argv) {
  CommonOptionsParser optionsParser(argc, argv, MyToolCategory);

  // TODO avoid the need to run things via ClangTool
  ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());

  MatchFinder finder;

  // TODO use an anonymous function?
  auto init_matcher = ApiCallFinder();
  finder.addMatcher(InitMatcher, &init_matcher);

  // TODO use MatchFinder::matchAST(ASTContext &)

  llvm::outs() << "finding ROS API calls...\n";
  auto res = tool.run(newFrontendActionFactory(&finder).get());
  return res;
}
