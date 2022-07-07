#pragma once

#include <string>

#include <clang/AST/Expr.h>
#include <clang/AST/Type.h>

#include "Stmt.h"

namespace rosdiscover {

class SymbolicDeclRef : public SymbolicStmt {
public:
  SymbolicDeclRef(
    bool isInstanceMember,
    bool isClassMember,
    std::string typeName,
    std::string name
  ) : isInstanceMember(isInstanceMember),
      isClassMember(isClassMember),
      typeName(typeName),
      name(name)
   {}

  ~SymbolicDeclRef(){}

  static std::string createName(clang::DeclRefExpr* declRef) {
    std::string name = declRef->getNameInfo().getAsString();
    if (declRef->hasQualifier()) {
      name = declRef->getQualifier()->getAsNamespace()->getNameAsString() + "::" + name;
    }
    return name;
  }

  SymbolicDeclRef(clang::DeclRefExpr* declRef
  ) : isInstanceMember(declRef->getDecl()->isCXXInstanceMember()), 
      isClassMember(declRef->getDecl()->isCXXClassMember()),
      typeName(declRef->getType().getAsString()),
      name(createName(declRef)) {}

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
    };
  }
    
  std::string getTypeName() const {
    return typeName;
  }

  std::string getName() const {
    return name;
  }

private:
  bool isInstanceMember;
  bool isClassMember;
  std::string typeName;
  std::string name;
};

} // rosdiscover
