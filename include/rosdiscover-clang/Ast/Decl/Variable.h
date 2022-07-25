#pragma once

#include <string>

#include "../../Value/Value.h"

namespace rosdiscover {

class SymbolicVariable {
public:
  virtual ~SymbolicVariable(){}
  virtual std::string getName() const = 0;
  virtual SymbolicValueType getType() const = 0;

  std::string getTypeAsString() const {
    return SymbolicValue::getSymbolicTypeAsString(getType());
  }
  
};

class VariableReference :
  public virtual SymbolicValue,
  public virtual SymbolicString,
  public virtual SymbolicBool,
  public virtual SymbolicInteger
{
public:
  VariableReference(SymbolicVariable const *variable) : variable(variable) {
    assert(this->variable != nullptr);
  }
  ~VariableReference(){}

  SymbolicVariable const * getVariable() const {
    return variable;
  }

  void print(llvm::raw_ostream &os) const override {
    os << toString();
  }

  std::string toString() const override {
    return variable->getName();
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "variable-reference"},
      {"variable", variable->getName()},
      {"type", variable->getTypeAsString()}
    };
  }

private:
  SymbolicVariable const *variable;
};

} // rosdiscover
