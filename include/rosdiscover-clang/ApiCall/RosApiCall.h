#pragma once

#include <vector>

#include <clang/AST/Expr.h>


namespace rosdiscover {
namespace api_call {


// TODO differentiate between Bare and NodeHandle API calls

class RosApiCall {
public:
  clang::CallExpr const * getCallExpr() const { return call; }

  class Finder : public clang::ast_matchers::MatchFinder::MatchCallback {
  public:
    void run(const clang::ast_matchers::MatchFinder::MatchResult &result) {
      if (auto *api_call = build(result)) {
        found.push_back(api_call);
      }
    }

  protected:
    Finder(std::vector<RosApiCall*> &found) : found(found) {}

    virtual RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) = 0;
    virtual const clang::ast_matchers::StatementMatcher getPattern() = 0;

  private:
    std::vector<RosApiCall*> &found;
  };

protected:
  RosApiCall(clang::CallExpr const *call) : call(call) {}

private:
  clang::CallExpr const *call;
}; // RosApiCall


// ros::init
class RosInitCall : public RosApiCall {
public:
  RosInitCall(clang::CallExpr const *call) : RosApiCall(call) {}

  class Finder : RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

  protected:
    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::init")))
      ).bind("call");
    }

    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new RosInitCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };

  // hasKnownName() -> bool
  // getNameExpr() -> Expr
}; // RosInitCall


// ros::NodeHandle::advertise
class AdvertiseTopicCall : public RosApiCall {
public:
  AdvertiseTopicCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


// ros::NodeHandle::subscribe
class SubscribeTopicCall : public RosApiCall {
public:
  SubscribeTopicCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


// ros::service::call
class BareServiceCall : public RosApiCall {
public:
  BareServiceCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


// ros::NodeHandle::serviceClient
class ServiceClientCall : public RosApiCall {
public:
  ServiceClientCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


// ros::NodeHandle::advertiseService
class AdvertiseServiceCall : public RosApiCall {
public:
  AdvertiseServiceCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


// ros::NodeHandle::getParam
class GetParamCall : public RosApiCall {
public:
  GetParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};



// ros::NodeHandle::param
class GetParamWithDefaultCall : public RosApiCall {
public:
  GetParamWithDefaultCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


// ros::param::get
class BareGetParamCall : public RosApiCall {
public:
  BareGetParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


/// ros::param::param
class BareGetParamWithDefaultCall : public RosApiCall {
public:
  BareGetParamWithDefaultCall(clang::CallExpr const *call) : RosApiCall(call) {}
};



// ros::param::getCached
class BareGetParamCachedCall : public RosApiCall {
public:
  BareGetParamCachedCall(clang::CallExpr const *call) : RosApiCall(call) {}
};



// ros::NodeHandle::getParamCached
// ros::NodeHandle::getCached
class GetParamCachedCall : public RosApiCall {
public:
  GetParamCachedCall(clang::CallExpr const *call) : RosApiCall(call) {}
};



// ros::NodeHandle::setParam
class SetParamCall : public RosApiCall {
public:
  SetParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


// ros::param::set
class BareSetParamCall : public RosApiCall {
public:
  BareSetParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


// ros::NodeHandle::hasParam
class HasParamCall : public RosApiCall {
public:
  HasParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


// ros::param::has
class BareHasParamCall : public RosApiCall {
public:
  BareHasParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


// ros::NodeParam::deleteParam
class DeleteParamCall : public RosApiCall {
public:
  DeleteParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


// ros::param::del
class BareDeleteParamCall : public RosApiCall {
public:
  BareDeleteParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


} // rosdiscover::api_call
} // rosdiscover
