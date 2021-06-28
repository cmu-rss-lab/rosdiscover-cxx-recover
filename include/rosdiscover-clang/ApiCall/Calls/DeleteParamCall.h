#pragma once

#include "../RosApiCall.h"

namespace rosdiscover {
namespace api_call {

// ros::NodeParam::deleteParam
class DeleteParamCall : public RosApiCall {
public:
  DeleteParamCall(clang::CallExpr const *call) : RosApiCall(call) {}
};

} // rosdiscover::api_call
} // rosdiscover
