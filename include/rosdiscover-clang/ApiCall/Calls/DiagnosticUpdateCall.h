#pragma once

#include <clang/AST/ASTContext.h>

#include "../RosApiCall.h"
#include "./Util.h"
#include "./PublishCall.h"

namespace rosdiscover {
namespace api_call {

class DiagnosticUpdateCall : public PublishCall {
public:
  using PublishCall::PublishCall;

  virtual const std::string getPublisherName(clang::ASTContext &astContext) const override {
    return "diagnostic_updater";
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasAnyName("update", "force_update"), ofClass(hasName("diagnostic_updater::Updater"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      auto *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
      return new DiagnosticUpdateCall(call);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
