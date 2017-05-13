//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/BaggageTravel.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"

#include <vector>

namespace tse
{
class BaggageTravel;
class ServiceFeesGroup;
class SubCodeInfo;
class Itin;

struct CheckPortionOfTravelIndicator
{
  CheckPortionOfTravelIndicator(char portionOfTravelIndicator)
    : _portionOfTravelIndicator(portionOfTravelIndicator)
  {
  }

  bool operator()(const TravelSeg* travelSeg) const
  {
    return travelSeg->checkedPortionOfTravelInd() == _portionOfTravelIndicator;
  }
private:
  char _portionOfTravelIndicator;
};

struct CheckServiceGroup : std::unary_function<const ServiceFeesGroup*, bool>
{
  CheckServiceGroup(ServiceGroup groupCode) : _groupCode(groupCode) {}

  bool operator()(const ServiceFeesGroup* srvGroup) const
  {
    return (srvGroup->groupCode() == _groupCode);
  }

private:
  ServiceGroup _groupCode;
};

struct CheckRequestedServiceGroup : std::unary_function<RequestedOcFeeGroup, bool>
{
  CheckRequestedServiceGroup(const ServiceGroup& groupCode) : _groupCode(groupCode) {}

  bool operator()(const RequestedOcFeeGroup& srvGroup) const
  {
    return (srvGroup.groupCode() == _groupCode);
  }

private:
  const ServiceGroup& _groupCode;
};

struct CheckOCFeesSeg : std::unary_function<const OCFees::OCFeesSeg*, bool>
{
  CheckOCFeesSeg(const OptionalServicesInfo* s7) : _optServInfo(s7) {}

  bool operator()(const OCFees::OCFeesSeg* ocFeesSeg) const
  {
    return (ocFeesSeg->_optFee == _optServInfo);
  }

private:
  const OptionalServicesInfo* _optServInfo;
};

struct CheckS5 : std::unary_function<const SubCodeInfo*, bool>
{
  CheckS5(const ServiceSubTypeCode& serviceSubTypeCode) : _serviceSubTypeCode(serviceSubTypeCode) {}

  bool operator()(const SubCodeInfo* s5) const
  {
    return (s5->fltTktMerchInd() == BAGGAGE_CHARGE) && (s5->serviceSubTypeCode() == _serviceSubTypeCode);
  }

private:
  const ServiceSubTypeCode _serviceSubTypeCode;
};

struct CheckServiceTypeNeeded : std::unary_function<RequestedOcFeeGroup, bool>
{
  CheckServiceTypeNeeded(const ServiceGroup& serviceGroup, const Indicator& ancillaryServiceType)
    : _serviceGroup(serviceGroup), _ancillaryServiceType(ancillaryServiceType)
  {
  }

  bool operator()(const RequestedOcFeeGroup& requestedOcFeeGroup) const
  {
    return requestedOcFeeGroup.groupCode() == _serviceGroup &&
           (requestedOcFeeGroup.isEmptyAncillaryServiceType() ||
            requestedOcFeeGroup.isAncillaryServiceType(_ancillaryServiceType));
  }

private:
  ServiceGroup _serviceGroup;
  Indicator _ancillaryServiceType;
};

} // tse
