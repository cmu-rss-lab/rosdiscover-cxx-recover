#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <clang/AST/ASTContext.h>

#include <nlohmann/json.hpp>

#include "Function.h"
#include "../Stmt/Stmt.h"

namespace rosdiscover {
namespace symbolic {

class SymbolicContext {
public:
  SymbolicContext() : nameToFunction() {}

  SymbolicFunction* declare(clang::ASTContext const &astContext, clang::FunctionDecl const *function) {
    auto qualifiedName = function->getQualifiedNameAsString();
    nameToFunction.emplace(
        qualifiedName,
        std::unique_ptr<SymbolicFunction>(SymbolicFunction::create(astContext, function))
    );
    auto symbolic = getDefinition(qualifiedName);
    llvm::outs() << "declared symbolic function: " << symbolic->getName() << "\n";
    return symbolic;
  }

  void define(clang::FunctionDecl const *function, std::unique_ptr<SymbolicCompound> body) {
    define(function->getQualifiedNameAsString(), std::move(body));
  }

  void define(std::string const &qualifiedName, std::unique_ptr<SymbolicCompound> body) {
    auto *function = getDefinition(qualifiedName);
    function->define(std::move(body));
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
  // no need for unique_ptr; getters should just return references
  std::unordered_map<std::string, std::unique_ptr<SymbolicFunction>> nameToFunction;
};

} // rosdiscover::symbolic
} // rosdiscover
