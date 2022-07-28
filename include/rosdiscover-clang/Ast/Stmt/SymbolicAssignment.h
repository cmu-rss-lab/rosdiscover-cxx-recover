#pragma once

#include <string>

#include "../Stmt/Stmt.h"
#include "../Stmt/VariableReference.h"
#include "../Stmt/SymbolicExpr.h"

namespace rosdiscover {

class SymbolicAssignment : public SymbolicStmt {
public:
  SymbolicAssignment(
    std::unique_ptr<SymbolicVariableReference> var,
    std::unique_ptr<SymbolicExpr> expr
  ) : var(std::move(var)), expr(std::move(expr)) {
    assert(this->var != nullptr);
    assert(this->expr != nullptr);
  }
  
  ~SymbolicAssignment(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(assign ";
    var->print(os);
    os << " = ";
    expr->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "assign"},
      {"var", var->toJson()},
      {"expr", expr->toJson()},
    };
  }

  const SymbolicVariableReference* getVar() const {
    return var.get();
  }

  const SymbolicExpr* getExpr() const {
    return expr.get();
  }

private: 
  std::unique_ptr<SymbolicVariableReference> var;
  std::unique_ptr<SymbolicExpr> expr;
};

} // rosdiscover
