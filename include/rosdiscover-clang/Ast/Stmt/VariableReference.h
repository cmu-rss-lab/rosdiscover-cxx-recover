#pragma once

#include <string>

#include "SymbolicDeclRef.h"

#include <clang/AST/Decl.h>

namespace rosdiscover {

class SymbolicVariableReference : public SymbolicDeclRef {
public:
  SymbolicVariableReference(
    clang::DeclRefExpr* varRef,
    clang::VarDecl* varDecl
  ) : SymbolicDeclRef(varRef),
      isFileVarDecl(varDecl->isFileVarDecl()),
      isLocalVarDeclOrParm(varDecl->isLocalVarDeclOrParm()),
      isModulePrivate(varDecl->isModulePrivate())
  {}
  ~SymbolicVariableReference(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(varRef " << getName() << " : " << getTypeName() << ")";
  }

  nlohmann::json toJson() const override {
    auto j = SymbolicDeclRef::toJson();
    j["kind"] = "varRef";
    j["isFileVarDecl"] = isFileVarDecl;
    j["isLocalVarDeclOrParm"] = isLocalVarDeclOrParm;
    j["isModulePrivate"] = isModulePrivate;
    return j;
  }

  bool getIsFileVarDecl() const {
    return isFileVarDecl;
  }

  bool getIsLocalVarDeclOrParm() const {
    return isLocalVarDeclOrParm;
  }

  bool getIsModulePrivate() const {
    return isModulePrivate;
  }

private: 
  bool isFileVarDecl;
  bool isLocalVarDeclOrParm;
  bool isModulePrivate;
};

} // rosdiscover
