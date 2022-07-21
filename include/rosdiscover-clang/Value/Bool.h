#pragma once

#include "Value.h"

namespace rosdiscover {

class BoolLiteral : public SymbolicBool {
public:
  BoolLiteral(bool literal) : literal(literal) {}
  ~BoolLiteral() {}

  void print(llvm::raw_ostream &os) const override {
    os << toString();
  }

  std::string toString() const override {
    if (literal) {
      return "'true'";
    } else {
      return "'false'";
    }
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "bool-literal"},
      {"literal", literal},
      {"string", toString()},
    };
  }

private:
  bool literal;
};

} // rosdiscover
