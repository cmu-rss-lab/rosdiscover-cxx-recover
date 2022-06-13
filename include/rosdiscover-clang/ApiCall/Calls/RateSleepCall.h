#pragma once

#include <llvm/ADT/APInt.h>

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
    return getCallExpr();
  }

  clang::Expr const * getRateExpr() const {
    if (const auto *E = clang::dyn_cast<clang::CXXMemberCallExpr>(getCallExpr())) {
      const clang::DeclRefExpr* declRef = nullptr;
      if (const auto *ME = clang::dyn_cast<clang::ImplicitCastExpr>(E->getImplicitObjectArgument())) {
        if (const auto *SE = clang::dyn_cast<clang::DeclRefExpr>(ME->getSubExpr())) {
          declRef  = SE;
        }
      } 
      else if (const auto *ME = clang::dyn_cast<clang::DeclRefExpr>(E->getImplicitObjectArgument())) {
        declRef  = ME;
      }
      if (declRef && declRef->getDecl()) {
        if (auto *vd = clang::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
          if (vd->hasInit()) {
            if (const auto *constructor = clang::dyn_cast<clang::CXXConstructExpr>(vd->getInit())) {
              return constructor->getArg(0);
            }
          }
        }
      }
    }
    return getCallExpr();
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
