#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class BareGetParamCall : public BareRosApiCall {
public:
  BareGetParamCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : BareRosApiCall(call, context)
  {}

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::BareGetParamCall;
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
        callee(functionDecl(hasName("ros::param::get")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new BareGetParamCall(result.Nodes.getNodeAs<clang::CallExpr>("call"), result.Context);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
