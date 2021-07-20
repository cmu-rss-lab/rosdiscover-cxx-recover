#pragma once

#include <string>

#include "../Symbolic/Bool.h"
#include "../Symbolic/String.h"
#include "../Symbolic/Value.h"

namespace rosdiscover {
namespace symbolic {

class ValueBuilder {
public:
    std::unique_ptr<SymbolicBool> boolLiteral(bool literal) const {
        return std::make_unique<SymbolicBool>(literal);
    }

    std::unique_ptr<StringLiteral> stringLiteral(std::string const &string) const {
        return std::make_unique<StringLiteral>(string);
    }

    std::unique_ptr<SymbolicUnknown> unknown() const {
        return std::make_unique<SymbolicUnknown>();
    }
};

} // rosdiscover::symbolic
} // rosdiscover