#pragma once

#include <llvm/IR/Instruction.h>

namespace rosdiscover {

class RosApiCall { }; // RosApiCall


class RosInitCall : public RosApiCall {
public:
  RosInitCall(llvm::Instruction *instruction) : instruction(instruction) { }

  llvm::Instruction getInstruction() const {
    return instruction;
  }
private:
  llvm::Instruction *instruction;
}; // RosInitCall


} // rosdiscover
