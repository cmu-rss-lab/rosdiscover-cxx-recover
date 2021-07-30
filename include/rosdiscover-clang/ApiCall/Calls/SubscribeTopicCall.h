#pragma once

#include <clang/AST/TemplateBase.h>

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class SubscribeTopicCall : public NodeHandleRosApiCall {
public:
  using NodeHandleRosApiCall::NodeHandleRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::SubscribeTopicCall;
  }

  clang::Expr const * getNameExpr() const override {
    return getCallExpr()->getArg(0);
  }

  clang::CXXRecordDecl const * getTopicType() const override {
    // TODO implement!
    return nullptr;
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(
          hasTemplateArgument(0, templateArgument().bind("templateArg")),
          hasName("subscribe"), ofClass(hasName("ros::NodeHandle"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      llvm::outs() << "nice!\n";
      auto qualType = result.Nodes.getNodeAs<clang::TemplateArgument>("templateArg")->getAsType().getNonReferenceType().getUnqualifiedType();
      auto *recordType = clang::dyn_cast<clang::RecordType>(qualType.getTypePtr());
      auto const *recordDecl = recordType->getDecl();
      if (auto *specializationDecl = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(recordDecl)) {
        recordDecl = specializationDecl->getSpecializedTemplate()->getTemplatedDecl();
        llvm::outs() << "decl name: " << recordDecl->getQualifiedNameAsString() << "\n";
      }
      recordDecl->dump();
      llvm::outs() << "\n";
      return new SubscribeTopicCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
