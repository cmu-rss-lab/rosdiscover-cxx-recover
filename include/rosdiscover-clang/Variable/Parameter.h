#pragma once

#include <string>

#include "Variable.h"

namespace rosdiscover {
namespace symbolic {

class Parameter : public SymbolicVariable {
public:
  size_t getIndex() const { return index; }
  std::string getName() const override { return name; }
  SymbolicValueType getType() const override { return type; }

  // TODO add support for default values here!
  Parameter(size_t index, std::string const &name, SymbolicValueType const type)
    : index(index), name(name), type(type)
  {}

  ~Parameter(){}

  nlohmann::json toJson() const {
    return {
      {"index", index},
      {"name", name},
      {"type", SymbolicValue::getSymbolicTypeAsString(type)}
    };
  }

  void print(llvm::raw_ostream &os) const {
    os << index
      << ":"
      << name
      << ":"
      << SymbolicValue::getSymbolicTypeAsString(type);
  }

private:
  size_t const index;
  std::string const name;
  SymbolicValueType const type;
};

} // rosdiscover::symbolic
} // rosdiscover