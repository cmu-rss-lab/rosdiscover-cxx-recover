#pragma once

#include <string>

#include "Value.h"

namespace rosdiscover {
namespace symbolic {

class BoolLiteral : public SymbolicBool {
public:
  BoolLiteral(bool literal) : literal(literal) {}
  ~BoolLiteral() {}

  void print(llvm::raw_ostream &os) const {
      os << ("true" ? literal : "false");
  }

private:
  bool literal;
};

} // rosdiscover::symbolic
} // rosdiscover
