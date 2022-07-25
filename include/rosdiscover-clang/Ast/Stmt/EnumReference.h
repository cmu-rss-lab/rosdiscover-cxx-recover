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
    long value
  ) : SymbolicDeclRef(false, false, typeName, name), value(value)
   {}

  ~SymbolicEnumReference(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(enumRef " << toString() << " : " << getTypeName() << ")";
  }

  std::string toString() const override {
    return fmt::format("{}:={}", getName(), value);
  }

  long getValue() const {
    return value;
  }

  nlohmann::json toJson() const override {
    auto j = SymbolicDeclRef::toJson();
    j["kind"] = "enumRef";
    j["value"] = value;
    return j;
  }
private:
  long value;
};

} // rosdiscover
