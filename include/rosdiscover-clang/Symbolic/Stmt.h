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
    return symbolicStmt.get();
  }

  clang::Stmt* getClangStmt() {
    return clangStmt;
  }

  nlohmann::json toJson() const override {
    auto j = symbolicStmt->toJson();
    j["source-location"] = location;
    return j;
  }

  static std::unique_ptr<AnnotatedSymbolicStmt> create(
      clang::ASTContext &context,
      std::unique_ptr<SymbolicStmt> symbolicStmt,
      RawStatement *rawStmt
  ) {
    auto *clangStmt = rawStmt->getUnderlyingStmt();
    return std::make_unique<AnnotatedSymbolicStmt>(
        std::move(symbolicStmt),
        rawStmt->getUnderlyingStmt(),
        clangStmt->getSourceRange().printToString(context.getSourceManager())
    );
  }

  AnnotatedSymbolicStmt(
    std::unique_ptr<SymbolicStmt> symbolicStmt,
    clang::Stmt* clangStmt,
    std::string const &location
  ) : symbolicStmt(std::move(symbolicStmt)), clangStmt(clangStmt), location(location)
  {}

private:
  // TODO this probably ought to be const?
  std::unique_ptr<SymbolicStmt> symbolicStmt;
  clang::Stmt *clangStmt;
  std::string const location;
};

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
    auto j = nlohmann::json::array();
    for (auto const &statement : statements) {
      j.push_back(statement->toJson());
    }
    return j;
  }

private:
  std::vector<std::unique_ptr<SymbolicStmt>> statements;
};

} // rosdiscover::symbolic
} // rosdiscover
