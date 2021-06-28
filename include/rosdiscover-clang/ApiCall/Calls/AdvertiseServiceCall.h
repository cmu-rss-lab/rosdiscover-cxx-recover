#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeHandle::advertiseService
class AdvertiseServiceCall : public RosApiCall {
public:
  AdvertiseServiceCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : RosApiCall(call, context)
  {}
};

} // rosdiscover::api_call
} // rosdiscover
