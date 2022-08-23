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
    std::string qualitfiedName,
    bool isFileVarDecl,
    bool isLocalVarDeclOrParm,
    bool isModulePrivate,
    std::unique_ptr<SymbolicValue> initialValue
  ) : SymbolicDeclRef(isInstanceMember, isClassMember, typeName, name, qualitfiedName),
      isFileVarDecl(isFileVarDecl),
      isLocalVarDeclOrParm(isLocalVarDeclOrParm),
      isModulePrivate(isModulePrivate),
      initialValue(std::move(initialValue))
  {}
  
  SymbolicVariableReference(
    const clang::DeclRefExpr* varRef,
    const clang::VarDecl* varDecl,
    std::unique_ptr<SymbolicValue> initialValue
  ) : SymbolicDeclRef(varRef),
      isFileVarDecl(varDecl->isFileVarDecl()),
      isLocalVarDeclOrParm(varDecl->isLocalVarDeclOrParm()),
      isModulePrivate(varDecl->isModulePrivate()),
      initialValue(std::move(initialValue))
  {}
  ~SymbolicVariableReference(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(var-ref " << getName() << " : " << getTypeName() << ")";
  }

  nlohmann::json toJson() const override {
    auto j = SymbolicDeclRef::toJson();
    j["kind"] = "var-ref";
    j["isFileVarDecl"] = isFileVarDecl;
    j["isLocalVarDeclOrParm"] = isLocalVarDeclOrParm;
    j["isModulePrivate"] = isModulePrivate;
    j["initialValue"] = initialValue->toJson();
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
  std::unique_ptr<SymbolicValue> initialValue;
};

class SymbolicMemberVariableReference : public SymbolicVariableReference {
public:
  SymbolicMemberVariableReference(
    bool isInstanceMember,
    bool isClassMember,
    std::string typeName,
    std::string name,
    std::string qualitfiedName,
    bool isFileVarDecl,
    bool isLocalVarDeclOrParm,
    bool isModulePrivate,
    std::unique_ptr<SymbolicExpr> base,
    std::unique_ptr<SymbolicValue> initialValue
  ) : SymbolicVariableReference(isInstanceMember, isClassMember, typeName, name, qualitfiedName, isFileVarDecl, isLocalVarDeclOrParm, isModulePrivate, std::move(initialValue)),
      base(std::move(base)) {
        assert(this->base != nullptr); 
  }
  
  SymbolicMemberVariableReference(
    const clang::DeclRefExpr* varRef,
    const clang::VarDecl* varDecl,
    std::unique_ptr<SymbolicExpr> base,
    std::unique_ptr<SymbolicValue> initialValue
  ) : SymbolicVariableReference(varRef, varDecl, std::move(initialValue)),
      base(std::move(base)) {
        assert(this->base != nullptr); 
  }
  ~SymbolicMemberVariableReference(){}

  std::string toString() const override {
    return fmt::format("{}.{}", base->toString(), getName());
  }

  void print(llvm::raw_ostream &os) const override {
    os << "(member-var-ref " << toString() << " : " << getTypeName() << ")";
  }

  nlohmann::json toJson() const override {
    auto j = SymbolicVariableReference::toJson();
    j["kind"] = "member-var-ref";
    j["base"] = base->toJson();
    return j;
  }

  std::vector<const SymbolicExpr*> getChildren() const override {
    return {base.get()};
  }

private: 
  std::unique_ptr<SymbolicExpr> base;
};

} // rosdiscover
