#pragma once

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include "../../BackwardSymbolizer/ExprSymbolizer.h"

namespace rosdiscover {

class FindVarAssignVisitor
  : public clang::RecursiveASTVisitor<FindVarAssignVisitor> {
public:
  FindVarAssignVisitor(
    clang::ASTContext &astContext, 
    std::unordered_map<clang::Expr const *, SymbolicVariable *> &apiCallToVar) : exprSymbolizer(astContext, apiCallToVar), results() {}

  bool shouldVisitImplicitCode () const {
    return true;
  }

  bool shouldVisitLambdaBody () const {
    return true;
  }

  bool VisitBinaryOperator(const clang::BinaryOperator *assign) {
    if (!assign->isAssignmentOp()) {
      return true;
    }
    results.push_back(assign);
    return true;
  }

  // WARNING: Do not call more than once!
  std::vector<const clang::BinaryOperator *> getResults() {
    return results;
  }

  static std::vector<const clang::BinaryOperator *> findAssignments(clang::ASTContext &astContext, 
    std::unordered_map<clang::Expr const *, SymbolicVariable *> &apiCallToVar,
    const clang::FunctionDecl *function) {
    FindVarAssignVisitor visitor(astContext, apiCallToVar);
    visitor.TraverseDecl(const_cast<clang::FunctionDecl*>(function));
    return visitor.getResults();
  }

private:
  ExprSymbolizer exprSymbolizer;
  std::vector<const clang::BinaryOperator *> results;
};

} // rosdiscover
