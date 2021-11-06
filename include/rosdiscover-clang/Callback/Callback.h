#pragma once

#include <memory>

#include <clang/AST/Stmt.h>

#include "../ApiCall/RosApiCall.h"
#include "../Helper/utils.h"

namespace rosdiscover {

class Callback {
public:
  static Callback* fromArgExpr(
    clang::ASTContext &context,
    api_call::RosApiCall const *apiCall,
    clang::Expr const *argExpr
  ) {
    llvm::outs() << "DEBUG: attempting to extract callback from expr: ";
    argExpr->dump();
    llvm::outs() << "\n";

    auto *unaryOp = clang::dyn_cast<clang::UnaryOperator>(argExpr);
    if (unaryOp == nullptr) {
      return unableToResolve(argExpr);
    }

    auto *subExpr = unaryOp->getSubExpr();
    if (subExpr == nullptr) {
      return unableToResolve(argExpr);
    }

    auto *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(subExpr);
    if (declRefExpr == nullptr) {
      return unableToResolve(argExpr);
    }

    auto *valueDecl = declRefExpr->getDecl();
    if (valueDecl == nullptr) {
      return unableToResolve(argExpr);
    }

    auto *functionDecl = clang::dyn_cast<clang::FunctionDecl>(valueDecl);
    if (functionDecl == nullptr) {
      return unableToResolve(argExpr);
    }

    return create(context, apiCall, functionDecl);
  }

  static Callback* create(
    clang::ASTContext &context,
    api_call::RosApiCall const *apiCall,
    clang::FunctionDecl const *target
  ) {
    auto *callExpr = apiCall->getExpr();
    auto *parent = getParentFunctionDecl(context, callExpr);
    target = target->getCanonicalDecl();
    return new Callback(apiCall, parent, target);
  }

  api_call::RosApiCall const * getApiCall() const {
    return apiCall;
  }

  clang::FunctionDecl const * getParentFunction() const {
    return parent;
  }

  clang::FunctionDecl const * getTargetFunction() const {
    return target;
  }

  void print(llvm::raw_ostream &os) const {
    os << "Callback(@[";
    apiCall->print(os);
    os 
      << "], "
      << parent->getQualifiedNameAsString()
      << " {"
      << &(*parent)
      << "} -> "
      << target->getQualifiedNameAsString()
      << &(*target)
      << " {"
      << "})";
  }

private:
  Callback(
    api_call::RosApiCall const *apiCall,
    clang::FunctionDecl const *parent,
    clang::FunctionDecl const *target
  ) : apiCall(apiCall), parent(parent), target(target) {}

  static Callback* unableToResolve(clang::Expr const *argExpr) {
    llvm::outs() << "WARNING: unable to resolve callback from expression: ";
    argExpr->dump();
    llvm::outs() << "\n";
    return nullptr;
  }

  api_call::RosApiCall const *apiCall;
  clang::FunctionDecl const *parent;
  clang::FunctionDecl const *target;
};

}