#pragma once

#include <clang/AST/Stmt.h>

namespace rosdiscover {

class CallOrConstructExpr {
public:
  virtual ~CallOrConstructExpr(){}

  virtual clang::Expr* getExpr() = 0;
  virtual clang::Expr* const getExpr() const = 0;

  virtual clang::Expr* getArg(unsigned Arg) = 0;
  virtual clang::Expr** getArgs() = 0;
  virtual clang::Expr* const * getArgs() const = 0;
  virtual unsigned getNumArgs() const = 0;

  virtual clang::SourceLocation getBeginLoc() const = 0;
  virtual clang::SourceLocation getEndLoc() const = 0;

  virtual bool isCallExpr() const = 0;
  virtual bool isConstructExpr() const = 0;
};

class CallExpr : public CallOrConstructExpr {
public:
  CallExpr(clang::CallExpr *expr) : expr(expr) {}
  CallExpr(clang::CallExpr const *expr)
    : expr(const_cast<clang::CallExpr*>(expr))
  {}

  clang::Expr* getExpr() override { return expr; }
  clang::Expr* const getExpr() const override { return expr; }

  clang::CallExpr* getCallExpr() { return expr; }
  clang::CallExpr* const getCallExpr() const { return expr; }

  clang::Expr* getArg(unsigned index) override { return expr->getArg(index); }
  clang::Expr** getArgs() override { return expr->getArgs(); }
  clang::Expr* const * getArgs() const override { return expr->getArgs(); }
  unsigned getNumArgs() const override { return expr->getNumArgs(); }

  clang::SourceLocation getBeginLoc() const override { return expr->getBeginLoc(); }
  clang::SourceLocation getEndLoc() const override { return expr->getEndLoc(); }

  bool isCallExpr() const override { return true; }
  bool isConstructExpr() const override { return false; }

private:
  clang::CallExpr *expr;
};

class ConstructExpr : public CallOrConstructExpr {
public:
  ConstructExpr(clang::CXXConstructExpr *expr) : expr(expr) {}
  ConstructExpr(clang::CXXConstructExpr const *expr)
    : expr(const_cast<clang::CXXConstructExpr*>(expr))
  {}

  clang::Expr* getExpr() override { return expr; }
  clang::Expr* const getExpr() const override { return expr; }

  clang::CXXConstructExpr* getConstructExpr() { return expr; }
  clang::CXXConstructExpr* const getConstructExpr() const { return expr; }

  clang::Expr* getArg(unsigned index) override { return expr->getArg(index); }
  clang::Expr** getArgs() override { return expr->getArgs(); }
  clang::Expr* const * getArgs() const override { return expr->getArgs(); }
  unsigned getNumArgs() const override { return expr->getNumArgs(); }

  clang::SourceLocation getBeginLoc() const override { return expr->getBeginLoc(); }
  clang::SourceLocation getEndLoc() const override { return expr->getEndLoc(); }

  bool isCallExpr() const override { return false; }
  bool isConstructExpr() const override { return true; }

private:
  clang::CXXConstructExpr *expr;
};

}