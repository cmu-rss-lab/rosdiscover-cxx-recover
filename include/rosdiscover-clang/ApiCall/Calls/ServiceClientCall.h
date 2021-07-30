#pragma once

#include <clang/AST/TemplateBase.h>

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class ServiceClientCall : public NodeHandleRosApiCall {
public:
  using NodeHandleRosApiCall::NodeHandleRosApiCall;


  RosApiCallKind const getKind() const override {
    return RosApiCallKind::ServiceClientCall;
  }

  clang::Expr const * getNameExpr() const override {
    return getCallExpr()->getArg(0);
  }

  std::string getServiceTypeName() const {
    return "TODO-SERVICE-TYPE-NAME";
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("serviceClient"), ofClass(hasName("ros::NodeHandle"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new ServiceClientCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };

private:
  clang::TemplateArgument const getServiceTypeTemplateArg() const {
    return getCallExpr()->getDirectCallee()->getTemplateSpecializationArgs()->get(0);
  }
};

} // rosdiscover::api_call
} // rosdiscover
