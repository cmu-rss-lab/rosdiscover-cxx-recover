#pragma once

#include <string>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>

#include "CFGEdge.h"

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

  std::string getConditionStr(clang::ASTContext &astContext) const {
    const auto *condition = clangBlock->getTerminatorCondition();
    if (condition == nullptr) {
      return "[" + std::to_string(clangBlock->getBlockID()) + "]";
    }
    return rosdiscover::prettyPrint(condition, astContext);
  }

  std::string getFullConditionStr(bool includeSelf, clang::ASTContext &astContext, bool negate) const {
    std::string result = "";
    for (auto edge: predecessors) {
      auto pStr = edge->getPredecessor()->getFullConditionStr(true, astContext, edge->getType() == CFGEdge::EdgeType::False);
      if (pStr == "")
        continue;

      if (edge->getType() == CFGEdge::EdgeType::False) {
        if (result == "") 
          result = pStr;
        else 
          result = pStr + " || " + result;
      }
      else if (edge->getType() == CFGEdge::EdgeType::True) {
        if (result == "") 
          result = pStr;
        else
          result = pStr + " || " + result;
      }
      else if (edge->getType() == CFGEdge::EdgeType::Normal) {
        abort();
      }
    }
    if (!includeSelf) {
      return result;
    }
    auto myStr = negate ? "!(" + getConditionStr(astContext) + ")" : getConditionStr(astContext);
    if (result == "") 
      return myStr;
    else
      return "(" + result + ") && " + myStr;
  }

  std::string getFullConditionStr(clang::ASTContext &astContext) const {
    return getFullConditionStr(false, astContext, false);
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
