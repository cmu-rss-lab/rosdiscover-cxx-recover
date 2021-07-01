#pragma once

#include <clang/AST/Expr.h>

namespace rosdiscover {
namespace name {

/** Attempts to produce a simple symbolic string expression from a ROS name expr **/
class NameExpr {};

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
  FormalExpr(clang::ParmValDecl const *parameter) : parameter(parameter) {}

 clang::ParmValDecl const * getParameter() const { return parameter; }

private:
  clang::ParmValDecl const *parameter;
};

} // rosdiscover::name
} // rosdiscover
