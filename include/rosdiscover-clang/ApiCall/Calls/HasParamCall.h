#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeHandle::hasParam
class HasParamCall : public RosApiCall {
public:
  HasParamCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : RosApiCall(call, context)
  {}
};

} // rosdiscover::api_call
} // rosdiscover
