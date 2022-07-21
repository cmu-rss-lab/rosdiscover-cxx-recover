#pragma once

#include <string>

#include "../../Value/Value.h"
#include "../Decl/LocalVariable.h"
#include "Stmt.h"
#include "Compound.h"

namespace rosdiscover {

class SymbolicWhileStmt : public SymbolicStmt {
public:
  SymbolicWhileStmt(
    clang::Stmt* stmt,
    std::unique_ptr<SymbolicBool> condition,
    std::unique_ptr<SymbolicCompound> body
  ) : stmt(stmt), condition(std::move(condition)), body(std::move(body)) {    
    assert(this->body != nullptr);
    assert(this->condition != nullptr);
  }
  ~SymbolicWhileStmt(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(while ";
    condition->print(os);
    os << " ";
    body->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "while"},
      {"condition", condition->toJson()},
      {"body" ,  body->toJson()}
    };
  }

  clang::Stmt* getStmt() {
    return stmt;
  }

private:
  clang::Stmt* stmt;
  std::unique_ptr<SymbolicBool> condition;
  std::unique_ptr<SymbolicCompound> body;
};

} // rosdiscover
