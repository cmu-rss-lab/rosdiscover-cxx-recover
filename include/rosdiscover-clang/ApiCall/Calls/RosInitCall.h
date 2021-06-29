#pragma once

#include <llvm/ADT/APInt.h>

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class RosInitCall : public BareRosApiCall {
public:
  RosInitCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : BareRosApiCall(call, context)
  {}

  clang::Expr const * getNameExpr() const {
    // name expr
    auto *nameExpr = getCallExpr()->getArg(2);
    //dynamic_cast<clang::MaterializeTemporaryExpr> nameExpr
    return nameExpr;
  }

  // llvm::Optional<std::string> getLiteralName() const {
  //
  // }

  // llvm::Optional<std::string> getStaticName() const {
  //
  // }

  virtual void print(llvm::raw_ostream &os) const override {
    RosApiCall::print(os);
    os << " ";
    getNameExpr()->dumpColor();
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::init")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      auto *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
      return new RosInitCall(call, result.Context);
    }
  };
};

} // rosdiscover::api_call
} // rosdiscover
