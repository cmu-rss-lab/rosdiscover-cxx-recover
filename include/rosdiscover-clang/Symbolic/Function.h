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
    os << "function " << qualifiedName << " ";
    body.print(os);
  }

  void define(SymbolicCompound &body) {
    this->body = body;
  }

  std::string getName() const {
    return qualifiedName;
  }

private:
  std::string qualifiedName;
  SymbolicCompound body;
};

// TODO record symbolic function call arguments
class SymbolicFunctionCall : public virtual SymbolicStmt {
public:
  SymbolicFunctionCall(SymbolicFunction *callee) : callee(callee) {}
  ~SymbolicFunctionCall(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(call " << callee->getName() << ")";
  }

private:
  SymbolicFunction *callee;
};

} // rosdiscover::symbolic
} // rosdiscover
