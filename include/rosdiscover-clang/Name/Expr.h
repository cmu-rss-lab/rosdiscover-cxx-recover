#pragma once

#include <clang/AST/Expr.h>

#include "../ApiCall/RosApiCall.h"

namespace rosdiscover {
namespace name {

/** Attempts to produce a simple symbolic string expression from a ROS name expr */
class NameExpr {};

/** Represents an expression that we are unable to symbolize. */
class Unknown : public NameExpr {};

// TODO implement!
class ArgvExpr : public NameExpr {};

class StringLiteral : public NameExpr {
public:
  StringLiteral(std::string const literal) : literal(literal) {}
  std::string const literal;
};

class RosApiCallExpr : public NameExpr {
  RosApiCallExpr(api_call::RosApiCall *call) : call(call) {}

  api_call::RosApiCall* getCall() const { return call; }

private:
  api_call::RosApiCall *call;
};

class FormalExpr : public NameExpr {
public:
  FormalExpr(clang::ParmVarDecl const *parameter) : parameter(parameter) {}

 clang::ParmVarDecl const * getParameter() const { return parameter; }

private:
  clang::ParmVarDecl const *parameter;
};

} // rosdiscover::name
} // rosdiscover
