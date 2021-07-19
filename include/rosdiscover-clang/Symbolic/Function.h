#pragma once

#include "Value.h"
#include "Stmt.h"

namespace rosdiscover {
namespace symbolic {

// TODO for now, we don't record the arguments provided to the function call!
class SymbolicFunction {
public:
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

  static SymbolicFunction* create(
      clang::FunctionDecl const *function
  ) {
    // TODO check whether this is the "main" function
    auto numParams = function->getNumParams();
    for (size_t paramIndex = 0; paramIndex < numParams; ++paramIndex) {
      auto *param = function->getParamDecl(paramIndex);

      // TODO does this have a default?

      // get the name of the underlying unqualified type
      auto typeName = param->getOriginalType().getUnqualifiedType().getAsString();
      llvm::outs() << "Parameter type: " << typeName << "\n";
    }


    auto qualifiedName = function->getQualifiedNameAsString();
    return new SymbolicFunction(qualifiedName);
  }

private:
  std::string qualifiedName;
  SymbolicCompound body;

  SymbolicFunction(std::string const &qualifiedName)
    : qualifiedName(qualifiedName), body()
  {}
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
