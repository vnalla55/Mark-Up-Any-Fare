#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"

#include <set>

namespace tse
{
class ShoppingTrx;
class Itin;
class AirSeg;
class FareMarket;
class TravelSeg;

typedef std::set<CarrierCode> CarriersSet;

class CodeShareValidatorSOL
{
public:
  CodeShareValidatorSOL(ShoppingTrx& trx,
                        const Itin* itin,
                        int32_t legIndex,
                        GeoTravelType geoTvlTypeForThruFM);

  bool shouldCreateLocalFareMarkets();
  bool shouldSkipLocalFareMarketPair(const std::vector<TravelSeg*>& fareMarketSegments1,
                                     const std::vector<TravelSeg*>& fareMarketSegments2) const;
  bool hasCodeSharedGoverningSegment(const FareMarket* fareMarket) const;
  bool isGovernigSegmentFound() const;
  bool isLocalForeignDomesticHasCodeShare(const std::vector<TravelSeg*>& segs) const;

private:
  void gatherGoverningCarriers();
  AirSeg* findItinGoverningSegment() const;
  bool isCodeSharedTwoSegmentMarket() const;
  bool isCodeSharedSegment(const AirSeg* segment) const;
  bool isCodeShareValidationEnabled(GeoTravelType geoTvlTypeForThruFM) const;
  bool isAcceptableFareMarket(const std::vector<TravelSeg*>& fareMarketSegments) const;

private:
  ShoppingTrx& _trx;
  const Itin* _itin;
  int32_t _legIndex;
  AirSeg* _governingSegment = nullptr;
  bool _codeShareValidationEnabled;
  CarriersSet _governingCarriers;
  GeoTravelType _geoTvlTypeForThruFM;
};
}

