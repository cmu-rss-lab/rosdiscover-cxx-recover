#pragma once

#include <nlohmann/json.hpp>

#include "Value.h"
#include "Stmt.h"

namespace rosdiscover {
namespace symbolic {

class SymbolicFunctionParameter {
public:
  size_t getIndex() const { return index; }
  std::string const & getName() const { return name; }
  SymbolicValueType getType() const { return type; }

  // TODO add support for default values here!

  SymbolicFunctionParameter(size_t index, std::string const &name, SymbolicValueType const type)
    : index(index), name(name), type(type)
  {}

  nlohmann::json toJson() const {
    return {
      {"index", index},
      {"name", name},
      {"type", SymbolicValue::getSymbolicTypeAsString(type)}
    };
  }

  void print(llvm::raw_ostream &os) const {
    os << index
      << ":"
      << name
      << ":"
      << SymbolicValue::getSymbolicTypeAsString(type);
  }

private:
  size_t const index;
  std::string const name;
  SymbolicValueType const type;
};

class SymbolicFunction {
public:
  void print(llvm::raw_ostream &os) const {
    os << "function " << qualifiedName << " [";
    for (auto const &paramEntry : parameters) {
      paramEntry.second.print(os);
      os << "; ";
    }
    os << "] ";
    body.print(os);
  }

  nlohmann::json toJson() const {
    auto jsonParams = nlohmann::json::array();
    for (auto const &entry : parameters) {
      jsonParams.push_back(entry.second.toJson());
    }

    return {
      {"name", qualifiedName},
      {"parameters", jsonParams},
      {"source-location", "TODO: add this info!"},
      {"body", body.toJson()}
    };
  }

  void define(SymbolicCompound &body) {
    this->body = body;
  }

  std::string getName() const {
    return qualifiedName;
  }

  static SymbolicFunction* create(
      clang::FunctionDecl const *function
  ) {
    auto qualifiedName = function->getQualifiedNameAsString();
    auto symbolic = new SymbolicFunction(qualifiedName);

    // TODO check whether this is the "main" function
    auto numParams = function->getNumParams();
    for (size_t paramIndex = 0; paramIndex < numParams; ++paramIndex) {
      symbolic->addParam(paramIndex, function->getParamDecl(paramIndex));
    }

    return symbolic;
  }

private:
  std::string qualifiedName;
  SymbolicCompound body;
  std::unordered_map<size_t, SymbolicFunctionParameter> parameters;

  SymbolicFunction(std::string const &qualifiedName)
    : qualifiedName(qualifiedName), body(), parameters()
  {}

  void addParam(size_t index, clang::ParmVarDecl const *param) {
    // TODO does this have a default?
    auto name = param->getNameAsString();
    auto type = SymbolicValue::getSymbolicType(param->getOriginalType());

    if (type == SymbolicValueType::Unsupported)
      return;

    addParam(SymbolicFunctionParameter(index, name, type));
  }

  void addParam(SymbolicFunctionParameter const &param) {
    parameters.emplace(param.getIndex(), param);
  }
};

// TODO record symbolic function call arguments
class SymbolicFunctionCall : public virtual SymbolicStmt {
public:
  SymbolicFunctionCall(SymbolicFunction *callee) : callee(callee) {}
  ~SymbolicFunctionCall(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(call " << callee->getName() << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "call"},
      {"callee", callee->getName()}
    };
  }

private:
  SymbolicFunction *callee;
};

} // rosdiscover::symbolic
} // rosdiscover
