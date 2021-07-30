#pragma once

#include "../../Helper/FormatHelper.h"
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

  std::string getFormatName() const {
    return typeNameToFormatName(getServiceTypeName());
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

private:
  std::string getServiceTypeName() const {
    return "todo::service";
  }
};

} // rosdiscover::api_call
} // rosdiscover
