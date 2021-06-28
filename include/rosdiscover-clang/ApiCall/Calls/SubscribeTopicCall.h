#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeHandle::subscribe
//
class SubscribeTopicCall : public RosApiCall {
public:
  SubscribeTopicCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : RosApiCall(call, context)
  {}
};

} // rosdiscover::api_call
} // rosdiscover
