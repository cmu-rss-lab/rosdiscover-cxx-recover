#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

using namespace llvm;

namespace {
struct MyHello : public FunctionPass {
  static char ID;
  MyHello() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    errs() << "Hello: ";
    errs().write_escaped(F.getName()) << '\n';
    return false;
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

