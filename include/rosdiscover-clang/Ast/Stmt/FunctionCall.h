#pragma once

#include <string>

#include "SymbolicDeclRef.h"

namespace rosdiscover {

class SymbolicCall : public SymbolicDeclRef {
public:
  SymbolicCall(
    clang::DeclRefExpr* call
  ) : SymbolicDeclRef(call) {}
  ~SymbolicCall(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(call " << getName() << " : " << getType().getAsString() << ")";
  }

  nlohmann::json toJson() const override {
    auto j = SymbolicDeclRef::toJson();
    j["kind"] = "call";
    return j;
  }

};

} // rosdiscover
