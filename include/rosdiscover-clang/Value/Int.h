#pragma once

#include <string>

#include "Value.h"

namespace rosdiscover {

class IntegerLiteral : public virtual SymbolicInteger {
public:
  IntegerLiteral(int const &literal) : literal(literal) {}
  ~IntegerLiteral() {}

  static IntegerLiteral* create(int const &literal) {
    return new IntegerLiteral(literal);
  }

  void print(llvm::raw_ostream &os) const override {
    os << "\"" << literal << "\"";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "integer-literal"},
      {"literal", literal}
    };
  }

private:
  int const literal;
};

} // rosdiscover
