#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class SubscribeTopicCall : public NodeHandleRosApiCall {
public:
  SubscribeTopicCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : NodeHandleRosApiCall(call, context)
  {}

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("subscribe"), ofClass(hasName("ros::NodeHandle"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new SubscribeTopicCall(result.Nodes.getNodeAs<clang::CallExpr>("call"), result.Context);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
