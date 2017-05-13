//----------------------------------------------------------------------------
//
//  Copyright Sabre 2005
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
#pragma once

#include "AddonConstruction/AtpcoGatewayPair.h"
#include "AddonConstruction/GatewayPair.h"
#include "AddonConstruction/SitaGatewayPair.h"
#include "AddonConstruction/SmfGatewayPair.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/NonSerializable.h"

namespace tse
{
// just to allow a non serializeable object's template instantiation
// to compile in contexts where Serializability is assumed.
class ConstructedCacheDataWrapper : public NonSerializable
{
public:
  ConstructedCacheDataWrapper() {}

  ~ConstructedCacheDataWrapper() {}

  size_t size() { return _cachedFares.size(); }

  static inline size_t objectVersion()
  {
    // increment the version number when there are any data members
    // added or removed from this or any containing objects
    return 1;
  }

  CacheConstructedFareInfoVec& ccFares() { return _cachedFares; }

  CacheGatewayPairVec& gateways() { return _gateways; }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _cachedFares);
    FLATTENIZE(archive, _gateways);
  }

  bool operator==(const ConstructedCacheDataWrapper& rhs) const;

  static void dummyData(ConstructedCacheDataWrapper& obj);

  friend inline std::ostream& dumpObject(std::ostream& os, const ConstructedCacheDataWrapper& obj);

protected:
  /* std::shared_ptr doesn't have serialize method.
  friend class boost::serialization::access ;
  */

private:
  CacheConstructedFareInfoVec _cachedFares;

  CacheGatewayPairVec _gateways;
};
}

