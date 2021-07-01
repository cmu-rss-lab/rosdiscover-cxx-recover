#pragma once

#include <clang/AST/Expr.h>

namespace rosdiscover {

/** Attempts to produce a simple symbolic string expression from a ROS name expr **/

class NameSymbolizer {
public:
  void symbolize(clang::Expr const *nameExpr) const;

}; // rosdiscover::NameSymbolizer

} // rosdiscover
