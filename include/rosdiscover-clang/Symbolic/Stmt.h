#pragma once

#include "Value.h"
#include "Stmt.h"

namespace rosdiscover {
namespace symbolic {

class SymbolicStmt {
public:
  virtual ~SymbolicStmt(){};
  virtual void print(llvm::raw_ostream &os) const = 0;
};

class SymbolicCompound {
public:
  SymbolicCompound() : statements() {}

  void append(SymbolicStmt *statement) {
    statements.push_back(statement);
  }

  void print(llvm::raw_ostream &os) const {
    os << "{\n";
    for (auto const &statement : statements) {
      statement->print(os);
      os << "\n";
    }
    os << "}";
  }

private:
  std::vector<SymbolicStmt*> statements;
};

} // rosdiscover::symbolic
} // rosdiscover
