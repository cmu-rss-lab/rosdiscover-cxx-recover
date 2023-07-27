#pragma once

#include <clang/AST/APValue.h>
#include <clang/AST/Expr.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/APFloat.h>

#include "./Util.h"
#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class ConstSleepCall: public BareRosApiCall {
public:

  using BareRosApiCall::BareRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::ConstSleepCall;
  }

  virtual clang::APValue const * getDuration(const clang::ASTContext &ctx) const = 0;

  virtual int getRate(const clang::ASTContext &ctx) const = 0;
}; //ConstSleepCall


class DurationSleepCall : public ConstSleepCall {
public:
  using ConstSleepCall::ConstSleepCall;

  virtual int getRate(const clang::ASTContext &ctx) const {
    return 1;
  }

  virtual clang::APValue const * getDuration(const clang::ASTContext &ctx) const {
    const auto *callExpr = clang::dyn_cast<clang::CXXMemberCallExpr>(getCallExpr()->IgnoreImpCasts());
    if (callExpr == nullptr) {
      llvm::outs() << "ERROR [DurationSleepCall]: Not a CXXMemberCallExpr: ";
      getCallExpr()->dump();
      llvm::outs() << "\n";
      return nullptr;
    }
    auto const* arg = callExpr->getImplicitObjectArgument()->IgnoreImpCasts();
    llvm::outs() << "DEBUG [DurationSleepCall]: Getting Duration for: ";
    arg->dump();
    llvm::outs() << "\n";
    

    return RateSymbolizer::symbolizeRate(arg, ctx);
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("sleep"), ofClass(hasName("ros::Duration"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      auto *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
      return new DurationSleepCall(call);
    }
  };
}; //DurationSleep

class USleepCall : public ConstSleepCall {
public:
  using ConstSleepCall::ConstSleepCall;

  virtual int getRate(const clang::ASTContext &ctx) const {
    const auto *callExpr = clang::dyn_cast<clang::CallExpr>(getCallExpr());
    if (callExpr->getDirectCallee()->getNameAsString() == "sleep") {
      return 1;
    } else if (callExpr->getDirectCallee()->getNameAsString() == "usleep") { 
      return 1000000;
    } else {
      llvm::outs() << "ERROR [USleepCall]: Sleep call is " << callExpr->getDirectCallee()->getNameAsString();
      callExpr->getDirectCallee()->dump();
      llvm::outs() << "\n";
      return -1;   
    }
  }

  virtual clang::APValue const * getDuration(const clang::ASTContext &ctx) const {
    llvm::outs() << "DEBUG [USleepCall]: Getting Sleep Time for: ";
    getCallExpr()->dump();
    llvm::outs() << "\n";
    
    //Check input
    const auto *callExpr = clang::dyn_cast<clang::CallExpr>(getCallExpr());
    if (callExpr == nullptr) {
      llvm::outs() << "ERROR [USleepCall]: Sleep call is not a CallExpr: ";
      getCallExpr()->dump();
      llvm::outs() << "\n";
      return nullptr;
    }
    llvm::outs() << "DEBUG [USleepCall]: Sleep Call Qualified Name:" << callExpr->getDirectCallee()->getQualifiedNameAsString() << "\n";
    
    const auto *durationArg = callExpr->getArg(0)->IgnoreImpCasts();
    llvm::outs() << "DEBUG [USleepCall]: Duration found (" << durationArg->getStmtClassName() << ")\n";
    return evaluateNumber("USleepCall", durationArg, ctx);
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(anyOf(hasName("usleep"), hasName("::sleep"))))
      ).bind("call"); //TODO: Handle std::this_thread::sleep_for
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      auto *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
      return new USleepCall(call);
    }
  };
}; //USleepCall

class ThreadSleepCall : public ConstSleepCall {
public:
  using ConstSleepCall::ConstSleepCall;


  virtual int getRate(const clang::ASTContext &ctx) const {
    llvm::outs() << "DEBUG [ThreadSleepCall]: Getting Sleep Time for: ";
    getCallExpr()->dump();
    llvm::outs() << "\n";
    
    //Check input
    const auto *callExpr = clang::dyn_cast<clang::CallExpr>(getCallExpr());
    if (callExpr == nullptr) {
      llvm::outs() << "ERROR [ThreadSleepCall]: Sleep call is not a CallExpr: ";
      getCallExpr()->dump();
      llvm::outs() << "\n";
      return -1;
    }
    
    //Get arg
    const auto *constructExpr = clang::dyn_cast<clang::CXXConstructExpr>(callExpr->getArg(0)->IgnoreCasts());
    if (constructExpr == nullptr) {
      llvm::outs() << "ERROR [ThreadSleepCall]: Sleep call arg is not a CXXConstructExpr: ";
      callExpr->getArg(0)->dump();
      llvm::outs() << "\n";
      return -1;
    }

    auto unit = constructExpr->getType().getAsString();
    if (unit == "std::chrono::milliseconds") {
      return 1000;
    } else if (unit == "std::chrono::nanoseconds") {
      return 1000000;
    } else if (unit == "std::chrono::seconds") {
      return 1;
    } else {
      llvm::outs() << "ERROR [ThreadSleepCall]: Sleep constructor is " << unit;
      constructExpr->getConstructor()->dump();
      llvm::outs() << "\n";
      return -1;
    }

  }

  virtual clang::APValue const * getDuration(const clang::ASTContext &ctx) const {
    llvm::outs() << "DEBUG [ThreadSleepCall]: Getting Sleep Time for: ";
    getCallExpr()->dump();
    llvm::outs() << "\n";
    
    //Check input
    const auto *callExpr = clang::dyn_cast<clang::CallExpr>(getCallExpr());
    if (callExpr == nullptr) {
      llvm::outs() << "ERROR [ThreadSleepCall]: Sleep call is not a CallExpr: ";
      getCallExpr()->dump();
      llvm::outs() << "\n";
      return nullptr;
    }
    
    //Get arg
    const auto *constructExpr = clang::dyn_cast<clang::CXXConstructExpr>(callExpr->getArg(0)->IgnoreCasts());
    if (constructExpr == nullptr) {
      llvm::outs() << "ERROR [ThreadSleepCall]: Sleep call arg is not a CXXConstructExpr: ";
      callExpr->getArg(0)->dump();
      llvm::outs() << "\n";
      return nullptr;
    }

    //if constructExpr()->getConstructor()->getNameAsString()  TODO: Check for unit!

    llvm::outs() << "DEBUG [ThreadSleepCall]: Duration found (" << constructExpr->getArg(0)->IgnoreCasts()->getStmtClassName() << ")\n";
    return evaluateNumber("ThreadSleepCall", constructExpr->getArg(0)->IgnoreCasts(), ctx);
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("std::this_thread::sleep_for")))
      ).bind("call"); 
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      auto *call = result.Nodes.getNodeAs<clang::CallExpr>("call");
      return new ThreadSleepCall(call);
    }
  };
}; //ThreadSleepCall


} // rosdiscover::api_call
} // rosdiscover
