#pragma once

#include <clang/AST/ASTContext.h>

#include "../RosApiCall.h"
#include "./Util.h"

namespace rosdiscover {
namespace api_call {

class SendTransformCall : public BareRosApiCall {
public:
  using BareRosApiCall::BareRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::PublishCall;
  }

  const std::string getPublisherName(clang::ASTContext &astContext) const {
    return "tf_broadcaster";
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("sendTransform"), ofClass(hasName("tf::TransformBroadcaster"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      auto *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
      return new SendTransformCall(call);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
