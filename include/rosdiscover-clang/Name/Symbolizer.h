#pragma once

#include <clang/AST/Expr.h>

#include "Expr.h"

namespace rosdiscover {
namespace name {

class NameSymbolizer {
public:
  NameExpr* symbolize(clang::Expr const *nameExpr) const;

}; // rosdiscover::name::NameSymbolizer

} // rosdiscover::name
} // rosdiscover
