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

class SymbolicConstant : public SymbolicExpr {
public:
  SymbolicConstant(
    clang::APValue const value
  ) : value(value) {}
  ~SymbolicConstant(){}
  
  void print(llvm::raw_ostream &os) const override {
    os << "(SymbolicConstant " << toString() << ")";
  }

  std::string toString() const override {
    if (value.isFloat()) {
      return std::to_string(value.getFloat().convertToDouble());
    } else if (value.isInt()) {
      return std::to_string(value.getInt().getSExtValue());
    }
    return "unsupported type";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "SymbolicConstant"},
      {"string", toString()},
    };
  }

private:
  clang::APValue value;
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

  OrExpr(
    std::unique_ptr<SymbolicExpr> expr1,
    std::unique_ptr<SymbolicExpr> expr2
  ) : BinaryExpr(std::move(expr1), std::move(expr2)) {}

  std::string binaryOperator() const override {
    return "||";
  }
};

class AndExpr : public BinaryExpr {
public:

  AndExpr(
    std::unique_ptr<SymbolicExpr> expr1,
    std::unique_ptr<SymbolicExpr> expr2
  ) : BinaryExpr(std::move(expr1), std::move(expr2)) {}

  std::string binaryOperator() const override {
    return "&&";
  }
};

enum class CompareOperator {
  EQ,
  NE,
  LT,
  LE,
  GT,
  GE
};

class CompareExpr : public BinaryExpr {
public:

  CompareExpr(
    std::unique_ptr<SymbolicExpr> expr1,
    std::unique_ptr<SymbolicExpr> expr2,
    const CompareOperator op
    ) : BinaryExpr(std::move(expr1), std::move(expr2)), op(op) {}

  std::string binaryOperator() const override {
    switch (op) {
      case CompareOperator::EQ:
        return "==";
      case CompareOperator::NE:
        return "!=";
      case CompareOperator::LT:
        return "<";
      case CompareOperator::LE:
        return "<=";
      case CompareOperator::GT:
        return ">";
      case CompareOperator::GE:
        return ">=";
    }
  }

  const CompareOperator getOperator() const {
    return op;
  }

private:
  const CompareOperator op;
};

enum class BinaryMathOperator {
  Add,
  Sub,
  Mul,
  Div,
  Rem
};

class BinaryMathExpr : public BinaryExpr {
public:

  BinaryMathExpr(
    std::unique_ptr<SymbolicExpr> expr1,
    std::unique_ptr<SymbolicExpr> expr2,
    const BinaryMathOperator op
    ) : BinaryExpr(std::move(expr1), std::move(expr2)), op(op) {}

  std::string binaryOperator() const override {
    switch (op) {
      case BinaryMathOperator::Add:
        return "+";
      case BinaryMathOperator::Sub:
        return "-";
      case BinaryMathOperator::Mul:
        return "*";
      case BinaryMathOperator::Div:
        return "/";
      case BinaryMathOperator::Rem:
        return "%";        
    }
  }

  const BinaryMathOperator getOperator() const {
    return op;
  }

private:
  const BinaryMathOperator op;
};

} // rosdiscover
