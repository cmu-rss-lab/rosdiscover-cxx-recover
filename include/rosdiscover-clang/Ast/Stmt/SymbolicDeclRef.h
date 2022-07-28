#pragma once

#include <string>

#include <clang/AST/Expr.h>
#include <clang/AST/Type.h>

#include "SymbolicExpr.h"
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
      typeName(typeName),
      name(name),
      qualifiedName(qualifiedName)
   {}

  ~SymbolicDeclRef(){}

  SymbolicDeclRef(const clang::DeclRefExpr* declRef
  // Complex logic needed here to avoid duplication in sub-classes.
  ) : isInstanceMember(declRef->getDecl()->isCXXInstanceMember()), 
      isClassMember(declRef->getDecl()->isCXXClassMember()),
      typeName(declRef->getType().getAsString()),
      name(createName(declRef)),
      qualifiedName(declRef->getDecl()->getQualifiedNameAsString()) {}

  virtual std::string toString() const override {
    return name;
  }

  void print(llvm::raw_ostream &os) const override {
    os << "(declRef " << name << " : " << typeName << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "declRef"},
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
