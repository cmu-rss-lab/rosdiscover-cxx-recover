#pragma once

#include <clang/AST/APValue.h>
#include <clang/AST/Expr.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/APFloat.h>

#include "./Util.h"
#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class RateSleepCall : public BareRosApiCall {
public:
  using BareRosApiCall::BareRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::RateSleepCall;
  }

  clang::Expr const * getNameExpr() const override {
    if (const auto *E = clang::dyn_cast<clang::CXXMemberCallExpr>(getCallExpr())) {
      if (const auto *ME = clang::dyn_cast<clang::ImplicitCastExpr>(E->getImplicitObjectArgument())) {
        return E->getImplicitObjectArgument();
      }
    } 
    return nullptr;
  }

  clang::APValue const * getRate(const clang::ASTContext &Ctx) const {
    llvm::outs() << "DEBUG [RateSleepCall]: Getting Rate for: ";
    getCallExpr()->dump();
    llvm::outs() << "\n";
    
    //Check input
    const auto *memberCallExpr = clang::dyn_cast<clang::CXXMemberCallExpr>(getCallExpr());
    if (memberCallExpr == nullptr) {
      llvm::outs() << "ERROR [RateSleepCall]: Sleep call is not a CXXMemberCallExpr: ";
      getCallExpr()->dump();
      llvm::outs() << "\n";
      return nullptr;
    }
    
    //Get rate declaration
    const clang::Decl* decl = getCallerDecl("RateSleepCall", memberCallExpr);
    if (decl == nullptr) {
      return nullptr;
    }

    //check if declaration of rate object is a VarDecl,
    auto *varDecl = clang::dyn_cast<clang::VarDecl>(decl);
    if (varDecl == nullptr) {
      llvm::outs() << "ERROR [RateSleepCall]: Unsupported rate declaration type: ";
      decl->dump();
      return nullptr;
    }

    //Get the initialization of the the rate object.
    if (!varDecl->hasInit()) {
      llvm::outs() << "ERROR [RateSleepCall]: Rate declaration has no init: ";
      decl->dump();
      return nullptr;     
    }
    auto *rateInit = varDecl->getInit();

    //Get the constructor of the rate object initializtion.
    const auto *rateConstructor = clang::dyn_cast<clang::CXXConstructExpr>(rateInit);
    if (rateConstructor == nullptr) {
      llvm::outs() << "ERROR [RateSleepCall]: Decl has no init: ";
      decl->dump();
      return nullptr;         
    }

    //Get the frequency argument of the rate constructor
    const auto *frequencyArg = rateConstructor->getArg(0)->IgnoreImpCasts();
    llvm::outs() << "DEBUG [RateSleepCall]: Rate found (" << frequencyArg->getStmtClassName() << ")\n";
    return evaluateNumber("RateSleepCall", frequencyArg, Ctx);
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("sleep"), ofClass(hasName("ros::Rate"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      auto *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
      return new RateSleepCall(call);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
