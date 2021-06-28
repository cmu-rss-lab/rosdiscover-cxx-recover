#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeHandle::getParam
class GetParamCall : public RosApiCall {
public:
  GetParamCall(clang::CallExpr const *call, clang::ASTContext const *context)
    : RosApiCall(call, context)
  {}
};

} // rosdiscover::api_call
} // rosdiscover
