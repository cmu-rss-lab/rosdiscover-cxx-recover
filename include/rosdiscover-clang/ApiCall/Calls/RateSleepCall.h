#pragma once

#include <clang/AST/APValue.h>
#include <clang/AST/Expr.h>

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

  clang::APValue getRate(const clang::ASTContext &Ctx) const {
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
              const auto *arg = constructor->getArg(0)->IgnoreImpCasts();
              llvm::outs() << "Rate found (" << arg->getStmtClassName() << ")\n";
              clang::Expr::EvalResult result;
              arg->EvaluateAsInt(result, Ctx);
              return result.Val;
              /*clang::IntegerLiteral const *int_arg;
              if (const auto *argv = clang::dyn_cast<clang::DeclRefExpr>(arg)) {
                llvm::outs() << "RateDecl Type: (" << argv->getDecl()->getDeclKindName() << ")\n";
                if (auto *argdecl = clang::dyn_cast<clang::VarDecl>(argv->getDecl())) {
                    if (argdecl->hasInit()) {
                      llvm::outs() << "RateDecl Init: (" << argdecl->getInit()->getStmtClassName() << ")\n";
                      if (const auto *argv = clang::dyn_cast<clang::IntegerLiteral>(argdecl->getInit())) {
                         int_arg = argv;
                      }
                    }
                }
                llvm::errs() << "Rate unknown: " << arg->getStmtClassName() << ")\n";
                return arg;

              } else if (const auto *argv = clang::dyn_cast<clang::IntegerLiteral>(arg)) {
                int_arg = argv;
              } else {
                llvm::errs() << "Rate type unknown: " << arg->getStmtClassName() << ")\n";
                return arg;
              }
              
              return int_arg;*/
            }
          }
        }
      }
    }
    return nullptr;
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
