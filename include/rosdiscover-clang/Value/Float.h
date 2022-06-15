#pragma once

#include <string>

#include "Value.h"

namespace rosdiscover {

class FloatingLiteral : public virtual SymbolicFloat {
public:
  FloatingLiteral(double const &literal) : literal(literal) {}
  ~FloatingLiteral() {}

  static FloatingLiteral* create(double const &literal) {
    return new FloatingLiteral(literal);
  }

  void print(llvm::raw_ostream &os) const override {
    os << "\"" << literal << "\"";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "float-literal"},
      {"literal", literal}
    };
  }

private:
  double const literal;
};

} // rosdiscover
