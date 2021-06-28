#pragma once

#include "RosApiCall.h"

namespace rosdiscover {
namespace api_call {

class RosInitCall : public RosApiCall {
public:
  RosInitCall(clang::CallExpr const *call) : RosApiCall(call) {}

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::init")))
      ).bind("call");
    }

  protected:
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


class BareServiceCall : public RosApiCall {
public:
  BareServiceCall(clang::CallExpr const *call) : RosApiCall(call) {}

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::service::call")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new BareServiceCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
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


class BareGetParamCall : public RosApiCall {
public:
  BareGetParamCall(clang::CallExpr const *call) : RosApiCall(call) {}

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::param::get")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new BareGetParamCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
};


class BareGetParamWithDefaultCall : public RosApiCall {
public:
  BareGetParamWithDefaultCall(clang::CallExpr const *call) : RosApiCall(call) {}

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::param::param")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new BareGetParamWithDefaultCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
};


class BareGetParamCachedCall : public RosApiCall {
public:
  BareGetParamCachedCall(clang::CallExpr const *call) : RosApiCall(call) {}

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::param::getCached")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new BareGetParamCachedCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
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

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::param::set")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new BareSetParamCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
};


// ros::NodeHandle::hasParam
class HasParamCall : public RosApiCall {
public:
  HasParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


class BareHasParamCall : public RosApiCall {
public:
  BareHasParamCall(clang::CallExpr const *call) : RosApiCall(call) {}

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::param::has")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new BareHasParamCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
};


// ros::NodeParam::deleteParam
class DeleteParamCall : public RosApiCall {
public:
  DeleteParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};


class BareDeleteParamCall : public RosApiCall {
public:
  BareDeleteParamCall(clang::CallExpr const *call) : RosApiCall(call) {}

  class Finder : public RosApiCall::Finder {
  public:
    Finder(std::vector<RosApiCall*> &found) : RosApiCall::Finder(found) {}

    const clang::ast_matchers::StatementMatcher getPattern() override {
      using namespace clang::ast_matchers;
      return callExpr(
        callee(functionDecl(hasName("ros::param::del")))
      ).bind("call");
    }

  protected:
    RosApiCall* build(clang::ast_matchers::MatchFinder::MatchResult const &result) override {
      return new BareDeleteParamCall(result.Nodes.getNodeAs<clang::CallExpr>("call"));
    }
  };
};


} // rosdiscover::api_call
} // rosdiscover
