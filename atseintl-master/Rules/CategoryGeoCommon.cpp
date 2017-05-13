#include "Rules/CategoryGeoCommon.h"

#include "Common/LocUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/LocKey.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/CommonPredicates.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

namespace tse
{

CategoryGeoCommon::CategoryGeoCommon()
  : //_root(0),
    _r2Vendor(""),
    _paxTypeFare(nullptr),
    _farePath(nullptr),
    _pricingUnit(nullptr),
    _trx(nullptr),
    _defaultScope(RuleConst::TSI_SCOPE_PARAM_JOURNEY),
    _travelSegs(nullptr)
{
}

bool
CategoryGeoCommon::callGeo(const uint32_t geoTable,
                           RuleUtil::TravelSegWrapperVector& appSegVec,
                           GeoBools& geoBools,
                           DiagCollector* diag) const
{
  TSICode tsi;
  LocKey loc1;
  LocKey loc2;

  bool ret =
      RuleUtil::validateGeoRuleItem(geoTable,
                                    _r2Vendor,
                                    _defaultScope,
                                    false,
                                    false,
                                    false,
                                    *_trx,
                                    _farePath,
                                    nullptr,
                                    _pricingUnit,
                                    _paxTypeFare->fareMarket(),
                                    _trx->getRequest()->ticketingDT(),
                                    appSegVec,
                                    geoBools.orig,
                                    geoBools.dest,
                                    geoBools.fltStop,
                                    tsi,
                                    loc1,
                                    loc2,
                                    (diag != nullptr) ? diag->diagnosticType() : Diagnostic311,
                                    (geoBools.matchAll) ? RuleUtil::LOGIC_AND : RuleUtil::LOGIC_OR);

  if (UNLIKELY(diag && diag->isActive()))
  {
    DiagCollector& dc = *diag;
    dc << " GEO NO " << geoTable << " RTN: ";
    if (ret)
      dc << "TRUE";
    else
      dc << "FALSE";

    dc << std::endl << " TSI " << tsi;
    if (!loc1.loc().empty())
    {
      dc << " LOC1 " << loc1.locType() << " " << loc1.loc();
    }
    if (!loc2.loc().empty())
    {
      dc << " LOC2 " << loc2.locType() << " " << loc2.loc();
    }
    dc << std::endl;
  }

  if (tsi > 0)
    geoBools.existTSI = true;

  return ret;
}

bool
CategoryGeoCommon::getTvlSegsBtwAndGeo(const uint32_t itemNoBtw,
                                       const uint32_t itemNoAnd,
                                       std::vector<TravelSeg*>& filteredTvlSegs,
                                       GeoBools& geoBools,
                                       DiagCollector* diag) const
{
  return RuleUtil::getTvlSegBtwAndGeo(*_trx,
                                      itemNoBtw,
                                      itemNoAnd,
                                      _r2Vendor,
                                      filteredTvlSegs,
                                      nullptr,
                                      _defaultScope,
                                      _farePath,
                                      _pricingUnit,
                                      _paxTypeFare->fareMarket(),
                                      false,
                                      _trx->getRequest()->ticketingDT(),
                                      geoBools.fltStop,
                                      diag);
}

bool
CategoryGeoCommon::getTvlSegsWithinGeo(const uint32_t itemNo,
                                       std::vector<TravelSeg*>& filteredTvlSegs,
                                       GeoBools& geoBools,
                                       DiagCollector* diag) const
{
  RuleUtil::TravelSegWrapperVector appSegVec;

  if (!callGeo(itemNo, appSegVec, geoBools, diag))
    return false;

  // when geoBools.fltStop is false, callGeo may return segments that only
  // matches orig or only matches dest, we need to filter them out
  const bool requireBothOrigDestMatch = !geoBools.fltStop;

  RuleUtil::travelSegWrapperVec2TSVec(appSegVec, filteredTvlSegs, requireBothOrigDestMatch);
  return !filteredTvlSegs.empty();
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    CategoryGeoCommon::getTvlSegsFromToViaGeo()
//
// getTvlSegsFromToViaGeo()   get travel segments that is OrigFrom or
//                            DestTo a Loc defined by GeoTblItemNo,
//                            as well as travel segment pairs that are via
//                            a Loc defined by GeoTblItemNo.
//
//  @param const uint32_t itemNo&   - Geo Table Item No
//  @param std::vector<TravelSeg*>& - [OUT] result travel segments
//  @param GeoBools&                - Function call options
//  @param DiagCollector*           - (optional) for diagnostic output
//
//  @return bool -
//           false              failed to get any travel segments
//           true               got travel segment(s)
//
// </PRE>
//-------------------------------------------------------------------
bool
CategoryGeoCommon::getTvlSegsFromToViaGeo(const uint32_t itemNo,
                                          std::vector<TravelSeg*>& filteredTvlSegs,
                                          GeoBools& geoBools,
                                          DiagCollector* diag) const
{
  bool checkOrig = geoBools.orig;
  bool checkDest = geoBools.dest;
  TSICode tsi;
  LocKey locKey1;
  LocKey locKey2;

  // Get LocKeys
  if (UNLIKELY(!RuleUtil::getOrigDestLocFromGeoRuleItem(
          itemNo, _r2Vendor, *_trx, checkOrig, checkDest, tsi, locKey1, locKey2)))
  {
    if (UNLIKELY(diag && diag->isActive()))
    {
      *diag << " FAILED GETTING GEO RULE ITEM " << itemNo << std::endl;
    }
    return false;
  }

  if (UNLIKELY(diag && diag->isActive()))
  {
    DiagCollector& dc = *diag;
    dc << " GEO NO " << itemNo << " TSI " << tsi;
    if (locKey1.locType() != LOCTYPE_NONE)
    {
      dc << " LOC1 " << locKey1.locType() << " " << locKey1.loc();
    }
    if (locKey2.locType() != LOCTYPE_NONE)
    {
      dc << " LOC2 " << locKey2.locType() << " " << locKey2.loc();
    }
    dc << std::endl;
  }

  if (tsi)
  {
    RuleUtil::TravelSegWrapperVector applTravelSegment;
    RuleConst::TSIScopeParamType defaultScopeParam = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;

    if (UNLIKELY(!RuleUtil::scopeTSIGeo(tsi,
                               locKey1,
                               locKey2,
                               defaultScopeParam,
                               false, // allowJourneyScopeOverride
                               false, // allowPUScopeOverride,
                               true, // allowFCScopeOverride,
                               *_trx,
                               _farePath,
                               nullptr,
                               _pricingUnit,
                               _paxTypeFare->fareMarket(),
                               _trx->getRequest()->ticketingDT(),
                               applTravelSegment,
                               diag ? (diag->diagnosticType()) : DiagnosticNone,
                               _r2Vendor)))
    {
      // TSI error
      return false;
    }

    geoBools.existTSI = true;

    RuleUtil::travelSegWrapperVec2TSVec(applTravelSegment, filteredTvlSegs);

    return true;
  }

  getTvlSegs(itemNo, locKey1, filteredTvlSegs, geoBools, checkOrig, checkDest);

  if (locKey2.locType() != LOCTYPE_NONE)
  {
    getTvlSegs(itemNo, locKey2, filteredTvlSegs, geoBools, checkOrig, checkDest);
  }

  if (filteredTvlSegs.empty())
  {
    return false;
  }
  return true;
}

bool
CategoryGeoCommon::getTvlSegs(const uint32_t itemNo,
                              const LocKey& locKey,
                              std::vector<TravelSeg*>& filteredTvlSegs,
                              GeoBools& geoBools,
                              const bool& checkOrig,
                              const bool& checkDest) const
{
  // We only support TSI_SCOPE_FARE_COMPONENT or FareMarket now
  // TODO, to fully support other SCOPE
  const FareMarket* const fareMarket = _paxTypeFare->fareMarket();

  if (checkOrig || checkDest)
  {
    if (LIKELY(fareMarket))
    {
      if (checkOrig && LocUtil::isAirportInLoc(
                           *fareMarket->origin(), locKey.locType(), locKey.loc(), _r2Vendor) &&
          !LocUtil::isAirportInLoc(
              *fareMarket->destination(), locKey.locType(), locKey.loc(), _r2Vendor))
      {
        filteredTvlSegs.push_back(*fareMarket->travelSeg().begin());
      }
      else if (checkDest &&
               LocUtil::isAirportInLoc(
                   *fareMarket->destination(), locKey.locType(), locKey.loc(), _r2Vendor) &&
               !LocUtil::isAirportInLoc(
                   *fareMarket->origin(), locKey.locType(), locKey.loc(), _r2Vendor))
      {
        std::vector<TravelSeg*>::const_iterator tvlSegI = fareMarket->travelSeg().end();
        tvlSegI--;
        filteredTvlSegs.push_back(*tvlSegI);
      }
    }
    // else, @TODO
  }

  if (filteredTvlSegs.empty() && ((geoBools.via == true) || (geoBools.fltStop == true)))
  {
    if (LIKELY(fareMarket))
    {
      RuleUtil::TravelSegWrapperVector appSegVecHiddenVia;
      std::vector<TravelSeg*> filteredTvlSegsHiddenVia;
      std::vector<TravelSeg*> filteredTvlSegsVia;

      // get travel segments that matches the hidden stop with VIA
      if (geoBools.fltStop)
      {
        if (callGeo(itemNo, appSegVecHiddenVia, geoBools))
        {
          RuleUtil::travelSegWrapperVec2TSVec(appSegVecHiddenVia, filteredTvlSegsHiddenVia);
        }
      }

      if ((fareMarket->travelSeg().size() > 1) && (geoBools.via == true))
      {
        // Get travel segment pair that to and then from a via point
        for (std::vector<TravelSeg*>::const_iterator tvlSegI = fareMarket->travelSeg().begin();
             tvlSegI != fareMarket->travelSeg().end();
             tvlSegI++)
        {
          if (((tvlSegI + 1 != fareMarket->travelSeg().end()) &&
               LocUtil::isAirportInLoc(
                   *(*tvlSegI)->destination(), locKey.locType(), locKey.loc(), _r2Vendor)) ||
              ((tvlSegI != fareMarket->travelSeg().begin()) &&
               LocUtil::isAirportInLoc(
                   *(*tvlSegI)->origin(), locKey.locType(), locKey.loc(), _r2Vendor)))
          {
            filteredTvlSegsVia.push_back(*tvlSegI);
          }
        }
      } // fm travelseg size > 1

      filteredTvlSegs.insert(
          filteredTvlSegs.end(), filteredTvlSegsVia.begin(), filteredTvlSegsVia.end());

      filteredTvlSegs.insert(
          filteredTvlSegs.end(), filteredTvlSegsHiddenVia.begin(), filteredTvlSegsHiddenVia.end());
    } // fareMarket
    // else, @TODO
  }

  if (filteredTvlSegs.empty())
    return false;
  else
    return true;
}
}
