#pragma once

#include <string>

#include "../Value/Bool.h"
#include "../Value/String.h"
#include "../Value/Int.h"
#include "../Value/Float.h"
#include "../Value/Value.h"
#include "../Ast/Decl/Decls.h"

namespace rosdiscover {

class ValueBuilder {
public:
  std::unique_ptr<BoolLiteral> boolLiteral(bool literal) const {
    return std::make_unique<BoolLiteral>(literal);
  }

  std::unique_ptr<StringLiteral> stringLiteral(std::string const &string) const {
    return std::make_unique<StringLiteral>(string);
  }

  std::unique_ptr<FloatingLiteral> floatingLiteral(double const &i) const {
    return std::make_unique<FloatingLiteral>(i);
  }  

  std::unique_ptr<IntegerLiteral> integerLiteral(int const &i) const {
    return std::make_unique<IntegerLiteral>(i);
  }  

  std::unique_ptr<NodeName> nodeName() const {
    return std::make_unique<NodeName>();
  }

  std::unique_ptr<SymbolicUnknown> unknown() const {
    return std::make_unique<SymbolicUnknown>();
  }

  std::unique_ptr<SymbolicRateImpl> unknownRate() const {
    return std::make_unique<SymbolicRateImpl>(unknown());
  }

  std::unique_ptr<SymbolicPublisherImpl> publisher(std::string qualifiedName) const {
    return std::make_unique<SymbolicPublisherImpl>(stringLiteral(qualifiedName));
  }
  
  std::unique_ptr<SymbolicPublisherImpl> unknownPublisher() const {
    return std::make_unique<SymbolicPublisherImpl>(unknown());
  }

  std::unique_ptr<SymbolicNodeHandle> unknownNodeHandle() const {
    return std::make_unique<SymbolicNodeHandleImpl>(unknown());
  }

  std::unique_ptr<SymbolicArg> arg(std::string const &name) const {
    return std::make_unique<SymbolicArg>(name);
  }

  std::unique_ptr<SymbolicArg> arg(Parameter const *param) const {
    return arg(param->getName());
  }

  std::unique_ptr<VariableReference> varRef(SymbolicVariable const *var) {
    return std::make_unique<VariableReference>(var);
  }

  std::unique_ptr<Concatenate> concatenate(
    std::unique_ptr<SymbolicString> lhs,
    std::unique_ptr<SymbolicString> rhs
  ) const {
    return std::make_unique<Concatenate>(std::move(lhs), std::move(rhs));
  }

  std::unique_ptr<SymbolicNodeHandle> publicNodeHandle() const {
    return nodeHandle(stringLiteral(""));
  }

  std::unique_ptr<SymbolicNodeHandle> privateNodeHandle() const {
    return nodeHandle(stringLiteral("~"));
  }

  std::unique_ptr<SymbolicNodeHandle> nodeHandle(std::string const &name) const {
    return nodeHandle(stringLiteral(name));
  }

  std::unique_ptr<SymbolicNodeHandle> nodeHandle(std::unique_ptr<SymbolicString> name) const {
    return std::make_unique<SymbolicNodeHandleImpl>(std::move(name));
  }
};

} // rosdiscover
