#pragma once

#include <nlohmann/json.hpp>

#include <llvm/Support/raw_ostream.h>

namespace rosdiscover {
namespace symbolic {

class SymbolicStmt {
public:
  virtual ~SymbolicStmt(){};
  virtual void print(llvm::raw_ostream &os) const = 0;
  virtual nlohmann::json toJson() const = 0;
};

} // rosdiscover::symbolic
} // rosdiscover
