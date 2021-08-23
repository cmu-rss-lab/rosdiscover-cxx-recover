#pragma once

#include <nlohmann/json.hpp>

#include <llvm/Support/raw_ostream.h>

namespace rosdiscover {
namespace symbolic {

enum class SymbolicValueType {
  String,
  Bool,
  Integer,
  Unsupported,
  NodeHandle
};

class SymbolicValue {
public:
  virtual ~SymbolicValue(){};
  virtual void print(llvm::raw_ostream &os) const = 0;
  virtual nlohmann::json toJson() const = 0;

  static SymbolicValueType getSymbolicType(clang::QualType clangType) {
    clangType = clangType.getUnqualifiedType();
    auto typeName = clangType.getAsString();
    if (typeName == "std::string") {
      return SymbolicValueType::String;
    } else if (typeName == "bool") {
      return SymbolicValueType::Bool;
    } else if (typeName == "int") {
      return SymbolicValueType::Integer;
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
      case SymbolicValueType::Integer:
        return "integer";
      case SymbolicValueType::Unsupported:
        return "unsupported";
      case SymbolicValueType::NodeHandle:
        return "node-handle";
    }
  }
}; // SymbolicValue

class SymbolicString : public virtual SymbolicValue {};

class SymbolicBool : public virtual SymbolicValue {};

class SymbolicInteger : public virtual SymbolicValue {};

class SymbolicNodeHandle : public virtual SymbolicValue {
public:
  SymbolicNodeHandle(std::unique_ptr<SymbolicString> name)
    : name(std::move(name))
  {}

  ~SymbolicNodeHandle();

  void print(llvm::raw_ostream &os) const override {
    os << "NodeHandle[";
    name.get()->print(os);
    os << "]";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "node-handle"},
      {"name", name->toJson()}
    };
  }

private:
  std::unique_ptr<SymbolicString> name;
};

class SymbolicUnknown :
  public virtual SymbolicString,
  public virtual SymbolicBool,
  public virtual SymbolicInteger
{
public:
  SymbolicUnknown(){}
  ~SymbolicUnknown(){}

  void print(llvm::raw_ostream &os) const override {
    os << "UNKNOWN";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "unknown"}
    };
  }
};

} // rosdiscover::symbolic
} // rosdiscover
