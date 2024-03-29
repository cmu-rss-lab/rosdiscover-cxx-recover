#pragma once

#include <string>

#include "Value.h"

namespace rosdiscover {

class StringLiteral : public virtual SymbolicString {
public:
  StringLiteral(std::string const &literal) : literal(literal) {}
  ~StringLiteral() {}

  static StringLiteral* create(std::string const &literal) {
    return new StringLiteral(literal);
  }

  void print(llvm::raw_ostream &os) const override {
    os << toString();
  }
  std::string toString() const override {
    return fmt::format("'\"{}\"'", literal);
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "string-literal"},
      {"literal", literal},
      {"string", toString()},
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

  std::string toString() const override {
    return "node-name";
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
  ) : lhs(std::move(lhs)), rhs(std::move(rhs)) {
    assert(this->lhs != nullptr); 
    assert(this->rhs != nullptr);
  }
  ~Concatenate(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(concatenate ";
    lhs->print(os);
    os << " ";
    rhs->print(os);
    os << ")";
  }

  std::string toString() const override {
    return fmt::format("{} {}", lhs->toString(), rhs->toString());
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "concatenate"},
      {"lhs", lhs->toJson()},
      {"rhs", rhs->toJson()},
      {"string", toString()},
    };
  }

private:
  std::unique_ptr<SymbolicString> lhs;
  std::unique_ptr<SymbolicString> rhs;
};

} // rosdiscover
