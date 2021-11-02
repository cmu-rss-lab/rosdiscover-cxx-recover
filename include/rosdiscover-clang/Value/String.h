#pragma once

#include <string>

#include "Value.h"

namespace rosdiscover {
namespace symbolic {

class StringLiteral : public virtual SymbolicString {
public:
  StringLiteral(std::string const &literal) : literal(literal) {}
  ~StringLiteral() {}

  static StringLiteral* create(std::string const &literal) {
    return new StringLiteral(literal);
  }

  void print(llvm::raw_ostream &os) const override {
    os << "\"" << literal << "\"";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "string-literal"},
      {"literal", literal}
    };
  }

private:
  std::string const literal;
};

class NodeName : public virtual SymbolicString {
public:
  NodeName(){}
  ~NodeName(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(node-name)";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "node-name"}
    };
  }
};

class Concatenate : public virtual SymbolicString {
public:
  Concatenate(
    std::unique_ptr<SymbolicString> lhs,
    std::unique_ptr<SymbolicString> rhs
  ) : lhs(std::move(lhs)), rhs(std::move(rhs))
  {}
  ~Concatenate(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(concatenate ";
    lhs->print(os);
    os << " ";
    rhs->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "concatenate"},
      {"lhs", lhs->toJson()},
      {"rhs", rhs->toJson()}
    };
  }

private:
  std::unique_ptr<SymbolicString> lhs;
  std::unique_ptr<SymbolicString> rhs;
};

} // rosdiscover::symbolic
} // rosdiscover
