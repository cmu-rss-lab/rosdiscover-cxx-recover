#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class BareServiceCall : public BareRosApiCall {
public:
  using BareRosApiCall::BareRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::BareServiceCall;
  }

  clang::Expr const * getNameExpr() const override {
    return getCallExpr()->getArg(0);
  }

  std::string getServiceTypeName() const {
    return "TODO-BARE-SERVICE-CALL-TYPE-NAME";
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::service::call")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new BareServiceCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
