#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeHandle::setParam
class SetParamCall : public RosApiCall {
public:
  SetParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};

} // rosdiscover::api_call
} // rosdiscover
