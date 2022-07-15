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

  void addSuccessor(CFGEdge* edge) {
    successors.push_back(edge);
  }

  void addPredecessor(CFGEdge* edge) {
    predecessors.push_back(edge);
  }

  std::string getConditionStr(clang::ASTContext &astContext) const {
    const auto *condition = clangBlock->getTerminatorCondition();
    if (condition == nullptr) {
      llvm::outs() << "no terminator condition\n";
      return "unknown";
    }
    return rosdiscover::prettyPrint(condition, astContext);
  }

  std::string getFullConditionStr(clang::ASTContext &astContext, bool negate = false) const {
    std::string result = "";
    for (auto p: predecessors) {
      auto pStr = p->getPredecessor()->getFullConditionStr(astContext, p->getType() == CFGEdge::EdgeType::False);
      if (pStr == "")
        continue;

      if (p->getType() == CFGEdge::EdgeType::False) {
        if (result == "") 
          result = pStr;
        else 
          result = pStr + " || " + result;
      }
      else if (p->getType() == CFGEdge::EdgeType::True) {
        if (result == "") 
          result = pStr;
        else
          result = pStr + " || " + result;
      }
      else if (p->getType() == CFGEdge::EdgeType::Normal) {
        abort();
      }
    }
    auto myStr = negate ? "!(" + getConditionStr(astContext) + ")" : getConditionStr(astContext);
    if (result == "") 
      return myStr;
    else
      return "(" + result + ") && " + myStr;
  }

  void createEdge(CFGBlock* successor, CFGEdge::EdgeType type) {
    auto edge = new CFGEdge(this, successor, type);
    this->addSuccessor(edge);
    successor->addPredecessor(edge);
  }
   
private: 
  const clang::CFGBlock* clangBlock;
  std::vector<CFGEdge*> predecessors;
  std::vector<CFGEdge*> successors;

};
} // rosdiscover
