#pragma once

#include <llvm/IR/Instruction.h>

namespace rosdiscover {

class RosApiCall { }; // RosApiCall


/**
 * Warning: there are at least three versions of ros::init!
 *  https://docs.ros.org/en/api/roscpp/html/namespaceros.html#a7f5c939b8a0548ca9057392cc78d7ecb
 *
 * Example C++ usage:
 *
 *  ros::init(argc, argv, "turtlebot3_drive");
 *
 * Demangled name:
 *
 *  ros::init(
 *    int&,
 *    char**,
 *    std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&,
 *    unsigned int)
 */
class RosInitCall : public RosApiCall {
public:
  llvm::CallBase* getCall() const {
    return instruction;
  }

  llvm::Value* getName() const {
    return instruction->getArgOperand(2);
  }

  static RosInitCall* create(llvm::CallBase *instruction) {
    return new RosInitCall(instruction);
  }

private:
  llvm::CallBase *instruction;

  RosInitCall(llvm::CallBase *instruction) : instruction(instruction) { }
}; // RosInitCall


} // rosdiscover
