#pragma once

#include "../RawStatement.h"
#include "Value.h"
#include "Stmt.h"

namespace rosdiscover {
namespace symbolic {

class SymbolicStmt {
public:
  virtual ~SymbolicStmt(){};
  virtual void print(llvm::raw_ostream &os) const = 0;
};

class AnnotatedSymbolicStmt : public SymbolicStmt {
public:
  ~AnnotatedSymbolicStmt(){};
  void print(llvm::raw_ostream &os) const override {
    os << "(@ ";
    symbolicStmt->print(os);
    os << ")";
  }

  SymbolicStmt* getSymbolicStmt() {
    return symbolicStmt;
  }

  clang::Stmt* getClangStmt() {
    return clangStmt;
  }

  static AnnotatedSymbolicStmt* create(
      SymbolicStmt *symbolicStmt,
      RawStatement *rawStmt
  ) {
    return new AnnotatedSymbolicStmt(symbolicStmt, rawStmt->getUnderlyingStmt());
  }

private:
  // TODO this probably ought to be const?
  SymbolicStmt *symbolicStmt;
  clang::Stmt *clangStmt;

  AnnotatedSymbolicStmt(SymbolicStmt* symbolicStmt, clang::Stmt* clangStmt)
  : symbolicStmt(symbolicStmt), clangStmt(clangStmt)
  {}
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
