#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "Function.h"
#include "Stmt.h"

namespace rosdiscover {
namespace symbolic {

// this is responsible for holding everything in memory
//
// idea: use this to manage creation and destruction of stmts? or use shared pointers?
class SymbolicContext {
public:
  SymbolicContext() : nameToFunction() {}

  SymbolicFunction* declare(clang::FunctionDecl const *function) {
    auto qualifiedName = function->getQualifiedNameAsString();
    nameToFunction.emplace(qualifiedName, std::unique_ptr<SymbolicFunction>(SymbolicFunction::create(function)));
    auto symbolic = getDefinition(qualifiedName);
    llvm::outs() << "declared symbolic function: " << symbolic->getName() << "\n";
    return symbolic;
  }

  void define(clang::FunctionDecl const *function, SymbolicCompound &body) {
    define(function->getQualifiedNameAsString(), body);
  }

  void define(std::string const &qualifiedName, SymbolicCompound &body) {
    auto *function = getDefinition(qualifiedName);
    function->define(body);
  }

  SymbolicFunction* getDefinition(clang::FunctionDecl const *function) {
    return getDefinition(function->getQualifiedNameAsString());
  }

  SymbolicFunction* getDefinition(std::string const &qualifiedName) {
    return nameToFunction[qualifiedName].get();
  }

  void print(llvm::raw_ostream &os) const {
    os << "context {\n";
    for (auto const &entry : nameToFunction) {
      entry.second->print(os);
      os << "\n";
    }
    os << "}";
  }

  nlohmann::json toJson() const {
    auto j = nlohmann::json::array();
    for (auto const &entry : nameToFunction) {
      j.push_back(entry.second->toJson());
    }
    return {{"functions", j}};
  }

private:
  std::unordered_map<std::string, std::unique_ptr<SymbolicFunction>> nameToFunction;
};

} // rosdiscover::symbolic
} // rosdiscover
