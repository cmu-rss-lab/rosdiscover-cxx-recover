#pragma once

#include <clang/AST/ASTContext.h>

#include "../RosApiCall.h"
#include "./Util.h"
#include "./PublishCall.h"

namespace rosdiscover {
namespace api_call {

class CameraPublisherPublishCall : public PublishCall {
public:
  using PublishCall::PublishCall;

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("publish"), ofClass(hasName("image_transport::CameraPublisher"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      auto *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
      return new CameraPublisherPublishCall(call);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
