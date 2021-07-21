#pragma once

#include <nlohmann/json.hpp>

#include "../RawStatement.h"
#include "Value.h"
#include "Stmt.h"

namespace rosdiscover {
namespace symbolic {

class SymbolicStmt {
public:
  virtual ~SymbolicStmt(){};
  virtual void print(llvm::raw_ostream &os) const = 0;
  virtual nlohmann::json toJson() const = 0;
};

class AssignmentStmt : public SymbolicStmt {
public:
  AssignmentStmt(
    std::string const &varName,
    SymbolicValue *valueExpr
  ) : varName(varName), valueExpr(valueExpr)
  {}
  ~AssignmentStmt(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(assign " << varName << " ";
    valueExpr->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "assignment"},
      {"variable", varName},
      {"value", valueExpr->toJson()}
    };
  }

private:
  std::string varName;
  SymbolicValue *valueExpr; // TODO use unique_ptr!
};

class AnnotatedSymbolicStmt : public SymbolicStmt {
public:
  ~AnnotatedSymbolicStmt(){};

  void print(llvm::raw_ostream &os) const override {
    os << "(@ ";
    symbolicStmt->print(os);
    os << " " << location << ")";
  }

  SymbolicStmt* getSymbolicStmt() {
    return symbolicStmt;
  }

  clang::Stmt* getClangStmt() {
    return clangStmt;
  }

  nlohmann::json toJson() const override {
    auto j = symbolicStmt->toJson();
    j["source-location"] = location;
    return j;
  }

  static AnnotatedSymbolicStmt* create(
      clang::ASTContext &context,
      SymbolicStmt *symbolicStmt,
      RawStatement *rawStmt
  ) {
    auto *clangStmt = rawStmt->getUnderlyingStmt();
    return new AnnotatedSymbolicStmt(
        symbolicStmt,
        rawStmt->getUnderlyingStmt(),
        clangStmt->getSourceRange().printToString(context.getSourceManager())
    );
  }

private:
  // TODO this probably ought to be const?
  SymbolicStmt *symbolicStmt;
  clang::Stmt *clangStmt;
  std::string const location;

  AnnotatedSymbolicStmt(SymbolicStmt* symbolicStmt, clang::Stmt* clangStmt, std::string const &location)
  : symbolicStmt(symbolicStmt), clangStmt(clangStmt), location(location)
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

  nlohmann::json toJson() const {
    auto j = nlohmann::json::array();
    for (auto const &statement : statements) {
      j.push_back(statement->toJson());
    }
    return j;
  }

private:
  // TODO use std::unique_ptr here!
  std::vector<SymbolicStmt*> statements;
};

} // rosdiscover::symbolic
} // rosdiscover
