#pragma once

#include <unordered_map>

#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

namespace rosdiscover {
namespace name {

// TODO: use an outer visitor to build function summaries for the entire program

// TODO: use a post-processing step to simplify/compact function summaries

// TODO: turn this a FunctionDeclVisitor: FunctionSummaryVisitor
class FunctionSummaryVisitor : public clang::RecursiveASTVisitor<SymbolicVisitor> {
public:

  // TODO outline the summary return type
  static void summarize(clang::FunctionDecl const *decl) {
    
  }

  // TODO: maintain state [use immutable data structure?]
  //
  // - mapping from clang::Decl -> symbolic value
  // - maintain path condition
  // - maintain function decl [FunctionSummaryBuilder]
  //    - create symbolic variables for each formal
  //    - store those variables in the table

  /**
   * visit the body of the function: FunctionDecl::getDefinition()
   */


  // IntegerExpr
  // RealExpr [?]
  // StringExpr
  // BoolExpr

  bool shouldVisitImplicitCode() const override {
    return true;
  }

  bool TraverseStmt(clang::Stmt *stmt) override {

  }


private:
  clang::FunctionDecl const &function;
  std::unordered_map<clang::Decl const *, Expr *> state;
  BoolExpr * pathCondition;
};

} // rosdiscover::name
} // rosdiscover
