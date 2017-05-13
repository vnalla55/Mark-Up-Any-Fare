//----------------------------------------------------------------------------
//
//  File:           PaxTypeFilter.cpp
//  Created:        4/7/2004
//  Authors:
//
//  Description:    Common functions required for ATSE shopping/pricing.
//
//  Updates:
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
//----------------------------------------------------------------------------

#include "Common/PaxTypeFilter.h"

#include "Common/PaxTypeUtil.h"
#include "Common/TseConsts.h"
#include "Common/Vendor.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/PaxTypeInfo.h"

#include <algorithm>
#include <set>

namespace tse
{
class MatchPaxType
{

public:
  MatchPaxType(std::set<PaxTypeCode>& paxTypes, PaxTypeFilter::PaxType type)
    : _paxTypes(paxTypes), _type(type)
  {
  }

  bool operator()(const PaxTypeInfo* info)
  {
    if (info == nullptr)
      return false;

    switch (_type)
    {
    case PaxTypeFilter::ALL:
    {
      _paxTypes.insert(info->paxType());
    }
    break;
    case PaxTypeFilter::ADULT:
    {
      if (info->isAdult())
      {
        _paxTypes.insert(info->paxType());
      }
    }
    break;
    case PaxTypeFilter::CHILD:
    {
      if (info->isChild())
      {
        _paxTypes.insert(info->paxType());
      }
      break;
    }
    case PaxTypeFilter::INFANT:

    {
      if (info->isInfant())
      {
        _paxTypes.insert(info->paxType());
      }
      break;
    }
    default:
      return false;
      break;
    }
    return true;
  }

private:
  std::set<PaxTypeCode>& _paxTypes;
  PaxTypeFilter::PaxType _type;
};

bool
PaxTypeFilter::getAllAdultPaxType(PricingTrx& trx, std::set<PaxTypeCode>& paxTypes)
{
  const std::vector<PaxTypeInfo*>& allPaxTypes = trx.dataHandle().getAllPaxType();
  std::for_each(
      allPaxTypes.begin(), allPaxTypes.end(), MatchPaxType(paxTypes, PaxTypeFilter::ADULT));
  return !paxTypes.empty();
}

bool
PaxTypeFilter::getAllInfantPaxType(PricingTrx& trx, std::set<PaxTypeCode>& paxTypes)
{

  const std::vector<PaxTypeInfo*>& allPaxTypes = trx.dataHandle().getAllPaxType();

  std::for_each(
      allPaxTypes.begin(), allPaxTypes.end(), MatchPaxType(paxTypes, PaxTypeFilter::INFANT));
  return !paxTypes.empty();
}

bool
PaxTypeFilter::getAllChildPaxType(PricingTrx& trx, std::set<PaxTypeCode>& paxTypes)
{

  const std::vector<PaxTypeInfo*>& allPaxTypes = trx.dataHandle().getAllPaxType();

  std::for_each(
      allPaxTypes.begin(), allPaxTypes.end(), MatchPaxType(paxTypes, PaxTypeFilter::CHILD));
  return !paxTypes.empty();
}

bool
PaxTypeFilter::getAllPaxType(PricingTrx& trx, std::set<PaxTypeCode>& paxTypes)
{

  const std::vector<PaxTypeInfo*>& allPaxTypes = trx.dataHandle().getAllPaxType();

  std::for_each(allPaxTypes.begin(), allPaxTypes.end(), MatchPaxType(paxTypes, PaxTypeFilter::ALL));
  return !paxTypes.empty();
}
}
