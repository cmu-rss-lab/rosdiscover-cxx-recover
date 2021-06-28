#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeHandle::param
class GetParamWithDefaultCall : public RosApiCall {
public:
  GetParamWithDefaultCall(clang::CallExpr const *call) : RosApiCall(call) {}
};

} // rosdiscover::api_call
} // rosdiscover
