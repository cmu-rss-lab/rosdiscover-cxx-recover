#pragma once

#include <string>

#include "Variable.h"

namespace rosdiscover {
namespace symbolic {

class LocalVariable : public SymbolicVariable {
public:
  LocalVariable(std::string const &name, SymbolicValueType const &type)
  : name(name), type(type)
  {}

  ~LocalVariable(){}

  std::string getName() const override {
    return name;
  }

  SymbolicValueType getType() const override {
    return type;
  }

private:
  std::string name;
  SymbolicValueType const type;
};

} // rosdiscover::symbolic
} // rosdiscover