#pragma once

namespace rosdiscover {
namespace summary {

// NOTE
// - we don't need to summarize every possible function
// - we only need to visit the transitive closure of functions
//   that call the ROS API
// - we can traverse the call graph to determine which functions to summarize

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


// this is an unresolved summary
class FunctionSummary {
private:
  std::vector<ConditionalCall> calls;
};


} // rosdiscover::summary
} // rosdiscover
