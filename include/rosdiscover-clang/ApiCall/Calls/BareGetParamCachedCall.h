#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class BareGetParamCachedCall : public BareRosApiCall {
public:
  BareGetParamCachedCall(clang::CallExpr const *call) : BareRosApiCall(call) {}

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::param::getCached")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new BareGetParamCachedCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
