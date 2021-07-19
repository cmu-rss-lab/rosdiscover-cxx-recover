#pragma once

#include <nlohmann/json.hpp>

#include "Context.h"

namespace rosdiscover {
namespace symbolic {

class SymbolicProgram {
public:
  SymbolicProgram() : context() {}

  void save(std::string const &filename) const {
    // convert to JSON and save
  }

  nlohmann::json toJson() const {
    return {
      {"program", context.toJson()}
    };
  }

private:
  SymbolicContext context;
};

} // rosdiscover::symbolic
} // rosdiscover
