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
  RosInitCall,
  MessageFiltersSubscriberCall,
  ServiceClientCall,
  SetParamCall,
  SubscribeTopicCall
};

} // rosdiscover::api_call
} // rosdiscover
