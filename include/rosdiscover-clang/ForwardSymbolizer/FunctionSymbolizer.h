#pragma once

#include <clang/AST/RecursiveASTVisitor.h>

namespace rosdiscover {

// store summaries in a separate directory
// -> SymbolicProgram
// -> SymbolicFunction


// TODO
// - basically, we want to create a new body [SymbolicCompound]

// use a Visitor
class SymbolicFunctionSummaryVisitor
  : public RecursiveASTVisitor<SymbolicFunctionSummaryVisitor> {
public:
  bool VisitCXXMethodDecl(CXXMethodDecl *decl) {
    if (!decl->hasBody()) {
      llvm::errs() << "function definition does not provide a body\n";
      return false;
    }

    if (decl->isVariadic()) {
      // this will be harder to deal with
      // for now, pretend that the var. args don't exist
    }

    // TODO symbolize the parameters
    for (auto const *param : decl->parameters()) {
      // TODO might have a default argument
      createSymbolicParam(param);
    }

    // The return value indicates whether we want the visitation to proceed.
    // Return false to stop the traversal of the AST.
    return true;
  }


private:
  void createSymbolicParam(clang::VarDecl const *decl) {
    // is this a supported type?
    if (!canBeSymbolized(decl->getType())) {
      llvm::errs() << "not symbolizing var decl [unsupported type]: ";
      decl->print(llvm::errs());
      llvm::errs() << "\n";
      return;
    }

    // TODO how do we elegantly deal with defaults? probably still just treat them as unknown

    // if there is no definition, Top
    // assign Top by default when we declare a symbolic var
    declare(decl);

    if (decl->hasInit()) {
      redefine(decl, decl->getInit());
    }

    // TODO this might have an initializer
    // decl->getNameAsString();
    // symbolizeValue();
  }

  /**
* Converts a Clang expression to its symbolic form
   */
  SymbolicValue symbolizeExpr(clang::Expr const *expr) {

  }

  bool canBeSymbolized(clang::QualType const *type) const {

  }

  // TODO all statements should have an optional SourceLocation

  // TODO emit control statements in summary
  // if () {} else {} // while () {} [for/for-each statements are harder: can model with a while instead]

  // State
  std::shared_ptr<SymbolicProgram> program;

  // see http://klee.github.io/doxygen/html/classklee_1_1ExecutionState.html
  // TODO pushFrame() / popFrame()
  // -----
  // vars: std::map<std::string, std::shared_ptr<SymbolicVarDecl>>
  // functions: std::map<std::string, std::shared_ptr<SymbolicFunction>>
  // state: std::map<std::string, std::shared_ptr<SymbolicValue>>
  std::unique_ptr<SymbolicState> state;
};

}
