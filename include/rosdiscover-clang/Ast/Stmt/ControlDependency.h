#pragma once

#include <string>

#include "../../Value/Value.h"
#include "../Decl/LocalVariable.h"
#include "Stmt.h"
#include "FunctionCall.h"
#include "VariableReference.h"

namespace rosdiscover {

class SymbolicControlDependency : public SymbolicStmt {
public:
  SymbolicControlDependency(
    clang::Stmt* stmt,
    std::vector<std::unique_ptr<SymbolicCall>> functionCalls,
    std::vector<std::unique_ptr<SymbolicVariableReference>> variableReferences
  ) : stmt(stmt), functionCalls(std::move(functionCalls)), variableReferences(std::move(variableReferences)) {}
  ~SymbolicControlDependency(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(controlDependency)";
  }

  nlohmann::json toJson() const override {
    auto j_calls = nlohmann::json::array();
    for (auto const &functionCall : functionCalls) {
      j_calls.push_back(functionCall->toJson());
    }
    auto j_varrefs = nlohmann::json::array();
    for (auto const &variableReference : variableReferences) {
      j_varrefs.push_back(variableReference->toJson());
    }
    return {
      {"kind", "controlDependency"},
      {"calls", j_calls},
      {"variableReferences", j_varrefs},
    };
  }

  clang::Stmt* getStmt() {
    return stmt;
  }

private:
  clang::Stmt* stmt;
  std::vector<std::unique_ptr<SymbolicCall>> functionCalls;
  std::vector<std::unique_ptr<SymbolicVariableReference>> variableReferences;
};

} // rosdiscover
