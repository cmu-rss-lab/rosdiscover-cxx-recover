#pragma once

#include "CFGBlock.h"

namespace rosdiscover {
class ControlDependenceGraph {
public:
  ControlDependenceGraph() : idToBlockDict() {}
  ~ControlDependenceGraph(){}
  
  static std::unique_ptr<ControlDependenceGraph> buildGraph(
      const clang::CFGBlock* clangBlockOfInterest,
      const llvm::SmallVector<clang::CFGBlock *, 4> &deps,
      clang::CFGDominatorTreeImpl<true> &postdominatorAnalysis,
      clang::CFGDominatorTreeImpl<false> &dominatorAnalysis,
      clang::ASTContext &astContext,
      ExprSymbolizer &exprSymbolizer
    ) {
    std::vector<const clang::CFGBlock*> analyzed;
    std::unique_ptr<ControlDependenceGraph> graph = std::make_unique<ControlDependenceGraph>(); //maps BlockID to CFGBlockObject
    auto cfgBlockOfInterest = graph->getBlock(clangBlockOfInterest);
    assert(cfgBlockOfInterest != nullptr);
    std::vector<CFGBlock*> controlDependencyGraphNodes = {cfgBlockOfInterest};
    for (auto depsBlock: deps) {
      assert(depsBlock != nullptr);
      if (postdominatorAnalysis.dominates(depsBlock, clangBlockOfInterest) && !dominatorAnalysis.dominates(depsBlock, clangBlockOfInterest))
        continue; //ignore CFG blocks that come after the block of interest.
      auto depsCfgBlock = graph->getBlock(depsBlock);
      assert(depsCfgBlock != nullptr);
      controlDependencyGraphNodes.push_back(depsCfgBlock);
    }
    llvm::outs() << "#### buildGraph ####\n";
    graph->buildGraph(true, clangBlockOfInterest, deps, postdominatorAnalysis, dominatorAnalysis, analyzed, controlDependencyGraphNodes, astContext, exprSymbolizer, 20);
    llvm::outs() << "#### graph built ####\n";
    return graph;
  }
  
  CFGBlock* getBlock(const clang::CFGBlock* block) {
    auto id = block->getBlockID();
    if (!idToBlockDict.count(id)) { //lazy creation of CFG blocks 
        idToBlockDict.emplace(id, std::make_unique<CFGBlock>(block));
    }
    return idToBlockDict.at(id).get();
  }

private: 
  std::unordered_map<long, std::unique_ptr<CFGBlock>> idToBlockDict;
  

  // Recursively builds a graph of control dependencies starting from the last block.
  std::vector<CFGBlock*> buildGraph(
    bool first,
    const clang::CFGBlock* block,
    const llvm::SmallVector<clang::CFGBlock *, 4> &deps,
    clang::CFGDominatorTreeImpl<true> &postdominatorAnalysis,
    clang::CFGDominatorTreeImpl<false> &dominatorAnalysis,
    std::vector<const clang::CFGBlock*> &analyzed,
    std::vector<CFGBlock*> &controlDependencyGraphNodes,
    clang::ASTContext &astContext,
    ExprSymbolizer &exprSymbolizer,
    int maxDepth
  ) {
    std::vector<CFGBlock*> predecessors;
    if (block == nullptr || block->pred_empty() || llvm::is_contained(analyzed, block) || maxDepth < 1) {
      return predecessors;
    }
    analyzed.push_back(block);
    
    llvm::outs() << "buildGraph for: ";
    block->dump();
    llvm::outs() << "\n";

    for (const clang::CFGBlock::AdjacentBlock predecessorBlock: block->preds()) {
      auto indirectPredecessors = buildGraph(
        false,
        predecessorBlock.getReachableBlock(),
        deps,
        postdominatorAnalysis,
        dominatorAnalysis,
        analyzed,
        controlDependencyGraphNodes,
        astContext,
        exprSymbolizer,
        maxDepth-1
      );
      predecessors.insert(predecessors.end(), indirectPredecessors.begin(), indirectPredecessors.end()); //merge results
    }

    // Create a node for control dependencies and the first block
    if (first || llvm::is_contained(deps, block)) {
      auto newControlDependencyNode = this->getBlock(block);
      assert(newControlDependencyNode != nullptr);

      // Create edges for predecessors
      for (auto *predecessor: predecessors) {
        assert(predecessor != nullptr);
        assert(predecessor->getClangBlock() != nullptr);
        // Check all nodes of the control dependency graph for potential edges to be created and their type
        for(auto *depBlock : controlDependencyGraphNodes) {
          assert(depBlock != nullptr);
          assert(depBlock->getClangBlock() != nullptr);

          int i = 0;
          int edge = -1;
          for (const clang::CFGBlock *sBlock: predecessor->getClangBlock()->succs()) {
            if (sBlock == nullptr) {
              continue;
            }
            // The only post-dominating control dependency is the directly following control dependency,
            // The exception to this is the head of a loop, which is the only control dependency which then also pre-dominiates the 
            // inner statement. Hence those edges need to be ignored to avoid circles in the control dependcy graph. 
            llvm::outs() << "DEBUG: Check Dom.\n";
            if ((postdominatorAnalysis.dominates(depBlock->getClangBlock(), sBlock) 
              && !dominatorAnalysis.dominates(depBlock->getClangBlock(), sBlock)) 
              || depBlock->getClangBlock()->getBlockID() == sBlock->getBlockID()) {
                llvm::outs() << "DEBUG: Dom Checked.\n";
              if (edge != -1) {
                  llvm::outs() << "Warning! Multiple Edges\n";
                  depBlock->getClangBlock()->dump();
                  llvm::outs() << "\npredecessor->getClangBlock(): ";
                  predecessor->getClangBlock()->dump();
                  //abort();
              }
              edge = i;
              llvm::outs() << "DEBUG: Found Edge " << i << ".\n";
            }
            i++;
          }
          llvm::outs() << "DEBUG: caseEdge: " << edge << ".\n";
          if (edge < 0) {
            continue; // No edge needed
          }

          llvm::outs() << "DEBUG: Creating edge ";// << predecessor->getClangBlock()->getTerminatorStmt()->getStmtClassName();
          // Creating edge
          CFGEdge::EdgeType type;
          if (predecessor->getClangBlock()->getTerminatorStmt() != nullptr && predecessor->getClangBlock()->getTerminatorStmt()->getStmtClass() == clang::Stmt::SwitchStmtClass){
            type = CFGEdge::EdgeType::Case; 
          } else if (edge == 0) {
            type = CFGEdge::EdgeType::True;
          } else if (edge == 1) {
            type = CFGEdge::EdgeType::False;
          } else {
            type = CFGEdge::EdgeType::Unknown;
            llvm::outs() << "ERROR: Unknown edge type\n";
          }

          if (predecessor->createEdge(depBlock, type)) {
            llvm::outs() << "DEBUG: Created edge\n";// between " << predecessor->getConditionStr(astContext, exprSymbolizer) << " and " << depBlock->getConditionStr(astContext, exprSymbolizer) << " of type " << CFGEdge::getEdgeTypeName(type) << "\n";
          }
        }
        llvm::outs() << "DEBUG: Graph Built\n";
      }
      return {newControlDependencyNode}; // Return newly created control dependency node
    } else { 
      return predecessors; // For non-control dependency nodes forward the predecessors to the next dependency node
    }
  }
  
};
} // rosdiscover
