#pragma once

#include <string>

#include "../Value/Bool.h"
#include "../Value/String.h"
#include "../Value/Value.h"
#include "../Variable/Variable.h"
#include "../Symbolic/Function.h"

namespace rosdiscover {
namespace symbolic {

class StmtBuilder {
public:
  StmtBuilder(SymbolicFunction &function) : function(function) {}

  std::unique_ptr<Publisher> publisher(std::unique_ptr<SymbolicString> name) {
    return std::make_unique<Publisher>(name);
  }

  std::unique_ptr<DeleteParam> deleteParam(std::unique_ptr<SymbolicString> name) {
    return std::make_unique<DeleteParam>(name);
  }

  std::unique_ptr<WriteParam> writeParam(std::unique_ptr<SymbolicString> name, std::unique_ptr<SymbolicValue> value) {
    return std::make_unique<WriteParam>(name, value);
  }

  LocalVariable& symbolizeApiCall(api_call::GetParamWithDefaultCall *apiCall) {
    return createAssignment(new ReadParamWithDefault(symbolizeApiCallName(apiCall), valueBuilder.unknown()));
  }

  std::unique_ptr<AssignmentStmt> readParam(std::unique_ptr<SymbolicString> name) {
    return varDef(std::make_unique<ReadParam>(name));
  }

  std::unique_ptr<AssignmentStmt> readParamWithDefault(std::unique_ptr<SymbolicString> name) {
    return varDef(std::make_unique<ReadParamWithDefault>(name));
  }

private:
  SymbolicFunction &function;

  std::unique_ptr<AssignmentStmt> varDef(std::unique_ptr<SymbolicValue> value) {
    // TODO determine type!
    auto &local = function.createLocal(SymbolicValueType::Unsupported);
    return std::make_unique<AssignmentStmt>(local, std::move(value));
  }
};

} // rosdiscover::symbolic
} // rosdiscover
