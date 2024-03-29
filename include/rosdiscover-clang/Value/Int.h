#pragma once

#include <string>

#include "Value.h"

namespace rosdiscover {

class IntegerLiteral : public virtual SymbolicInteger {
public:
  IntegerLiteral(long const &literal) : literal(literal) {}
  ~IntegerLiteral() {}

  static IntegerLiteral* create(long const &literal) {
    return new IntegerLiteral(literal);
  }

  void print(llvm::raw_ostream &os) const override {
    os << toString();
  }
  
  std::string toString() const override {
    return fmt::format("'{}'", literal);
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "int-literal"},
      {"literal", literal},
      {"string", toString()},
    };
  }

private:
  long const literal;
};

} // rosdiscover
