#pragma once

#include <string>

#include <llvm/Support/raw_ostream.h>

#include "Value.h"

namespace rosdiscover {
namespace symbolic {

class StringLiteral : public SymbolicString {
public:
  StringLiteral(std::string const &literal) : literal(literal) {}
  ~StringLiteral() {}

  void print(llvm::raw_ostream &os) const {
    os << "\"" << literal << "\"";
  }

private:
  std::string literal;
};

} // rosdiscover::symbolic
} // rosdiscover
