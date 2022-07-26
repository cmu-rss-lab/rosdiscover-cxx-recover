#pragma once

#include "Stmt.h"

namespace rosdiscover {

class SymbolicExpr : public SymbolicStmt {
public:
  virtual ~SymbolicExpr(){};
  virtual void print(llvm::raw_ostream &os) const = 0;
  virtual nlohmann::json toJson() const = 0;
  virtual std::string toString() const = 0;

  virtual std::vector<SymbolicExpr*> getChildren() const {
    return {};
  }

  std::vector<SymbolicExpr*> getTransitiveChildren() const {
    std::vector<SymbolicExpr*> result = getChildren();
    for (auto child: getChildren()) {
      auto transitiveChildren = child->getTransitiveChildren();
      result.insert(result.end(), transitiveChildren.begin(), transitiveChildren.end());
    }
    return result;
  }

};

class ThisExpr : public SymbolicExpr {
public:
  ThisExpr() {}
  ~ThisExpr(){}
  
  void print(llvm::raw_ostream &os) const override {
    os << toString();
  }

  std::string toString() const override {
    return "this";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "ThisExpr"},
      {"string", toString()},
    };
  }
};

class NullExpr : public SymbolicExpr {
public:
  NullExpr() {}
  ~NullExpr(){}
  
  void print(llvm::raw_ostream &os) const override {
    os << toString();
  }

  std::string toString() const override {
    return "NULL";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "NullExpr"},
      {"string", toString()},
    };
  }
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
  ) : subExpr(std::move(subExpr)) {
    assert(this->subExpr != nullptr);
  }
  ~NegateExpr(){}
  
  void print(llvm::raw_ostream &os) const override {
    os << "(NegateExpr ";
    subExpr->print(os);
    os << ")";
  }

  std::string toString() const override {
    return fmt::format("!({})", subExpr->toString());
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "NegateExpr"},
      {"subExpr", subExpr->toJson()},
      {"string", toString()},
    };
  }

  std::vector<SymbolicExpr*> getChildren() const override {
    return {subExpr.get()};
  }

private:
  std::unique_ptr<SymbolicExpr> subExpr;
};

class BinaryExpr : public SymbolicExpr {
public:
  BinaryExpr(
    std::unique_ptr<SymbolicExpr> expr1,
    std::unique_ptr<SymbolicExpr> expr2
  ) : expr1(std::move(expr1)), expr2(std::move(expr2)) {
    assert(this->expr1 != nullptr);
    assert(this->expr2 != nullptr);
  }
 
  virtual std::string binaryOperator() const = 0;
  
  std::string toString() const override {
    return fmt::format("({} {} {})", expr1->toString(), binaryOperator(), expr2->toString());
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

  std::vector<SymbolicExpr*> getChildren() const override {
    return {expr1.get(), expr2.get()};
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
  GE,
  Spaceship
};

class CompareExpr : public BinaryExpr {
public:

  static CompareOperator compareOperatorFromOpCode(clang::BinaryOperator::Opcode opCode) {
    switch (opCode) {
      case clang::BinaryOperator::Opcode::BO_EQ:
        return CompareOperator::EQ;
      case clang::BinaryOperator::Opcode::BO_NE:
        return CompareOperator::NE;
      case clang::BinaryOperator::Opcode::BO_LT:
        return CompareOperator::LT;
      case clang::BinaryOperator::Opcode::BO_LE:
        return CompareOperator::LE;
      case clang::BinaryOperator::Opcode::BO_GT:
        return CompareOperator::GT;
      case clang::BinaryOperator::Opcode::BO_GE:
        return CompareOperator::GE;
      case clang::BinaryOperator::Opcode::BO_Cmp: 
        return CompareOperator::Spaceship;
      default:
        llvm::outs() << "ERROR: Invalid compare operator (opCode): " << opCode;
        abort();
    }
  }

  CompareExpr(
    std::unique_ptr<SymbolicExpr> expr1,
    std::unique_ptr<SymbolicExpr> expr2,
    const CompareOperator op
    ) : BinaryExpr(std::move(expr1), std::move(expr2)), op(op) {}

  static const CompareOperator compareOperatorFromOverloadedOperatorKind(const clang::OverloadedOperatorKind opCode) {
    switch (opCode) {
      case clang::OO_EqualEqual: 
        return CompareOperator::EQ;
      case clang::OO_ExclaimEqual: 
        return CompareOperator::NE;
      case clang::OO_Less:
        return CompareOperator::LT;
      case clang::OO_LessEqual:
        return CompareOperator::LE;
      case clang::OO_Greater:
        return CompareOperator::GT;
      case clang::OO_GreaterEqual:
        return CompareOperator::GE;
      case clang::OO_Spaceship:
        return CompareOperator::Spaceship;
      default:
        llvm::outs() << "ERROR: Invalid compare operator (opCode): " << opCode;
        abort();
    }
  }

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
      case CompareOperator::Spaceship:
        return "<=>";
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

  static BinaryMathOperator binaryMathOperatorFromOpCode(clang::BinaryOperator::Opcode opCode) {
    switch (opCode) {
      case clang::BinaryOperator::Opcode::BO_Add: 
        return BinaryMathOperator::Add;
      case clang::BinaryOperator::Opcode::BO_Sub: 
        return BinaryMathOperator::Sub;
      case clang::BinaryOperator::Opcode::BO_Mul: 
        return BinaryMathOperator::Mul;
      case clang::BinaryOperator::Opcode::BO_Div: 
        return BinaryMathOperator::Div;
      case clang::BinaryOperator::Opcode::BO_Rem: 
        return BinaryMathOperator::Rem;
      default:
        llvm::outs() << "ERROR: Invalid binary math operator: " << opCode;
        abort();
    }
  }
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
