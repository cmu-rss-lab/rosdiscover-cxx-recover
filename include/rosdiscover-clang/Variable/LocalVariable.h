#pragma once

#include <string>

#include "Variable.h"

namespace rosdiscover {
namespace symbolic {

class LocalVariable : public SymbolicVariable {
  ~LocalVariable(){}

  std::string getName() const override {
    return name;
  }

  SymbolicValueType getType() const override {
    return type;
  }

private:
  std::string name;
  SymbolicValuetype type;
};

} // rosdiscover::symbolic
} // rosdiscover