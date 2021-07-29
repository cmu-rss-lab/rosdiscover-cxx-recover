#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class GetParamWithDefaultCall : public NodeHandleRosApiCall {
public:
  using NodeHandleRosApiCall::NodeHandleRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::GetParamWithDefaultCall;
  }

  clang::Expr const * getNameExpr() const override {
    return getCallExpr()->getArg(0);
  }

  clang::Expr const * getResultExpr() const override {
    auto *callExpr = getCallExpr();
    auto numArgs = callExpr->getNumArgs();

    // if this call has two arguments, then it simply returns the parameter value
    if (numArgs == 2) {
      return callExpr;

    // if the call has three arguments, then it writes the parameter value to a given reference
    } else if (numArgs == 3) {
      return callExpr->getArg(2);
    }

    assert(!"Expected NodeHandle::param call to have two or three arguments");
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("param"), ofClass(hasName("ros::NodeHandle"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new GetParamWithDefaultCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
