#pragma once

#include <clang/AST/Expr.h>


namespace rosdiscover {
namespace api_call {


// TODO differentiate between Bare and NodeHandle API calls

class RosApiCall {
public:
  clang::CallExpr* getCallExpr() const { return call; }

private:
  RosApiCall(clang::CallExpr *call) : call(call) {}

  clang::CallExpr *call;
}; // RosApiCall


// ros::init
class RosInitCall : public RosApiCall {
public:
  RosInitCall(clang::CallExpr *call) : RosApiCall(call) {}

  class Finder : public MatchFinder::MatchCallback {
  public:
    virtual void run(const MatchFinder::MatchResult &result) override;
  };

  static void addFinder(RosApiCallFinder &finder);

  // hasKnownName() -> bool
  // getNameExpr() -> Expr
}; // RosInitCall


// ros::NodeHandle::advertise
class AdvertiseTopicCall : public RosApiCall {
public:
  AdvertiseTopicCall(clang::CallExpr *call) : RosApiCall(call) {}
};


// ros::NodeHandle::subscribe
class SubscribeTopicCall : public RosApiCall {
public:
  SubscribeTopicCall(clang::CallExpr *call) : RosApiCall(call) {}
};


// ros::service::call
class BareServiceCall : public RosApiCall {
public:
  BareServiceCall(clang::CallExpr *call) : RosApiCall(call) {}
};


// ros::NodeHandle::serviceClient
class ServiceClientCall : public RosApiCall {
public:
  ServiceClientCall(clang::CallExpr *call) : RosApiCall(call) {}
}:


// ros::NodeHandle::advertiseService
class AdvertiseServiceCall : public RosApiCall {
public:
  AdvertiseServiceCall(clang::CallExpr *call) : RosApiCall(call) {}
};


// ros::NodeHandle::getParam
class GetParamCall : public RosApiCall {
public:
  GetParamCall(clang::CallExpr *call) : RosApiCall(call) {}
};



// ros::NodeHandle::param
class GetParamWithDefaultCall : public RosApiCall {
public:
  GetParamWithDefaultCall(clang::CallExpr *call) : RosApiCall(call) {}
};


// ros::param::get
class BareGetParamCall : public RosApiCall {
public:
  BareGetParamCall(clang::CallExpr *call) : RosApiCall(call) {}
};


/// ros::param::param
class BareGetParamWithDefaultCall : public RosApiCall {
public:
  BareGetParamWithDefaultCall(clang::CallExpr *call) : RosApiCall(call) {}
};



// ros::param::getCached
class BareGetParamCachedCall : public RosApiCall {
public:
  BareGetParamCachedCall(clang::CallExpr *call) : RosApiCall(call) {}
};



// ros::NodeHandle::getParamCached
// ros::NodeHandle::getCached
class GetParamCachedCall : public RosApiCall {
public:
  GetParamCachedCall(clang::CallExpr *call) : RosApiCall(call) {}
};



// ros::NodeHandle::setParam
class SetParamCall : public RosApiCall {
public:
  SetParamCall(clang::CallExpr *call) : RosApiCall(call) {}
};


// ros::param::set
class BareSetParamCall : public RosApiCall {
public:
  BareSetParamCall(clang::CallExpr *call) : RosApiCall(call) {}
};


// ros::NodeHandle::hasParam
class HasParamCall : public RosApiCall {
public:
  HasParamCall(clang::CallExpr *call) : RosApiCall(call) {}
};


// ros::param::has
class BareHasParamCall : public RosApiCall {
public:
  BareHasParamCall(clang::CallExpr *call) : RosApiCall(call) {}
};


// ros::NodeParam::deleteParam
class DeleteParamCall : public RosApiCall {
public:
  DeleteParamCall(clang::CallExpr *call) : RosApiCall(call) {}
};


// ros::param::del
class BareDeleteParamCall : public RosApiCall {
public:
  BareDeleteParamCall(clang::CallExpr *call) : RosApiCall(call) {}
};


} // rosdiscover::api_call
} // rosdiscover
