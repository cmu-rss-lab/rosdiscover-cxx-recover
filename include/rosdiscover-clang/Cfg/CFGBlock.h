#pragma once

#include <string>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>

#include "CFGEdge.h"
#include "../BackwardSymbolizer/ExprSymbolizer.h"


namespace rosdiscover {
class CFGEdge;
class CFGBlock {
public:
  CFGBlock(
    const clang::CFGBlock* clangBlock
  ) : clangBlock(clangBlock), predecessors(), successors() {}
  ~CFGBlock(){}

  const clang::CFGBlock* getClangBlock() const {
    return clangBlock;
  }

  std::vector<CFGEdge*> getSuccessors() {
    return successors;
  }

  std::vector<CFGEdge*> getPredecessors() {
    return predecessors;
  }

  std::string getConditionStr(const clang::ASTContext &astContext, ExprSymbolizer &exprSymbolizer) const {
    const auto *condition = clangBlock->getTerminatorCondition();
    if (condition == nullptr) {
      return "true"; //No condition in block
    }
    auto *conditionExpr = clang::dyn_cast<clang::Expr>(condition);
    if (conditionExpr == nullptr) {
      llvm::outs() << "ERROR: Terminiator condition is no expression: ";
      condition->dump();
      abort();
    }
    auto symbolicCondition = exprSymbolizer.symbolize(conditionExpr);

    auto result = symbolicCondition->toString();
    llvm::outs() << "[DEBUG] Symbolized Expr: " << result << " for: " << prettyPrint(condition, astContext) << "\n";
    
    return result;
  }

  std::unique_ptr<SymbolicExpr> getFullConditionExpr(
      bool includeSelf,
      const clang::ASTContext &astContext,
      bool negate,
      ExprSymbolizer &exprSymbolizer
    ) const {
    std::unique_ptr<SymbolicExpr> result = nullptr;
    for (auto edge: predecessors) {
      auto pExpr = edge->getPredecessor()->getFullConditionExpr(true, astContext, edge->getType() == CFGEdge::EdgeType::False, exprSymbolizer);
      if (pExpr == nullptr)
        continue;

      if (edge->getType() == CFGEdge::EdgeType::False ||
          edge->getType() == CFGEdge::EdgeType::True
      ) {
        if (result == nullptr) 
          result = std::move(pExpr);
        else 
          result = std::make_unique<OrExpr>(std::move(pExpr), std::move(result));
      }
      else if (edge->getType() == CFGEdge::EdgeType::Normal) {
        abort();
      }
    }

    if (!includeSelf || clangBlock->getTerminatorCondition() == nullptr) {
      return result;
    }
    auto condExpr = clang::dyn_cast<clang::Expr>(clangBlock->getTerminatorCondition());
    if (condExpr == nullptr) {
      llvm::outs() << "ERROR: Terminiator condition is no expression: ";
      clangBlock->getTerminatorCondition()->dump();
      abort();
    }

    auto symbolicCondition = exprSymbolizer.symbolize(condExpr);
    auto symbolicConditionStr = symbolicCondition->toString();
    llvm::outs() << "[DEBUG] Symbolized Expr: " << symbolicConditionStr << " for: " << prettyPrint(condExpr, astContext) << "\n";
    
    std::unique_ptr<SymbolicExpr> myExpr = negate ? std::make_unique<NegateExpr>(std::move(symbolicCondition)) : std::move(symbolicCondition);
    if (result == nullptr) 
      return myExpr;
    else
      return std::make_unique<AndExpr>(std::move(result), std::move(myExpr));
  }

  std::unique_ptr<SymbolicExpr> getFullConditionExpr(clang::ASTContext &astContext, ExprSymbolizer &exprSymbolizer) const {
    auto result = getFullConditionExpr(false, astContext, false, exprSymbolizer);
    if(result == nullptr) {
      return std::make_unique<BoolLiteral>(true);
    }
    return result;
  }

  bool createEdge(CFGBlock* successor, CFGEdge::EdgeType type) {
    auto edge = new CFGEdge(this, successor, type);
    for (auto pEdge : successors) {
      if (*pEdge == *edge) {
        llvm::outs() << "Skip redundant edge\n";
        return false;
      }
    }
    this->addSuccessor(edge);
    successor->addPredecessor(edge);
    return true;
  }
   
private: 
  const clang::CFGBlock* clangBlock;
  std::vector<CFGEdge*> predecessors;
  std::vector<CFGEdge*> successors;

  void addSuccessor(CFGEdge* edge) {
    successors.push_back(edge);
  }

  void addPredecessor(CFGEdge* edge) {
    predecessors.push_back(edge);
  }
  
};
} // rosdiscover
