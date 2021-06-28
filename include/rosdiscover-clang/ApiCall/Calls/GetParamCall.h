#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeHandle::getParam
class GetParamCall : public RosApiCall {
public:
  GetParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};

} // rosdiscover::api_call
} // rosdiscover
