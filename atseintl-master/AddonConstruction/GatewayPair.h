//-------------------------------------------------------------------
//
//  File:        GatewayPair.h
//  Created:     May 14, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Abstract class to represent common data and methods
//               to build single- and double- ended constructed fares:
//               1. [Gateway,            Construction Point]
//               2. [Construction Point, Gateway           ]
//               3. [Construction Point, Construction Point]
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

#pragma once

#include "AddonConstruction/ConstructionDefs.h"
#include "AddonConstruction/GatewayPairFactory.h"
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
#include "AddonConstruction/SpecifiedFareCache.h"
#endif
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/Flattenizable.h"

#include <memory>
#include <vector>

namespace tse
{
class GatewayPair;

namespace flattenizer
{
template <class GatewayPair>
inline void
flatten(Flattenizable::Archive& archive, const std::vector<std::shared_ptr<GatewayPair>>& v);
template <class GatewayPair>
inline void
unflatten(Flattenizable::Archive& archive, std::vector<std::shared_ptr<GatewayPair>>& v);
template <class GatewayPair>
inline void
calcmem(Flattenizable::Archive& archive, const std::vector<std::shared_ptr<GatewayPair>>& v);
}
}

namespace tse
{
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
class AtpcoFareDateInterval;
#else
class DateIntervalBase;
#endif
class ConstructionVendor;

class GatewayPair
{
public:
  // construction/destruction
  // ========================
  GatewayPair() = default;

  virtual ~GatewayPair() {};

  void initialize(ConstructionJob* cj, ConstructionVendor* vendor);

  void initialize(ConstructionJob* cj,
                  ConstructionVendor* vendor,
                  const LocCode& gateway1,
                  const LocCode& gateway2,
                  bool isGw1CP,
                  bool isGw2CP,
                  unsigned int gw1FirstFare,
                  unsigned int gw1FareCount,
                  unsigned int gw2FirstFare,
                  unsigned int gw2FareCount);

  void clear();

  // main interface
  // ==== =========

  void process(CacheConstructedFareInfoVec& response);
  void prepareData();

  // accessors
  // =========

  const LocCode& gateway1() const
  {
    return _gateway1;
  };
  const LocCode& multiCity1() const
  {
    return _multiCity1;
  };

  const LocCode& gateway2() const
  {
    return _gateway2;
  };
  const LocCode& multiCity2() const
  {
    return _multiCity2;
  };

  bool& needsReconstruction()
  {
    return _needsReconstruction;
  };
  const bool needsReconstruction() const
  {
    return _needsReconstruction;
  };

  const bool isGw1ConstructPoint() const
  {
    return _isGw1ConstructPoint;
  };
  const bool isGw2ConstructPoint() const
  {
    return _isGw2ConstructPoint;
  };

  unsigned int& gw1FirstFare()
  {
    return _gw1FirstFare;
  };
  const unsigned int gw1FirstFare() const
  {
    return _gw1FirstFare;
  };

  unsigned int& gw1FareCount()
  {
    return _gw1FareCount;
  };
  const unsigned int gw1FareCount() const
  {
    return _gw1FareCount;
  };

  unsigned int& gw2FirstFare()
  {
    return _gw2FirstFare;
  };
  const unsigned int gw2FirstFare() const
  {
    return _gw2FirstFare;
  };

  unsigned int& gw2FareCount()
  {
    return _gw2FareCount;
  };
  const unsigned int gw2FareCount() const
  {
    return _gw2FareCount;
  };

  virtual eGatewayPairType objectType() const = 0;

  virtual void flattenize(Flattenizable::Archive& archive);

  virtual bool operator==(const GatewayPair& rhs) const;

  static void dummyData(GatewayPair& obj);

protected:
  ConstructionJob* _cJob = nullptr;
  ConstructionVendor* _vendor = nullptr;

  LocCode _gateway1;
  LocCode _multiCity1;

  LocCode _gateway2;
  LocCode _multiCity2;

  DateTime _date;

  bool _needsReconstruction = false;

  bool _isGw1ConstructPoint = false;
  bool _isGw2ConstructPoint = false;

  unsigned int _gw1FirstFare = 0;
  unsigned int _gw1FareCount = 0;

  unsigned int _gw2FirstFare = 0;
  unsigned int _gw2FareCount = 0;

  ConstructedFareList _constructedFares;

  virtual FareMatchCode finalMatch()
  {
    return FM_GOOD_MATCH;
  };

  void adjustPrevValues();

  void defineOriginDestination();

  void populateResponse(CacheConstructedFareInfoVec& response);

