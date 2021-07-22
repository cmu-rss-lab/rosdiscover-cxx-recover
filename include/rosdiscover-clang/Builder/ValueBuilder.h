#pragma once

#include <string>

#include "../Value/Bool.h"
#include "../Value/String.h"
#include "../Value/Value.h"
#include "../Symbolic/Variable.h"

namespace rosdiscover {
namespace symbolic {

class ValueBuilder {
public:
  std::unique_ptr<BoolLiteral> boolLiteral(bool literal) const {
    return std::make_unique<BoolLiteral>(literal);
  }

  std::unique_ptr<StringLiteral> stringLiteral(std::string const &string) const {
    return std::make_unique<StringLiteral>(string);
  }

  std::unique_ptr<SymbolicUnknown> unknown() const {
    return std::make_unique<SymbolicUnknown>();
  }

  std::unique_ptr<VariableReference> varRef(SymbolicVariable const *var) {
    return std::make_unique<VariableReference>(var);
  }
};

} // rosdiscover::symbolic
} // rosdiscover