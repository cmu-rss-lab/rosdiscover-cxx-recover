#pragma once

#include "CFGBlock.h"

namespace rosdiscover {
class ControlDependenceGraph {
public:
  ControlDependenceGraph() : idToBlockDict() {}
  ~ControlDependenceGraph(){}

  CFGBlock* getBlock(const clang::CFGBlock* block) {
    if (!idToBlockDict.count(block->getBlockID())) { //lazy creation of CFG blocks 
        idToBlockDict.emplace(block->getBlockID(), std::make_unique<CFGBlock>(block));
    }
    return idToBlockDict.at(block->getBlockID()).get();
  }

private: 
  std::unordered_map<long, std::unique_ptr<CFGBlock>> idToBlockDict;
  
};
} // rosdiscover
