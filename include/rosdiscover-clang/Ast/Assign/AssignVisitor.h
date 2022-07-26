#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include "../../BackwardSymbolizer/ExprSymbolizer.h"

namespace rosdiscover {

class FindVarAssignVisitor
  : public clang::RecursiveASTVisitor<FindVarAssignVisitor> {
public:
  FindVarAssignVisitor(clang::ASTContext &astContext, std::unordered_map<clang::Expr const *, SymbolicVariable *> &apiCallToVar) : exprSymbolizer(astContext, apiCallToVar) {}

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
    llvm::outs() << "[FindVarAssignVisitor] \n";
    if (auto *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(assign->getLHS()->IgnoreCasts()->IgnoreImpCasts())) {
      llvm::outs() << "[FindVarAssignVisitor] declRefExpr assign: " << declRefExpr->getDecl()->getQualifiedNameAsString();
      llvm::outs() << ":= " << exprSymbolizer.symbolize(assign->getRHS())->toString() << "\n";
    } else if (auto *memberExpr = clang::dyn_cast<clang::MemberExpr>(assign->getLHS()->IgnoreCasts()->IgnoreImpCasts())) {
      llvm::outs() << "[FindVarAssignVisitor] memberExpr assign: " << memberExpr->getMemberDecl()->getQualifiedNameAsString() << "\n";
      llvm::outs() << ":= " << exprSymbolizer.symbolize(assign->getRHS())->toString() << "\n";
    }
    return true;
  }


private:
  ExprSymbolizer exprSymbolizer;
};

} // rosdiscover
