#pragma once

#include <string>

#include "../Value/Value.h"
#include "../Variable/LocalVariable.h"
#include "Stmt.h"

namespace rosdiscover {
namespace symbolic {

class AssignmentStmt : public SymbolicStmt {
public:
  AssignmentStmt(
    LocalVariable const *variable,
    std::unique_ptr<SymbolicValue> value
  ) : variable(variable), value(std::move(value))
  {}
  ~AssignmentStmt(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(assign " << variable->getName() << " ";
    value->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "assignment"},
      {"variable", variable->getName()},
      {"value", value->toJson()}
    };
  }

private:
  LocalVariable const *variable;
  std::unique_ptr<SymbolicValue> value;
};

} // rosdiscover::symbolic
} // rosdiscover