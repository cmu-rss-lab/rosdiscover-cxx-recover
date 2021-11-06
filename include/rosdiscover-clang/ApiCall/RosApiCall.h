#pragma once

#include <string>
#include <vector>

#include <clang/AST/ASTTypeTraits.h>
#include <clang/AST/Expr.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <llvm/Support/raw_ostream.h>

#include "../Helper/utils.h"
#include "../Helper/CallOrConstructExpr.h"
#include "Calls/Kind.h"

namespace rosdiscover {

class Callback;

namespace api_call {

class RosApiCall {
public:
  RosApiCall(CallOrConstructExpr const *call) : call(call) {}
  virtual ~RosApiCall(){
    delete call; // TODO use unique_ptr or shared_ptr
  }

  clang::Expr const * getExpr() const { return call->getExpr(); }
  CallOrConstructExpr const * getCallOrConstructExpr() const { return call; }

  /** Returns the callback, if any, that is associated with this call. */
  virtual Callback* getCallback(clang::ASTContext &context) const {
    return nullptr;
  }

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
        auto const *callExpr = apiCall->getCallOrConstructExpr();

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
    call->getExpr()->printPretty(os, nullptr, printPolicy);
  }

protected:
  clang::CXXRecordDecl const * getTypeDeclFromTemplateArgument(clang::TemplateArgument const &templateArgument) const {
    llvm::outs() << "DEBUG: fetching type decl for template argument: ";
    templateArgument.dump();
    llvm::outs() << "\n";

    auto qualType = templateArgument.getAsType().getNonReferenceType().getUnqualifiedType();

    llvm::outs() << "DEBUG: found unqualified type: ";
    qualType.dump();
    llvm::outs() << "\n";

    auto *recordType = clang::dyn_cast<clang::RecordType>(qualType.getTypePtr());
    auto const *recordDecl = clang::dyn_cast<clang::CXXRecordDecl>(recordType->getDecl());
    if (auto *specializationDecl = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(recordDecl)) {
      recordDecl = specializationDecl->getSpecializedTemplate()->getTemplatedDecl();
    }
    return recordDecl;
  }

private:
  CallOrConstructExpr const *call;
}; // RosApiCall


class BareRosApiCall : public RosApiCall {
public:
  virtual ~BareRosApiCall(){}

  BareRosApiCall(clang::CallExpr const *call)
    : BareRosApiCall(new rosdiscover::CallExpr(call))
  {}

  bool hasNodeHandle() const override { return false; }

  clang::CallExpr const * getCallExpr() const {
    return static_cast<rosdiscover::CallExpr const *>(getCallOrConstructExpr())->getCallExpr();
  }

private:
  BareRosApiCall(CallOrConstructExpr const *call)
    : RosApiCall(call)
  {}

}; // BareRosApiCall


class RosApiCallWithNodeHandle : public RosApiCall {
public:
  virtual ~RosApiCallWithNodeHandle(){}

  RosApiCallWithNodeHandle(CallOrConstructExpr const *call)
    : RosApiCall(call)
  {}
  RosApiCallWithNodeHandle(clang::CallExpr const *call)
    : RosApiCallWithNodeHandle(new rosdiscover::CallExpr(call))
  {}
  RosApiCallWithNodeHandle(clang::CXXConstructExpr const *call)
    : RosApiCallWithNodeHandle(new rosdiscover::ConstructExpr(call))
  {}

  bool hasNodeHandle() const override { return true; }

  virtual clang::Expr const * getNodeHandleExpr() const = 0;

  clang::ValueDecl const * getNodeHandleDecl() const {
    auto const *nodeHandleExpr = getNodeHandleExpr();

    // node handle is provided by a parameter or local variable
    if (auto const *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(nodeHandleExpr)) {
      return declRefExpr->getDecl();

    // node handle is provided by a CXX field
    } else if (auto const *memberExpr = clang::dyn_cast<clang::MemberExpr>(nodeHandleExpr)) {
      return memberExpr->getMemberDecl();

    } else {
      llvm::errs() << "unable to fetch decl for node handle expr: ";
      nodeHandleExpr->dumpColor();
      llvm::errs() << "\n";
      abort();
    }
  }
}; // RosApiCallWithNodeHandle


class NodeHandleRosApiCall : public RosApiCallWithNodeHandle {
public:
  NodeHandleRosApiCall(clang::CallExpr const *call)
    : RosApiCallWithNodeHandle(call)
  {}

  virtual ~NodeHandleRosApiCall(){}

  clang::CallExpr const * getCallExpr() const {
    return static_cast<rosdiscover::CallExpr const *>(getCallOrConstructExpr())->getCallExpr();
  }

  clang::Expr const * getNodeHandleExpr() const override {
    return clang::dyn_cast<clang::MemberExpr>(getCallExpr()->getCallee())
      ->getBase()
      ->IgnoreImpCasts();
  }
}; // NodeHandleRosApiCall

} // rosdiscover::api_call
} // rosdiscover
