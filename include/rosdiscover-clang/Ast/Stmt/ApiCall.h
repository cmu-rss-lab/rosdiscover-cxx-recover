#pragma once

#include "Stmt.h"
#include "../../Value/String.h"
#include "../../Value/Value.h"
#include "../../Value/Bool.h"

#include "ControlDependency.h"

namespace rosdiscover {

class SymbolicRosApiCall : public virtual SymbolicStmt {
public:
  ~SymbolicRosApiCall(){}
};

class NamedSymbolicRosApiCall : public virtual SymbolicRosApiCall {
public:
  ~NamedSymbolicRosApiCall(){}

  SymbolicString const * getName() const {
    return name.get();
  }

protected:
  NamedSymbolicRosApiCall(std::unique_ptr<SymbolicString> name) : name(std::move(name)) {
    assert(getName() != nullptr);
  }

private:
  std::unique_ptr<SymbolicString> name;
};

class RosInit : public NamedSymbolicRosApiCall {
public:
  RosInit(std::unique_ptr<SymbolicString> name) : NamedSymbolicRosApiCall(std::move(name)) {}

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

class Publisher : public NamedSymbolicRosApiCall {
public:
  Publisher(std::unique_ptr<SymbolicString> name, std::string const &format)
  : NamedSymbolicRosApiCall(std::move(name)), format(format) {}

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

class Subscriber : public NamedSymbolicRosApiCall {
public:
  Subscriber(std::unique_ptr<SymbolicString> name, std::string const &format, std::unique_ptr<SymbolicFunctionCall> callback)
  : NamedSymbolicRosApiCall(std::move(name)), format(format), callback(std::move(callback)) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(subscribes-to ";
    getName()->print(os);
    os << " " << format << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "subscribes-to"},
      {"name", getName()->toJson()},
      {"format", format},
      {"callback-name", callback->getCalleeName()->toJson()}
    };
  }

private:
  std::string const format;
  std::unique_ptr<SymbolicFunctionCall> callback;
};

class RateSleep : public SymbolicRosApiCall {
public:
  RateSleep(std::unique_ptr<SymbolicFloat> rate) : SymbolicRosApiCall(), rate(std::move(rate)) {
    assert(this->rate != nullptr);
  }

  void print(llvm::raw_ostream &os) const override {
    os << "(ratesleep ";
    rate->print(os);
    os << ")";
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "ratesleep"},
      {"rate", rate->toJson()}
    };
  }

private:
  std::unique_ptr<SymbolicFloat> rate;
};

class Publish : public SymbolicRosApiCall {
public:
  Publish(std::string const &publisher, std::unique_ptr<SymbolicExpr> pathCondition = std::make_unique<BoolLiteral>(true)) : 
    SymbolicRosApiCall(), 
    publisher(publisher), 
    pathCondition(std::move(pathCondition)
  ) {
    assert(this->pathCondition != nullptr);
  }

  void print(llvm::raw_ostream &os) const override {
    os << "(publish " << publisher << ")";
  }

  nlohmann::json toJson() const override {    
    return {
      {"kind", "publish"},
      {"publisher", publisher},
      {"path_condition", pathCondition->toString()}
    };
  }

private:
  std::string const publisher;
  std::unique_ptr<SymbolicExpr> pathCondition;
};

class ServiceCaller : public NamedSymbolicRosApiCall {
public:
  ServiceCaller(std::unique_ptr<SymbolicString> name, std::string const &format)
  : NamedSymbolicRosApiCall(std::move(name)), format(format) {}

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

class ServiceProvider : public NamedSymbolicRosApiCall {
public:
  ServiceProvider(
    std::unique_ptr<SymbolicString> name,
    std::string const &requestFormat,
    std::string const &responseFormat
  ) : NamedSymbolicRosApiCall(std::move(name)),
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
  public NamedSymbolicRosApiCall,
  public virtual SymbolicValue
{
public:
  ReadParam(std::unique_ptr<SymbolicString> name) : NamedSymbolicRosApiCall(std::move(name)) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(reads-param ";
    getName()->print(os);
    os << ")";
  }

  std::string toString() const override {
    return fmt::format("ros::param::get(param={})", getName()->toString());
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "reads-param"},
      {"name", getName()->toJson()}
    };
  }
};

class WriteParam : public NamedSymbolicRosApiCall {
public:
  WriteParam(std::unique_ptr<SymbolicString> name, std::unique_ptr<SymbolicValue> value)
    : NamedSymbolicRosApiCall(std::move(name)), value(std::move(value))
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

class DeleteParam : public NamedSymbolicRosApiCall {
public:
  DeleteParam(std::unique_ptr<SymbolicString> name) : NamedSymbolicRosApiCall(std::move(name)) {}

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
  public NamedSymbolicRosApiCall,
  public virtual SymbolicBool
{
public:
  HasParam(std::unique_ptr<SymbolicString> name) : NamedSymbolicRosApiCall(std::move(name)) {}

  void print(llvm::raw_ostream &os) const override {
    os << "(checks-for-param ";
    getName()->print(os);
    os << ")";
  }

  std::string toString() const override {    
    return fmt::format("ros::param::has(param={})", getName()->toString());
  }

  nlohmann::json toJson() const override {
    return {
      {"kind", "checks-for-param"},
      {"name", getName()->toJson()}
    };
  }
};

class ReadParamWithDefault :
  public NamedSymbolicRosApiCall,
  public virtual SymbolicValue
{
public:
  ReadParamWithDefault(
    std::unique_ptr<SymbolicString> name,
    std::unique_ptr<SymbolicValue> defaultValue
  ) : NamedSymbolicRosApiCall(std::move(name)), defaultValue(std::move(defaultValue)) {
    assert(getName() != nullptr);    
    assert(getDefaultValue() != nullptr);
  }

  SymbolicValue const * getDefaultValue() const {
    return defaultValue.get();
  }

  std::string toString() const override {
    return fmt::format("ros::param::read(param={}, default={})", getName()->toString(), getDefaultValue()->toString());
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

} // rosdiscover
