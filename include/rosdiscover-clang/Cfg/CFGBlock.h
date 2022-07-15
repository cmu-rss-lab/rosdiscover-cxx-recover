#pragma once

#include <string>

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
