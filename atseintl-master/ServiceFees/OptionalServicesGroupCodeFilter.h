/*
 * OptionalServicesGroupCodeFilter.h
 *
 *  Created on: Dec 16, 2014
 *      Author: SG0221190
 */

#pragma once

#include "Common/TseCodeTypes.h"
#include "DataModel/PricingOptions.h"

#include <set>
#include <vector>

namespace tse {

class ServiceGroupInfo;
class PricingTrx;

class OptionalServicesGroupCodeFilter
{
public:
  OptionalServicesGroupCodeFilter(PricingTrx& trx,
                                  bool shouldProcessAllGroups,
                                  const std::vector<ServiceGroupInfo*>& allGroupCodes,
                                  const std::set<ServiceGroup>& grActive) :
    _shouldProcessAllGroups(shouldProcessAllGroups),
    _trx(trx),
    _allGroupCodes(allGroupCodes),
    _grActive(grActive)
  {
    identifyGroupCodesToBeProcessed();
    updateNotProcessedSet();
  }

  ~OptionalServicesGroupCodeFilter()
  {

  }

  const std::set<ServiceGroup>& grValid() const { return _grValid; }
  bool isInSetValid(const ServiceGroup& sg) const;
  bool isInSetValid(const char* sg) const;

  const std::set<ServiceGroup>& grNotValid() const { return _grNotValid; }
  bool isInSetNotValid(const ServiceGroup& sg) const;
  bool isInSetNotValid(const char* sg) const;

  const std::set<ServiceGroup>& grNotActive() const { return _grNotActive; }
  bool isInSetNotActive(const ServiceGroup& sg) const;
  bool isInSetNotActive(const char* sg) const;

  const std::set<ServiceGroup>& grNotProcessed() const { return _grNotProcessed; }
  bool isInSetNotProcessed(const ServiceGroup& sg) const;
  bool isInSetNotProcessed(const char* sg) const;

  const ServiceGroupInfo* getServiceGroupInfo(const ServiceGroup& sg) const;

private:
  enum GroupCodeValidationResult
  {
    GROUP_VALID = 0,
    GROUP_INVALID_FOR_TKT_DATE = 1,
    GROUP_NOT_IN_ALL_GROUPS = 2,
    GROUP_NOT_ACTIVE = 3,
    GROUP_NOT_PROCESSED = 4
  };

  bool isGroupActive(const ServiceGroup& sg)
  {
    return (_grActive.find(sg) != _grActive.end());
  }

  void identifyGroupCodesToBeProcessed();
  void updateNotProcessedSet();

  GroupCodeValidationResult validateGroupCode(const ServiceGroup& sg,
                                              RequestedOcFeeGroup::RequestedInformation rqInfo);

  void addGroupCodeToProperSet(const ServiceGroup& groupCode,
                               RequestedOcFeeGroup::RequestedInformation rqInfo,
                               bool markInactiveGroupsAsNotProcessed = false);

  bool shouldProcessAllGroups() { return _shouldProcessAllGroups; }

  std::set<ServiceGroup> _grValid;
  std::set<ServiceGroup> _grNotValid;
  std::set<ServiceGroup> _grNotActive;
  std::set<ServiceGroup> _grNotProcessed;

  bool _shouldProcessAllGroups;

  PricingTrx& _trx;
  const std::vector<ServiceGroupInfo*>& _allGroupCodes;
  const std::set<ServiceGroup>& _grActive;
};

inline bool
OptionalServicesGroupCodeFilter::isInSetValid(const ServiceGroup& sg) const
{
  return (_grValid.find(sg) != _grValid.end());
}

inline bool
OptionalServicesGroupCodeFilter::isInSetValid(const char* sg) const
{
  return (_grValid.find(sg) != _grValid.end());
}

inline bool
OptionalServicesGroupCodeFilter::isInSetNotValid(const ServiceGroup& sg) const
{
  return (_grNotValid.find(sg) != _grNotValid.end());
}

inline bool
OptionalServicesGroupCodeFilter::isInSetNotValid(const char* sg) const
{
  return (_grNotValid.find(sg) != _grNotValid.end());
}

inline bool
OptionalServicesGroupCodeFilter::isInSetNotActive(const ServiceGroup& sg) const
{
  return (_grNotActive.find(sg) != _grNotActive.end());
}

inline bool
OptionalServicesGroupCodeFilter::isInSetNotActive(const char* sg) const
{
  return (_grNotActive.find(sg) != _grNotActive.end());
}

inline bool
OptionalServicesGroupCodeFilter::isInSetNotProcessed(const ServiceGroup& sg) const
{
  return (_grNotProcessed.find(sg) != _grNotProcessed.end());
}

inline bool
OptionalServicesGroupCodeFilter::isInSetNotProcessed(const char* sg) const
{
  return (_grNotProcessed.find(sg) != _grNotProcessed.end());
}

}; // namespace tse

