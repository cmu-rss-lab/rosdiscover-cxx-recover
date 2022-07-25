#pragma once

#include "CFGBlock.h"

namespace rosdiscover {
class ControlDependenceGraph {
public:
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
    std::unique_ptr<ControlDependenceGraph> graph; //maps BlockID to CFGBlockObject
    auto cfgBlockOfInterest = graph->getBlock(clangBlockOfInterest);
    std::vector<CFGBlock*> controlDependencyGraphNodes = {cfgBlockOfInterest};
    for (auto depsBlock: deps) {
      if (postdominatorAnalysis.dominates(depsBlock, clangBlockOfInterest) && !dominatorAnalysis.dominates(depsBlock, clangBlockOfInterest))
        continue; //ignore CFG blocks that come after the block of interest.
      controlDependencyGraphNodes.push_back(graph->getBlock(depsBlock));
    }
    llvm::outs() << "#### buildGraph ####\n";
    graph->buildGraph(true, clangBlockOfInterest, deps, postdominatorAnalysis, dominatorAnalysis, analyzed, controlDependencyGraphNodes, astContext, exprSymbolizer);
    llvm::outs() << "#### graph built ####\n";
    return graph;
  }
  
  CFGBlock* getBlock(const clang::CFGBlock* block) {
    if (!idToBlockDict.count(block->getBlockID())) { //lazy creation of CFG blocks 
        idToBlockDict.emplace(block->getBlockID(), std::make_unique<CFGBlock>(block));
    }
    return idToBlockDict.at(block->getBlockID()).get();
  }

private: 
  std::unordered_map<long, std::unique_ptr<CFGBlock>> idToBlockDict;

  ControlDependenceGraph() : idToBlockDict() {}

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
    ExprSymbolizer &exprSymbolizer
  ) {
    std::vector<CFGBlock*> predecessors;
    if (block == nullptr || block->pred_empty() || llvm::is_contained(analyzed, block)) {
      return predecessors;
    }
    analyzed.push_back(block);
    
    llvm::outs() << "buildGraph for: ";
    block->dump();
    llvm::outs() << "\n";

    // Recursively build the graph for the block's predecessors
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
        exprSymbolizer
      );
      predecessors.insert(predecessors.end(), indirectPredecessors.begin(), indirectPredecessors.end()); //merge results
    }

    // Create a node for control dependencies and the first block
    if (first || llvm::is_contained(deps, block)) {
      auto newControlDependencyNode = this->getBlock(block);

      // Create edges for predecessors
      for (auto *predecessor: predecessors) {

        // Check all nodes of the control dependency graph for potential edges to be created and their type
        for(auto *depBlock : controlDependencyGraphNodes) {
          bool trueBranchDominates = false;
          bool falseBranchDominates = false;

          int i = 0;
          for (const clang::CFGBlock *sBlock: predecessor->getClangBlock()->succs()) {
            // The only post-dominating control dependency is the directly following control dependency,
            // The exception to this is the head of a loop, which is the only control dependency which then also pre-dominiates the 
            // inner statement. Hence those edges need to be ignored to avoid circles in the control dependcy graph. 
            if ((postdominatorAnalysis.dominates(depBlock->getClangBlock(), sBlock) 
              && !dominatorAnalysis.dominates(depBlock->getClangBlock(), sBlock)) 
              || depBlock->getClangBlock()->getBlockID() == sBlock->getBlockID()) {
              if (i == 0) { //true branch, as defined by clang's order of successors                
                  llvm::outs() << "true branch dominates stmt\n";
                  trueBranchDominates = true;
              } else if (i == 1) { //false branch
                  llvm::outs() << "false branch dominates stmt\n";
                  falseBranchDominates = true;
              } 
            } 
            if (i > 1) {
              //TODO: Handle switch-case here.
              llvm::outs() << "Too many branches. Switch not yet supported\n";
              abort();
            }
            i++;
          }
          if (!trueBranchDominates && !falseBranchDominates) {
            continue; // No edge needed
          }
          if (trueBranchDominates && falseBranchDominates){
            llvm::outs() << "ERROR: falseBranchDominates: " << falseBranchDominates << " trueBranchDominates: " << trueBranchDominates << "\n";
            llvm::outs() << "depBlock->getClangBlock(): ";
            depBlock->getClangBlock()->dump();
            llvm::outs() << "\npredecessor->getClangBlock(): ";
            predecessor->getClangBlock()->dump();
            abort();
          }
          
          // Creating edge
          CFGEdge::EdgeType type;
          if (i == 1) {
            type = CFGEdge::EdgeType::Normal;
          } else if (i == 2) {
            type = falseBranchDominates ? CFGEdge::EdgeType::False : CFGEdge::EdgeType::True;
          } else {
            type = CFGEdge::EdgeType::Unknown;
            llvm::outs() << "ERROR: Unknown edge type\n";
          }

          if (predecessor->createEdge(depBlock, type)) {
            llvm::outs() << "created edge between " << predecessor->getConditionStr(astContext, exprSymbolizer) << " and " << depBlock->getConditionStr(astContext, exprSymbolizer) << " of type " << CFGEdge::getEdgeTypeName(type) << "\n";
          }
        }
      }
      return {newControlDependencyNode}; // Return newly created control dependency node
    } else { 
      return predecessors; // For non-control dependency nodes forward the predecessors to the next dependency node
    }
  }
  
};
} // rosdiscover
