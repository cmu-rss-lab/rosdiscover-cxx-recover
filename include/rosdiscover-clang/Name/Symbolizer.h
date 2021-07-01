#pragma once

#include <string>

#include <clang/AST/Expr.h>
#include <llvm/ADT/APInt.h>

#include "Expr.h"

namespace rosdiscover {
namespace name {

class NameSymbolizer {
public:
  NameExpr* symbolize(clang::Expr const *nameExpr) const;

private:
  NameExpr* symbolize(clang::DeclRefExpr const *nameExpr) const;
  NameExpr* symbolize(clang::StringLiteral const *literal) const;

  llvm::Optional<std::string> getLiteral(clang::Expr const *expr) const;
}; // rosdiscover::name::NameSymbolizer

} // rosdiscover::name
} // rosdiscover
