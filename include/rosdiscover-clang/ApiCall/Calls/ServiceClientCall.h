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
    return getServiceTypeDecl()->getQualifiedNameAsString();
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
  // FIXME there are three versions of serviceClient; each has different template arguments
  // https://docs.ros.org/en/api/roscpp/html/classros_1_1NodeHandle.html
  clang::TemplateArgument const getServiceTypeTemplateArg() const {
    return getCallExpr()->getDirectCallee()->getTemplateSpecializationArgs()->get(0);
  }

  // TODO lift some of this code into a helper function
  clang::CXXRecordDecl const * getServiceTypeDecl() const {
    auto qualType = getServiceTypeTemplateArg().getAsType().getNonReferenceType().getUnqualifiedType();
    auto *recordType = clang::dyn_cast<clang::RecordType>(qualType.getTypePtr());
    auto const *recordDecl = clang::dyn_cast<clang::CXXRecordDecl>(recordType->getDecl());
    if (auto *specializationDecl = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(recordDecl)) {
      recordDecl = specializationDecl->getSpecializedTemplate()->getTemplatedDecl();
    }
    return recordDecl;
  }
};

} // rosdiscover::api_call
} // rosdiscover
