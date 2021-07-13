#pragma once

namespace rosdiscover {
namespace api_call {

class SymbolicRosApiCall {
public:
  SymbolicString const * getName() const {
    return name;
  }

protected:
  SymbolicRosApiCall(SymbolicString const *name) : name(name) {}

private:
  SymbolicString const *name;
};

class Publisher : public SymbolicRosApiCall {
public:
  Publisher(StringExpr const *name) : SymbolicRosApiCall(name) {}
}

class Subscriber : public SymbolicRosApiCall {
public:
  Subscriber(StringExpr const *name) : SymbolicRosApiCall(name) {}
}

class ServiceCaller : public SymbolicRosApiCall {
public:
  ServiceCall(StringExpr const *name) : SymbolicRosApiCall(name) {}
}

class ServiceProvider : public SymbolicRosApiCall {
public:
  ServiceProvider(StringExpr const *name) : SymbolicRosApiCall(name) {}
}

// also: SymbolicValue
class ReadParam : public SymbolicRosApiCall {
public:
  ServiceProvider(StringExpr const *name) : SymbolicRosApiCall(name) {}
}

class WriteParam : public SymbolicRosApiCall {
public:
  WriteParam(StringExpr const *name, SymbolicValue const *value)
    : SymbolicRosApiCall(name), value(value)
  {}

private:
  SymbolicValue const *value;
}

// also: SymbolicBool
class HasParam : public SymbolicRosApiCall {
public:
  ServiceProvider(StringExpr const *name) : SymbolicRosApiCall(name) {}
}

// also: SymbolicValue
class ReadParamWithDefault : public SymbolicRosApiCall {
public:
  ServiceProvider(StringExpr const *name, SymbolicValue const *defaultValue)
    : SymbolicRosApiCall(name), defaultValue(defaultValue)
  {}

  SymbolicValue const * getDefaultValue() const {
    return defaultValue;
  }

private:
  SymbolicValue const *defaultValue;
}

/** Stores a symbolic ROS API call and its underlying corresponding raw call. */
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

} // rosdiscover::api_call
} // rosdiscover
