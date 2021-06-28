#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeHandle::getParamCached
// ros::NodeHandle::getCached
class GetParamCachedCall : public BareRosApiCall {
public:
  GetParamCachedCall(clang::CallExpr const *call) : BareRosApiCall(call) {}
};

} // rosdiscover::api_call
} // rosdiscover
