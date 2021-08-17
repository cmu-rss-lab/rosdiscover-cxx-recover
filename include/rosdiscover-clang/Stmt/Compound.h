#pragma once

#include <vector>

#include "Stmt.h"

namespace rosdiscover {
namespace symbolic {

class SymbolicCompound {
public:
  SymbolicCompound() : statements() {}
  ~SymbolicCompound(){}

  SymbolicCompound(SymbolicCompound&& other)
  : statements(std::move(other.statements))
  {}

  void append(std::unique_ptr<SymbolicStmt> statement) {
    statements.push_back(std::move(statement));
  }

  void print(llvm::raw_ostream &os) const {
    os << "{\n";
    for (auto const &statement : statements) {
      statement->print(os);
      os << "\n";
    }
    os << "}";
  }

  nlohmann::json toJson() const {

    auto j_statements = nlohmann::json::array();
    for (auto const &statement : statements) {
      j_statements.push_back(statement->toJson());
    }
    return {
      {"kind", "compound"},
      {"statements", j_statements}
    };
  }

private:
  std::vector<std::unique_ptr<SymbolicStmt>> statements;
};

} // rosdiscover::symbolic
} // rosdiscover
