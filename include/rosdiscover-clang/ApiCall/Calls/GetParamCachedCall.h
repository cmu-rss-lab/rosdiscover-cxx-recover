#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeHandle::getParamCached
// ros::NodeHandle::getCached
class GetParamCachedCall : public NodeHandleRosApiCall {
public:
  GetParamCachedCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : NodeHandleRosApiCall(call, context)
  {}

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::GetParamCachedCall;
  }

  clang::Expr const * getNameExpr() const override {
    return getCallExpr()->getArg(0);
  }

  clang::Expr const * getResultExpr() const override {
    return getCallExpr()->getArg(1);
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(anyOf(hasName("getParamCached"), hasName("getCached")), ofClass(hasName("ros::NodeHandle"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new GetParamCachedCall(result.Nodes.getNodeAs<clang::CallExpr>("call"), result.Context);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
