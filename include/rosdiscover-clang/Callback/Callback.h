#pragma once

#include <memory>

#include <clang/AST/Stmt.h>

#include "../ApiCall/RosApiCall.h"
#include "../Helper/utils.h"

namespace rosdiscover {

class Callback {
public:

  // Used for unwrapping bind calls
  static clang::Expr const * unwrapMaterializeTemporaryExpr(
    clang::MaterializeTemporaryExpr const *tempExpr
  ) {
      auto *bindExpr = clang::dyn_cast<clang::CXXBindTemporaryExpr>(tempExpr->getSubExpr()->IgnoreImpCasts());
      if (bindExpr == nullptr) {
        auto *callExpr = clang::dyn_cast<clang::CallExpr>(tempExpr->getSubExpr()->IgnoreImpCasts());
        if (callExpr == nullptr || callExpr->getNumArgs() < 1) {
          unableToResolve(tempExpr);
          return nullptr;
        }
        llvm::outs() << "[Callback] CallExpr: ";
        callExpr->dump();
        llvm::outs() << "\n";
        // TODO: Read the other arguments of bind calls such as in, ``boost::bind(CmdCallBack, _1, accel_rate)`` 
        return callExpr->getArg(0);
      }
      auto *constructExpr = clang::dyn_cast<clang::CXXConstructExpr>(bindExpr->getSubExpr()->IgnoreImpCasts());
      if (constructExpr == nullptr) {
        auto *callExpr = clang::dyn_cast<clang::CallExpr>(bindExpr->getSubExpr()->IgnoreImpCasts());
        if (callExpr == nullptr || callExpr->getNumArgs() < 1) {
          unableToResolve(bindExpr);
          return nullptr;
        }
        llvm::outs() << "[Callback] CallExpr: ";
        callExpr->dump();
        llvm::outs() << "\n";
        // TODO: Read the other arguments of bind calls such as in, ``boost::bind(CmdCallBack, _1, accel_rate)`` 
        return callExpr->getArg(0);
      }
      clang::CXXConstructExpr* constructExpr2 = clang::dyn_cast<clang::CXXConstructExpr>(constructExpr->getArg(0));
      if (constructExpr2 == nullptr) {

        auto *bindExpr2 = clang::dyn_cast<clang::CXXBindTemporaryExpr>(constructExpr->getArg(0));
        if (bindExpr2 != nullptr) {
          if (auto *subExpr = clang::dyn_cast<clang::CXXConstructExpr>(bindExpr2->getSubExpr()->IgnoreImpCasts())) {
            constructExpr2 = subExpr;
          } else {
            llvm::outs() << "[Callback] subExpr was expecting CXXConstructExpr";
            unableToResolve(constructExpr->getArg(0));
            return nullptr;
          }
        } else {
          llvm::outs() << "[Callback] constructExpr2 was expecting CXXConstructExpr or CXXBindTemporaryExpr";
          unableToResolve(constructExpr->getArg(0));
          return nullptr;
        }
      } 
      auto *arg = clang::dyn_cast<clang::MaterializeTemporaryExpr>(constructExpr2->getArg(0));
      if (arg == nullptr) {
        llvm::outs() << "[Callback] arg was expecting MaterializeTemporaryExpr";
        unableToResolve(constructExpr2->getArg(0));
        return nullptr;
      } 

      return unwrapMaterializeTemporaryExpr(arg);
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
      auto *castExpr = clang::dyn_cast<clang::ImplicitCastExpr>(argExpr);
      if (castExpr == nullptr) {
        // It could be a MaterializeTemporaryExpr
        auto *tempExpr = clang::dyn_cast<clang::MaterializeTemporaryExpr>(argExpr);
        if (tempExpr == nullptr) {
          return unableToResolve(argExpr);
        }
        llvm::outs() << "[Callback] is MaterializeTemporaryExpr: ";
        tempExpr->dump();
        llvm::outs() << "\n";

        
        return fromArgExpr(context, apiCall, unwrapMaterializeTemporaryExpr(tempExpr));
      }

      subExpr = castExpr->IgnoreImpCasts();
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
