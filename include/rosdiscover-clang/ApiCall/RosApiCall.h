#pragma once

#include <string>
#include <vector>

#include <clang/AST/ASTTypeTraits.h>
#include <clang/AST/Expr.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <llvm/Support/raw_ostream.h>

#include "../Name/Symbolizer.h"
#include "../utils.h"

namespace rosdiscover {
namespace api_call {

class RosApiCall {
public:
  clang::CallExpr const * getCallExpr() const { return call; }

  /** Returns the expression that provides the name associated with this call. */
  virtual clang::Expr const * getNameExpr() const = 0;

  name::NameExpr* symbolize() const {
    // this method really needs to go outside of RosApiCall and take place after we've
    // identified API calls
    //
    // this is just here for debugging
    auto symbolizer = name::NameSymbolizer();
    return symbolizer.symbolize(getNameExpr());
  }

  /** Returns the string literal name used by this API call, if there is one. */
  llvm::Optional<std::string> getConstantName() const {
    using namespace clang;
    using namespace clang::ast_type_traits;

    // MaterializeTemporaryExpr 0x8c335e0 'const std::string':'const class std::__cxx11::basic_string<char>' lvalue
    // `-CXXBindTemporaryExpr 0x8c335c0 'const std::string':'const class std::__cxx11::basic_string<char>' (CXXTemporary 0x8c335c0)
    //   `-CXXConstructExpr 0x8c33580 'const std::string':'const class std::__cxx11::basic_string<char>' 'void (const char *, const class std::allocator<char> &)'
    //     |-ImplicitCastExpr 0x8c33548 'const char *' <ArrayToPointerDecay>
    //     | `-StringLiteral 0x8c32438 'const char [5]' lvalue "odom"
    //     `-CXXDefaultArgExpr 0x8c33560 'const class std::allocator<char>':'const class std::allocator<char>' lvalue

    static auto none = llvm::Optional<std::string>();

    auto const *materializeTempExpr = DynTypedNode::create(*getNameExpr()).get<MaterializeTemporaryExpr>();
    if (materializeTempExpr == nullptr) {
      return none;
    }

    auto const *bindTempExpr = DynTypedNode::create(*(materializeTempExpr->getSubExpr())).get<CXXBindTemporaryExpr>();
    if (bindTempExpr == nullptr) {
      return none;
    }

    auto const *constructExpr = DynTypedNode::create(*(bindTempExpr->getSubExpr())).get<CXXConstructExpr>();
    if (constructExpr == nullptr) {
      return none;
    }

    // does this call the std::string constructor?
    // FIXME this is a bit hacky and may break when other libc++ versions are used
    if (constructExpr->getConstructor()->getParent()->getQualifiedNameAsString() != "std::__cxx11::basic_string") {
      return none;
    }

    auto const *castExpr = DynTypedNode::create(*(constructExpr->getArg(0))).get<ImplicitCastExpr>();
    if (castExpr == nullptr) {
      return none;
    }

    auto const *literal = DynTypedNode::create(*(castExpr->getSubExpr())).get<StringLiteral>();
    if (literal == nullptr) {
      return none;
    }

    return llvm::Optional<std::string>(literal->getString().str());
  }

  class Finder : public clang::ast_matchers::MatchFinder::MatchCallback {
  public:
    void run(const clang::ast_matchers::MatchFinder::MatchResult &result) {
      if (auto *apiCall = build(result)) {
        auto const *callExpr = apiCall->getCallExpr();

        // NOTE c++20 provides std::string::ends_with
        // ignore any calls that happen within the ROS language bindings
        std::string filename = clang::FullSourceLoc(callExpr->getBeginLoc(), *result.SourceManager).getFileEntry()->getName().str();
        if (ends_with(filename, "/include/ros/node_handle.h")) {
          llvm::outs() << "ignoring API call in file: " << filename << "\n";
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

//  virtual void print(llvm::raw_ostream &os, clang::SourceManager const &sourceManager) const {
  virtual void print(llvm::raw_ostream &os) const {
    static clang::LangOptions langOptions;
    static clang::PrintingPolicy printPolicy(langOptions);
    getCallExpr()->printPretty(os, nullptr, printPolicy);
    os << " [" << locationString << "]";

    auto maybeName = getConstantName();
    if (maybeName.hasValue()) {
      os << " [\"" << maybeName.getValue() << "\"]";
    } else {
      // TODO: write to os
      os << "\n";
      getNameExpr()->dumpColor();
    }

    symbolize();
  }

protected:
  RosApiCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : call(call),
      context(context),
      locationString(call->getBeginLoc().printToString(context->getSourceManager()))
  {}

private:
  clang::CallExpr const *call;
  clang::ASTContext const *context;
  std::string const locationString;
}; // RosApiCall


class BareRosApiCall : public RosApiCall {
protected:
  BareRosApiCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : RosApiCall(call, context)
  {}
}; // BareRosApiCall


class NodeHandleRosApiCall : public RosApiCall {
protected:
  NodeHandleRosApiCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : RosApiCall(call, context)
  {}
}; // NodeHandleRosApiCall


} // rosdiscover::api_call
} // rosdiscover
