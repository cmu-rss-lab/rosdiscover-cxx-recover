#pragma once

#include <clang/AST/APValue.h>
#include <clang/AST/Expr.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/APFloat.h>

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

  static const clang::Decl *getCallerDecl(const std::string className, const clang::CXXMemberCallExpr * memberCallExpr) {
    const auto *caller = memberCallExpr->getImplicitObjectArgument()->IgnoreImpCasts();
    
    const auto *declRef = clang::dyn_cast<clang::DeclRefExpr>(caller);
    if (declRef == nullptr || !declRef->getDecl()) {
      llvm::outs() << "ERROR [" << className << "] Can't find declaration of CXXMemberCallExpr: ";
      memberCallExpr->dump();
      llvm::outs() << "\n";
      return nullptr;
    }
    return declRef->getDecl();
  }
} // rosdiscover::api_call
} // rosdiscover
