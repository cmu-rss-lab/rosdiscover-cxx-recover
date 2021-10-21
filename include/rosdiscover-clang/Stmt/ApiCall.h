#pragma once

#include "Stmt.h"
#include "../Value/String.h"
#include "../Value/Value.h"

namespace rosdiscover {
namespace symbolic {

class SymbolicRosApiCall : public virtual SymbolicStmt {
public:
  ~SymbolicRosApiCall(){}

  SymbolicString const * getName() const {
    return name.get();
  }

protected:
  SymbolicRosApiCall(std::unique_ptr<SymbolicString> name) : name(std::move(name)) {}

private:
  std::unique_ptr<SymbolicString> name;
};

class RosInit : public SymbolicRosApiCall {
public:
  RosInit(std::unique_ptr<SymbolicString> name) : SymbolicRosApiCall(std::move(name)) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(ros-init ";
    getName()->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "ros-init"},
      {"name", getName()->toJson()}
    };
  }
};

class Publisher : public SymbolicRosApiCall {
public:
  Publisher(std::unique_ptr<SymbolicString> name, std::string const &format)
  : SymbolicRosApiCall(std::move(name)), format(format) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(publishes-to ";
    getName()->print(os);
    os << " " << format << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "publishes-to"},
      {"name", getName()->toJson()},
      {"format", format}
    };
  }

private:
  std::string const format;
};

class Subscriber : public SymbolicRosApiCall {
public:
  Subscriber(std::unique_ptr<SymbolicString> name, std::string const &format)
  : SymbolicRosApiCall(std::move(name)), format(format) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(subscribes-to ";
    getName()->print(os);
    os << " " << format << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "subscribes-to"},
      {"name", getName()->toJson()},
      {"format", format}
    };
  }

private:
  std::string const format;
};

class ServiceCaller : public SymbolicRosApiCall {
public:
  ServiceCaller(std::unique_ptr<SymbolicString> name, std::string const &format)
  : SymbolicRosApiCall(std::move(name)), format(format) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(calls-service ";
    getName()->print(os);
    os << " " << format << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "calls-service"},
      {"name", getName()->toJson()},
      {"format", format}
    };
  }

private:
  std::string const format;
};

class ServiceProvider : public SymbolicRosApiCall {
public:
  ServiceProvider(
    std::unique_ptr<SymbolicString> name,
    std::string const &requestFormat,
    std::string const &responseFormat
  ) : SymbolicRosApiCall(std::move(name)),
      requestFormat(requestFormat),
      responseFormat(responseFormat)
  {}

  void print(llvm::raw_ostream &os) const override {
    os << "(provides-service ";
    getName()->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "provides-service"},
      {"name", getName()->toJson()},
      {"request-format", requestFormat},
      {"response-format", responseFormat}
    };
  }

private:
  std::string const requestFormat;
  std::string const responseFormat;
};

class ReadParam :
  public SymbolicRosApiCall,
  public virtual SymbolicValue
{
public:
  ReadParam(std::unique_ptr<SymbolicString> name) : SymbolicRosApiCall(std::move(name)) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(reads-param ";
    getName()->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "reads-param"},
      {"name", getName()->toJson()}
    };
  }
};

class WriteParam : public SymbolicRosApiCall {
public:
  WriteParam(std::unique_ptr<SymbolicString> name, std::unique_ptr<SymbolicValue> value)
    : SymbolicRosApiCall(std::move(name)), value(std::move(value))
  {}

  void print(llvm::raw_ostream &os) const override {
    os << "(writes-to-param ";
    getName()->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "writes-to-param"},
      {"name", getName()->toJson()},
      {"value", value->toJson()}
    };
  }

private:
  std::unique_ptr<SymbolicValue> value;
};

class DeleteParam : public SymbolicRosApiCall {
public:
  DeleteParam(std::unique_ptr<SymbolicString> name) : SymbolicRosApiCall(std::move(name)) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(deletes-param ";
    getName()->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "deletes-param"},
      {"name", getName()->toJson()}
    };
  }
};

class HasParam :
  public SymbolicRosApiCall,
  public virtual SymbolicBool
{
public:
  HasParam(std::unique_ptr<SymbolicString> name) : SymbolicRosApiCall(std::move(name)) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(checks-for-param ";
    getName()->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "checks-for-param"},
      {"name", getName()->toJson()}
    };
  }
};

class ReadParamWithDefault :
  public SymbolicRosApiCall,
  public virtual SymbolicValue
{
public:
  ReadParamWithDefault(
    std::unique_ptr<SymbolicString> name,
    std::unique_ptr<SymbolicValue> defaultValue
  ) : SymbolicRosApiCall(std::move(name)), defaultValue(std::move(defaultValue))
  {}

  SymbolicValue const * getDefaultValue() const {
    return defaultValue.get();
  }

  void print(llvm::raw_ostream &os) const override {
    os << "(reads-param-with-default ";
    getName()->print(os);
    os << " ";
    getDefaultValue()->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "reads-param-with-default"},
      {"name", getName()->toJson()},
      {"default", getDefaultValue()->toJson()}
    };
  }

private:
  std::unique_ptr<SymbolicValue> defaultValue;
};

} // rosdiscover::symbolic
} // rosdiscover
