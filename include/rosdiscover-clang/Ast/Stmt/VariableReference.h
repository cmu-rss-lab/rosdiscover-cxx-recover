#pragma once

#include <string>

#include "SymbolicDeclRef.h"

#include <clang/AST/Decl.h>

namespace rosdiscover {

class SymbolicVariableReference : public SymbolicDeclRef {
public:
  SymbolicVariableReference(
    bool isInstanceMember,
    bool isClassMember,
    std::string typeName,
    std::string name,
    bool isFileVarDecl,
    bool isLocalVarDeclOrParm,
    bool isModulePrivate
  ) : SymbolicDeclRef(isInstanceMember, isClassMember, typeName, name),
      isFileVarDecl(isFileVarDecl),
      isLocalVarDeclOrParm(isLocalVarDeclOrParm),
      isModulePrivate(isModulePrivate)
  {}
  
  SymbolicVariableReference(
    const clang::DeclRefExpr* varRef,
    const clang::VarDecl* varDecl
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

class SymbolicMemberVariableReference : public SymbolicVariableReference {
public:
  SymbolicMemberVariableReference(
    bool isInstanceMember,
    bool isClassMember,
    std::string typeName,
    std::string name,
    bool isFileVarDecl,
    bool isLocalVarDeclOrParm,
    bool isModulePrivate,
    std::unique_ptr<SymbolicExpr> base
  ) : SymbolicVariableReference(isInstanceMember, isClassMember, typeName, name, isFileVarDecl, isLocalVarDeclOrParm, isModulePrivate),
      base(std::move(base)) {
        assert(this->base != nullptr); 
  }
  
  SymbolicMemberVariableReference(
    const clang::DeclRefExpr* varRef,
    const clang::VarDecl* varDecl,
    std::unique_ptr<SymbolicExpr> base
  ) : SymbolicVariableReference(varRef, varDecl),
      base(std::move(base)) {
        assert(this->base != nullptr); 
  }
  ~SymbolicMemberVariableReference(){}

  std::string toString() const override {
    return fmt::format("{}.{}", base->toString(), getName());
  }

  void print(llvm::raw_ostream &os) const override {
    os << "(memberVarRef " << toString() << " : " << getTypeName() << ")";
  }

  nlohmann::json toJson() const override {
    auto j = SymbolicVariableReference::toJson();
    j["kind"] = "memberVarRef";
    j["base"] = base->toJson();
    return j;
  }

  std::vector<SymbolicExpr*> getChildren() const override {
    return {base.get()};
  }

private: 
  std::unique_ptr<SymbolicExpr> base;
};

} // rosdiscover
