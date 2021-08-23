#pragma once

#include <string>
#include <vector>

#include <clang/AST/ASTTypeTraits.h>
#include <clang/AST/Expr.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <llvm/Support/raw_ostream.h>

#include "../Helper/utils.h"
#include "Calls/Kind.h"

namespace rosdiscover {
namespace api_call {

class RosApiCall {
public:
  RosApiCall(clang::CallExpr const *call) : call(call) {}
  virtual ~RosApiCall(){}

  clang::CallExpr const * getCallExpr() const { return call; }

  /** Returns the expression that provides the name associated with this call. */
  virtual clang::Expr const * getNameExpr() const = 0;

  /** Indicates whether or not this API call has an associated node handle. */
  virtual bool hasNodeHandle() const = 0;

  /**
   * Returns the expression, if any, that holds the value produced by this call.
   * Depending on the kind of API call, this may either be:
   * - the return value of the API call (e.g., hasParam), or
   * - the variable to which the result of the API call is written (e.g., getParam)
   *
   * If there is no such value associated with this API call (e.g., publishing to a topic),
   * then a null pointer is returned.
   */
  virtual clang::Expr const * getResultExpr() const {
    return nullptr;
  }

  /** Returns the kind of this API call. Used for dynamic dispatch. */
  virtual RosApiCallKind const getKind() const = 0;

  class Finder : public clang::ast_matchers::MatchFinder::MatchCallback {
  public:
    void run(const clang::ast_matchers::MatchFinder::MatchResult &result) {
      if (auto *apiCall = build(result)) {
        auto const *callExpr = apiCall->getCallExpr();

        // NOTE c++20 provides std::string::ends_with
        // ignore any calls that happen within the ROS language bindings
        std::string filename = clang::FullSourceLoc(callExpr->getBeginLoc(), *result.SourceManager).getFileEntry()->getName().str();
        if (
             ends_with(filename, "/include/ros/node_handle.h")
          || ends_with(filename, "/include/ros/param.h")
          || ends_with(filename, "/include/ros/service.h")
        ) {
          return;
        }

        found.push_back(apiCall);
      }
    }

    virtual const clang::ast_matchers::StatementMatcher getPattern() = 0;

  protected:
    Finder(std::vector<RosApiCall*> &found) : found(found) {}

    virtual RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) = 0;

  private:
    std::vector<RosApiCall*> &found;
  };

  virtual void print(llvm::raw_ostream &os) const {
    static clang::LangOptions langOptions;
    static clang::PrintingPolicy printPolicy(langOptions);
    getCallExpr()->printPretty(os, nullptr, printPolicy);
  }

private:
  clang::CallExpr const *call;
}; // RosApiCall


class BareRosApiCall : public RosApiCall {
public:
  virtual ~BareRosApiCall(){}
  BareRosApiCall(clang::CallExpr const *call) : RosApiCall(call) {}
  bool hasNodeHandle() const override { return false; }
}; // BareRosApiCall


class NodeHandleRosApiCall : public RosApiCall {
public:
  virtual ~NodeHandleRosApiCall(){}
  NodeHandleRosApiCall(clang::CallExpr const *call) : RosApiCall(call) {}
  bool hasNodeHandle() const override { return true; }

  void getNodeHandleExpr() const {
    auto callExpr = getCallExpr();
    llvm::outs() << "NODE HANDLE? ";
    clang::DeclRefExpr const *nhDeclRef = clang::dyn_cast<clang::DeclRefExpr>(
      clang::dyn_cast<clang::MemberExpr>(callExpr->getCallee())
      ->getBase()
      ->IgnoreImpCasts()
    );
    // getDecl vs. getFoundDecl?
    auto const *nhDecl = nhDeclRef->getDecl();
    nhDecl->dumpColor();
    llvm::outs() << "\n";
  }
}; // NodeHandleRosApiCall


} // rosdiscover::api_call
} // rosdiscover