  virtual ConstructedFare* getConstructedFare() = 0;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

  void getSpecifiedFares(SpecifiedFareList*& specFares);

  void retrieveSpecifiedFares(const LocCode& loc1,
                              const LocCode& loc2,
                              const char* logMsg,
                              SpecifiedFareList& specFares);

  ConstructedFare* getConstructedFare(const FareInfo* specFare, bool oppositeSpecified);

  void matchCFsAndAddons();

  virtual FareMatchCode
  matchAddonAndSpecified(AddonFareCortege& addonFare,
                         SpecifiedFare& specFare,
                         const bool isOriginAddon,
                         const bool oppositeSpecified,
                         AtpcoFareDateInterval& validInterval) = 0;

  virtual AddonFareCortegeVec::iterator partition(AddonFareCortegeVec::iterator firstAddon,
                                                  AddonFareCortegeVec::iterator endOfAddons);

  virtual std::pair<AddonFareCortegeVec::iterator, AddonFareCortegeVec::iterator>
  getOwrtMatchingAddonRange(const FareInfo& sfi,
                            AddonFareCortegeVec::iterator firstAddon,
                            AddonFareCortegeVec::iterator endOfAddons,
                            AddonFareCortegeVec::iterator roundTripMayNotBeHalvedAddonBound);

  void getAddonsForCurrency(const std::set<CurrencyCode>& currencies,
                            AddonFareCortegeVec::iterator firstAddon,
                            AddonFareCortegeVec::iterator endOfAddons,
                            std::set<std::pair<CurrencyCode,
                                               const AddonFareInfo*> >& addonsValidForCurrency);

  void cacheSpecFares(SpecifiedFareList*);

  SpecifiedFareList* _specFares = nullptr;
  bool _specFaresFromCache = false;

#else

  uint32_t getSpecifiedFares(std::vector<const FareInfo*>& specFares);

  uint32_t retrieveSpecifiedFares(const LocCode& loc1,
                                  const LocCode& loc2,
                                  const char* logMsg,
                                  std::vector<const FareInfo*>& specFares);

  void createConstructedFares(std::vector<const FareInfo*>& specFares);

  void matchCFsAndAddons(bool matchOriginAddons);

  FareMatchCode matchAddonAndSpecified(AddonFareCortege& addonFare,
                                       ConstructedFare& constrFare,
                                       const bool isOriginAddon);

  virtual FareMatchCode
  matchAddonAndSpecified(AddonFareCortege& addonFare,
                         ConstructedFare& constrFare,
                         const bool isOriginAddon,
                         std::vector<DateIntervalBase*>& constrFareIntervals) = 0;

  virtual AddonFareCortegeVec::iterator partition(const AddonFareCortegeVec::iterator& firstAddon,
                                                  const AddonFareCortegeVec::iterator& endOfAddons);

  virtual std::pair<AddonFareCortegeVec::iterator, AddonFareCortegeVec::iterator>
  getAddonIterators(const FareInfo& sfi,
                    const AddonFareCortegeVec::iterator& firstAddon,
                    const AddonFareCortegeVec::iterator& endOfAddons,
                    const AddonFareCortegeVec::iterator& roundTripMayNotBeHalvedAddonBound);

  std::vector<const FareInfo*> _specFares;

#endif

private:
  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  GatewayPair(const GatewayPair& rhs);
  GatewayPair& operator=(const GatewayPair& rhs);

}; // End class GatewayPair

namespace flattenizer
{
template <>
inline void
flatten(Flattenizable::Archive& archive, const std::vector<std::shared_ptr<GatewayPair>>& v)
{
  archive.append(v.size());
  for (const auto& elem : v)
  {
    archive.append(static_cast<size_t>(elem->objectType()));
    flatten(archive, *elem);
  }
}

template <>
inline void
unflatten(Flattenizable::Archive& archive, std::vector<std::shared_ptr<GatewayPair>>& v)
{
  size_t sz(0);
  archive.extract(sz);
  v.clear();

  while (sz--)
  {
    size_t type;
    archive.extract(type);

    GatewayPair* pair(GatewayPairFactory::create(static_cast<eGatewayPairType>(type)));
    unflatten(archive, *pair);
    v.push_back(std::shared_ptr<GatewayPair>(pair));
  }
}

template <>
inline void
calcmem(Flattenizable::Archive& archive, const std::vector<std::shared_ptr<GatewayPair>>& v)
{
  archive.addSize(sizeof(size_t));
  for (const auto& elem : v)
  {
    archive.addSize(sizeof(size_t));
    calcmem(archive, *elem);
  }
}
}

} // End namespace tse

