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
    std::vector<std::unique_ptr<SymbolicCall>> functionCalls,
    std::vector<std::unique_ptr<SymbolicVariableReference>> variableReferences, 
    std::string const location,
    std::string const condition=""
  ) : functionCalls(std::move(functionCalls)), 
      variableReferences(std::move(variableReferences)), 
      location(location), 
      condition(condition) {}
  ~SymbolicControlDependency(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(controlDependency)";
  }

  nlohmann::json toJson() const override {
    auto jCalls = nlohmann::json::array();
    for (auto const &functionCall : functionCalls) {
      jCalls.push_back(functionCall->toJson());
    }
    auto jVarrefs = nlohmann::json::array();
    for (auto const &variableReference : variableReferences) {
      jVarrefs.push_back(variableReference->toJson());
    }
    return {
      {"kind", "controlDependency"},
      {"calls", jCalls},
      {"variableReferences", jVarrefs},
      {"source-location", location},
      {"condition", condition},
    };
  }

private:
  std::vector<std::unique_ptr<SymbolicCall>> functionCalls;
  std::vector<std::unique_ptr<SymbolicVariableReference>> variableReferences;
  std::string const location;
  std::string const condition;
};

} // rosdiscover
