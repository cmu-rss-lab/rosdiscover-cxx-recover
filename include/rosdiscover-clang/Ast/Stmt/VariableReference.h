#pragma once

#include <string>

#include "SymbolicDeclRef.h"

#include <clang/AST/Decl.h>

namespace rosdiscover {

class SymbolicVariableReference : public SymbolicDeclRef {
public:
  SymbolicVariableReference(
    clang::DeclRefExpr* varRef
  ) : SymbolicDeclRef(varRef) {}
  ~SymbolicVariableReference(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(varRef " << getName() << " : " << getType().getAsString() << ")";
  }

  nlohmann::json toJson() const override {
    auto j = SymbolicDeclRef::toJson();
    j["kind"] = "varRef";
    j["isFileVarDecl"] = isFileVarDecl();
    j["isLocalVarDeclOrParm"] = isLocalVarDeclOrParm();
    j["isModulePrivate"] = isModulePrivate();
    return j;
  }

  bool isFileVarDecl() const {
    return getVarDecl()->isFileVarDecl();
  }

  bool isLocalVarDeclOrParm() const {
    return getVarDecl()->isLocalVarDeclOrParm();
  }

  bool isModulePrivate() const {
    return getVarDecl()->isModulePrivate();
  }

  clang::VarDecl* getVarDecl() const {
    return clang::dyn_cast<clang::VarDecl>(getDeclRef()->getDecl());
  }

};

} // rosdiscover
