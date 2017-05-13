//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#include "AddonConstruction/ConstructedCacheDataWrapper.h"

#include "Common/ObjectComparison.h"

namespace tse
{

bool
ConstructedCacheDataWrapper::
operator==(const ConstructedCacheDataWrapper& rhs) const
{
  bool eq(_cachedFares.size() == rhs._cachedFares.size() &&
          _gateways.size() == rhs._gateways.size());

  for (size_t i = 0; (eq && (i < _cachedFares.size())); ++i)
  {
    eq = (*(_cachedFares[i]) == *(rhs._cachedFares[i]));
  }

  for (size_t i = 0; (eq && (i < _gateways.size())); ++i)
  {
    eq = (*(_gateways[i]) == *(rhs._gateways[i]));
  }

  return eq;
}

void
ConstructedCacheDataWrapper::dummyData(ConstructedCacheDataWrapper& obj)
{
  ConstructedFareInfo* info1 = new ConstructedFareInfo;
  ConstructedFareInfo* info2 = new ConstructedFareInfo;
  ConstructedFareInfo::dummyData(*info1);
  ConstructedFareInfo::dummyData(*info2);
  obj._cachedFares.push_back(std::shared_ptr<ConstructedFareInfo>(info1));
  obj._cachedFares.push_back(std::shared_ptr<ConstructedFareInfo>(info2));

  GatewayPair* atpPair = new AtpcoGatewayPair;
  GatewayPair* sitaPair = new SitaGatewayPair;
  GatewayPair* smfPair = new SmfGatewayPair;
  AtpcoGatewayPair::dummyData(*atpPair);
  SitaGatewayPair::dummyData(*sitaPair);
  SmfGatewayPair::dummyData(*smfPair);
  obj._gateways.push_back(std::shared_ptr<GatewayPair>(atpPair));
  obj._gateways.push_back(std::shared_ptr<GatewayPair>(sitaPair));
  obj._gateways.push_back(std::shared_ptr<GatewayPair>(smfPair));
}

std::ostream&
dumpObject(std::ostream& os, const ConstructedCacheDataWrapper& obj)
{
  os << "["
     << "_cachedFares:" << obj._cachedFares.size();
  for (const auto& elem : obj._cachedFares)
  {
    dumpObject(os, *elem);
  }
  for (const auto& elem : obj._gateways)
  {
    dumpObject(os, *elem);
  }
  os << "]";

  return os;
}
}
