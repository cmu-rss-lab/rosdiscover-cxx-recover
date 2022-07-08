#pragma once

#include <vector>

#include "Stmt.h"

namespace rosdiscover {

class SymbolicCompound : public SymbolicStmt {
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
    auto jStatements = nlohmann::json::array();
    for (auto const &statement : statements) {
      jStatements.push_back(statement->toJson());
    }
    return {
      {"kind", "compound"},
      {"statements", jStatements}
    };
  }

private:
  std::vector<std::unique_ptr<SymbolicStmt>> statements;
};

} // rosdiscover
