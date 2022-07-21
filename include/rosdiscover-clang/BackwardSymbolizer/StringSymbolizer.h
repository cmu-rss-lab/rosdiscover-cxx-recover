#pragma once

#include <string>
#include <unordered_map>

#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <llvm/ADT/APInt.h>

#include "../Builder/ValueBuilder.h"
#include "../Value/String.h"
#include "../Helper/FindDefVisitor.h"

namespace rosdiscover {

class StringSymbolizer {
public:
  StringSymbolizer(
    clang::ASTContext &astContext,
    std::unordered_map<clang::Expr const *, SymbolicVariable *> &apiCallToVar
  )
  : astContext(astContext), apiCallToVar(apiCallToVar), valueBuilder() {}

  std::unique_ptr<SymbolicString> symbolize(const clang::Expr *expr) {
    if (expr == nullptr) {
      llvm::outs() << "ERROR! Symbolizing (str): NULLPTR";
      return valueBuilder.unknown();
    }

    expr = expr->IgnoreParenCasts();

    llvm::outs() << "symbolizing (str): ";
    expr->dump();
    llvm::outs() << "\n";

    if (auto *callExpr = clang::dyn_cast<clang::CallExpr>(expr)) {
      // is this expression mapped to a ROS API call?
      if (apiCallToVar.find(callExpr) != apiCallToVar.end()) {
        return valueBuilder.varRef(apiCallToVar[callExpr]);
      }

      if (auto *callee = callExpr->getDirectCallee()) {
        auto calleeName = callee->getQualifiedNameAsString();
        llvm::outs() << "DEBUG: checking call to function [" << calleeName << "]\n";
        if (calleeName == "ros::this_node::getName") {
          return valueBuilder.nodeName();
        } else if (calleeName == "std::operator+") {
          return symbolizeConcatenation(callExpr);
        }
      }

    }

    if (auto *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(expr)) {
      return symbolize(declRefExpr);
    } else if (auto *literal = clang::dyn_cast<clang::StringLiteral>(expr)) {
      return symbolize(literal);
    } else if (auto *literal = clang::dyn_cast<clang::IntegerLiteral>(expr)) {
      return symbolize(literal);
    } else if (auto *implicitCastExpr = clang::dyn_cast<clang::ImplicitCastExpr>(expr)) {
      return symbolize(implicitCastExpr);
    } else if (auto *constructExpr = clang::dyn_cast<clang::CXXConstructExpr>(expr)) {
      return symbolize(constructExpr);
    } else if (auto *bindTempExpr = clang::dyn_cast<clang::CXXBindTemporaryExpr>(expr)) {
      return symbolize(bindTempExpr);
    } else if (auto *materializeTempExpr = clang::dyn_cast<clang::MaterializeTemporaryExpr>(expr)) {
      return symbolize(materializeTempExpr);
    }

    llvm::outs() << "unable to symbolize expression (str): treating as unknown\n";
    return valueBuilder.unknown();
  }

private:
  clang::ASTContext &astContext;
  std::unordered_map<clang::Expr const *, SymbolicVariable *> &apiCallToVar;
  ValueBuilder valueBuilder;

  std::unique_ptr<SymbolicString> symbolize(const clang::StringLiteral *literal) {
    return valueBuilder.stringLiteral(literal->getString().str());
  }

  std::unique_ptr<SymbolicString> symbolize(const clang::IntegerLiteral *literal) {
    return valueBuilder.stringLiteral(std::to_string(literal->getValue().getSExtValue()));
  }

  std::unique_ptr<SymbolicString> symbolize(const clang::CXXConstructExpr *expr) {
    // does this call the std::string constructor?
    // FIXME this is a bit hacky and may break when other libc++ versions are used
    //
    auto constructorName = expr->getConstructor()->getParent()->getQualifiedNameAsString();
    llvm::outs() << "calling constructor: " << constructorName << "\n";
    if (constructorName == "std::__cxx11::basic_string" || constructorName == "std::basic_string") {
      if (expr->getNumArgs() == 0) {
        llvm::outs() << "DEBUG: unimplemented [resolve indirect string variable definition]: ";
        expr->dump();
        llvm::outs() << "\n";
        return valueBuilder.unknown();
      }

      return symbolize(expr->getArg(0));
    }

    llvm::outs() << "call to unknown constructor: " << constructorName << "\n";
    return valueBuilder.unknown();
  }

  std::unique_ptr<SymbolicString> symbolizeConcatenation(const clang::CallExpr *expr) {
    assert(expr->getNumArgs() == 2 && "string concentation should have two arguments");
    auto lhs = symbolize(expr->getArg(0));
    auto rhs = symbolize(expr->getArg(1));
    return valueBuilder.concatenate(std::move(lhs), std::move(rhs));
  }

  std::unique_ptr<SymbolicString> symbolize(const clang::ImplicitCastExpr *expr) {
    // TODO check that we're dealing with strings or char[]
    return symbolize(expr->getSubExpr());
  }

  std::unique_ptr<SymbolicString> symbolize(const clang::DeclRefExpr *nameExpr) {
    // TODO does this refer to a parameter?

    if (auto *varDecl = clang::dyn_cast<clang::VarDecl>(nameExpr->getDecl())) {
      llvm::outs() << "DEBUG: attempting to find definition for var: ";
      varDecl->dump();
      llvm::outs() << "\n";

      auto *initExpr = varDecl->getInit();
      if (initExpr != nullptr) {
        auto symbolized = symbolize(initExpr);
        if (!symbolized->isUnknown()) {
          return symbolized;
        }
      }

      if (auto *def = findDef(varDecl, nameExpr)) {
        return symbolize(def);
      }

      llvm::outs() << "WARNING: unable to find definition for var: ";
      varDecl->dump();
      llvm::outs() << "\n";
    }

    return valueBuilder.unknown();
  }

  std::unique_ptr<SymbolicString> symbolize(const clang::CXXBindTemporaryExpr *expr) {
    return symbolize(expr->getSubExpr());
  }

  std::unique_ptr<SymbolicString> symbolize(const clang::MaterializeTemporaryExpr *expr) {
    return symbolize(expr->getSubExpr());
  }

  clang::Expr* findDef(const clang::VarDecl *decl, const clang::Expr *location) {
    return FindDefVisitor::find(astContext, decl, location);
  }
};

} // rosdiscover
