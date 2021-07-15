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

} // rosdiscover::symbolic
} // rosdiscover
