#pragma once

#include <vector>

#include <clang/AST/LexicallyOrderedRecursiveASTVisitor.h>

namespace rosdiscover {

class FindVirtualsVisitor
  : public clang::LexicallyOrderedRecursiveASTVisitor<FindVirtualsVisitor>
{
public:
  static std::vector<clang::CXXMethodDecl*> find(clang::ASTContext &context) {
    auto visitor = FindVirtualsVisitor(context);
    visitor.TraverseAST(context);
    llvm::outs() << "DEBUG: found " << visitor.results.size() << " virtual methods\n";
    return std::move(visitor.results);
  }

  bool VisitCXXMethodDecl(clang::CXXMethodDecl *decl) {
    if (decl->isVirtual()) {
      results.push_back(decl);
    }
    return true;
  }

private:
  std::vector<clang::CXXMethodDecl*> results;

  FindVirtualsVisitor(
    const clang::ASTContext &astContext
  ) : LexicallyOrderedRecursiveASTVisitor(astContext.getSourceManager()),
      results()
  {}
};

}