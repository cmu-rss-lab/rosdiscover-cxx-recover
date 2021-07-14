#pragma once

#include "Value.h"
#include "Stmt.h"

namespace rosdiscover {
namespace symbolic {

// TODO for now, we don't record the arguments provided to the function call!
class SymbolicFunction {
public:
  SymbolicFunction(std::string const &qualifiedName)
    : qualifiedName(qualifiedName), body()
  {}

  void print(llvm::raw_ostream &os) const {
    os << "function " << qualifiedName << " {\n";
    body.print(os);
    os << "\n}";
  }

  void define(SymbolicCompound &body) {
    body = body;
  }

private:
  std::string qualifiedName;
  SymbolicCompound body;
};

// TODO record symbolic function call arguments
class SymbolicFunctionCall : public SymbolicStmt {
public:
  SymbolicFunctionCall(SymbolicFunction *callee) : callee(callee) {}

private:
  SymbolicFunction *callee;
};

} // rosdiscover::symbolic
} // rosdiscover
