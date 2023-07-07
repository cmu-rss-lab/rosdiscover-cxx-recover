#pragma once

#include <clang/AST/APValue.h>
#include <clang/AST/Expr.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/APFloat.h>

#include "./Util.h"
#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class ConstSleepCall : public BareRosApiCall {
public:
  using BareRosApiCall::BareRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::ConstSleepCall;
  }

  clang::APValue const * getDuration(const clang::ASTContext &ctx) const {
    llvm::outs() << "DEBUG [ConstSleepCall]: Getting Sleep Time for: ";
    getCallExpr()->dump();
    llvm::outs() << "\n";
    
    //Check input
    const auto *callExpr = clang::dyn_cast<clang::CallExpr>(getCallExpr());
    if (callExpr == nullptr) {
      llvm::outs() << "ERROR [ConstSleepCall]: Sleep call is not a CallExpr: ";
      getCallExpr()->dump();
      llvm::outs() << "\n";
      return nullptr;
    }
    
    const auto *durationArg = callExpr->getArg(0)->IgnoreImpCasts();
    llvm::outs() << "DEBUG [ConstSleepCall]: Duration found (" << durationArg->getStmtClassName() << ")\n";
    return evaluateNumber("ConstSleepCall", durationArg, ctx);
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("usleep")))
      ).bind("call"); //TODO: Handle std::this_thread::sleep_for
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      auto *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
      return new ConstSleepCall(call);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
