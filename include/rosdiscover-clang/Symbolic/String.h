#pragma once

#include <string>

#include "Value.h"

namespace rosdiscover {
namespace symbolic {

class StringLiteral : public virtual SymbolicString {
public:
  StringLiteral(std::string const &literal) : literal(literal) {}
  ~StringLiteral() {}

  void print(llvm::raw_ostream &os) const override {
    os << "\"" << literal << "\"";
  }

private:
  std::string const literal;
};

} // rosdiscover::symbolic
} // rosdiscover
