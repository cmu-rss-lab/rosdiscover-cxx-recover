#pragma once

#include <string>

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <llvm/ADT/APInt.h>

#include "Expr.h"

namespace rosdiscover {
namespace name {

class NameSymbolizer {
public:
  NameSymbolizer(clang::ASTContext &context) : context(context) {}
  NameExpr* symbolize(clang::Expr *nameExpr);

private:
  clang::ASTContext &context;

  NameExpr* symbolize(clang::CXXConstructExpr *constructExpr);
  NameExpr* symbolize(clang::ImplicitCastExpr *castExpr);
  NameExpr* symbolize(clang::DeclRefExpr *nameExpr);
  NameExpr* symbolize(clang::StringLiteral *literal);

  llvm::Optional<std::string> getLiteral(clang::Expr *expr);
}; // rosdiscover::name::NameSymbolizer

} // rosdiscover::name
} // rosdiscover
