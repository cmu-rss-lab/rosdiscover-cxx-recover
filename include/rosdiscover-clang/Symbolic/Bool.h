#pragma once

#include "Value.h"

namespace rosdiscover {
namespace symbolic {

class BoolLiteral : public SymbolicBool {
public:
  BoolLiteral(bool literal) : literal(literal) {}
  ~BoolLiteral() {}

  void print(llvm::raw_ostream &os) const override {
    if (literal) {
      os << "true";
    } else {
      os << "false";
    }
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "bool-literal"},
      {"literal", literal}
    };
  }

private:
  bool literal;
};

} // rosdiscover::symbolic
} // rosdiscover
