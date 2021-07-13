#pragma once

#include "Value.h"

namespace rosdiscover {
namespace symbolic {

// TODO for now, we don't record the arguments provided to the function call!
class SymbolicFunction {
public:
  SymbolicFunction(
      std::string qualifiedName,
      SymbolicCompound body
  ) : qualifiedName(qualifiedName), body(body)
  {}

  void print(llvm::raw_ostream &os) const {
    os << "function " << qualifiedName << " {\n";
    body.print(os);
    os << "\n}";
  }

private:
  std::string qualifiedName;
  SymbolicCompound body;
};

} // rosdiscover::symbolic
} // rosdiscover
