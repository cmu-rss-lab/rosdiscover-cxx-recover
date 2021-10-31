#pragma once

#include <tuple>

#include "../../Helper/FormatHelper.h"
#include "../../Callback/Callback.h"
#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class AdvertiseServiceCall : public NodeHandleRosApiCall {
public:
  using NodeHandleRosApiCall::NodeHandleRosApiCall;

  RosApiCallKind const getKind() const override {
    return RosApiCallKind::AdvertiseServiceCall;
  }

  Callback* getCallback(clang::ASTContext &context) const override {
    auto *callExpr = getCallExpr();
    auto numArgs = callExpr->getNumArgs();

    // if the call only has one argument, then we don't know what the callback is for now
    if (numArgs < 2) {
      return nullptr;
    }

    // otherwise the callback should be given by the second argument
    auto *callbackArg = callExpr->getArg(1);
    return Callback::fromArgExpr(context, this, callbackArg);
  }

  clang::Expr const * getNameExpr() const override {
    return getCallExpr()->getArg(0);
  }

  std::tuple<std::string, std::string> getRequestResponseFormatNames() const {
    auto typeNames = getRequestResponseTypeNames();
    return std::make_tuple(
      typeNameToFormatName(std::get<0>(typeNames)),
      typeNameToFormatName(std::get<1>(typeNames))
    );
  }

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(cxxMethodDecl(hasName("advertiseService"), ofClass(hasName("ros::NodeHandle"))))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new AdvertiseServiceCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };

private:
  // std::tuple<clang::TemplateArgument const &, clang::TemplateArgument const &> const getRequestResponseTemplateArgs() const {
  std::tuple<clang::CXXRecordDecl const *, clang::CXXRecordDecl const *> const getRequestResponseTypeDecls() const {
    llvm::outs() << "obtaining template arguments for API call: \n";
    print(llvm::outs());
    llvm::outs() << "\n";

    auto const *templateArgs = getCallExpr()->getDirectCallee()->getTemplateSpecializationArgs();

    if (templateArgs == nullptr) {
      llvm::errs() << "FATAL ERROR: unable to obtain template arguments for NodeHandle::advertiseService call\n";
      getCallExpr()->dumpColor();
      llvm::errs() << "\n";
      abort();
    }

    auto numTemplateArgs = templateArgs->size();

    llvm::outs() << "DEBUG: template args [" << numTemplateArgs << "]:\n";
    for (auto const &arg : templateArgs->asArray()) {
      llvm::outs() << " * ";
      arg.dump();
      llvm::outs() << "\n";
    }

    if (numTemplateArgs == 0) {
      // FIXME https://docs.ros.org/en/api/roscpp/html/classros_1_1NodeHandle.html#ae659319707eb40e8ef302763f7d632da
      llvm::errs() << "FATAL ERROR: unable to obtain format for NodeHandle::advertiseService(AdvertiseServiceOptions &ops)\n";
      abort();
    } else if (numTemplateArgs == 3) {
      auto *request = getTypeDeclFromTemplateArgument(templateArgs->get(1));
      auto *response = getTypeDeclFromTemplateArgument(templateArgs->get(2));
      return std::make_tuple(request, response);
    } else if (numTemplateArgs == 2) {
      auto *request = getTypeDeclFromTemplateArgument(templateArgs->get(0));
      auto *response = getTypeDeclFromTemplateArgument(templateArgs->get(1));
      return std::make_tuple(request, response);
    } else {
      llvm::errs()
        << "FATAL ERROR: unexpected number of template args for NodeHandle::advertiseService: "
        << numTemplateArgs
        << "\n";
      abort();
    }
  }

  std::tuple<std::string, std::string> getRequestResponseTypeNames() const {
    auto typeDecls = getRequestResponseTypeDecls();

    llvm::outs() << "DEBUG: Found request type decl: ";
    std::get<0>(typeDecls)->dump();
    llvm::outs() << "\n";

    llvm::outs() << "DEBUG: Found response type decl: ";
    std::get<1>(typeDecls)->dump();
    llvm::outs() << "\n";

    return std::make_tuple(
      std::get<0>(typeDecls)->getQualifiedNameAsString(),
      std::get<1>(typeDecls)->getQualifiedNameAsString()
    );
  }
};

} // rosdiscover::api_call
} // rosdiscover
