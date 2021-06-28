#pragma once

#include <vector>

#include <clang/AST/Expr.h>


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

protected:
  RosApiCall(clang::CallExpr const *call) : call(call) {}

private:
  clang::CallExpr const *call;
}; // RosApiCall


} // rosdiscover::api_call
} // rosdiscover
