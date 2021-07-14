#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Function.h"
#include "Stmt.h"

namespace rosdiscover {
namespace symbolic {

// this is responsible for holding everything in memory
class SymbolicContext {
public:
  SymbolicContext() : nameToFunction() {}

  SymbolicFunction* declare(clang::FunctionDecl const *function) {
    return declare(function->getQualifiedNameAsString());
  }

  SymbolicFunction* declare(std::string const &qualifiedName) {
    nameToFunction.emplace(qualifiedName, std::make_unique<SymbolicFunction>(qualifiedName));
    llvm::outs() << "declared symbolic function: " << qualifiedName << "\n";
    return getDefinition(qualifiedName);
  }

  void define(clang::FunctionDecl const *function, SymbolicCompound &body) {
    define(function->getQualifiedNameAsString(), body);
  }

  void define(std::string const &qualifiedName, SymbolicCompound &body) {
    auto *function = getDefinition(qualifiedName);
    function->define(body);
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

private:
  std::unordered_map<std::string, std::unique_ptr<SymbolicFunction>> nameToFunction;
};

} // rosdiscover::symbolic
} // rosdiscover
