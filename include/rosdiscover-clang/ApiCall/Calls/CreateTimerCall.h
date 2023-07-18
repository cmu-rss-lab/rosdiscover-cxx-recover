#pragma once

#include <clang/AST/TemplateBase.h>

#include "../../Helper/FormatHelper.h"
#include "../RosApiCall.h"
#include "../../BackwardSymbolizer/RateSymbolizer.h"

namespace rosdiscover {
namespace api_call {

class CreateTimerCall : public NodeHandleRosApiCall {
public:
  using NodeHandleRosApiCall::NodeHandleRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::CreateTimerCall;
  }

  clang::Expr const * getRateArg() const {
    auto *callExpr = getCallExpr();
    auto numArgs = callExpr->getNumArgs();

    llvm::outs() << "[CreateTimerCall] Finding Rate Arg\n";

    // if the call only has one argument, then we don't know what the callback is for now
    if (numArgs < 2) {
      llvm::outs() << "[CreateTimerCall] Incorrect number of arguments (" << numArgs << ")\n";
      return nullptr;
    }

    // otherwise the callback should be given by the third argument
    return callExpr->getArg(0)->IgnoreCasts();
  }
  
  clang::APValue const * getRate(const clang::ASTContext &ctx) const {

    auto const* rateArg = getRateArg();
    llvm::outs() << "DEBUG [CreateTimerCall]: Getting Rate for: ";
    rateArg->dump();
    llvm::outs() << "\n";
    

    return RateSymbolizer::symbolizeRate(rateArg, ctx);
  }
  
  Callback* getCallback(clang::ASTContext &context) const override {
    auto *callExpr = getCallExpr();
    auto numArgs = callExpr->getNumArgs();

    llvm::outs() << "[CreateTimerCall] Finding Callback\n";

    // if the call only has one argument, then we don't know what the callback is for now
    if (numArgs < 2) {
      llvm::outs() << "[CreateTimerCall] Incorrect number of arguments (" << numArgs << ")\n";
      return nullptr;
    }

    // otherwise the callback should be given by the third argument
    auto *callbackArg = callExpr->getArg(1);
    auto callback = Callback::fromArgExpr(context, this, callbackArg);
    //llvm::outs() << "[CreateTimerCall] Callback Found\n";
    return callback;
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("createTimer"), ofClass(hasName("ros::NodeHandle"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new CreateTimerCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
