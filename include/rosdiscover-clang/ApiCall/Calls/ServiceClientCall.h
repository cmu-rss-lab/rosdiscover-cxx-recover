#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeHandle::serviceClient
class ServiceClientCall : public RosApiCall {
public:
  ServiceClientCall(clang::CallExpr const *call) : RosApiCall(call) {}
};

} // rosdiscover::api_call
} // rosdiscover
