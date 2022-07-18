#pragma once

#include "Stmt.h"

namespace rosdiscover {

class SymbolicExpr : public SymbolicStmt {
public:
  virtual ~SymbolicExpr(){};
  virtual void print(llvm::raw_ostream &os) const = 0;
  virtual nlohmann::json toJson() const = 0;
  virtual std::string toString() const = 0;

};

class NegateExpr : public SymbolicExpr {
public:
  NegateExpr(
    std::unique_ptr<SymbolicExpr> subExpr
  ) : subExpr(std::move(subExpr)) {}
  ~NegateExpr(){}
  
  void print(llvm::raw_ostream &os) const override {
    os << "(NegateExpr ";
    subExpr->print(os);
    os << ")";
  }

  std::string toString() const override {
    return "!(" + subExpr->toString() + ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "NegateExpr"},
      {"subExpr", subExpr->toJson()},
      {"string", toString()},
    };
  }

private:
  std::unique_ptr<SymbolicExpr> subExpr;
};

class BinaryExpr : public SymbolicExpr {
public:
  BinaryExpr(
    std::unique_ptr<SymbolicExpr> expr1,
    std::unique_ptr<SymbolicExpr> expr2
  ) : expr1(std::move(expr1)), expr2(std::move(expr2)) {}
 
  virtual std::string binaryOperator() const = 0;
  
  std::string toString() const override {
    return "(" + expr1->toString() + " " +  binaryOperator() + " " + expr2->toString() + ")";
  }

  void print(llvm::raw_ostream &os) const override {
    os << "(BinaryExpr ";
    expr1->print(os);
    os << " " << binaryOperator() << " ";
    expr2->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "BinaryExpr"},
      {"operator", binaryOperator()},
      {"expr1", expr1->toJson()},
      {"expr2", expr2->toJson()},
      {"string", toString()},
    };
  }

private:
  std::unique_ptr<SymbolicExpr> expr1;
  std::unique_ptr<SymbolicExpr> expr2;
};

class OrExpr : public BinaryExpr {
public:
  std::string binaryOperator() const override {
    return "||";
  }
};

class AndExpr : public BinaryExpr {
public:
  std::string binaryOperator() const override {
    return "&&";
  }
};

} // rosdiscover
