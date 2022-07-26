#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include "../../BackwardSymbolizer/ExprSymbolizer.h"
#include "SymbolicAssignment.h"

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
    std::string varName;
    std::unique_ptr<SymbolicVariableReference> var;

    if (auto *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(assign->getLHS()->IgnoreCasts()->IgnoreImpCasts())) {
      varName = declRefExpr->getDecl()->getQualifiedNameAsString();
      llvm::outs() << "[FindVarAssignVisitor] declRefExpr assign: " << varName;
      auto *varDecl = clang::dyn_cast<clang::VarDecl>(declRefExpr->getDecl());
      if (varDecl == nullptr) {
        llvm::outs() << "[FindVarAssignVisitor] Unsupported LHS of Assignment: ";
        declRefExpr->dump();
        abort();
      }
      var = std::make_unique<SymbolicVariableReference>(declRefExpr, varDecl);
    } else if (auto *memberExpr = clang::dyn_cast<clang::MemberExpr>(assign->getLHS()->IgnoreCasts()->IgnoreImpCasts())) {
      varName = memberExpr->getMemberDecl()->getQualifiedNameAsString();
      llvm::outs() << "[FindVarAssignVisitor] memberExpr assign: " << varName;
      var = exprSymbolizer.symbolizeMemberExpr(memberExpr);
    } else {
      llvm::outs() << "[FindVarAssignVisitor] Unsupported LHS of Assignment: ";
      assign->dump();
      abort();
    }
    results.push_back(std::make_unique<SymbolicAssignment>(std::move(var), exprSymbolizer.symbolize(assign->getRHS())));
    return true;
  }

  // WARNING: Do not call more than once!
  std::vector<std::unique_ptr<SymbolicAssignment>> getResults() {
    return std::move(results);
  }

private:
  ExprSymbolizer exprSymbolizer;
  std::vector<std::unique_ptr<SymbolicAssignment>> results;
};

} // rosdiscover
