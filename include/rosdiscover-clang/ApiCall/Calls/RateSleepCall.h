#pragma once

#include <llvm/ADT/APInt.h>

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class RateSleepCall : public BareRosApiCall {
public:
  using BareRosApiCall::BareRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::RateSleepCall;
  }

  clang::Expr const * getNameExpr() const override {
    return getCallExpr()->getArg(2);
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("sleep"), ofClass(hasName("ros::Rate"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      auto *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
      return new RateSleepCall(call);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
