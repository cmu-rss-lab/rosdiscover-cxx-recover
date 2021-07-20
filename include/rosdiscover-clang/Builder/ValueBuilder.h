#pragma once

#include <string>

#include "../Symbolic/Bool.h"
#include "../Symbolic/String.h"
#include "../Symbolic/Value.h"

namespace rosdiscover {

class ValueBuilder {
public:
    std::unique_ptr<SymbolicBool> boolLiteral(bool literal) const {
        return std::make_unique<SymbolicBool>(literal);
    }

    std::unique_ptr<StringLiteral> stringLiteral(std::string const &string) const {
        return std::make_unique<StringLiteral>(string);
    }

    std::unique_ptr<Unknown> unknown() const {
        return std::make_unique<Unknown>():
    }
};

} // rosdiscover