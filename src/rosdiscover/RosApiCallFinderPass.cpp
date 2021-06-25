/**
 * This pass finds all ROS API calls within a given LLVM module.
 */

#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

using namespace llvm;

namespace {

struct RosApiCallFinderPass : llvm::FunctionPass {
  static char ID;

  RosApiCallFinderPass() : FunctionPass(ID) {}

  void getAnalysisUsage(llvm::AnalysisUsage &info) const override {
    info.setPreservesAll();
  }

  bool runOnFunction(llvm::Function &function) override {
    llvm::outs() << "checking: " << function.getName() << "\n";

    for (auto &block : function) {
      for (auto &instruction : block) {
        if (auto *call_inst = llvm::dyn_cast<llvm::CallInst>(&instruction)) {
          runOnCallInst(call_inst);
        }
      }
    }

    return false;
  }

  void runOnCallInst(llvm::CallInst *instruction) {
    // ignore instrinsics
    if (llvm::isa<llvm::DbgInfoIntrinsic>(instruction)) {
      return;
    }

    auto *function = instruction->getCalledFunction();
    if (function == nullptr) {
      return;
    }

    // is this a relevant ROS API call?
    llvm::outs() << "function: " << function->getName() << "\n";
  }

}; // RosApiCallFinderPass


}

char RosApiCallFinderPass::ID = 0;

static llvm::RegisterPass<RosApiCallFinderPass> X(
  "find-ros-api-calls",
  "ROS API Call Finder",
  false,
  true
);

// ensure that the pass runs before any optimisations are applied
static llvm::RegisterStandardPasses Y(
  llvm::PassManagerBuilder::EP_EarlyAsPossible,
  [](const llvm::PassManagerBuilder &Builder, llvm::legacy::PassManagerBase &PM) {
    PM.add(new RosApiCallFinderPass());
  }
);
