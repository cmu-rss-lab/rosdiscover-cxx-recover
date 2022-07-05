#pragma once

#include <string>

#include <clang/AST/Expr.h>
#include <clang/AST/Type.h>

#include "Stmt.h"

namespace rosdiscover {

class SymbolicDeclRef : public SymbolicStmt {
public:
  SymbolicDeclRef(
    clang::DeclRefExpr* declRef
  ) : declRef(declRef) {}
  ~SymbolicDeclRef(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(declRef " << getName() << " : " << getType().getAsString() << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "declRef"},
      {"isCXXInstanceMember", declRef->getDecl()->isCXXInstanceMember()},
      {"isCXXClassMember", declRef->getDecl()->isCXXInstanceMember()},
      {"type", getType().getAsString()},
      {"name", getName()},
    };
  }

  clang::QualType getType() const {
    return declRef->getType();
  }

  std::string getName() const {
    std::string name = declRef->getNameInfo().getAsString();
    if (declRef->hasQualifier()) {
      return declRef->getQualifier()->getAsNamespace()->getNameAsString() + "::" + name;
    }
    return name;
  }

  clang::DeclRefExpr* getDeclRef() const {
    return declRef;
  }

private:
  clang::DeclRefExpr* declRef;
};

} // rosdiscover
