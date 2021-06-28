#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class RosInitCall : public BareRosApiCall {
public:
  RosInitCall(clang::CallExpr const *call) : BareRosApiCall(call) {}

  clang::Expr const * getNameExpr() const {
    return getCallExpr()->getArg(0);
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::init")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new RosInitCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
