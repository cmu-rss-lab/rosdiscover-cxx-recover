#pragma once

#include <clang/AST/LexicallyOrderedRecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

#include "utils.h"

namespace rosdiscover {

class FindDefVisitor
  : public clang::LexicallyOrderedRecursiveASTVisitor<StmtOrderingVisitor>
{
private:
  [[maybe_unused]] clang::Decl const *decl;
  clang::Expr const *location;
  // NOTE we assume a single reaching definition for now;
  //  once we have path handling, we may have multiple reach definitions
  clang::Expr *definition;

  FindDefVisitor(
      const clang::ASTContext &astContext,
      clang::Decl const *decl,
      clang::Expr const *location
  ) : LexicallyOrderedRecursiveASTVisitor(astContext.getSourceManager()),
      decl(decl),
      location(location),
      definition(nullptr)
  {}

  // ? walk from top to bottom
  // ? terminate when we reach the querying node

  // VarDecl: if it's relevant and it has an initializer, add the initializer
  // DeclStmt

  // do we pass the decl as a reference to a given CallExpr?
  // if so, check if we have a model for the corresponding function? [it's probably just sufficient to say, yes, it writes to the decl]
  // if we pass a const ref, we can safely ignore the CallExpr

public:
  static clang::Expr* find(
      clang::ASTContext &astContext,
      clang::Decl const *decl,
      clang::Expr *location
  ) {
    auto visitor = FindDefVisitor(astContext, decl, location);

    // note: this is intra-procedural for now
    // find the function to which the querying location belongs
    auto *parentFunction = const_cast<clang::FunctionDecl*>(getParentFunctionDecl(astContext, location));
    visitor.TraverseDecl(parentFunction);

    return visitor.definition;
  }

  bool VisitStmt(clang::Stmt *stmt) {
    // have we reached the querying node?
    if (stmt == location)
      return false;

    llvm::outs() << "visiting stmt:\n";
    stmt->dumpColor();
    llvm::outs() << "\n";

    return true;
  }
};

} // rosdiscover