#pragma once

#include <llvm/IR/Instruction.h>

namespace rosdiscover {

class RosApiCall { }; // RosApiCall


class RosInitCall : public RosApiCall {
public:
  llvm::CallBase* getCall() const {
    return instruction;
  }

  static RosInitCall* create(llvm::CallBase *instruction) {
    return new RosInitCall(instruction);
  }

private:
  llvm::CallBase *instruction;

  RosInitCall(llvm::CallBase *instruction) : instruction(instruction) { }
}; // RosInitCall


} // rosdiscover
