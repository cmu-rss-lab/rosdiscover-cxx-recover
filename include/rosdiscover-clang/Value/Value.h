#pragma once

#include <memory>

#include <nlohmann/json.hpp>
#include <llvm/Support/raw_ostream.h>
#include "../Ast/Stmt/SymbolicExpr.h"
#include <fmt/core.h>

namespace rosdiscover {

enum class SymbolicValueType {
  String,
  Bool,
  Float,
  Integer,
  Unsupported,
  NodeHandle
};

class SymbolicValue : public SymbolicExpr {
public:
  virtual ~SymbolicValue(){};
  virtual void print(llvm::raw_ostream &os) const = 0;
  virtual nlohmann::json toJson() const = 0;
  virtual bool isUnknown() const { return false; }

  static SymbolicValueType getSymbolicType(clang::QualType clangType) {
    clangType = clangType.getUnqualifiedType();
    auto typeName = clangType.getAsString();
    return getSymbolicType(typeName);
  }

  static SymbolicValueType getSymbolicType(std::string typeName) {
    llvm::outs() << "DEBUG: determining symbolic type for Clang type [" << typeName << "]\n";
    if (typeName == "std::string"
     || typeName.find("const char") != std::string::npos
     || typeName == "std::string &"
     || typeName == "const std::string &") {
      return SymbolicValueType::String;
    } else if (typeName == "bool" ||
               typeName == "_Bool" ) {
      return SymbolicValueType::Bool;
    } else if (typeName == "int"
            || typeName == "long") {
      return SymbolicValueType::Integer;
    } else if (typeName == "float" 
            || typeName == "double") {
      return SymbolicValueType::Float;
    } else if (typeName == "ros::NodeHandle"
            || typeName == "ros::NodeHandle &"
            || typeName == "ros::NodeHandle *") {
      return SymbolicValueType::NodeHandle;
    } else {
      return SymbolicValueType::Unsupported;
    }
  }

  static std::string getSymbolicTypeAsString(SymbolicValueType const &type) {
    switch (type) {
      case SymbolicValueType::String:
        return "string";
      case SymbolicValueType::Bool:
        return "bool";
      case SymbolicValueType::Float:
        return "float";        
      case SymbolicValueType::Integer:
        return "integer";
      case SymbolicValueType::Unsupported:
        return "unsupported";
      case SymbolicValueType::NodeHandle:
        return "node-handle";
    }
  }
}; // SymbolicValue


class SymbolicConstant : public SymbolicValue {
public:
  SymbolicConstant(
    clang::APValue const value
  ) : value(value) {}
  ~SymbolicConstant(){}
  
  void print(llvm::raw_ostream &os) const override {
    os << "(symbolic-constant " << toString() << ")";
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
      {"kind", "symbolic-constant"},
      {"string", toString()},
    };
  }

private:
  clang::APValue value;
};


class SymbolicString : public virtual SymbolicValue {};

class SymbolicBool : public virtual SymbolicValue {};

class SymbolicFloat : public virtual SymbolicValue {};

class SymbolicInteger : public virtual SymbolicValue {};

class SymbolicNodeHandle :
  public virtual SymbolicString,
  public virtual SymbolicValue
{};

class SymbolicUnknown :
  public virtual SymbolicString,
  public virtual SymbolicBool,
  public virtual SymbolicInteger,
  public virtual SymbolicFloat,
  public virtual SymbolicNodeHandle
{
public:
  SymbolicUnknown(){}
  ~SymbolicUnknown(){}

  bool isUnknown() const override {
    return true;
  }

  std::string toString() const override {
    return "UNKNOWN";
  }

  void print(llvm::raw_ostream &os) const override {
    os << "UNKNOWN";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "unknown"}
    };
  }
};

class SymbolicArg:
  public virtual SymbolicString,
  public virtual SymbolicBool,
  public virtual SymbolicFloat,
  public virtual SymbolicInteger,
  public virtual SymbolicNodeHandle
{
public:
  SymbolicArg(std::string const &name) : name(name) {}
  ~SymbolicArg(){}

  void print(llvm::raw_ostream &os) const override {
    os << "(arg " << name << ")";
  }

  std::string toString() const override {
    return name;
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "arg"},
      {"name", name}
    };
  }

private:
  std::string const name;
};

class SymbolicNodeHandleImpl :
  public virtual SymbolicNodeHandle
{
public:
  SymbolicNodeHandleImpl(std::unique_ptr<SymbolicString> name)
    : name(std::move(name)) { 
      assert(this->name != nullptr); 
  }
  SymbolicNodeHandleImpl(SymbolicNodeHandleImpl &&other)
    : name(std::move(other.name)) { 
    assert(this->name != nullptr); 
  }
  ~SymbolicNodeHandleImpl(){}

  static std::unique_ptr<SymbolicNodeHandle> unknown() {
    return std::make_unique<SymbolicNodeHandleImpl>(std::make_unique<SymbolicUnknown>());
  }

  bool isUnknown() const override {
    return name->isUnknown();
  }

  void print(llvm::raw_ostream &os) const override {
    os << "(node-handle ";
    name->print(os);
    os << ")";
  }

  std::string toString() const override {
    return name->toString();
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "node-handle"},
      {"namespace", name->toJson()}
    };
  }

private:
  std::unique_ptr<SymbolicString> name;
};

} // rosdiscover
