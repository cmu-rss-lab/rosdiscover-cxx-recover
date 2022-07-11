#pragma once

#include <string>

#include "SymbolicDeclRef.h"

namespace rosdiscover {

class SymbolicCall : public SymbolicDeclRef {
public:
  SymbolicCall(
    const clang::DeclRefExpr* call
  ) : SymbolicDeclRef(call) {}
  ~SymbolicCall(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(call " << getName() << " : " << getTypeName() << ")";
  }

  nlohmann::json toJson() const override {
    auto j = SymbolicDeclRef::toJson();
    j["kind"] = "call";
    return j;
  }
};

} // rosdiscover
