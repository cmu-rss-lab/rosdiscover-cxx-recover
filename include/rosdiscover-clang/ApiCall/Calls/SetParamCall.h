#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeHandle::setParam
class SetParamCall : public RosApiCall {
public:
  SetParamCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : RosApiCall(call, context)
  {}
};

} // rosdiscover::api_call
} // rosdiscover
