#pragma once

#include <string>

#include "../Value/Bool.h"
#include "../Value/String.h"
#include "../Value/Value.h"
#include "../Variable/Variable.h"

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

  std::unique_ptr<Concatenate> concatenate(
    std::unique_ptr<StringLiteral> lhs,
    std::unique_ptr<StringLiteral> rhs
  ) const {
    return std::make_unique<Concatenate>(std::move(lhs), std::move(rhs));
  }

  std::unique_ptr<SymbolicNodeHandle> nodeHandle(std::string const &name) const {
    return nodeHandle(stringLiteral(name));
  }

  std::unique_ptr<SymbolicNodeHandle> nodeHandle(std::unique_ptr<SymbolicString> name) const {
    return std::make_unique<SymbolicNodeHandle>(std::move(name));
  }
};

} // rosdiscover::symbolic
} // rosdiscover
