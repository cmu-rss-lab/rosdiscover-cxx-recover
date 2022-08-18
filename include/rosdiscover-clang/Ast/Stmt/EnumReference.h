#pragma once

#include <string>

#include <clang/AST/Decl.h>

#include "SymbolicDeclRef.h"
#include "../../Value/Value.h"

namespace rosdiscover {

class SymbolicEnumReference : public SymbolicDeclRef {
public:
  SymbolicEnumReference(
    std::string typeName,
    std::string name,
    std::string qualifiedName,
    long value
  ) : SymbolicDeclRef(false, false, typeName, name, qualifiedName), value(value)
   {}

  ~SymbolicEnumReference(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(enum-ref " << toString() << " : " << getTypeName() << ")";
  }

  std::string toString() const override {
    return fmt::format("{}:={}", getName(), value);
  }

  long getValue() const {
    return value;
  }

  nlohmann::json toJson() const override {
    auto j = SymbolicDeclRef::toJson();
    j["kind"] = "enum-ref";
    j["value"] = value;
    return j;
  }
private:
  long value;
};

} // rosdiscover
