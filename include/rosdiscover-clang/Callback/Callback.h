#pragma once

#include <memory>

#include <clang/AST/Stmt.h>

#include "../ApiCall/RosApiCall.h"
#include "../Helper/utils.h"
#include "../ApiCall/Calls/Util.h"

namespace rosdiscover {

class Callback {
public:

  // Used for unwrapping bind calls
  static  const clang::Expr * unwrapMaterializeTemporaryExpr(
    const clang::MaterializeTemporaryExpr* tempExpr
  ) {
      std::vector<const clang::Stmt*> calls = rosdiscover::getTransitiveChildenByType(tempExpr, false, true);
      for (auto* c: calls) {
        auto *call = clang::dyn_cast<clang::CallExpr>(c);
        if (auto* declRef = clang::dyn_cast<clang::DeclRefExpr>(call->getArg(0)->IgnoreImpCasts())) {
          return declRef;
        }
        if (auto* unaryOperator = clang::dyn_cast<clang::UnaryOperator>(call->getArg(0)->IgnoreImpCasts())) {
          return unaryOperator;
        }
      }
      llvm::outs() << "[Callback] ERROR: Couldn't find UnaryOperator or DeclRefExpr call argument in MaterializeTemporaryExpr.";
      return nullptr;
  }
  
  static Callback* fromArgExpr(
    clang::ASTContext &context,
    api_call::RosApiCall const *apiCall,
    clang::Expr const *argExpr
  ) {
    llvm::outs() << "DEBUG: attempting to extract callback from expr: ";
    argExpr->dump();
    llvm::outs() << "\n";
    if (argExpr == nullptr) {
      return unableToResolve(argExpr);
    }

    // Callbacks can either contain a reference to a member function of 
    // a static class (e.g., ``&callback_name``) 
    // or just the name of a static metod (e.g., ``callback_name``).
    auto *unaryOp = clang::dyn_cast<clang::UnaryOperator>(argExpr);
    clang::Expr const *subExpr;
    if (unaryOp == nullptr) {
      // The callback doesn't have an unary operator
      auto *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(argExpr);
      if (declRefExpr != nullptr) {
        subExpr = declRefExpr;
      } else {
        auto *castExpr = clang::dyn_cast<clang::ImplicitCastExpr>(argExpr);
        if (castExpr == nullptr) {
          // It could be a MaterializeTemporaryExpr
          const auto *tempExpr = clang::dyn_cast<clang::MaterializeTemporaryExpr>(argExpr);
          if (tempExpr == nullptr) {
            return unableToResolve(argExpr);
          }
          llvm::outs() << "[Callback] is MaterializeTemporaryExpr: ";
          tempExpr->dump();
          llvm::outs() << "\n";

          
          return fromArgExpr(context, apiCall, unwrapMaterializeTemporaryExpr(tempExpr));
        }

        subExpr = castExpr->IgnoreImpCasts();
      }
    } else {
      subExpr = unaryOp->getSubExpr();
    }

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
