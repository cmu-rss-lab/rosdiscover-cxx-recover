#pragma once

#include <clang/AST/TemplateBase.h>

#include "../../Helper/FormatHelper.h"
#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class MessageFiltersRegisterCallbackCall : public BareRosApiCall, public NamedRosApiCall  {
public:

  using BareRosApiCall::BareRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::MessageFiltersRegisterCallbackCall;
  }
  
  Callback* getCallback(clang::ASTContext &context) const override {
    auto *callExpr = getCallExpr();
    auto numArgs = callExpr->getNumArgs();

    llvm::outs() << "[MessageFiltersRegisterCallbackCall] Finding Callback\n";

    // if the call only has one argument, then we don't know what the callback is for now
    if (numArgs < 1) {
      llvm::outs() << "[MessageFiltersRegisterCallbackCall] Incorrect number of arguments (" << numArgs << ")\n";
      return nullptr;
    }

    // otherwise the callback should be given by the third argument
    auto *callbackArg = callExpr->getArg(0);
    auto callback = Callback::fromArgExpr(context, this, callbackArg);
    llvm::outs() << "[MessageFiltersRegisterCallbackCall] Callback Found\n";
    return callback;
  }

  clang::Expr const * getNameExpr() const override {
    return getCallExpr()->getArg(0);
  }

  std::string getFormatName() const {
    return typeNameToFormatName(getTopicTypeName());
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
 //       callee(cxxMethodDecl(hasName("registerCallback"), ofClass(hasName("message_filters::Subscriber"))))
          callee(cxxMethodDecl(hasName("registerCallback"), ofClass(anyOf(hasName("message_filters::Synchronizer"), hasName("message_filters::Subscriber")))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new MessageFiltersRegisterCallbackCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };

private:
  std::string getTopicTypeName() const {
    return getTopicTypeDecl()->getQualifiedNameAsString();
  }

  clang::TemplateArgument const getTopicTemplateArg() const {
    return getCallExpr()->getDirectCallee()->getTemplateSpecializationArgs()->get(0);
  }

  clang::CXXRecordDecl const * getTopicTypeDecl() const {
    auto qualType = getTopicTemplateArg().getAsType().getNonReferenceType().getUnqualifiedType();
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
