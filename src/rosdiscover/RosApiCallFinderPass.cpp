/**
 * This pass finds all ROS API calls within a given LLVM module.
 */

#include <llvm/Demangle/Demangle.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#include <rosdiscover/RosApiCall.h>


using namespace llvm;
using namespace rosdiscover;

namespace {

struct RosApiCallFinderPass : llvm::FunctionPass {
  static char ID;

  RosApiCallFinderPass() : FunctionPass(ID) {}

  void getAnalysisUsage(llvm::AnalysisUsage &info) const override {
    info.setPreservesAll();
  }

  bool runOnFunction(llvm::Function &function) override {
    // ignore intrinsics
    if (function.isIntrinsic()) {
      return false;
    }

    // note that this also runs on undefined (externally linked) functions
    std::string demangledName = llvm::demangle(function.getName().str());
    // llvm::outs() << "checking: " << demangledName << "\n";

    for (auto &block : function) {
      for (auto &instruction : block) {
        // TODO check if this is a CallInst or InvokeInst!
        if (auto *call_inst = llvm::dyn_cast<llvm::CallInst>(&instruction)) {
          runOnCallInst(call_inst);
        } else if (auto *invoke_inst = llvm::dyn_cast<llvm::InvokeInst>(&instruction)) {
          runOnCallInst(invoke_inst);
        }
      }
    }

    return false;
  }

  void runOnCallInst(llvm::CallBase *instruction) {
    // ignore instrinsics
    if (llvm::isa<llvm::IntrinsicInst>(instruction)) {
      return;
    }

    auto *function = instruction->getCalledFunction();
    if (function == nullptr) {
      return;
    }

    // ignore any called functions that lack debug information
    // auto *functionDI = function->getSubprogram();
    // if (functionDI == nullptr) {
    //   return;
    // }
    // functionDI->print(llvm::outs());
    // llvm::outs() << "\n\n";

    std::string demangledName = llvm::demangle(function->getName().str());

    // is this a relevant ROS API call?
    // llvm::outs() << "function: " << demangledName << "\n";

    // is this a ros::init call?
    if (demangledName.rfind("ros::init", 0) == 0) {
      auto *init = RosInitCall::create(instruction);
      auto *name = init->getName();

      if (auto *nameAlloca = llvm::dyn_cast<llvm::AllocaInst>(name)) {
        for (auto *user : init->getName()->users()) {
          // find all of the places where we store to this alloca
          llvm::outs() << "USER: ";
          user->print(llvm::outs());
          llvm::outs() << "\n\n";
        }
      }

      llvm::outs() << "NAME: ";
      init->getName()->print(llvm::outs());
      llvm::outs() << "\n\n";

      // we need to figure out where we last wrote to the value
      // - alloca must dominate store
          }
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
