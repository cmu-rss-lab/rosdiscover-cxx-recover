#pragma once

#include <string>

#include <clang/AST/Stmt.h>

#include "CFGEdge.h"

namespace rosdiscover {
class CFGEdge;
class CFGBlock {
public:
  CFGBlock(
    const clang::Stmt* stmt
  ) : stmt(stmt), predecessors(), successors() {}
  ~CFGBlock(){}

  const clang::Stmt* getStmt() {
    return stmt;
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
  const clang::Stmt* stmt;
  std::vector<CFGEdge*> predecessors;
  std::vector<CFGEdge*> successors;

};
} // rosdiscover
