#pragma once

#include "../../Helper/FormatHelper.h"
#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class AdvertiseTopicCall : public NodeHandleRosApiCall {
public:
  using NodeHandleRosApiCall::NodeHandleRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::AdvertiseTopicCall;
  }

  clang::Expr const * getNameExpr() const override {
    return getCallExpr()->getArg(0);
  }

  std::string getFormatName() const {
    return typeNameToFormatName(getFormatDecl()->getQualifiedNameAsString());
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("advertise"), ofClass(hasName("ros::NodeHandle"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new AdvertiseTopicCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };

private:
  clang::TemplateArgument const getFormatTemplateArg() const {
    return getCallExpr()->getDirectCallee()->getTemplateSpecializationArgs()->get(0);
  }

  // TODO lift some of this code into a helper function?
  clang::CXXRecordDecl const * getFormatDecl() const {
    auto qualType = getFormatTemplateArg().getAsType().getNonReferenceType().getUnqualifiedType();
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
