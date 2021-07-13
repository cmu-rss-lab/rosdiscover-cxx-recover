#pragma once

#include "Value.h"

namespace rosdiscover {
namespace symbolic {

// TODO for now, we don't record the arguments provided to the function call!
class FunctionCall : public SymbolicStmt {
public:
  FunctionCall(Function &literal) : literal(literal) {}
  ~StringLiteral() {}

  void print(llvm::raw_ostream &os) const {
    os << "\"" << literal << "\"";
  }

private:
  std::string literal;
};

} // rosdiscover::symbolic
} // rosdiscover
