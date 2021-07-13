#pragma once

#include <string>
#include <unordered_map>

#include "Function.h"

namespace rosdiscover {
namespace symbolic {

// this is responsible for holding everything in memory
class SymbolicContext {
public:
  SymbolicContext() : functionDefinitions(functionDefinitions) {}
  ~SymbolicContext() {
    for (auto function : functionDefinitions) {
      delete function;
    }
  }

  SymbolicFunction* getDefinition(std::string const &qualifiedName) {
    return functionDefinitions[qualifiedName];
  }

private:
  std::unordered_map<std::string, SymbolicFunction*> functionDefinitions;
};

} // rosdiscover::symbolic
} // rosdiscover
