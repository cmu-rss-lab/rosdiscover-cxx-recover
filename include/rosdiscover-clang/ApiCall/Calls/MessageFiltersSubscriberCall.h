#pragma once

#include "../RosApiCall.h"
#include "../../Helper/CallOrConstructExpr.h"

namespace rosdiscover {
namespace api_call {

class MessageFiltersSubscriberCall : public RosApiCallWithNodeHandle {
public:
  using RosApiCallWithNodeHandle::RosApiCallWithNodeHandle;

  MessageFiltersSubscriberCall(clang::CXXConstructExpr const *expr) :
    RosApiCallWithNodeHandle(expr)
  {}

  clang::CXXConstructExpr const * getConstructExpr() const {
    return static_cast<rosdiscover::ConstructExpr const *>(getCallOrConstructExpr())->getConstructExpr();
  }

  clang::CXXConstructorDecl const * getConstructorDecl() const {
    return getConstructExpr()->getConstructor();
  }
  
  clang::CXXRecordDecl const * getRecordDecl() const {
    return getConstructorDecl()->getParent();
  }

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::MessageFiltersSubscriberCall;
  }

  clang::Expr const * getNameExpr() const override {
    return getConstructExpr()->getArg(1);
  }

  clang::Expr const * getNodeHandleExpr() const override {
    return getConstructExpr()->getArg(0);
  }

  std::string getFormatName() const {
    return typeNameToFormatName(getFormatDecl()->getQualifiedNameAsString());
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return cxxConstructExpr(
        hasDeclaration(hasDeclContext(namedDecl(hasName("message_filters::Subscriber"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new MessageFiltersSubscriberCall(result.Nodes.getNodeAs<clang::CXXConstructExpr>("call"));
    }
  };

private:
  clang::TemplateArgument const getFormatTemplateArg() const {
    llvm::outs() << "DEBUG: fetching format template argument for message_filters::Subscriber call\n";
    auto *recordDecl = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(getRecordDecl());
    auto const &templateArgs = recordDecl->getTemplateArgs();
    assert (templateArgs.size() == 1 && "expected message_filters::Subscriber to have exactly one template argument");
    auto const &arg = templateArgs.get(0);
    return arg;
  }

  clang::CXXRecordDecl const * getFormatDecl() const {
    llvm::outs() << "DEBUG: fetching formatdecl for message_filters::Subscriber call\n";
    auto qualType = getFormatTemplateArg().getAsType().getNonReferenceType().getUnqualifiedType();
    auto *recordType = clang::dyn_cast<clang::RecordType>(qualType.getTypePtr());
    auto const *recordDecl = clang::dyn_cast<clang::CXXRecordDecl>(recordType->getDecl());
    if (auto *specializationDecl = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(recordDecl)) {
      recordDecl = specializationDecl->getSpecializedTemplate()->getTemplatedDecl();
    }
    return recordDecl;
  }
};

}
}