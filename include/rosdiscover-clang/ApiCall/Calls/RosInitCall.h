#pragma once

#include <llvm/ADT/APInt.h>

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class RosInitCall : public BareRosApiCall {
public:
  using BareRosApiCall::BareRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::RosInitCall;
  }

  clang::Expr const * getNameExpr() const override {
    return getCallExpr()->getArg(2);
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
      auto *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
      return new RosInitCall(call);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
