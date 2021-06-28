#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeHandle::getParamCached
// ros::NodeHandle::getCached
class GetParamCachedCall : public RosApiCall {
public:
  GetParamCachedCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : RosApiCall(call, context)
  {}
};

} // rosdiscover::api_call
} // rosdiscover
