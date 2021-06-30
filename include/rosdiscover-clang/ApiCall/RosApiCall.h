#pragma once

#include <vector>

#include <clang/AST/Expr.h>
#include <clang/AST/PrettyPrinter.h>
#include <llvm/Support/raw_ostream.h>

namespace rosdiscover {
namespace api_call {

// TODO differentiate between Bare and NodeHandle API calls

class RosApiCall {
public:
  clang::CallExpr const * getCallExpr() const { return call; }

  class Finder : public clang::ast_matchers::MatchFinder::MatchCallback {
  public:
    void run(const clang::ast_matchers::MatchFinder::MatchResult &result) {
      if (auto *api_call = build(result)) {
        found.push_back(api_call);
      }
    }

    virtual const clang::ast_matchers::StatementMatcher getPattern() = 0;

  protected:
    Finder(std::vector<RosApiCall*> &found) : found(found) {}

    virtual RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) = 0;

  private:
    std::vector<RosApiCall*> &found;
  };

//  virtual void print(llvm::raw_ostream &os, clang::SourceManager const &sourceManager) const {
  virtual void print(llvm::raw_ostream &os) const {
    static clang::LangOptions langOptions;
    static clang::PrintingPolicy printPolicy(langOptions);
    getCallExpr()->printPretty(os, nullptr, printPolicy);
    os << " [" << locationString << "]";
  }

protected:
  RosApiCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : call(call),
      context(context),
      locationString(call->getBeginLoc().printToString(context->getSourceManager()))
  {}

private:
  clang::CallExpr const *call;
  clang::ASTContext const *context;
  std::string const locationString;
}; // RosApiCall


class BareRosApiCall : public RosApiCall {
protected:
  BareRosApiCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : RosApiCall(call, context)
  {}
}; // BareRosApiCall


class NodeHandleRosApiCall : public RosApiCall {
protected:
  NodeHandleRosApiCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : RosApiCall(call, context)
  {}
}; // NodeHandleRosApiCall


} // rosdiscover::api_call
} // rosdiscover
