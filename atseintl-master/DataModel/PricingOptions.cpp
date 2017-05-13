//-------------------------------------------------------------------
//
//  File:        PricingOptions.cpp
//  Created:     March 10, 2004
//  Authors:
//
//  Description: Process options
//
//  Updates:
//          03/10/04 - VN - file created.
//          04/13/04 - Mike Carroll - added alternateCurrency
//          04/14/05 - Quan Ta - rename from "Options" to PricingOptions
//
//  Copyright Sabre 2004
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

#include "DataModel/PricingOptions.h"

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/bind.hpp>

#include <algorithm>
#include <functional>

using namespace tse;

namespace
{
class CheckServiceTypePredicate
{
public:
  CheckServiceTypePredicate(const Indicator& ancillaryServiceType)
                         : _ancillaryServiceType(ancillaryServiceType)
  {
  }

  bool operator()(const RequestedOcFeeGroup& requestedOcFeeGroup) const
  {
    return (requestedOcFeeGroup.isAncillaryServiceType(_ancillaryServiceType));
  }

private:
  Indicator _ancillaryServiceType;
};
}

void
PricingOptions::getGroupCodes(const std::vector<RequestedOcFeeGroup>& requestedGroups,
                              std::vector<ServiceGroup>& groupCodes) const
{
  std::vector<RequestedOcFeeGroup>::const_iterator groupsIter = requestedGroups.begin();
  std::vector<RequestedOcFeeGroup>::const_iterator groupsIterEnd = requestedGroups.end();

  for (; groupsIter != groupsIterEnd; ++groupsIter)
  {
    groupCodes.push_back((*groupsIter).groupCode());
  }
}

bool
PricingOptions::isOcOrBaggageDataRequested(RequestedOcFeeGroup::RequestedInformation ri) const
{
  if (_processAllGroups && ri == RequestedOcFeeGroup::AncillaryData)
    return true;

  return isGroupRequested(ri);
}

bool
PricingOptions::isGroupRequested(RequestedOcFeeGroup::RequestedInformation ri) const
{
  return std::any_of(_serviceGroupsVec.begin(), _serviceGroupsVec.end(),
    boost::bind(&RequestedOcFeeGroup::getRequestedInformation, _1) == ri);
}

bool
PricingOptions::isServiceTypeRequested(const Indicator& ancillaryServiceType) const
{
  if (_processAllGroups)
  {
    return true;
  }
  else
  {
    return boost::algorithm::any_of(_serviceGroupsVec.begin(),
                                    _serviceGroupsVec.end(),
                                    CheckServiceTypePredicate(ancillaryServiceType));
  }
}


bool
RequestedOcFeeGroup::isAncillaryServiceType(const Indicator& ancillaryServiceType) const
{
  return _ancillaryServiceTypes.find(ancillaryServiceType) != _ancillaryServiceTypes.end();
}

void
RequestedOcFeeGroup::addAncillaryServiceType(const Indicator& ancillaryServiceType)
{
  _ancillaryServiceTypes.insert(ancillaryServiceType);
}

void
RequestedOcFeeGroup::emptyAncillaryServiceType()
{
  _ancillaryServiceTypes.erase(_ancillaryServiceTypes.begin(), _ancillaryServiceTypes.end());
}

bool
RequestedOcFeeGroup::isEmptyAncillaryServiceType() const
{
  return _ancillaryServiceTypes.empty();
}
