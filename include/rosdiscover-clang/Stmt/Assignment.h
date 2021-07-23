#pragma once

#include <string>

#include "Stmt.h"
#include "../Value/Value.h"

namespace rosdiscover {
namespace symbolic {

class AssignmentStmt : public SymbolicStmt {
public:
  AssignmentStmt(
    std::string const &varName,
    std::unique_ptr<SymbolicValue> value
  ) : varName(varName), value(std::move(value))
  {}
  ~AssignmentStmt(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(assign " << varName << " ";
    value->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "assignment"},
      {"variable", varName},
      {"value", value->toJson()}
    };
  }

private:
  std::string varName;
  std::unique_ptr<SymbolicValue> value;
};

} // rosdiscover::symbolic
} // rosdiscover