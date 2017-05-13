#pragma once

#include "Common/TseEnums.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

#include <vector>

namespace tse
{

class PaxTypeFare;
class FarePath;
class PricingUnit;
class PricingTrx;
class TravelSeg;
class DiagCollector;
class Predicate;

class CategoryGeoCommon
{
protected:
  CategoryGeoCommon();
  virtual ~CategoryGeoCommon() {};

  struct GeoBools
  {
    bool orig;
    bool dest;
    bool fltStop;
    bool via;
    bool matchAll;
    bool existTSI;
    GeoBools()
      : orig(true), dest(true), fltStop(true), via(true), matchAll(true), existTSI(false) {};
  };

  bool callGeo(const uint32_t geoTable,
               RuleUtil::TravelSegWrapperVector& appSegVec,
               GeoBools& geoBools,
               DiagCollector* diag = nullptr) const;

  bool getTvlSegsBtwAndGeo(const uint32_t itemNoBtw,
                           const uint32_t itemNoAnd,
                           std::vector<TravelSeg*>& filteredTvlSegs,
                           GeoBools& geoBools,
                           DiagCollector* diag = nullptr) const;

  bool getTvlSegsWithinGeo(const uint32_t itemNo,
                           std::vector<TravelSeg*>& filteredTvlSegs,
                           GeoBools& geoBools,
                           DiagCollector* diag = nullptr) const;

  bool getTvlSegsFromToViaGeo(const uint32_t itemNo,
                              std::vector<TravelSeg*>& filteredTvlSegs,
                              GeoBools& geoBools,
                              DiagCollector* diag = nullptr) const;

  bool getTvlSegs(const uint32_t itemNo,
                  const LocKey& locKey,
                  std::vector<TravelSeg*>& filteredTvlSegs,
                  GeoBools& geoBools,
                  const bool& checkOrig,
                  const bool& checkDest) const;

  // Predicate* _root;

  VendorCode _r2Vendor;
  const PaxTypeFare* _paxTypeFare;
  const FarePath* _farePath;
  const PricingUnit* _pricingUnit;
  PricingTrx* _trx;
  RuleConst::TSIScopeParamType _defaultScope;
  const std::vector<TravelSeg*>* _travelSegs;
};

} // namespace tse

