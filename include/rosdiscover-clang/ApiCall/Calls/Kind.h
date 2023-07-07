#pragma once

namespace rosdiscover {
namespace api_call {

enum class RosApiCallKind {
  AdvertiseServiceCall,
  AdvertiseTopicCall,
  BareDeleteParamCall,
  BareGetParamCachedCall,
  BareGetParamCall,
  BareGetParamWithDefaultCall,
  BareHasParamCall,
  BareServiceCall,
  BareSetParamCall,
  DeleteParamCall,
  GetParamCachedCall,
  GetParamCall,
  GetParamWithDefaultCall,
  HasParamCall,
  PublishCall,
  RateSleepCall,
  ConstSleepCall,
  RosInitCall,
  MessageFiltersSubscriberCall,
  MessageFiltersRegisterCallbackCall,
  ServiceClientCall,
  SetParamCall,
  SubscribeTopicCall
};

} // rosdiscover::api_call
} // rosdiscover
