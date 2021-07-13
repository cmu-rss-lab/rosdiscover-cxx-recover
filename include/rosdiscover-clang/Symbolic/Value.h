#pragma once

#include <llvm/Support/raw_ostream.h>

namespace rosdiscover {

class SymbolicValue {
public:
  virtual ~SymbolicValue() = 0;

  virtual void print(llvm::raw_ostream &os) const;
}; // SymbolicValue


class SymbolicString : SymbolicValue {};

class SymbolicBool : SymbolicValue {};

class SymbolicInteger : SymbolicValue {};

class SymbolicStmt {
public:
  virtual ~SymbolicStmt() = 0;

  virtual void print(llvm::raw_ostream &os) const;
};

class SymbolicCompound {
private:
  std::vector<SymbolicStmt*> statements;
};

} // rosdiscover
