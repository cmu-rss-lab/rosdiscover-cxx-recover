#pragma once

#include <string>

#include "SymbolicDeclRef.h"

#include <clang/AST/Decl.h>

namespace rosdiscover {

class SymbolicEnumReference : public SymbolicDeclRef {
public:
  SymbolicEnumReference(
    std::string typeName,
    std::string name
  ) : SymbolicDeclRef(false, false, typeName, name)
   {}

  ~SymbolicEnumReference(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(enumRef " << getName() << " : " << getTypeName() << ")";
  }

  nlohmann::json toJson() const override {
    auto j = SymbolicDeclRef::toJson();
    j["kind"] = "enumRef";
    return j;
  }

};

} // rosdiscover
