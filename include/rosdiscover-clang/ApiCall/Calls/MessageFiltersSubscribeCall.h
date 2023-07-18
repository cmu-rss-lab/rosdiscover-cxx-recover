#pragma once

#include "../RosApiCall.h"
#include "../../Helper/CallOrConstructExpr.h"

namespace rosdiscover {
namespace api_call {

class MessageFiltersSubscribeCall : public RosApiCallWithNodeHandle, public NamedRosApiCall {
public:
  using RosApiCallWithNodeHandle::RosApiCallWithNodeHandle;

  MessageFiltersSubscribeCall(clang::CXXConstructExpr const *expr) :
    RosApiCallWithNodeHandle(expr)
  {}

  clang::CallExpr const * getCallExpr() const {
    return static_cast<rosdiscover::CallExpr const *>(getCallOrConstructExpr())->getCallExpr();
  }
  
  clang::CXXRecordDecl const * getRecordDecl() const {
    auto const *callee = getCallExpr()->getCallee();
    
    if (auto const *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(callee)) {
      llvm::outs() << "Warning: Finding parent for ";
      declRefExpr->getDecl();
      llvm::outs() << "\n";
    }
    llvm::outs() << "Warning: MessageFiltersSubscribeCall cannot find getRecordDecl for ";
    callee->dump();
    llvm::outs() << "\n";
    return nullptr;
  }

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::MessageFiltersSubscribeCall;
  }

  clang::Expr const * getNameExpr() const override {
    llvm::outs() << "DEBUG: MessageFiltersSubscribeCall::getNameExpr\n";
    if (getCallExpr()->getNumArgs() < 2) {
      llvm::outs() << "Warning: MessageFiltersSubscribeCall cannot getNameExpr\n";
      return nullptr;
    }
    return getCallExpr()->getArg(1);
  }

  clang::Expr const * getNodeHandleExpr() const override {
    llvm::outs() << "DEBUG: MessageFiltersSubscribeCall::getNodeHandleExpr\n";
    if (getCallExpr()->getNumArgs() < 1) {
      llvm::outs() << "Warning: MessageFiltersSubscribeCall cannot getNodeHandleExpr\n";
      return nullptr;
    }
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
        callee(cxxMethodDecl(hasName("subscribe"), ofClass(anyOf(
            hasName("message_filters::Synchronizer"), 
            hasName("message_filters::Subscriber"),
            hasName("message_filters::SimpleFilter")
          ))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new MessageFiltersSubscribeCall(result.Nodes.getNodeAs<clang::CXXConstructExpr>("call"));
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