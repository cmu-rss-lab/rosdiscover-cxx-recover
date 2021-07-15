#pragma once

#include <llvm/Support/raw_ostream.h>

namespace rosdiscover {
namespace symbolic {

class SymbolicValue {
public:
  virtual ~SymbolicValue(){};
  virtual void print(llvm::raw_ostream &os) const = 0;
}; // SymbolicValue

class SymbolicString : public virtual SymbolicValue {};

class SymbolicBool : public virtual SymbolicValue {};

class SymbolicInteger : public virtual SymbolicValue {};

class SymbolicUnknown :
  public virtual SymbolicString,
  public virtual SymbolicBool,
  public virtual SymbolicInteger
{
public:
  ~SymbolicUnknown(){}
  void print(llvm::raw_ostream &os) const override {
    os << "UNKNOWN";
  }

  static SymbolicUnknown* create() {
    return new SymbolicUnknown();
  }

private:
  SymbolicUnknown(){}
};

} // rosdiscover::symbolic
} // rosdiscover
