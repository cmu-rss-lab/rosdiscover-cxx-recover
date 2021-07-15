#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class GetParamCall : public NodeHandleRosApiCall {
public:
  GetParamCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : NodeHandleRosApiCall(call, context)
  {}

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::GetParamCall;
  }

  clang::Expr const * getNameExpr() const override {
    return getCallExpr()->getArg(0);
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("getParam"), ofClass(hasName("ros::NodeHandle"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new GetParamCall(result.Nodes.getNodeAs<clang::CallExpr>("call"), result.Context);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
