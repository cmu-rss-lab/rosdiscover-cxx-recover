#pragma once

#include <fstream>
#include <iomanip>
#include <ostream>

#include <nlohmann/json.hpp>

#include "Context.h"

namespace rosdiscover {
namespace symbolic {

class SymbolicProgram {
public:
  SymbolicProgram() : context() {}
  SymbolicProgram(const SymbolicProgram&) = delete;
  SymbolicProgram& operator=(const SymbolicProgram&) = delete;

  void save(std::string const &filename) const {
    std::ofstream o(filename);
    o << std::setw(2) << toJson() << std::endl;
  }

  nlohmann::json toJson() const {
    return {
      {"program", context.toJson()}
    };
  }

  SymbolicContext& getContext() {
    return context;
  }

private:
  SymbolicContext context;
};

} // rosdiscover::symbolic
} // rosdiscover
