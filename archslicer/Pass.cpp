#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Analysis/DependenceAnalysis.h>

#include <llvm/ADT/APFloat.h>
#include <llvm/IR/DebugInfoMetadata.h>


using namespace llvm;

namespace {
struct MyHello : public FunctionPass {
  static char ID;
  MyHello() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &info) const override {
      info.setPreservesAll();
      info.addRequired<DependenceAnalysisWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    // auto &AA = getAnalysis<AAResultsWrapperPass>().getAAResults();
    auto &dependencyAnalysis = getAnalysis<DependenceAnalysisWrapperPass>().getDI();

    if (MDNode *metadata = F.getMetadata("dbg")) {
      metadata->dump();
    }

    std::vector<Instruction *> instructions;
    for (auto &block : F) {
        for (Instruction &instruction : block) {
            instructions.push_back(&instruction);
            // analyzeInstruction(instruction);
        }
    }

    // compare all pairs of instructions
    size_t num_instructions = instructions.size();
    for (int i = 0; i < num_instructions; ++i) {
        for (int j = i + 1; j < num_instructions; ++j) {
            auto src = instructions.at(i);
            auto dst = instructions.at(j);
            auto dependence = dependencyAnalysis.depends(src, dst, false);
            if (dependence != nullptr) {
                dependence->dump(llvm::errs());
                errs() << "COOLPIG\n";
            }
        }
    }

    return false;
  }

  void analyzeInstruction(Instruction const &instruction) {
      if (MDNode *metadata = instruction.getMetadata("dbg")) {
          metadata->dump();
      }
  }
};  // struct Hello
}  // anonymous namespace

// initialize the pass ID
char MyHello::ID = 0;

// register the pass
static RegisterPass<MyHello> X(
  "hello",
  "Hello World Pass",
  false,  // only looks at CFG
  false  // analysis pass
  );

// ensure that the pass runs before any optimisations are applied
static llvm::RegisterStandardPasses Y(
    llvm::PassManagerBuilder::EP_EarlyAsPossible,
    [](const llvm::PassManagerBuilder &Builder, llvm::legacy::PassManagerBase &PM) {
      PM.add(new MyHello());
    });

