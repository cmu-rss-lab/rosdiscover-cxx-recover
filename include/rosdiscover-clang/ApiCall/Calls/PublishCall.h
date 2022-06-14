#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class PublishCall : public BareRosApiCall {
public:
  using BareRosApiCall::BareRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::PublishCall;
  }

  clang::Expr const * getNameExpr() const override {
    const auto *memberCall = clang::dyn_cast<clang::CXXMemberCallExpr>(getCallExpr());
    if (memberCall == nullptr) {
      llvm::outs() << "ERROR [PublishCall] publish call is no CXXMemberCallExpr\n";
      return nullptr;
    }

    auto *callee = memberCall->getImplicitObjectArgument();
    if (callee == nullptr) {
    llvm::outs() << "ERROR [PublishCall] no calle on publish call\n";
      return nullptr;
    }

    llvm::outs() << "DEBUG [PublishCall] Callee is: ";
    callee->IgnoreImpCasts()->dump();
    llvm::outs() << "\n";

    return callee->IgnoreImpCasts();
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("publish"), ofClass(hasName("ros::Publisher"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      auto *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
      return new PublishCall(call);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
