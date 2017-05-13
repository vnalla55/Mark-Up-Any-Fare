/*
 * OptionalServicesGroupCodeFilter.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: SG0221190
 */

#include "ServiceFees/OptionalServicesGroupCodeFilter.h"

#include "Common/ServiceFeeUtil.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/ServiceGroupInfo.h"


namespace tse {

typedef std::set<ServiceGroup> GroupSet;

void
OptionalServicesGroupCodeFilter::identifyGroupCodesToBeProcessed()
{
  // Since the serviceGroupsVec contains all the groups for all the services - only the groups
  // requested for AncillaryData (RFG/@GBA="OCF" in req v3) and NoData (older requests) should
  // be considered for this service
  std::vector<RequestedOcFeeGroup> requestedGroups;
  for (const auto& elem : _trx.getOptions()->serviceGroupsVec())
  {
    switch (elem.getRequestedInformation())
    {
      case RequestedOcFeeGroup::AncillaryData:
      case RequestedOcFeeGroup::NoData:
        requestedGroups.push_back(elem);
        break;
      default:
        break;
    }
  }

  if (_trx.activationFlags().isAB240())
  {
    // Special case for AB240 - all active groups should be processed
    // if there is an RFG element with GBA="OCF" and no S01 elements
    // or no RFG elements at all
    if (requestedGroups.empty() || _trx.getOptions()->isProcessAllGroups())
    {
      _grValid = _grActive;
      return;
    }
  }
  else if (requestedGroups.empty() || shouldProcessAllGroups())
  {
    // Handling of no requested groups when AB240 is not enabled
    for (const auto elem : _grActive)
    {
      addGroupCodeToProperSet(
          elem, RequestedOcFeeGroup::NoData, true /* markInactiveGroupsAsNotProcessed */);
    }
    return;
  }

  for(std::vector<RequestedOcFeeGroup>::const_iterator iOcFeeGroup = requestedGroups.begin();
      iOcFeeGroup != requestedGroups.end(); ++iOcFeeGroup)
  {
    addGroupCodeToProperSet(iOcFeeGroup->groupCode(), iOcFeeGroup->getRequestedInformation());
  }
}

void
OptionalServicesGroupCodeFilter::addGroupCodeToProperSet(const ServiceGroup& groupCode,
                                                         RequestedOcFeeGroup::RequestedInformation rqInfo,
                                                         bool markInactiveGroupsAsNotProcessed)
{
  switch(validateGroupCode(groupCode, rqInfo))
  {
  case GROUP_VALID:
    _grValid.insert(groupCode);
    break;
  case GROUP_INVALID_FOR_TKT_DATE:
    _grNotActive.insert(groupCode);
    break;
  case GROUP_NOT_IN_ALL_GROUPS:
    _grNotValid.insert(groupCode);
    break;
  case GROUP_NOT_ACTIVE:
    if (markInactiveGroupsAsNotProcessed)
      _grNotProcessed.insert(groupCode);
    else
      _grNotActive.insert(groupCode);
    break;
  case GROUP_NOT_PROCESSED:
    _grNotProcessed.insert(groupCode);
    break;
  }
}

class ServiceGroupIs
{
public:
  ServiceGroupIs(const ServiceGroup& sg) : _sg(sg) { }

  bool operator() (const ServiceGroupInfo* sgi) const { return (sgi->svcGroup() == _sg); }
private:
  const ServiceGroup& _sg;
};

const ServiceGroupInfo*
OptionalServicesGroupCodeFilter::getServiceGroupInfo(const ServiceGroup& sg) const
{
  std::vector<ServiceGroupInfo*>::const_iterator
  iServiceGroup = std::find_if(_allGroupCodes.begin(), _allGroupCodes.end(),
                               ServiceGroupIs(sg));
  if (iServiceGroup != _allGroupCodes.end())
    return (*iServiceGroup);
  else
    return nullptr;
}

OptionalServicesGroupCodeFilter::GroupCodeValidationResult
OptionalServicesGroupCodeFilter::validateGroupCode(const ServiceGroup& sg,
                                                   RequestedOcFeeGroup::RequestedInformation rqInfo)
{
  const ServiceGroupInfo* sgi = getServiceGroupInfo(sg);
  if (!sgi)
    return GROUP_NOT_IN_ALL_GROUPS;

  if (!isGroupActive(sg))
    return GROUP_NOT_ACTIVE;

  if (!_trx.ticketingDate().isBetween(sgi->effDate().date(),
                                      sgi->discDate().date()))
      return GROUP_INVALID_FOR_TKT_DATE;

  if (_trx.activationFlags().isAB240())
  {
    if (rqInfo == RequestedOcFeeGroup::AncillaryData)
      return GROUP_VALID;
    else
      return GROUP_NOT_PROCESSED;
  }

  const AncRequest* req = dynamic_cast<AncRequest*>(_trx.getRequest());
  if (_trx.billing()->requestPath() == ACS_PO_ATSE_PATH ||
      _trx.billing()->actionCode().substr(0, 5) == "MISC6" ||
      (req && req->isWPBGRequest()))
  {
    if (ServiceFeeUtil::checkServiceGroupForAcs(sg))
      return GROUP_NOT_PROCESSED;

    if(ServiceFeeUtil::isServiceGroupInvalidForAcs(sg))
      return GROUP_INVALID_FOR_TKT_DATE;
  }

  // If this point is reached - the group has passed all the checks - it is valid
  return GROUP_VALID;
}

void
OptionalServicesGroupCodeFilter::updateNotProcessedSet()
{
  for (const auto elem : _allGroupCodes)
  {
    const ServiceGroup& group = elem->svcGroup();
    if (!isInSetValid(group) && !isInSetNotActive(group) && !isInSetNotProcessed(group))
      _grNotProcessed.insert(elem->svcGroup());
  }
}

}; // namespace tse
