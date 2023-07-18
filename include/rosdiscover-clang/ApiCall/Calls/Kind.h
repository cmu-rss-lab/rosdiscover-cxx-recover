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
  MessageFiltersSubscribeCall,
  MessageFiltersRegisterCallbackCall,
  ServiceClientCall,
  SetParamCall,
  SubscribeTopicCall,
  CreateTimerCall
};

} // rosdiscover::api_call
} // rosdiscover
