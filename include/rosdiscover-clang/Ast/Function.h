#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include <fmt/core.h>

#include "../Value/Value.h"
#include "../Value/Bool.h"
#include "../Value/String.h"
#include "Decl/LocalVariable.h"
#include "Decl/Parameter.h"
#include "Stmt/Stmt.h"
#include "Stmt/Compound.h"
#include "Stmt/ControlDependency.h"

namespace rosdiscover {

class SymbolicFunction {
public:
  void print(llvm::raw_ostream &os) const {
    os << "function ";
    qualifiedName->print(os);
    os << " [";
    for (auto const &paramEntry : parameters) {
      paramEntry.second.print(os);
      os << "; ";
    }
    os << "] ";
    body->print(os);
  }

  nlohmann::json toJson() const {
    auto jsonParams = nlohmann::json::array();
    for (auto const &entry : parameters) {
      jsonParams.push_back(entry.second.toJson());
    }

    return {
      {"name", qualifiedName->toJson()},
      {"parameters", jsonParams},
      {"source-location", location},
      {"body", body.get()->toJson()}
    };
  }

  void define(std::unique_ptr<SymbolicCompound> body) {
    this->body = std::move(body);
  }

  LocalVariable* createLocal(SymbolicValueType const &type) {
    auto name = fmt::format("v{:d}", nextLocalNumber++);
    locals.emplace_back(std::make_unique<LocalVariable>(name, type));
    return locals.back().get();
  }

  SymbolicString* getName() const {
    return qualifiedName.get();
  }

  static SymbolicFunction* create(
      clang::ASTContext const &context,
      clang::FunctionDecl const *function
  ) {
    auto qualifiedName = function->getQualifiedNameAsString();
    auto location = function->getLocation().printToString(context.getSourceManager());
    auto symbolic = new SymbolicFunction(std::make_unique<StringLiteral>(qualifiedName), location);

    // TODO check whether this is the "main" function
    auto numParams = function->getNumParams();
    for (size_t paramIndex = 0; paramIndex < numParams; ++paramIndex) {
      symbolic->addParam(paramIndex, function->getParamDecl(paramIndex));
    }

    return symbolic;
  }

  std::unordered_map<size_t, Parameter>::iterator params_begin() {
    return parameters.begin();
  }

  std::unordered_map<size_t, Parameter>::iterator params_end() {
    return parameters.end();
  }

private:
  std::unique_ptr<SymbolicString> qualifiedName;
  std::string location;
  std::unique_ptr<SymbolicCompound> body;
  size_t nextLocalNumber;
  std::unordered_map<size_t, Parameter> parameters;
  std::vector<std::unique_ptr<LocalVariable>> locals;

  SymbolicFunction(
    std::unique_ptr<SymbolicString> qualifiedName,
    std::string const &location
  ) : qualifiedName(std::move(qualifiedName)),
      location(location),
      body(std::make_unique<SymbolicCompound>()),
      nextLocalNumber(0),
      parameters(),
      locals()
  {}

  void addParam(size_t index, clang::ParmVarDecl const *param) {
    // TODO does this have a default?
    auto paramType = param->getOriginalType();
    auto paramTypeName = paramType.getAsString();
    auto name = param->getNameAsString();
    auto type = SymbolicValue::getSymbolicType(paramType);

    if (type == SymbolicValueType::Unsupported) {
      llvm::outs()
        << "DEBUG: type ["
        << paramTypeName
        << "] of parameter ["
        << name
        << "] is unsupported\n";
      return;
    }

    llvm::outs() << "DEBUG: created symbolic parameter [" << name << "]\n";
    addParam(Parameter(index, name, type));
  }

  void addParam(Parameter const &param) {
    parameters.emplace(param.getIndex(), param);
  }
};

class SymbolicFunctionCall : public virtual SymbolicStmt {
public:
  SymbolicFunctionCall(
    SymbolicFunction *callee,
    std::unordered_map<std::string, std::unique_ptr<SymbolicValue>> &args,
    std::unique_ptr<SymbolicExpr> pathCondition = std::make_unique<BoolLiteral>(true)
  ) : callee(callee), args(std::move(args)), pathCondition(std::move(pathCondition)) {
    assert(this->pathCondition != nullptr);
  }
  ~SymbolicFunctionCall(){}

  static std::unique_ptr<SymbolicFunctionCall> create(
    SymbolicFunction *function,
    std::unordered_map<std::string, std::unique_ptr<SymbolicValue>> &args,
    std::unique_ptr<SymbolicExpr> pathCondition
  ) {
    return std::make_unique<SymbolicFunctionCall>(function, args, std::move(pathCondition));
  }

  static std::unique_ptr<SymbolicFunctionCall> create(
    SymbolicFunction *function
  ) {
    std::unordered_map<std::string, std::unique_ptr<SymbolicValue>> emptyArgs;
    return create(function, emptyArgs, std::make_unique<BoolLiteral>(true));
  }

  void print(llvm::raw_ostream &os) const override {
    os << "(call " << callee->getName();
    for (auto const &entry : args) {
      os << " [" << entry.first << ": ";
      entry.second->print(os);
      os << "]";
    }
    os << ")";
  }

  nlohmann::json toJson() const override {
    nlohmann::json argsJson = nlohmann::json::object();
    for (auto const &entry : args) {
      argsJson[entry.first] = entry.second->toJson();
    }
    return {
      {"kind", "call"},
      {"callee", callee->getName()->toJson()},
      {"arguments", argsJson},
      {"path_condition", pathCondition->toString()},
    };
  }

  virtual SymbolicString* const getCalleeName() const {
    return callee->getName();
  }

private:
  SymbolicFunction *callee;
  std::unordered_map<std::string, std::unique_ptr<SymbolicValue>> args;
  std::unique_ptr<SymbolicExpr> pathCondition;
};

class UnknownSymbolicFunctionCall : public SymbolicFunctionCall {
public:
  UnknownSymbolicFunctionCall(std::unordered_map<std::string, std::unique_ptr<SymbolicValue>> &args) : SymbolicFunctionCall(nullptr, args), calleeName(std::make_unique<SymbolicUnknown>()) {}
  ~UnknownSymbolicFunctionCall(){}
  
  static std::unique_ptr<UnknownSymbolicFunctionCall> create() {
    std::unordered_map<std::string, std::unique_ptr<SymbolicValue>> emptyArgs;
    return std::make_unique<UnknownSymbolicFunctionCall>(emptyArgs);
  }


  void print(llvm::raw_ostream &os) const override {
    os << "UNKNOWN";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "unknown"}
    };
  }

  virtual SymbolicString* const getCalleeName() const override {
    return calleeName.get();
  }

private:
  std::unique_ptr<SymbolicString> calleeName;



};

} // rosdiscover
