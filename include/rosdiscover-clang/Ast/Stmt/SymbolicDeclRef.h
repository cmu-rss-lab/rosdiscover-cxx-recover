#pragma once

#include <string>

#include <clang/AST/Expr.h>
#include <clang/AST/Type.h>

#include "SymbolicExpr.h"
#include "../../Value/Value.h"
#include "../../ApiCall/Calls/Util.h"

namespace rosdiscover {

class SymbolicDeclRef : public SymbolicExpr {
public:
  SymbolicDeclRef(
    bool isInstanceMember,
    bool isClassMember,
    std::string typeName,
    std::string name,
    std::string qualifiedName
  ) : isInstanceMember(isInstanceMember),
      isClassMember(isClassMember),
      typeName(normalizeTypeName(typeName)),
      name(name),
      qualifiedName(qualifiedName)
   {}

  ~SymbolicDeclRef(){}

  SymbolicDeclRef(const clang::DeclRefExpr* declRef
  // Complex logic needed here to avoid duplication in sub-classes.
  ) : isInstanceMember(declRef->getDecl()->isCXXInstanceMember()), 
      isClassMember(declRef->getDecl()->isCXXClassMember()),
      typeName(normalizeTypeName(declRef->getType().getAsString())),
      name(createName(declRef)),
      qualifiedName(declRef->getDecl()->getQualifiedNameAsString()) {}

  static std::string normalizeTypeName(std::string clangTypeName) {
    auto symbolicType = SymbolicValue::getSymbolicType(clangTypeName);
    if (symbolicType == SymbolicValueType::Unsupported) {
      return clangTypeName;
    }
    return SymbolicValue::getSymbolicTypeAsString(symbolicType);
  }

  virtual std::string toString() const override {
    return name;
  }

  void print(llvm::raw_ostream &os) const override {
    os << "(decl-ref " << name << " : " << typeName << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "decl-ref"},
      {"isInstanceMember", isInstanceMember},
      {"isClassMember", isClassMember},
      {"type", typeName},
      {"name", name},
      {"qualified_name", qualifiedName},
      {"string", this->toString()},
    };
  }
    
  std::string getTypeName() const {
    return typeName;
  }

  std::string getName() const {
    return name;
  }

  std::string getQualifiedName() const {
    return qualifiedName;
  }

private:
  bool isInstanceMember;
  bool isClassMember;
  std::string typeName;
  std::string name;
  std::string qualifiedName;
};

} // rosdiscover
