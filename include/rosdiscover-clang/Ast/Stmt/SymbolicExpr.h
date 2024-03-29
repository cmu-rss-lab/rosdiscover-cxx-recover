#pragma once

#include "Stmt.h"

namespace rosdiscover {

class SymbolicExpr : public SymbolicStmt {
public:
  virtual ~SymbolicExpr(){};
  virtual void print(llvm::raw_ostream &os) const = 0;
  virtual nlohmann::json toJson() const = 0;
  virtual std::string toString() const = 0;

  virtual std::vector<const SymbolicExpr*> getChildren() const {
    return {};
  }

  std::vector<const SymbolicExpr*> getDescendants() const {
    std::vector<const SymbolicExpr*> result = getChildren();
    for (auto child: getChildren()) {
      auto transitiveChildren = child->getDescendants();
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
      {"kind", "this-expr"},
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
      {"kind", "null-expr"},
      {"string", toString()},
    };
  }
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
    os << "(negate-expr ";
    subExpr->print(os);
    os << ")";
  }

  std::string toString() const override {
    return fmt::format("!({})", subExpr->toString());
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "negate-expr"},
      {"subExpr", subExpr->toJson()},
      {"string", toString()},
    };
  }

  std::vector<const SymbolicExpr*> getChildren() const override {
    return {subExpr.get()};
  }

private:
  std::unique_ptr<SymbolicExpr> subExpr;
};

class BinaryExpr : public SymbolicExpr {
public:
  BinaryExpr(
    std::unique_ptr<SymbolicExpr> lhs,
    std::unique_ptr<SymbolicExpr> rhs
  ) : lhs(std::move(lhs)), rhs(std::move(rhs)) {
    assert(this->lhs != nullptr);
    assert(this->rhs != nullptr);
  }
 
  virtual std::string binaryOperator() const = 0;
  
  std::string toString() const override {
    return fmt::format("({} {} {})", lhs->toString(), binaryOperator(), rhs->toString());
  }

  void print(llvm::raw_ostream &os) const override {
    os << "(binary-expr ";
    lhs->print(os);
    os << " " << binaryOperator() << " ";
    rhs->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "binary-expr"},
      {"operator", binaryOperator()},
      {"lhs", lhs->toJson()},
      {"rhs", rhs->toJson()},
      {"string", toString()},
    };
  }

  std::vector<const SymbolicExpr*> getChildren() const override {
    return {lhs.get(), rhs.get()};
  }


private:
  std::unique_ptr<SymbolicExpr> lhs;
  std::unique_ptr<SymbolicExpr> rhs;
};

class OrExpr : public BinaryExpr {
public:

  OrExpr(
    std::unique_ptr<SymbolicExpr> lhs,
    std::unique_ptr<SymbolicExpr> rhs
  ) : BinaryExpr(std::move(lhs), std::move(rhs)) {}

  std::string binaryOperator() const override {
    return "||";
  }
};

class AndExpr : public BinaryExpr {
public:

  AndExpr(
    std::unique_ptr<SymbolicExpr> lhs,
    std::unique_ptr<SymbolicExpr> rhs
  ) : BinaryExpr(std::move(lhs), std::move(rhs)) {}

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
    std::unique_ptr<SymbolicExpr> lhs,
    std::unique_ptr<SymbolicExpr> rhs,
    const CompareOperator op
    ) : BinaryExpr(std::move(lhs), std::move(rhs)), op(op) {}

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
    std::unique_ptr<SymbolicExpr> lhs,
    std::unique_ptr<SymbolicExpr> rhs,
    const BinaryMathOperator op
    ) : BinaryExpr(std::move(lhs), std::move(rhs)), op(op) {}

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
