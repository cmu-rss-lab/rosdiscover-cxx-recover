#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::param::set
class BareSetParamCall : public BareRosApiCall {
public:
  BareSetParamCall(clang::CallExpr const *call) : BareRosApiCall(call) {}

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::param::set")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new BareSetParamCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
