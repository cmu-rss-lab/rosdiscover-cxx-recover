#pragma once

namespace rosdiscover {
namespace summary {

class CallArgument {

};

// - extract each call, the symbolic arguments to that call, and its path condition
//  - a subset of those calls represent ROS API calls
// - for now, if we see a loop, we get scared and produce an "incomplete" summary
class ConditionalCall {
private:
  // TODO maintain the qualified name of the function/constructor being called instead?
  clang::NamedDecl* callee;
  std::vector<CallArgument> args;
  BoolExpr pathCondition;
};

// we need to state the symbolic variable type for each function parameter that we model
class FunctionParameter {

};


class FunctionSummary {

};


} // rosdiscover::summary
} // rosdiscover
