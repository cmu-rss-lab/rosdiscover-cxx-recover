#pragma once

#include <unordered_set>
#include <vector>

#include <clang/AST/LexicallyOrderedRecursiveASTVisitor.h>
#include <clang/AST/Stmt.h>

namespace rosdiscover {

class StmtOrderingVisitor
  : public clang::LexicallyOrderedRecursiveASTVisitor<StmtOrderingVisitor>
{
private:
  std::unordered_set<clang::Stmt *> statementsToFind;
  std::vector<clang::Stmt *> ordered;

  StmtOrderingVisitor(
      const clang::ASTContext &astContext,
      std::vector<clang::Stmt *> &statements
  ) : LexicallyOrderedRecursiveASTVisitor(astContext.getSourceManager()),
      statementsToFind(),
      ordered()
  {
    for (auto *stmt : statements) {
      statementsToFind.insert(stmt);
    }
  }

public:
  bool shouldVisitImplicitCode() const {
    return true;
  }

  static std::vector<clang::Stmt *> computeOrder(
      clang::ASTContext const &astContext,
      clang::FunctionDecl const *function,
      std::vector<clang::Stmt *> &statements
  ) {
    auto visitor = StmtOrderingVisitor(astContext, statements);
    visitor.TraverseDecl(const_cast<clang::FunctionDecl*>(function->getDefinition()));

    // ensure that we found every statement
    if (!visitor.statementsToFind.empty()) {
      llvm::errs() << "FATAL ERROR: failed to order all statements\n";
      for (auto *statement : visitor.statementsToFind) {
        llvm::errs() << "-> failed to find statement: ";
        statement->dumpColor();
        llvm::errs() << "\n";
        statement->getBeginLoc().dump(astContext.getSourceManager());
        llvm::errs() << "\n";
      }
      abort();
    }

    return visitor.ordered;
  }

  bool VisitStmt(clang::Stmt *stmt) {
    if (statementsToFind.find(stmt) != statementsToFind.end()) {
      statementsToFind.erase(stmt);
      ordered.push_back(stmt);
    }
    return true;
  }
};

} // rosdiscover
