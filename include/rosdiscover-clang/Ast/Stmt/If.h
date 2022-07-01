#pragma once

#include <string>

#include "../../Value/Value.h"
#include "../Decl/LocalVariable.h"
#include "Stmt.h"
#include "Compound.h"

namespace rosdiscover {

class SymbolicIfStmt : public SymbolicStmt {
public:
  SymbolicIfStmt(
    clang::Stmt* stmt,
    std::unique_ptr<SymbolicBool> condition,
    std::unique_ptr<SymbolicCompound> trueBranchBody,
    std::unique_ptr<SymbolicCompound> falseBranchBody
  ) : stmt(stmt), condition(std::move(condition)), trueBranchBody(std::move(trueBranchBody)), falseBranchBody(std::move(falseBranchBody))
  {}
  ~SymbolicIfStmt(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(if ";
    condition->print(os);
    os << " ";
    trueBranchBody->print(os);
    os << " else ";
    falseBranchBody->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "if"},
      {"condition", condition->toJson()},
      {"trueBranchBody" ,  trueBranchBody->toJson()},
      {"falseBranchBody", falseBranchBody->toJson()}
    };
  }

  clang::Stmt* getStmt() {
    return stmt;
  }

private:
  clang::Stmt* stmt;
  std::unique_ptr<SymbolicBool> condition;
  std::unique_ptr<SymbolicCompound> trueBranchBody;
  std::unique_ptr<SymbolicCompound> falseBranchBody;
};

} // rosdiscover
