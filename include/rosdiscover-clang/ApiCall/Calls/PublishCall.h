#pragma once

#include <clang/AST/ASTContext.h>

#include "../RosApiCall.h"
#include "./Util.h"

namespace rosdiscover {
namespace api_call {

class PublishCall : public BareRosApiCall {
public:
  using BareRosApiCall::BareRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::PublishCall;
  }

  virtual const std::string getPublisherName(clang::ASTContext &astContext) const {
    llvm::outs() << "DEBUG [PublishCall] Publish call is : ";
    getCallExpr()->dump();
    llvm::outs() << "\n";

    const auto *memberCall = clang::dyn_cast<clang::CXXMemberCallExpr>(getCallExpr());
    if (memberCall == nullptr) {
      llvm::outs() << "ERROR [PublishCall] Publish call is not a CXXMemberCallExpr: ";
      getCallExpr()->dump();
      llvm::outs() << "\n";
      return nullptr;
    }
    
    const clang::ValueDecl *decl = getCallerDecl("PublishCall", memberCall);
    if (decl == nullptr) {
      llvm::outs() << "ERROR [PublishCall] Decl is null: ";
      memberCall->dump();
      llvm::outs() << "\n";
      return rosdiscover::prettyPrint(memberCall->getCallee(), astContext);
    }

    llvm::outs() << "DEBUG [PublishCall] decl: ";
    decl->dump();
    llvm::outs() << "\n";

    const auto *identifier = decl->getIdentifier();
    if (identifier == nullptr) {
      llvm::outs() << "ERROR [PublishCall] Decl identifier is null: ";
      decl->dump();
      llvm::outs() << "\n";
      return nullptr;
    }
    llvm::outs() << "DEBUG [PublishCall] Callee is: " << identifier->getName() << "\n";

    return identifier->getName().str();
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
