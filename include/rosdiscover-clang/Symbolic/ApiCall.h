#pragma once

#include "Stmt.h"
#include "String.h"
#include "Value.h"

namespace rosdiscover {
namespace symbolic {

class SymbolicRosApiCall : public virtual SymbolicStmt {
public:
  ~SymbolicRosApiCall(){}

  SymbolicString const * getName() const {
    return name;
  }

protected:
  SymbolicRosApiCall(SymbolicString const *name) : name(name) {}

private:
  SymbolicString const *name;
};

class RosInit : public SymbolicRosApiCall {
public:
  RosInit(SymbolicString const *name) : SymbolicRosApiCall(name) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(ros-init ";
    getName()->print(os);
    os << ")";
  }
};

class Publisher : public SymbolicRosApiCall {
public:
  Publisher(SymbolicString const *name) : SymbolicRosApiCall(name) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(publishes_to ";
    getName()->print(os);
    os << ")";
  }
};

class Subscriber : public SymbolicRosApiCall {
public:
  Subscriber(SymbolicString const *name) : SymbolicRosApiCall(name) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(subscribes_to ";
    getName()->print(os);
    os << ")";
  }
};

class ServiceCaller : public SymbolicRosApiCall {
public:
  ServiceCaller(SymbolicString const *name) : SymbolicRosApiCall(name) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(calls_service ";
    getName()->print(os);
    os << ")";
  }
};

class ServiceProvider : public SymbolicRosApiCall {
public:
  ServiceProvider(SymbolicString const *name) : SymbolicRosApiCall(name) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(provides_service ";
    getName()->print(os);
    os << ")";
  }
};

// also: SymbolicValue
class ReadParam : public SymbolicRosApiCall {
public:
  ReadParam(SymbolicString const *name) : SymbolicRosApiCall(name) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(reads_param ";
    getName()->print(os);
    os << ")";
  }
};

class WriteParam : public SymbolicRosApiCall {
public:
  WriteParam(SymbolicString const *name, SymbolicValue const *value)
    : SymbolicRosApiCall(name), value(value)
  {}

  void print(llvm::raw_ostream &os) const override {
    os << "(writes_to_param ";
    getName()->print(os);
    os << ")";
  }

private:
  SymbolicValue const *value;
};

class DeleteParam : public SymbolicRosApiCall {
public:
  WriteParam(SymbolicString const) : SymbolicRosApiCall(name) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(deletes_param ";
    getName()->print(os);
    os << ")";
  }
};

// also: SymbolicBool
class HasParam : public SymbolicRosApiCall {
public:
  HasParam(SymbolicString const *name) : SymbolicRosApiCall(name) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(checks_for_param ";
    getName()->print(os);
    os << ")";
  }
};

// also: SymbolicValue
class ReadParamWithDefault : public SymbolicRosApiCall {
public:
  ReadParamWithDefault(SymbolicString const *name, SymbolicValue const *defaultValue)
    : SymbolicRosApiCall(name), defaultValue(defaultValue)
  {}

  SymbolicValue const * getDefaultValue() const {
    return defaultValue;
  }

  void print(llvm::raw_ostream &os) const override {
    os << "(reads_param_with_default ";
    getName()->print(os);
    os << " ";
    getDefaultValue()->print(os);
    os << ")";
  }

private:
  SymbolicValue const *defaultValue;
};

/** Stores a symbolic ROS API call and its underlying corresponding raw call. */
/*
class AnnotatedSymbolicRosApiCall {
public:
  AnnotatedSymbolicRosApiCall(
      RosApiCall const *apiCall,
      SymbolicRosApiCall const *symbolizedCall
  ) : apiCall(apiCall), symbolizedCall(symbolizedCall)
  {}

  RosApiCall const * getApiCall() const {
    return apiCall;
  }

  SymbolicRosApiCall const * getSymbolized() const {
    return symbolizedCall;
  }

  clang::CallExpr const * getCallExpr() const {
    return getApiCall()->getCallExpr();
  }

private:
  RosApiCall const *apiCall;
  SymbolicRosApiCall const *symbolizedCall;
};
*/

} // rosdiscover::symbolic
} // rosdiscover
