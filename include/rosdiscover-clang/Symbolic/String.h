#pragma once

#include <string>

#include "Value.h"

namespace rosdiscover {
namespace symbolic {

class StringLiteral : public virtual SymbolicString {
public:
  StringLiteral(std::string const &literal) : literal(literal) {}
  ~StringLiteral() {}

  static StringLiteral* create(std::string const &literal) {
    return new StringLiteral(literal);
  }

  void print(llvm::raw_ostream &os) const override {
    os << "\"" << literal << "\"";
  }

  nlohmann::json toJson() const {
    return {
      {"kind", "string-literal"},
      {"literal", literal}
    };
  }

private:
  std::string const literal;
};

} // rosdiscover::symbolic
} // rosdiscover
