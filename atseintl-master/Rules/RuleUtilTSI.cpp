//----------------------------------------------------------------------------
//  File:           RuleUtilTSI.cpp
//
//  Copyright Sabre 2004
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "Rules/RuleUtilTSI.h"

#include "Common/FallbackUtil.h"
#include "Common/RtwUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/AirlineAllianceContinentInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/TSIGateway.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(apo36040Cat6TSI5Check);
FALLBACK_DECL(reduceTemporaryVectors);
FIXEDFALLBACK_DECL(APO29538_StopoverMinStay);

log4cxx::LoggerPtr
RuleUtilTSI::_tsiLogger(log4cxx::Logger::getLogger("atseintl.Rules.RuleUtilTSI.TSI"));
log4cxx::LoggerPtr RuleUtilTSI::_dataErrLogger =
    log4cxx::Logger::getLogger("atseintl.AutoRulesErrors");

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::checkGeoData
//
// Description:
//
//  Validate location during TSI checkGeoData
//
// @param  trx                - Pricing transaction
// @param  tsiData            - TSIData structure filled with data for validation
// @param  seg                - validated Travel Segment
// @param  origMatch          - is geoData origin matching
// @param  destMatch          - is geo data destination matching
//
// @return true if match on origin or destination
//
// ----------------------------------------------------------------------------

bool
RuleUtilTSI::checkGeoData(const PricingTrx& trx,
                          const TSIData& tsiData,
                          const TravelSeg* seg,
                          bool& origMatch,
                          bool& destMatch)
{
  // by deafult - no match
  origMatch = false;
  destMatch = false;

  // if fail on GeoNotType, then fail geo checking
  if (UNLIKELY(!checkGeoNotType(tsiData)))
    return false;

  // get tsiInfo
  const TSIInfo& tsiInfo = tsiData.tsiInfo();

  // initalizae location matching to false, will be set in getLocationMatchings
  bool loc1OrigMatch = false;
  bool loc1DestMatch = false;
  bool loc2OrigMatch = false;
  bool loc2DestMatch = false;

  const RuleConst::TSIGeoOut geoOut = RuleConst::TSIGeoOut(tsiInfo.geoOut());
  const RuleConst::TSIApplicationType geoCheck = RuleConst::TSIApplicationType(tsiInfo.geoCheck());

  // check matching locations of TSIData to segment origin/destiantion
  getLocationMatchings(
      trx, tsiData, seg, loc1OrigMatch, loc1DestMatch, loc2OrigMatch, loc2DestMatch);

  // check if origin and destination match
  origMatch = loc1OrigMatch || loc2OrigMatch;
  destMatch = loc1DestMatch || loc2DestMatch;

  // if exclude, then invert matching
  if (RuleConst::TSI_GEO_OUT_EXCLUDE == geoOut)
  {
    origMatch = !origMatch;
    destMatch = !destMatch;
  }

  // if checking for origin, then ignore destination
  if (geoCheck == RuleConst::TSI_APP_CHECK_ORIG)
    destMatch = false;

  // if checking for destination then ignore origin
  if (geoCheck == RuleConst::TSI_APP_CHECK_DEST)
    origMatch = false;

  // return true if match on origin or destination
  return origMatch || destMatch;
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getLocationMatchings
//
// Description:
//
//  Validate location during TSI checkGeoData
//
// @param  trx                   - Pricing transaction
// @param  loc                   - validated tarvel segment location (orig/dest)
// @param  locType               - validated TSI location type
// @param  locCode               - validated TSI location
// @param  vendorCode            - validated vendor
// @param  geoTvlType            - Travel type
//
// @return true if the location is matching
//
// ----------------------------------------------------------------------------

bool
RuleUtilTSI::getLocationMatchings(const PricingTrx& trx,
                                  const Loc& loc,
                                  const LocTypeCode& locType,
                                  const LocCode& locCode,
                                  const VendorCode& vendorCode,
                                  GeoTravelType geoTvlType)
{
  // if loc type is empty, then return no match on location
  if (LOCTYPE_NONE == locType)
    return false;

  // if not looking for zone, don't use vendor
  VendorCode vendor = (LOCTYPE_ZONE == locType) ? vendorCode : EMPTY_VENDOR;
  return LocUtil::isInLoc(loc,
                          locType,
                          locCode,
                          vendor,
                          RESERVED,
                          LocUtil::OTHER,
                          geoTvlType,
                          EMPTY_STRING(),
                          trx.getRequest()->ticketingDT());
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getLocationMatchings
//
// Description:
//
//  Validate location during TSI checkGeoData
//
// @param  trx                - Pricing transaction
// @param  tsiData            - TSIData structure filled with data for validation
// @param  seg                - validated Travel Segment
// @param  loc1OrigMatch      - is loc1 macth to origin
// @param  loc1DestMatch      - is loc1 match to destination
// @param  loc2OrigMatch      - is loc2 match to origin
// @param  loc2DestMatch      - is loc2 match to destination
//
// @return nothing
//
// ----------------------------------------------------------------------------

void
RuleUtilTSI::getLocationMatchings(const PricingTrx& trx,
                                  const TSIData& tsiData,
                                  const TravelSeg* seg,
                                  bool& loc1OrigMatch,
                                  bool& loc1DestMatch,
                                  bool& loc2OrigMatch,
                                  bool& loc2DestMatch)
{
  // validated locations
  const LocTypeCode& locType1 = tsiData.locType1();
  const LocTypeCode& locType2 = tsiData.locType2();
  const LocCode& locCode1 = tsiData.locCode1();
  const LocCode& locCode2 = tsiData.locCode2();

  // chech travel type (From Itin if present, otherwise from TravelSeg)
  const Itin* itin = tsiData.itin();
  GeoTravelType geoTvlType = GeoTravelType::International;
  if (itin)
  {
    geoTvlType = itin->geoTravelType();
  }
  else if (LIKELY(seg))
  {
    geoTvlType = seg->geoTravelType();
  }

  if ((LOCTYPE_NONE == locType1) && (LOCTYPE_NONE == locType2) && (locCode1.empty()) &&
      (locCode2.empty()))
  {
    // No GeoData was provided, so all points are considered a match
    //
    loc1OrigMatch = loc1DestMatch = loc2OrigMatch = loc2DestMatch = true;
    return;
  }
  // try to match loc1 to origin and destination
  loc1OrigMatch = getLocationMatchings(
      trx, *seg->origin(), locType1, locCode1, tsiData.vendorCode(), geoTvlType);
  loc1DestMatch = getLocationMatchings(
      trx, *seg->destination(), locType1, locCode1, tsiData.vendorCode(), geoTvlType);

  // try to match loc2 to origin and destination
  loc2OrigMatch = getLocationMatchings(
      trx, *seg->origin(), locType2, locCode2, tsiData.vendorCode(), geoTvlType);
  loc2DestMatch = getLocationMatchings(
      trx, *seg->destination(), locType2, locCode2, tsiData.vendorCode(), geoTvlType);
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::checkGeoNotType
//
// Description:
//
//  Validate tag GeoNotType during TSI checkGeoData
//
// @param  tsiData            - TSIData structure filled with data for validation
//
// @return true if match of NotGeoType
//
// ----------------------------------------------------------------------------

bool
RuleUtilTSI::checkGeoNotType(const TSIData& tsiData)
{
  // get tsiInfo
  const TSIInfo& tsiInfo = tsiData.tsiInfo();

  // get location type
  const LocTypeCode& locType1 = tsiData.locType1();
  const LocTypeCode& locType2 = tsiData.locType2();

  const RuleConst::TSIGeoNotType notType = RuleConst::TSIGeoNotType(tsiInfo.geoNotType());

  // by default return set to true
  bool ret = true;
  switch (notType)
  {
  case RuleConst::TSI_GEO_NOT_TYPE_CITY:
    ret = (LOCTYPE_CITY != locType1) && (LOCTYPE_CITY != locType2);
    break;

  case RuleConst::TSI_GEO_NOT_TYPE_BOTH:
    ret = (LOCTYPE_CITY != locType1) && (LOCTYPE_CITY != locType2) &&
          (LOCTYPE_AIRPORT != locType1) && (LOCTYPE_AIRPORT != locType2);
    break;

  case RuleConst::TSI_GEO_NOT_TYPE_THREE:
    ret = (LOCTYPE_CITY != locType1) && (LOCTYPE_CITY != locType2) &&
          (LOCTYPE_AIRPORT != locType1) && (LOCTYPE_AIRPORT != locType2) &&
          (LOCTYPE_ZONE != locType1) && (LOCTYPE_ZONE != locType2);
    break;

  case RuleConst::TSI_GEO_NOT_TYPE_BLANK:
    ret = true;
    break;
  }
  return ret;
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getLoopItemToSet
//
// Description:
//
//  Set correct flags during TSI loopMatch validation
//
// @param tsiInfo
// @param addPrevTravSeg    - true if should add to previous segment
// @param addCurrTravSeg    - true if should add to current segment
// @param addNextTravSeg    - true if should add to next segment
//
// @return nothing
//
// ----------------------------------------------------------------------------

void
RuleUtilTSI::getLoopItemToSet(const TSIInfo& tsiInfo,
                              bool& addPrevTravSeg,
                              bool& addCurrTravSeg,
                              bool& addNextTravSeg)
{
  // get loopToSet
  const int16_t loopItemToSet = tsiInfo.loopToSet();

  // true for previous or current previous
  addPrevTravSeg = (RuleConst::TSI_LOOP_SET_PREVIOUS == loopItemToSet) ||
                   (RuleConst::TSI_LOOP_SET_CUR_PREV == loopItemToSet);

  // true for current, current previous, current next
  addCurrTravSeg = (RuleConst::TSI_LOOP_SET_CURRENT == loopItemToSet) ||
                   (RuleConst::TSI_LOOP_SET_CUR_NEXT == loopItemToSet) ||
                   (RuleConst::TSI_LOOP_SET_CUR_PREV == loopItemToSet);

  // true for next, current next
  addNextTravSeg = (RuleConst::TSI_LOOP_SET_NEXT == loopItemToSet) ||
                   (RuleConst::TSI_LOOP_SET_CUR_NEXT == loopItemToSet);
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::checkLoopMatch
//
// Description:
//
//  Validate tag GeoNotType during TSI checkGeoData
//
// @param tsiData             - TSIData structure
// @param prevMatch           - Previous validation result
// @param currMatch           - current validation result
// @param addPrevTravSeg      - return true if should add previous travel segment
// @param addCurrTravSeg      - return true if should add current travel segment
// @param addNextTravSeg      - return true if should add next travel segment
// @param continueLooping     - return true if should continue with loop
// @param locSpecified        - is location specified
//
// ----------------------------------------------------------------------------

void
RuleUtilTSI::checkLoopMatch(const TSIData& tsiData,
                            const RuleConst::TSIMatch& prevMatch,
                            const RuleConst::TSIMatch& currMatch,
                            bool& addPrevTravSeg,
                            bool& addCurrTravSeg,
                            bool& addNextTravSeg,
                            bool& continueLooping,
                            bool locSpecified)
{
  // set default return values
  addPrevTravSeg = addCurrTravSeg = addNextTravSeg = false;

  // get loopMatch
  const RuleConst::TSILoopMatch loopMatch = RuleConst::TSILoopMatch(tsiData.tsiInfo().loopMatch());

  switch (loopMatch)
  {
  case RuleConst::TSI_MATCH_ALL:
    checkLoopMatchAll(
        tsiData, currMatch, addPrevTravSeg, addCurrTravSeg, addNextTravSeg, continueLooping);
    break;
  case RuleConst::TSI_MATCH_FIRST:
    checkLoopMatchFirst(
        tsiData, currMatch, addPrevTravSeg, addCurrTravSeg, addNextTravSeg, continueLooping);
    break;
  case RuleConst::TSI_MATCH_ONCE:
    checkLoopMatchOnce(tsiData,
                       currMatch,
                       addPrevTravSeg,
                       addCurrTravSeg,
                       addNextTravSeg,
                       continueLooping,
                       locSpecified);
    break;
  case RuleConst::TSI_MATCH_LAST:
    checkLoopMatchLast(tsiData);
    break;
  case RuleConst::TSI_MATCH_FIRST_ALL:
    checkLoopMatchFirstAll(
        tsiData, currMatch, addPrevTravSeg, addCurrTravSeg, addNextTravSeg, continueLooping);
    break;
  case RuleConst::TSI_MATCH_SOFT_MATCH:
    checkLoopMatchSoft(
        tsiData, currMatch, addPrevTravSeg, addCurrTravSeg, addNextTravSeg, continueLooping);
    break;
  case RuleConst::TSI_MATCH_SECOND_FIRST:
    checkLoopMatchSecondFirst(tsiData,
                              prevMatch,
                              currMatch,
                              addPrevTravSeg,
                              addCurrTravSeg,
                              addNextTravSeg,
                              continueLooping);
    break;
  }
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::checkLoopMatchAll
//
// Description:
//
//  Validate loopMatch TSI_MATCH_ALL
//
// @param tsiData             - TSIData structure
// @param currMatch           - current validation result
// @param addPrevTravSeg      - return true if should add previous travel segment
// @param addCurrTravSeg      - return true if should add current travel segment
// @param addNextTravSeg      - return true if should add next travel segment
// @param continueLooping     - return true if should continue with loop
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::checkLoopMatchAll(const TSIData& tsiData,
                               const RuleConst::TSIMatch& currMatch,
                               bool& addPrevTravSeg,
                               bool& addCurrTravSeg,
                               bool& addNextTravSeg,
                               bool& continueLooping)
{
  // if current match, then get correct segments
  if (RuleConst::TSI_MATCH == currMatch)
  {
    getLoopItemToSet(tsiData.tsiInfo(), addPrevTravSeg, addCurrTravSeg, addNextTravSeg);
  }
  // always continue with loop
  continueLooping = true;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::checkLoopMatchFirst
//
// Description:
//
//  Validate loopMatch TSI_MATCH_FIRST
//
// @param tsiData             - TSIData structure
// @param currMatch           - current validation result
// @param addPrevTravSeg      - return true if should add previous travel segment
// @param addCurrTravSeg      - return true if should add current travel segment
// @param addNextTravSeg      - return true if should add next travel segment
// @param continueLooping     - return true if should continue with loop
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::checkLoopMatchFirst(const TSIData& tsiData,
                                 const RuleConst::TSIMatch& currMatch,
                                 bool& addPrevTravSeg,
                                 bool& addCurrTravSeg,
                                 bool& addNextTravSeg,
                                 bool& continueLooping)
{
  // if current macth, get correct segments
  if (RuleConst::TSI_MATCH == currMatch)
  {
    // matc is done, don't need to continue
    getLoopItemToSet(tsiData.tsiInfo(), addPrevTravSeg, addCurrTravSeg, addNextTravSeg);
    continueLooping = false;
  }
  else
  {
    // not valid, continue with next
    continueLooping = true;
  }
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::checkLoopMatchOnce
//
// Description:
//
//  Validate loopMatch TSI_MATCH_ONCE
//
// @param tsiData             - TSIData structure
// @param currMatch           - current validation result
// @param addPrevTravSeg      - return true if should add previous travel segment
// @param addCurrTravSeg      - return true if should add current travel segment
// @param addNextTravSeg      - return true if should add next travel segment
// @param continueLooping     - return true if should continue with loop
// @param locSpecified        - is location specified
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::checkLoopMatchOnce(const TSIData& tsiData,
                                const RuleConst::TSIMatch& currMatch,
                                bool& addPrevTravSeg,
                                bool& addCurrTravSeg,
                                bool& addNextTravSeg,
                                bool& continueLooping,
                                bool locSpecified)
{
  // if current match, get correct segments
  if (RuleConst::TSI_MATCH == currMatch)
  {
    getLoopItemToSet(tsiData.tsiInfo(), addPrevTravSeg, addCurrTravSeg, addNextTravSeg);
    continueLooping = false;
  }
  else if (tsiData.tsiInfo().tsi() == 52 && locSpecified)
  {
    // continue only when tsi is 52 and location is specified
    continueLooping = true;
  }
  else
    // try only once
    continueLooping = false;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::checkLoopMatchLast
//
// Description:
//
//  Validate loopMatch TSI_MATCH_LAST
//
// @param tsiData             - TSIData structure
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::checkLoopMatchLast(const TSIData& tsiData)
{
  // At this time, there are no TSIs using LAST.
  // Not sure if this will ever be needed since TSIs interested
  //  in the "last" item loop backward from the end.
  std::ostringstream msg;
  msg << "Rules.TSI: Bad TSI record. Loop Match = LAST not supported."
      << " TSI: " << tsiData.tsiInfo().tsi();
  LOG4CXX_ERROR(_tsiLogger, msg.str());
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::checkLoopMatchFirstAll
//
// Description:
//
//  Validate loopMatch TSI_MATCH_FIRST_ALL
//
// @param tsiData             - TSIData structure
// @param currMatch           - current validation result
// @param addPrevTravSeg      - return true if should add previous travel segment
// @param addCurrTravSeg      - return true if should add current travel segment
// @param addNextTravSeg      - return true if should add next travel segment
// @param continueLooping     - return true if should continue with loop
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::checkLoopMatchFirstAll(const TSIData& tsiData,
                                    const RuleConst::TSIMatch& currMatch,
                                    bool& addPrevTravSeg,
                                    bool& addCurrTravSeg,
                                    bool& addNextTravSeg,
                                    bool& continueLooping)
{
  // get tsi Scope
  const RuleConst::TSIScopeType& scope = tsiData.scope();

  // for sub journey always continue
  if (RuleConst::TSI_SCOPE_SUB_JOURNEY == scope)
  {
    // get correct travel segments only when validation pass
    if (RuleConst::TSI_MATCH == currMatch)
    {
      getLoopItemToSet(tsiData.tsiInfo(), addPrevTravSeg, addCurrTravSeg, addNextTravSeg);
    }
    continueLooping = true;
  }
  // for fare component continue only if not match
  else if (LIKELY(RuleConst::TSI_SCOPE_FARE_COMPONENT == scope))
  {
    // get correct travel segments only when validation pass
    if (RuleConst::TSI_MATCH == currMatch)
    {
      getLoopItemToSet(tsiData.tsiInfo(), addPrevTravSeg, addCurrTravSeg, addNextTravSeg);
      continueLooping = false;
    }
    else
    {
      continueLooping = true;
    }
  }
  // for other scope do nothing?
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::checkLoopMatchSoft
//
// Description:
//
//  Validate loopMatch TSI_MATCH_SOFT_MATCH
//
// @param tsiData             - TSIData structure
// @param currMatch           - current validation result
// @param addPrevTravSeg      - return true if should add previous travel segment
// @param addCurrTravSeg      - return true if should add current travel segment
// @param addNextTravSeg      - return true if should add next travel segment
// @param continueLooping     - return true if should continue with loop
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::checkLoopMatchSoft(const TSIData& tsiData,
                                const RuleConst::TSIMatch& currMatch,
                                bool& addPrevTravSeg,
                                bool& addCurrTravSeg,
                                bool& addNextTravSeg,
                                bool& continueLooping)
{
  // get segments when match
  if (RuleConst::TSI_MATCH == currMatch)
  {
    getLoopItemToSet(tsiData.tsiInfo(), addPrevTravSeg, addCurrTravSeg, addNextTravSeg);
    continueLooping = false;
  }
  // if soft match then don't continue
  else if (RuleConst::TSI_SOFT_MATCH == currMatch)
  {
    continueLooping = false;
  }
  else
  {
    continueLooping = true;
  }
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::checkLoopMatchSecondFirst
//
// Description:
//
//  Validate loopMatch TSI_MATCH_SECOND_FIRST
//
// @param tsiData             - TSIData structure
// @param prevMatch           - Previous validation result
// @param currMatch           - current validation result
// @param addPrevTravSeg      - return true if should add previous travel segment
// @param addCurrTravSeg      - return true if should add current travel segment
// @param addNextTravSeg      - return true if should add next travel segment
// @param continueLooping     - return true if should continue with loop
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::checkLoopMatchSecondFirst(const TSIData& tsiData,
                                       const RuleConst::TSIMatch& prevMatch,
                                       const RuleConst::TSIMatch& currMatch,
                                       bool& addPrevTravSeg,
                                       bool& addCurrTravSeg,
                                       bool& addNextTravSeg,
                                       bool& continueLooping)
{
  // get correct segments only when current match, and previous (soft) match
  if (((RuleConst::TSI_MATCH == prevMatch) || (RuleConst::TSI_SOFT_MATCH == prevMatch)) &&
      (RuleConst::TSI_MATCH == currMatch))
  {
    getLoopItemToSet(tsiData.tsiInfo(), addPrevTravSeg, addCurrTravSeg, addNextTravSeg);
    continueLooping = false;
  }
  else
  {
    continueLooping = true;
  }
}

RuleUtilTSI::TSIMatchCriteria::TSIMatchCriteria(const PricingTrx& trx,
                                                const TSIData& tsiData,
                                                const TSITravelSegMarkup& tsm,
                                                std::string& noMatchReason)
  : _trx(trx), _tsiData(tsiData), _tsm(tsm), _noMatchReason(noMatchReason)
{
  if (UNLIKELY(_trx.getOptions()->isRtw()))
  {
    const std::vector<AirlineAllianceCarrierInfo*>& allianceVec =
        _trx.dataHandle().getAirlineAllianceCarrier(
            _trx.itin().front()->fareMarket().front()->governingCarrier());

    if (!allianceVec.empty())
    {
      const AirlineAllianceCarrierInfo* alliance = allianceVec.front();
      if (alliance)
      {
        if (!fallback::reduceTemporaryVectors(&trx))
        {
          _trx.dataHandle().forEachAirlineAllianceContinent(
              alliance->genericAllianceCode(),
              [&](AirlineAllianceContinentInfo* continentInfo)
              {
                _rtwContinents.push_back(
                    std::make_pair(continentInfo->locType(), continentInfo->locCode()));
              });
        }
        else
        {
          const std::vector<AirlineAllianceContinentInfo*>& continents =
              _trx.dataHandle().getAirlineAllianceContinent(alliance->genericAllianceCode(), true);

          for (AirlineAllianceContinentInfo* continentInfo : continents)
          {
            if (LIKELY(continentInfo))
            {
              _rtwContinents.push_back(
                  std::make_pair(continentInfo->locType(), continentInfo->locCode()));
            }
          }
        }
      }
    }
  }
}

bool
RuleUtilTSI::TSIMatchCriteria::checkStopOver()
{
  // fail if this is the last segment
  if (_tsm.isLastTravelSeg())
  {
    _noMatchReason = RuleConst::NO_MATCH_LAST_SEGMENT;
    return false;
  }
  // pass is stopover or turn around point
  if (_tsm.isStopover() || _tsm.destIsTurnAroundPoint())
    return true;

  // if not any from above, then fail
  _noMatchReason = RuleConst::MATCH_STOP_OVER_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkInbound()
{
  // pass if directionality is inbound
  if (RuleConst::INBOUND == _tsm.direction())
    return true;

  // fail in all other cases
  _noMatchReason = RuleConst::MATCH_INBOUND_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkOutbound()
{
  // pass if directionalit is outbound
  if (RuleConst::OUTBOUND == _tsm.direction())
    return true;

  // fail in all other cases
  _noMatchReason = RuleConst::MATCH_OUTBOUND_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkFurthest()
{
  // pass if flag furthest is set
  if (_tsm.isFurthest())
    return true;

  // otherwise, fail
  _noMatchReason = RuleConst::MATCH_FURTHEST_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkDomestic()
{
  const Loc* orig = _tsm.travelSeg()->origin();
  const Loc* dest = _tsm.travelSeg()->destination();

  // The definition of domestic in LocUtil is different from
  //  the definition of domestic for TSI. TSI defines domestic
  //  as within any one ATPCO country code. LocUtil implements
  //  domestic as travel within the US or within Canada.
  if (orig->nation() == dest->nation())
    return true;

  // if not, then fail
  _noMatchReason = RuleConst::MATCH_DOMESTIC_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkOneCountry()
{
  const Loc* orig = _tsm.travelSeg()->origin();
  const Loc* dest = _tsm.travelSeg()->destination();

  // check if origgin and destiantion are same
  if (orig->nation() == dest->nation())
    return true;

  // if not then fail
  _noMatchReason = RuleConst::MATCH_ONE_COUNTRY_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkInternational()
{
  // pass if international
  if (_tsm.isInternational())
    return true;

  // otherwise fail
  _noMatchReason = RuleConst::MATCH_INTERNATIONAL_DESC;
  return false;
}

void
RuleUtilTSI::TSIMatchCriteria::getCheckOrigDest(bool& origCheck, bool& destCheck)
{
  RuleConst::TSIApplicationType saveType = RuleConst::TSIApplicationType(_tsiData.tsiInfo().type());
  // set origin for CHECK_ORIG or CHECK_ORIG_DEST
  origCheck = (RuleConst::TSI_APP_CHECK_ORIG == saveType) ||
              (RuleConst::TSI_APP_CHECK_ORIG_DEST == saveType);
  // set destination for CHECK_DEST or CHECK_ORIG_DEST
  destCheck = (RuleConst::TSI_APP_CHECK_DEST == saveType) ||
              (RuleConst::TSI_APP_CHECK_ORIG_DEST == saveType);
}

bool
RuleUtilTSI::TSIMatchCriteria::checkGateway()
{
  // set the flags for gateway validation
  bool origCheck, destCheck;
  getCheckOrigDest(origCheck, destCheck);

  // pass if origin match
  if (origCheck && (_tsm.departsOrigGateway() || _tsm.departsDestGateway()))
    return true;

  // pass if destination match
  if (destCheck && (_tsm.arrivesOrigGateway() || _tsm.arrivesDestGateway()))
    return true;

  // otherwise fail
  _noMatchReason = RuleConst::MATCH_GATEWAY_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkOrigGateway()
{
  // set the flags for gateway validation
  bool origCheck, destCheck;
  getCheckOrigDest(origCheck, destCheck);

  // pass if origin match
  if (origCheck && _tsm.departsOrigGateway())
    return true;

  // pass if destination match
  if (destCheck && _tsm.arrivesOrigGateway())
    return true;

  // otherwise fail
  _noMatchReason = RuleConst::MATCH_GATEWAY_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkDestGateway()
{
  // set the flags for gateway validation
  bool origCheck, destCheck;
  getCheckOrigDest(origCheck, destCheck);

  // pass if origin match
  if (origCheck && _tsm.departsDestGateway())
    return true;

  // pass if destiantion match
  if (destCheck && _tsm.arrivesDestGateway())
    return true;

  // otherwise fail
  _noMatchReason = RuleConst::MATCH_GATEWAY_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkTransAtlantic()
{
  const Loc* orig = _tsm.travelSeg()->origin();
  const Loc* dest = _tsm.travelSeg()->destination();

  // pass if transatlantic
  if (LocUtil::isTransAtlantic(*orig, *dest, _tsm.globalDirection()))
    return true;

  // otherwis fail
  _noMatchReason = RuleConst::MATCH_TRANS_ATLANTIC_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkTransPacific()
{
  const Loc* orig = _tsm.travelSeg()->origin();
  const Loc* dest = _tsm.travelSeg()->destination();

  // pass if transpacific
  if (LocUtil::isTransPacific(*orig, *dest, _tsm.globalDirection()))
    return true;

  // otherwise fail
  _noMatchReason = RuleConst::MATCH_TRANS_PACIFIC_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkTransOceanic()
{
  const Loc* orig = _tsm.travelSeg()->origin();
  const Loc* dest = _tsm.travelSeg()->destination();

  // pass if transoceanic
  if (LocUtil::isTransOceanic(*orig, *dest, _tsm.globalDirection()))
    return true;

  _noMatchReason = RuleConst::MATCH_TRANS_OCEANIC_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkIntercontinental()
{
  const Loc* orig = _tsm.travelSeg()->origin();
  const Loc* dest = _tsm.travelSeg()->destination();

  // pass if intercontinental
  if (LocUtil::isInterContinental(*orig, *dest, _tsiData.vendorCode()))
    return true;

  _noMatchReason = RuleConst::MATCH_INTERCONTINENTAL_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkIntercontinentalRtw()
{
  if (LocUtil::locsFromDifferentRegion(*_tsm.travelSeg()->origin(),
                                       *_tsm.travelSeg()->destination(),
                                       Vendor::SABRE,
                                       _rtwContinents,
                                       MANUAL))
    return true;

  _noMatchReason = RuleConst::MATCH_INTERCONTINENTAL_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkIntercontinentalFD()
{
  if (_tsm.globalDirection() == GlobalDirection::RW)
    return true;

  return checkIntercontinental();
}

bool
RuleUtilTSI::TSIMatchCriteria::checkIsOverWater()
{
  // pass if is over water
  if (_tsm.isOverWater())
    return true;

  // otherwise fail
  _noMatchReason = RuleConst::MATCH_OVER_WATER_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::checkIsIntlDomTransfer()
{
  // pass if isIntlDomTransfer flag is set
  if (_tsm.isIntlDomTransfer())
    return true;

  // otherwise fail
  _noMatchReason = RuleConst::MATCH_INTL_DOM_TRANSFER_DESC;
  return false;
}

bool
RuleUtilTSI::TSIMatchCriteria::process(TSIInfo::TSIMatchCriteria matchCriteria)
{
  // switch match criteria, get correct match criteria validation function
  switch (matchCriteria)
  {
  case TSIInfo::STOP_OVER:
    return checkStopOver();

  case TSIInfo::INBOUND:
    return checkInbound();

  case TSIInfo::OUTBOUND:
    return checkOutbound();

  case TSIInfo::FURTHEST:
    return checkFurthest();

  case TSIInfo::DOMESTIC:
    return checkDomestic();

  case TSIInfo::ONE_COUNTRY:
    return checkOneCountry();

  case TSIInfo::INTERNATIONAL:
    return checkInternational();

  case TSIInfo::GATEWAY:
    return checkGateway();

  case TSIInfo::ORIG_GATEWAY:
    return checkOrigGateway();

  case TSIInfo::DEST_GATEWAY:
    return checkDestGateway();

  case TSIInfo::TRANS_ATLANTIC:
    return checkTransAtlantic();

  case TSIInfo::TRANS_PACIFIC:
    return checkTransPacific();

  case TSIInfo::TRANS_OCEANIC:
    return checkTransOceanic();

  case TSIInfo::INTERCONTINENTAL:
    if (UNLIKELY(_trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX))
      return checkIntercontinentalFD();

    return _rtwContinents.empty() ? checkIntercontinental() : checkIntercontinentalRtw();

  case TSIInfo::OVER_WATER:
    return checkIsOverWater();

  case TSIInfo::INTL_DOM_TRANSFER:
    return checkIsIntlDomTransfer();

  default:
  {
    std::ostringstream msg;
    msg << "Rules.TSI: Bad TSI record. "
        << "Unrecognized Match Criteria."
        << " TSI: " << _tsiData.tsiInfo().tsi();
    LOG4CXX_ERROR(_tsiLogger, msg.str());
    _noMatchReason = "-ERROR-";
  }
  }
  return false;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::checkMatchCriteria
//
// Description:
//
// function validate TSI match criteria
//
// @param  tsiData                - The TSIData structure
// @param  tsm                    - TSITravelSegMarkup data structure
// @param  mc                     - TSIMatchCriteria vector
// @param  noMatchReason          - fail code description
//
// @return true if pass match criteria
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::checkMatchCriteria(const TSIData& tsiData,
                                const TSITravelSegMarkup& tsm,
                                const std::vector<TSIInfo::TSIMatchCriteria>& mc,
                                std::string& noMatchReason,
                                const PricingTrx& trx)
{
  if (!mc.empty())
  {
    TSIMatchCriteria tsiMc(trx, tsiData, tsm, noMatchReason);

    for (TSIInfo::TSIMatchCriteria it : mc)
    {
      if (!tsiMc.process(it))
        return false;
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::checkTSIMatchTravelSeg
//
// Description:
//
// function match travel segment data to the TSI
//
// @param  trx                - Pricing transaction
// @param  tsiData            - TSIData structure
// @param  tsm                - Travel segment markup
// @param  mc                 - vector of match criteria for validation
// @param  origMatch          - will be set to true if match to origin
// @param  destMatch          - will be set to true if match to destination
// @param  noMatchReason      - will be set with fail reason
//
// @return MATCH if pass GeoLocation and Match criteria, SOFT_MATCH if pass only match criteria
// or FAIL when fail match criteria
//
// ----------------------------------------------------------------------------
RuleConst::TSIMatch
RuleUtilTSI::checkTSIMatchTravelSeg(const PricingTrx& trx,
                                    const TSIData& tsiData,
                                    const TSITravelSegMarkup& tsm,
                                    const std::vector<TSIInfo::TSIMatchCriteria>& mc,
                                    bool& origMatch,
                                    bool& destMatch,
                                    std::string& noMatchReason)
{
  bool geoDataMatch = checkGeoData(trx, tsiData, tsm.travelSeg(), origMatch, destMatch);
  bool matchCriteriaMatch = checkMatchCriteria(tsiData, tsm, mc, noMatchReason, trx);

  if (matchCriteriaMatch)
  {
    // if match both
    if (geoDataMatch)
      return RuleConst::TSI_MATCH;

    // if fail geolocation
    noMatchReason = "LOCALE ";
    return RuleConst::TSI_SOFT_MATCH;
  }
  // if heare than fail match criteria

  // check if fail geo location also
  if (!geoDataMatch)
    noMatchReason += " LOCALE";

  return RuleConst::TSI_NOT_MATCH;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getGeoLocaleFromItinTurnAround
//
// Description:
//
// function get the turnaround locale information from the Itin
//
// @param  tsiData            - TSIData structure
// @param  loc                - Travel segment markup
//
// @return true if turnaround loaction is found
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::getGeoLocaleFromItinTurnAround(const TSIData& tsiData, const Loc*& loc)
{
  // get scope
  const RuleConst::TSIScopeType& scope = tsiData.scope();

  // for journey and fare component, check travel segments from Itin to find turnaround
  if ((RuleConst::TSI_SCOPE_JOURNEY == scope) || (RuleConst::TSI_SCOPE_FARE_COMPONENT == scope))
  {
    const Itin* itin = tsiData.itin();
    if (itin)
    {
      std::vector<TravelSeg*>::const_iterator iter = itin->travelSeg().begin();

      // loop thrue all segments
      for (; iter != itin->travelSeg().end(); ++iter)
        if ((*iter)->isFurthestPoint(*itin))
        {
          // if found match - we are done
          loc = (*iter)->origin();
          return true;
        }
    }
  }
  // get the pointer to the pricing unit
  const PricingUnit* pu = tsiData.pricingUnit();
  if (pu)
  {
    const TravelSeg* turnAround = pu->turnAroundPoint();
    // if we have pointer to the turnarounfd tarvel segment from pricing unit, then we're done
    if (turnAround)
    {
      loc = turnAround->origin();
      return true;
    }
  }
  return false;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getGeoLocaleFromItin
//
// Description:
//
// function get the locale information from the Itin
//
// @param  tsiData            - TSIData structure
// @param  loc                - Travel segment markup
//
// @return true if location is found
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::getGeoLocaleFromItin(const TSIData& tsiData, const Loc*& loc)
{
  // get the scope
  const RuleConst::TSIScopeType& scope = tsiData.scope();

  // get the data pointers
  const TSIInfo& tsiInfo = tsiData.tsiInfo();
  const Itin* itin = tsiData.itin();
  const PricingUnit* pu = tsiData.pricingUnit();
  const FareMarket* fm = tsiData.fareMarket();

  const RuleConst::TSIGeoItinPart itinPart = RuleConst::TSIGeoItinPart(tsiInfo.geoItinPart());

  // set correct vector of travel segments
  const std::vector<TravelSeg*>* ts =
      (RuleConst::TSI_SCOPE_JOURNEY == scope)
          ? &itin->travelSeg()
          : ((RuleConst::TSI_SCOPE_SUB_JOURNEY == scope)
                 ? &pu->travelSeg()
                 : ((RuleConst::TSI_SCOPE_FARE_COMPONENT == scope) ? &fm->travelSeg() : nullptr));

  // if not get the correct pointer of travel segments, ten this is out of scope
  if (!ts)
    return false;

  // get location for the correct geoItinPart
  switch (itinPart)
  {
  case RuleConst::TSI_GEO_ITIN_PART_ORIG:
    loc = ts->front()->origin();
    return true;

  case RuleConst::TSI_GEO_ITIN_PART_DEST:
    loc = ts->back()->destination();
    return true;

  case RuleConst::TSI_GEO_ITIN_PART_TURNAROUND:
    // Find the turn around point for the Journey. This is defined as
    // the furthest geographical point from the journey origin.
    return getGeoLocaleFromItinTurnAround(tsiData, loc);

  default:
  {
    std::ostringstream msg;
    msg << "Rules.TSI: Bad TSI record. Unrecognized GeoItinPart."
        << " TSI: " << tsiInfo.tsi();
    LOG4CXX_ERROR(_tsiLogger, msg.str());
    return false;
  }
  }
  return (loc != nullptr);
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getGeoDataLogErrMsg
//
// Description:
//
// function write error message to the logger
//
// @param  trx                    - reference to Pricing transaction
// @param  tsiData                - The TSIData structure
// @param  msg                    - message to be logged
//
// @return always return false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::getGeoDataLogErrMsg(const PricingTrx& trx, TSIData& tsiData, const char* msg)
{
  // just generate error message
  logErrMsg(trx, tsiData.tsiInfo(), msg);
  return false;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::logErrMsg
//
// Description:
//
// function write error message to the logger
//
// @param  trx                    - reference to Pricing transaction
// @param  tsiInfo                - The TSIInfo structure
// @param  msg                    - message to be logged
//
// @return always return false
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::logErrMsg(const PricingTrx& trx, const TSIInfo& tsiInfo, const char* msg)
{
  logErrMsg(trx, tsiInfo.tsi(), msg);
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::logErrMsg
//
// Description:
//
// function write error message to the logger
//
// @param  trx                    - reference to Pricing transaction
// @param  tsi                    - TSICode number
// @param  msg                    - message to be logged
//
// @return always return false
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::logErrMsg(const PricingTrx& trx, TSICode tsi, const char* msg)
{
  // just generate error message
  std::ostringstream errorMsg;
  errorMsg << "Rules.TSI: " << msg << " TSI: " << tsi;
  // if PricingOptions exists, add PNR and line entry
  if (trx.getOptions())
  {
    errorMsg << " PNR: " << trx.getOptions()->pnr()
             << " LINE ENTRY: " << trx.getOptions()->lineEntry();
  }
  LOG4CXX_ERROR(_tsiLogger, errorMsg.str());
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::logDbgMsg
//
// Description:
//
// function write debug message to the logger
//
// @param  trx                    - reference to Pricing transaction
// @param  tsiInfo                - The TSIInfo structure
// @param  msg                    - message to be logged
//
// @return always return false
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::logDbgMsg(const PricingTrx& trx, const TSIInfo& tsiInfo, const char* msg)
{
  logDbgMsg(trx, tsiInfo.tsi(), msg);
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::logDbgMsg
//
// Description:
//
// function write debug message to the logger
//
// @param  trx                    - reference to Pricing transaction
// @param  tsi                    - TSICode number
// @param  msg                    - message to be logged
//
// @return always return false
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::logDbgMsg(const PricingTrx& trx, TSICode tsi, const char* msg)
{
  // just generate error message
  std::ostringstream errorMsg;
  errorMsg << "Rules.TSI: " << msg << " TSI: " << tsi;
  // if PricingOptions exists, add PNR and line entry
  if (LIKELY(trx.getOptions()))
  {
    errorMsg << " PNR: " << trx.getOptions()->pnr()
             << " LINE ENTRY: " << trx.getOptions()->lineEntry();
  }
  LOG4CXX_DEBUG(_tsiLogger, errorMsg.str());
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getGeoDataGetLoc
//
// Description:
//
// function get the correct location from loc structure
//
// @param  loc                    - reference to Pricing transaction
// @param  locType                - location code type to be returned
//
// @return location code for loccType
//
// ----------------------------------------------------------------------------
const LocCode
RuleUtilTSI::getGeoDataGetLoc(const Loc* loc, const LocTypeCode& locType)
{
  // if non loc is passed, the do nothing
  if (!loc)
    return "";

  // switch loc type, return correct fiedl from loc
  switch (locType)
  {
  case LOCTYPE_AREA:
    return loc->area();
  case LOCTYPE_SUBAREA:
    return loc->subarea();
  // City and airport is the same for TSI validation
  case LOCTYPE_CITY:
  case LOCTYPE_AIRPORT:
    return loc->loc();
  case LOCTYPE_NATION:
    return loc->nation();
  case LOCTYPE_STATE:
    return loc->state();
  // zone was checed previously, so return empty location
  default:
    return "";
  }
  // shouldn't never get here, but just to get rid of warning
  return "";
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getGeoData
//
// Description:
//
// function get the correct location from loc structure
//
// @param  trx                    - reference to pricing transaction
// @param  tsiData                - The TSIData structure
// @param  locKey1                - first location used to obtain geoLocation
// @param  locKey2                - second location used to obtain geoLocation
//
// @return true if function succeeded, otherwise false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::getGeoData(const PricingTrx& trx,
                        TSIData& tsiData,
                        const LocKey& locKey1,
                        const LocKey& locKey2)
{
  //
  // Get the geodata requirements for the TSI
  //
  const Loc* loc = nullptr;

  switch (RuleConst::TSIGeoRequired(tsiData.tsiInfo().geoRequired()))
  {
  case RuleConst::TSI_GEO_GEOTYPE_REQUIRED:

    // must have at least one location type
    if ((LOCTYPE_NONE == locKey1.locType()) && (LOCTYPE_NONE == locKey2.locType()))
      return getGeoDataLogErrMsg(trx, tsiData, "GeoType required, but it is blank.");

    // ZOnes are not supported
    if ((LOCTYPE_ZONE == locKey1.locType()) || (LOCTYPE_ZONE == locKey2.locType()))
      return getGeoDataLogErrMsg(trx, tsiData, "GeoType = Zone not allowed.");

    // copy location type
    tsiData.locType1() = locKey1.locType();
    tsiData.locType2() = locKey2.locType();

    if (!getGeoLocaleFromItin(tsiData, loc))
      return getGeoDataLogErrMsg(trx, tsiData, "Could not get geoLocale from itinerary.");

    // get the correct locations
    tsiData.locCode1() = getGeoDataGetLoc(loc, tsiData.locType1());
    tsiData.locCode2() = getGeoDataGetLoc(loc, tsiData.locType2());

    break;

  case RuleConst::TSI_GEO_GET_FROM_ITIN:

    // Get the locale from the itinerary
    if (!getGeoLocaleFromItin(tsiData, loc))
      return getGeoDataLogErrMsg(trx, tsiData, "Could not get geoLocale from itinerary.");

    tsiData.locType1() = LOCTYPE_NATION;
    tsiData.locType2() = LOCTYPE_NONE;

    if (loc)
      tsiData.locCode1() = loc->nation();
    break;

  case RuleConst::TSI_GEO_BOTH_REQUIRED:

    // must have at least one loc type
    if (UNLIKELY((LOCTYPE_NONE == locKey1.locType()) && (LOCTYPE_NONE == locKey2.locType())))
      return getGeoDataLogErrMsg(trx, tsiData, "GeoLocale and GeoType required. GeoType is blank.");

    // must have at least one locale
    if (UNLIKELY(locKey1.loc().empty() && locKey2.loc().empty()))
      return getGeoDataLogErrMsg(
          trx, tsiData, "GeoLocale and GeoType required. GeoLocale is blank.");

  // NO BREAK - this is intended
  // just copy the data as for the blank
  case RuleConst::TSI_GEO_BLANK:
  default:
    // Geodata is not required for this TSI
    //  but if it is provided then use it

    tsiData.locType1() = locKey1.locType();
    tsiData.locType2() = locKey2.locType();

    tsiData.locCode1() = locKey1.loc();
    tsiData.locCode2() = locKey2.loc();
    break;
  }
  return true;
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::reverseTravel
//
// Description:
//
// function created reversed segmnet from the vector of segments
//
// @param  dataHandle               - DataHandle Object
// @param  oldTvlSegVec             - vector of segments for which revesed will be created
// @param  orderNum                 - orded of the segment, which will be reversed
//
// @return pointer to reversed segment if succeeded, otherwise 0
//
// ----------------------------------------------------------------------------
TravelSeg*
RuleUtilTSI::reverseTravel(DataHandle& dataHandle,
                           const std::vector<TravelSeg*>& oldTvlSegVec,
                           uint16_t orderNum)
{
  // get the vector size
  uint16_t vecSz = oldTvlSegVec.size();

  // check if not "out of range"
  if (orderNum >= vecSz)
    return nullptr;

  uint16_t backwardOrderNum = vecSz - orderNum - 1;

  // get the reference to the
  const TravelSeg& oldTvlSegForward = *oldTvlSegVec[orderNum];
  const TravelSeg& oldTvlSegBackward = *oldTvlSegVec[backwardOrderNum];

  TravelSeg* newTvlSeg = nullptr;

  // create returning value
  if (oldTvlSegBackward.segmentType() == Arunk)
  {
    ArunkSeg* arunkSeg = nullptr;
    dataHandle.get(arunkSeg);
    newTvlSeg = arunkSeg;
  }
  else
  {
    AirSeg* airSeg = nullptr;
    dataHandle.get(airSeg);
    newTvlSeg = airSeg;
  }

  // shouldn't never happend
  if (!newTvlSeg)
    return nullptr;

  // keep original pnr segment number and  openSegAfterDataSeg
  newTvlSeg->pnrSegment() = oldTvlSegForward.pnrSegment();
  newTvlSeg->openSegAfterDatedSeg() = oldTvlSegForward.openSegAfterDatedSeg();

  // rest of data is copied from back segment
  // reversed directions
  newTvlSeg->origAirport() = oldTvlSegBackward.destAirport();
  newTvlSeg->destAirport() = oldTvlSegBackward.origAirport();

  newTvlSeg->boardMultiCity() = oldTvlSegBackward.offMultiCity();
  newTvlSeg->offMultiCity() = oldTvlSegBackward.boardMultiCity();

  newTvlSeg->origin() = oldTvlSegBackward.destination();
  newTvlSeg->destination() = oldTvlSegBackward.origin();

  // just copied
  newTvlSeg->geoTravelType() = oldTvlSegBackward.geoTravelType();
  newTvlSeg->segmentType() = oldTvlSegBackward.segmentType();

  newTvlSeg->pssDepartureDate() = oldTvlSegBackward.pssDepartureDate();
  newTvlSeg->pssDepartureTime() = oldTvlSegBackward.pssDepartureTime();
  newTvlSeg->pssArrivalDate() = oldTvlSegBackward.pssArrivalDate();
  newTvlSeg->pssArrivalTime() = oldTvlSegBackward.pssArrivalTime();
  newTvlSeg->departureDT() = oldTvlSegBackward.departureDT();
  newTvlSeg->arrivalDT() = oldTvlSegBackward.arrivalDT();

  return newTvlSeg;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getReversedTravelSegs
//
// Description:
//
// function created reversed vector segmnet
//
// @param  dataHandle               - DataHandle Object
// @param  newTvlSegs               - vector of reversed travel segments
// @param  oldTvlSegVec             - vector of segments for which revesed will be created
//
// @return true if succeded, otherwise false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::getReversedTravelSegs(DataHandle& dataHandle,
                                   std::vector<TravelSeg*>& newTvlSegs,
                                   const std::vector<TravelSeg*>& oldTvlSegs)
{
  // get vector size
  uint16_t tvlSegSz = oldTvlSegs.size();
  // for each TravelSeg from the vector, create reversed segment and add this to vector
  for (uint16_t orderNum = 0; orderNum < tvlSegSz; orderNum++)
  {
    TravelSeg* newTvlSeg = reverseTravel(dataHandle, oldTvlSegs, orderNum);

    if (!newTvlSeg)
      return false;

    newTvlSegs.push_back(newTvlSeg);
  }

  // if only one element, then just copy furthest point
  if (oldTvlSegs.size() == 1)
  {
    newTvlSegs.front()->furthestPoint() = oldTvlSegs.front()->furthestPoint();
    return true;
  }

  // Old: A - B(s) B - C C - D
  // New: D - C C - B(s) B - A
  std::vector<TravelSeg*>::const_iterator oldSegI = oldTvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator oldSegIEnd = oldTvlSegs.end();
  std::vector<TravelSeg*>::reverse_iterator newSegI = newTvlSegs.rbegin();
  std::vector<TravelSeg*>::reverse_iterator newSegIEnd = newTvlSegs.rend();

  // copy stopover information
  for (; oldSegI != oldSegIEnd; oldSegI++, newSegI++)
  {
    if ((newSegI + 1) != newSegIEnd)
    {
      (*(newSegI + 1))->stopOver() = (*oldSegI)->stopOver();
      (*(newSegI + 1))->forcedStopOver() = (*oldSegI)->forcedStopOver();
      (*(newSegI + 1))->forcedConx() = (*oldSegI)->forcedConx();
      (*(newSegI + 1))->furthestPoint() = (*oldSegI)->furthestPoint();
    }
  }

  return true;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getTSIOrigDestCheck
//
// Description:
//
// function created reversed vector segmnet
//
// @param  tsiInfo                  - tsiInfo structure code pointer
// @param  checkOrig                - will be set to true if origin should be checked
// @param  checkDest                - will be set to true if destinations should be checked
//
// @return true if succeded, otherwise false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::getTSIOrigDestCheck(const TSIInfo* tsiInfo, bool& checkOrig, bool& checkDest)
{
  //
  // Get the Save Type
  //
  RuleConst::TSIApplicationType saveType = RuleConst::TSIApplicationType(tsiInfo->type());

  //
  // Set the origin and destination check return flags
  //
  switch (saveType)
  {
  case RuleConst::TSI_APP_CHECK_ORIG:
    checkOrig = true;
    checkDest = false;
    break;

  case RuleConst::TSI_APP_CHECK_DEST:
    checkOrig = false;
    checkDest = true;
    break;

  case RuleConst::TSI_APP_CHECK_ORIG_DEST:
    checkOrig = true;
    checkDest = true;
    break;

  default:
    LOG4CXX_ERROR(
        _tsiLogger,
        "Rules.TSI: Bad TSI record. Unrecognized saveType (type). TSI: " << tsiInfo->tsi());
    return false;
  }

  return true;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getTSIOrigDestCheck
//
// Description:
//
// function created reversed vector segmnet
//
// @param  tsi                      - tsi code noumber
// @param  trx                      - pricing transaction reference
// @param  checkOrig                - will be set to true if origin should be checked
// @param  checkDest                - will be set to true if destinations should be checked
//
// @return true if succeded, otherwise false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::getTSIOrigDestCheck(const TSICode tsi,
                                 PricingTrx& trx,
                                 bool& checkOrig,
                                 bool& checkDest)
{
  // Get the TSIInfo object from the dataHandle
  const TSIInfo* tsiInfo = trx.dataHandle().getTSI(tsi);

  // check if succeded
  if (UNLIKELY(!tsiInfo))
  {
    LOG4CXX_ERROR(_tsiLogger, "Rules.TSI: Invalid TSI number: " << tsi);
    return false;
  }
  // main logic
  return getTSIOrigDestCheck(tsiInfo, checkOrig, checkDest);
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getTSIOrigDestCheck
//
// Description:
//
// function created reversed vector segmnet
//
// @param  tsiInfo                  - tsiInfo structure code pointer
// @param  checkOrig                - will be set to true if origin should be checked
// @param  checkDest                - will be set to true if destinations should be checked
//
// @return true if succeded, otherwise false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::getTSIOrigDestCheckFromGeoRuleItem(const uint32_t itemNo,
                                                const VendorCode& vendorCode,
                                                PricingTrx& trx,
                                                bool& checkOrig,
                                                bool& checkDest)
{
  // const TSELatencyData metrics(trx,"GET GEO RULE ITEM 2");
  // get item Geo RUle Item (table995) from the DB
  //
  const std::vector<GeoRuleItem*>& geoRuleItemList =
      trx.dataHandle().getGeoRuleItem(vendorCode, itemNo);

  if (UNLIKELY(geoRuleItemList.size() != 1))
  {
    // no Geo Rule Item, log error and return false
    LOG4CXX_ERROR(_dataErrLogger,
                  "Geo Rule Item vector did not contain 1 item"
                      << " ItemNo: " << itemNo << " VendorCode: " << vendorCode);
    return false;
  }

  // get first element
  const GeoRuleItem* geoIt = geoRuleItemList.front();

  // Get the TSIInfo object from the dataHandle
  const TSIInfo* tsiInfo = trx.dataHandle().getTSI(geoIt->tsi());

  // check if succeded
  if (UNLIKELY(!tsiInfo))
  {
    LOG4CXX_ERROR(_tsiLogger, "Rules.TSI: Invalid TSI number: " << geoIt->tsi());
    return false;
  }

  return getTSIOrigDestCheck(tsiInfo, checkOrig, checkDest);
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getTSIScope
//
// Description:
//
// function get the scope for the passed tsi number
//
// @param  tsi                  - tsi noumber
// @param  trx                  - reference to the pricing transaction
// @param  scope                - returned tsi scope
//
// @return true if succeded, otherwise false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::getTSIScope(const TSICode tsi, PricingTrx& trx, RuleConst::TSIScopeType& scope)
{
  // Get the TSIInfo object from the dataHandle
  const TSIInfo* tsiInfo = trx.dataHandle().getTSI(tsi);

  if (!tsiInfo)
  {
    LOG4CXX_ERROR(_tsiLogger, "Rules.TSI: Invalid TSI number: " << tsi);
    return false;
  }

  //
  // Get the Save Scope (Scope Type) of the TSI
  //
  scope = RuleConst::TSIScopeType(tsiInfo->scope());
  return true;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getTSIScopeFromGeoRuleItem
//
// Description:
//
// function get the scope for the T995
//
// @param  itemNo               - T995 item noumber
// @param  vendorCode           - vendor
// @param  trx                  - reference to the pricing transaction
// @param  scope                - returned tsi scope
//
// @return true if succeded, otherwise false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::getTSIScopeFromGeoRuleItem(const uint32_t itemNo,
                                        const VendorCode& vendorCode,
                                        PricingTrx& trx,
                                        RuleConst::TSIScopeType& scope)
{
  // const TSELatencyData metrics(trx,"GET GEO RULE ITEM 1");
  // get item Geo RUle Item (table995) from the DB
  //
  const std::vector<GeoRuleItem*>& geoRuleItemList =
      trx.dataHandle().getGeoRuleItem(vendorCode, itemNo);

  if (UNLIKELY(geoRuleItemList.size() != 1))
  {
    // no Geo Rule Item, logg error and return fales
    //
    LOG4CXX_ERROR(_dataErrLogger,
                  "Geo Rule Item vector did not contain 1 item."
                      << " ItemNo: " << itemNo << " VendorCode: " << vendorCode);
    return false;
  }

  // get T995 pointer
  const GeoRuleItem* geoIt = geoRuleItemList.front();

  // get tsi number
  TSICode tsi;
  tsi = geoIt->tsi();

  // should be greatedr then zero
  if (tsi == 0)
    return false;

  return (getTSIScope(tsi, trx, scope));
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::identifyIntlDomTransfers
//
// Description:
//
// function goes thrue all TSITravelSegMarkup, and match all segments which have
//    International/Domestic transfer
//
// @param  tsiData              - TSIData structure
// @param  tsMarkup             - reference to the vector of TSITravelSegMarkup
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::identifyIntlDomTransfers(const TSIData& tsiData, TSITravelSegMarkupContainer& tsMarkup)
{
  const std::vector<TSIInfo::TSIMatchCriteria>& mc = tsiData.tsiInfo().matchCriteria();

  // if no INTL_DOM_TRANSFER, the do nothing
  if (UNLIKELY(std::find(mc.begin(), mc.end(), TSIInfo::INTL_DOM_TRANSFER) != mc.end()))
  {
    TSITravelSegMarkupI iter = tsMarkup.begin();
    TSITravelSegMarkupI iterEnd = tsMarkup.end();

    // loop thrue all TSITravelSegMarkup
    for (; iter != iterEnd; ++iter)
    {
      TSITravelSegMarkupI iterNext = iter;
      // Point iterNext to the following element and check for end.
      if (++iterNext == iterEnd)
        // No more
        return;

      TSITravelSegMarkup& tsm = *iter;
      TravelSeg* ts = tsm.travelSeg();

      // if not Air segemt, then try next one
      AirSeg* as = dynamic_cast<AirSeg*>(ts);
      if (!as)
        continue;

      // Find the next AirSeg
      //
      TSITravelSegMarkup& tsmNext = *iterNext; // Got the next AirSeg
      TravelSeg* tsNext = tsmNext.travelSeg();
      AirSeg* asNext = dynamic_cast<AirSeg*>(tsNext);

      // loop thrue all TravelSeg to find next valid AirSeg
      for (; (asNext == nullptr) && (iterNext != iterEnd); iterNext++)
      {
        // FIX THE BUG - iterNext wil be incamented after before leving loop for,
        // so need to set tsmNext here!
        // TSITravelSegMarkup& tempTsmNext2 = *iterNext;
        tsmNext = *iterNext;
        tsNext = tsmNext.travelSeg();
        asNext = dynamic_cast<AirSeg*>(tsNext);
      }
      // There are no more AirSegs
      if (!asNext)
        return;

      if ((as->carrier() != asNext->carrier()) || (as->flightNumber() != asNext->flightNumber()))
      {
        // The destination is a transfer point
        if ((!tsm.fareBreakAtDestination()) && (tsm.isInternational() != tsmNext.isInternational()))
        {
          tsm.isIntlDomTransfer() = true;
        }
      }
    }
  }
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::processTravelSegsSetupMatchCriteria
//
// Description:
//
// function splits TSIMatchCriteria vector into two vectors used in later validation
//
// @param  trx                  - PricingTrx reference
// @param  tsiInfo              - to TSIInfo structure
// @param  mc1                  - first returning vectors
// @param  mc2                  - second returning vecotr
//
// @return true if succeeded
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::processTravelSegsSetupMatchCriteria(const PricingTrx& trx,
                                                 const TSIInfo& tsiInfo,
                                                 std::vector<TSIInfo::TSIMatchCriteria>& mc1,
                                                 std::vector<TSIInfo::TSIMatchCriteria>& mc2)
{
  // get the loop match and match criteria vector
  const RuleConst::TSILoopMatch loopMatch = RuleConst::TSILoopMatch(tsiInfo.loopMatch());
  const std::vector<TSIInfo::TSIMatchCriteria>& matchCriteria = tsiInfo.matchCriteria();

  // for second first we should always have 3 segments
  if (UNLIKELY(RuleConst::TSI_MATCH_SECOND_FIRST == loopMatch))
  {
    if (3 != matchCriteria.size())
    {
      // Bad TSIInfo object... it must have 3 match criteria.
      //
      logErrMsg(trx, tsiInfo, "BadTSI record, Should have 3 match criteria.");
      return false;
    }
    //
    // Soft match the first two match criteria
    //
    mc1.push_back(matchCriteria[0]);
    mc1.push_back(matchCriteria[1]);

    //
    // Match third criteria after soft matching first two
    //
    mc2.push_back(matchCriteria[2]);
  }
  else
  {
    // just copy match cirteria to the first vector
    mc1.insert(mc1.end(), matchCriteria.begin(), matchCriteria.end());
  }
  return true;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::processTravelSegsSetupTSIMatch
//
// Description:
//
// function validate tarvel segment against TSI data
//
// @param  trx                  - PricingTrx reference
// @param  tsiData              - refference to TSIData structure
// @param  mc1                  - matching criteria used for validation
// @param  mc2                  - matching criteria used for validation
// @param  tsm                  - Markup of the validated travel segment
// @param  origMatch            - this flagg will be set to true if pass
//                                origin GEO validation pass
// @param  destMatch            - this flagg will be set to true if pass
//                                destination GEO validation pass
// @param  currMatch            - return validation status of current segment
// @param  prevMatch            - return validation status of previous segment
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::processTravelSegsSetupTSIMatch(const PricingTrx& trx,
                                            const TSIData& tsiData,
                                            const std::vector<TSIInfo::TSIMatchCriteria>& mc1,
                                            const std::vector<TSIInfo::TSIMatchCriteria>& mc2,
                                            TSITravelSegMarkup& tsm,
                                            bool& origMatch,
                                            bool& destMatch,
                                            RuleConst::TSIMatch& currMatch,
                                            RuleConst::TSIMatch& prevMatch)
{
  // get the loop match
  const RuleConst::TSILoopMatch loopMatch = RuleConst::TSILoopMatch(tsiData.tsiInfo().loopMatch());
  // lint -e{578}
  std::string noMatchReason;

  // for second first
  if (UNLIKELY(RuleConst::TSI_MATCH_SECOND_FIRST == loopMatch))
  {
    // if previous was not validated
    if (RuleConst::TSI_NOT_MATCH == prevMatch)
    {
      //
      // Soft match the first two criteria
      //
      prevMatch =
          checkTSIMatchTravelSeg(trx, tsiData, tsm, mc1, origMatch, destMatch, noMatchReason);
      // if previous match or soft match
      if ((RuleConst::TSI_MATCH == prevMatch) || (RuleConst::TSI_SOFT_MATCH == prevMatch))
      {
        noMatchReason = "- 1ST -";
      }
    }
    else
    {
      //
      // Match the third criteria after
      //  soft matching the first two
      //
      currMatch =
          checkTSIMatchTravelSeg(trx, tsiData, tsm, mc2, origMatch, destMatch, noMatchReason);
    }
  }
  else
  {
    // not second first call using mc1
    currMatch = checkTSIMatchTravelSeg(trx, tsiData, tsm, mc1, origMatch, destMatch, noMatchReason);
  }
  // save results
  tsm.match() = currMatch;
  tsm.origMatch() = origMatch;
  tsm.destMatch() = destMatch;

  // savre no match reason
  if (RuleConst::TSI_MATCH == currMatch)
  {
    tsm.noMatchReason().clear();
  }
  else
  {
    tsm.noMatchReason() = noMatchReason;
  }
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::processTravelSegsAddPrev
//
// Description:
//
// function set GEO matching and create segment wrapper for previous travel segment
//
// @param  trx                  - PricingTrx refference
// @param  prevTsm              - reference to the previous travel segment
// @param  origMatch            - is GEO validation pass on origin
// @param  destSave             - shouls save destination information
//
// @return pointer to the TravelSegWrapper structure
//
// ----------------------------------------------------------------------------
RuleUtilTSI::TravelSegWrapper*
RuleUtilTSI::processTravelSegsAddPrev(PricingTrx& trx,
                                      TSITravelSegMarkup& prevTsm,
                                      bool origMatch,
                                      bool destSave)
{
  // return pointer
  TravelSegWrapper* tsw = nullptr;
  // save destSave if origin match?
  if (LIKELY(origMatch))
    prevTsm.destSave() = destSave;

  // allocate data
  trx.dataHandle().get(tsw);
  // lint --e{413}
  // just to be sure
  if (LIKELY(tsw))
  {
    // set travel seg from previous trav seg markup
    tsw->travelSeg() = prevTsm.travelSeg();
    // save destSave if origin match?
    if (LIKELY(origMatch))
      tsw->destMatch() = destSave;
  }
  return tsw;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::processTravelSegsAddCurr
//
// Description:
//
// function set GEO matching and create segment wrapper for current travel segment
//
// @param  trx                  - PricingTrx refference
// @param  prevTsm              - reference to the current travel segment
// @param  origMatch            - is GEO validation pass on origin
// @param  destMatch            - is GEO validation pass on destination
// @param  origSave             - shouls save origin information
// @param  destSave             - shouls save destination information
//
// @return pointer to the TravelSegWrapper structure
//
// ----------------------------------------------------------------------------
RuleUtilTSI::TravelSegWrapper*
RuleUtilTSI::processTravelSegsAddCurr(PricingTrx& trx,
                                      TSITravelSegMarkup& tsm,
                                      bool origMatch,
                                      bool destMatch,
                                      bool origSave,
                                      bool destSave)
{
  // return pointer
  TravelSegWrapper* tsw = nullptr;
  // save origin and destination
  tsm.origSave() = origSave && origMatch;
  tsm.destSave() = destSave && destMatch;

  trx.dataHandle().get(tsw);
  // lint --e{413}
  // just to be sure
  if (LIKELY(tsw))
  {
    // save travel segment data
    tsw->travelSeg() = tsm.travelSeg();
    tsw->origMatch() = origSave && origMatch;
    tsw->destMatch() = destSave && destMatch;
  }
  return tsw;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::processTravelSegsAddNext
//
// Description:
//
// function set GEO matching and create segment wrapper for next travel segment
//
// @param  trx                  - PricingTrx refference
// @param  prevTsm              - reference to the next travel segment
// @param  destMatch            - is GEO validation pass on destiantion
// @param  origSave             - shouls save origin information
//
// @return pointer to the TravelSegWrapper structure
//
// ----------------------------------------------------------------------------
RuleUtilTSI::TravelSegWrapper*
RuleUtilTSI::processTravelSegsAddNext(PricingTrx& trx,
                                      TSITravelSegMarkup& nextTsm,
                                      bool destMatch,
                                      bool origSave)
{
  // return pointer
  TravelSegWrapper* tsw = nullptr;
  // save origSave if destMatch
  if (LIKELY(destMatch))
    nextTsm.origSave() = origSave;

  // alloc pointer
  trx.dataHandle().get(tsw);
  // lint --e{413}
  // just to be sure
  if (LIKELY(tsw))
  {
    tsw->travelSeg() = nextTsm.travelSeg();
    if (LIKELY(destMatch))
      tsw->origMatch() = origSave;
  }
  return tsw;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::processTravelSegsForward
//
// Description:
//
// function process travel segment informations, when loop direction is forward
//
// @param  trx                  - PricingTrx refference
// @param  tsiData              - TSIData structure
// @param  applTravelSegment    - returned vector of  TravelSegWrapper
// @param  tsMarkup             - vector of Travel segment markups
// @param  locSpecified         - is location specified
// @param  checkOrig            - should origin be checked
// @param  checkDest            - should destination be checked
// @param  origSave             - should keep origin infromation
// @param  destSave             - should keep destination information
// @param  checkFareBreaksOnly  - should check fare break
//
// @return true if succeded, otherwise false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::processTravelSegsForward(PricingTrx& trx,
                                      const TSIData& tsiData,
                                      TravelSegWrapperVector& applTravelSegment,
                                      TSITravelSegMarkupContainer& tsMarkup,
                                      bool locSpecified,
                                      bool checkOrig,
                                      bool checkDest,
                                      bool origSave,
                                      bool destSave,
                                      bool checkFareBreaksOnly)
{
  // get tsiInfo structure
  const TSIInfo& tsiInfo = tsiData.tsiInfo();

  std::vector<TSIInfo::TSIMatchCriteria> mc, mc2;
  // Set up the match criteria.
  // If failed, we're done
  if (UNLIKELY(!processTravelSegsSetupMatchCriteria(trx, tsiInfo, mc, mc2)))
    return false;

  // loop iterartors
  TSITravelSegMarkupI fIter = tsMarkup.begin();
  TSITravelSegMarkupI fIterEnd = tsMarkup.end();
  TSITravelSegMarkupI fIterPrev = tsMarkup.begin();
  ;

  // LoopOffset is used to determine if we need to skip the departure segment
  const int16_t loopOffset = tsiInfo.loopOffset();

  if ((1 == loopOffset) && (fIter != fIterEnd))
  {
    // Skip the departure item. For forward iteration,
    //  skip the first segment. For backward iteration
    //  skip the last segment.
    //
    fIter->noMatchReason() = "SKIPPED ";
    ++fIter;
  }

  //
  // Process the input travel segments for TSI MatchCriteria and GeoData
  //
  RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH;
  RuleConst::TSIMatch currMatch = RuleConst::TSI_NOT_MATCH;

  bool addPrev = false;
  bool addCurr = false;
  bool addNext = false;
  bool continueLooping = true;

  while (continueLooping && fIter != fIterEnd)
  {
    TSITravelSegMarkup& tsm = *fIter;
    TSITravelSegMarkup& prevTsm = *fIterPrev;

    if (checkFareBreaksOnly &&
        !((checkOrig && tsm.fareBreakAtOrigin()) || (checkDest && tsm.fareBreakAtDestination())))
    {
      fIterPrev = fIter;
      ++fIter;
      continue;
    }

    bool origMatch = false;
    bool destMatch = false;

    // process TSITravelSegments
    processTravelSegsSetupTSIMatch(
        trx, tsiData, mc, mc2, tsm, origMatch, destMatch, currMatch, prevMatch);

    // check loop match
    checkLoopMatch(
        tsiData, prevMatch, currMatch, addPrev, addCurr, addNext, continueLooping, locSpecified);

    // get segment wrapper for previous
    if (addPrev && (fIterPrev != fIter))
    {
      TravelSegWrapper* tsw = processTravelSegsAddPrev(trx, prevTsm, origMatch, destSave);
      applTravelSegment.push_back(tsw);
    }

    // get segment wrapper fo current
    if (addCurr)
    {
      TravelSegWrapper* tsw =
          processTravelSegsAddCurr(trx, tsm, origMatch, destMatch, origSave, destSave);
      applTravelSegment.push_back(tsw);
    }

    //  get segment wrapper for next
    if (UNLIKELY(addNext))
    {
      TSITravelSegMarkupI it = fIter;
      ++it;

      if (it != fIterEnd)
      {
        TravelSegWrapper* tsw = processTravelSegsAddNext(trx, *it, destMatch, origSave);
        applTravelSegment.push_back(tsw);
      }
    }

    // increase iterator
    fIterPrev = fIter;
    ++fIter;
  }
  return true;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::processTravelSegsBackward
//
// Description:
//
// function process travel segment informations, when loop direction is backward
//
// @param  trx                  - PricingTrx refference
// @param  tsiData              - TSIData structure
// @param  applTravelSegment    - returned vector of  TravelSegWrapper
// @param  tsMarkup             - vector of Travel segment markups
// @param  locSpecified         - is location specified
// @param  checkOrig            - should origin be checked
// @param  checkDest            - should destination be checked
// @param  origSave             - should keep origin infromation
// @param  destSave             - should keep destination information
// @param  checkFareBreaksOnly  - should check fare break
//
// @return true if succeded, otherwise false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::processTravelSegsBackward(PricingTrx& trx,
                                       const TSIData& tsiData,
                                       TravelSegWrapperVector& applTravelSegment,
                                       TSITravelSegMarkupContainer& tsMarkup,
                                       bool locSpecified,
                                       bool checkOrig,
                                       bool checkDest,
                                       bool origSave,
                                       bool destSave,
                                       bool checkFareBreaksOnly)
{
  // get tsiInfo structure
  const TSIInfo& tsiInfo = tsiData.tsiInfo();

  std::vector<TSIInfo::TSIMatchCriteria> mc, mc2;
  // Set up the match criteria.
  // If failed, we're done
  if (UNLIKELY(!processTravelSegsSetupMatchCriteria(trx, tsiInfo, mc, mc2)))
    return false;

  // set up the iterators
  TSITravelSegMarkupRI bIter = tsMarkup.rbegin();
  TSITravelSegMarkupRI bIterEnd = tsMarkup.rend();
  TSITravelSegMarkupRI bIterPrev = tsMarkup.rbegin();
  ;

  // LoopOffset is used to determine if we need to skip the departure segment
  const int16_t loopOffset = tsiInfo.loopOffset();

  if (UNLIKELY((1 == loopOffset) && (bIter != bIterEnd)))
  {
    // Skip the departure item. For forward iteration,
    //  skip the first segment. For backward iteration
    //  skip the last segment.
    //
    --bIterEnd;
    bIterEnd->noMatchReason() = "SKIPPED ";
  }

  //
  // Process the input travel segments for TSI MatchCriteria and GeoData
  //

  RuleConst::TSIMatch prevMatch = RuleConst::TSI_NOT_MATCH;
  RuleConst::TSIMatch currMatch = RuleConst::TSI_NOT_MATCH;

  bool addPrev = false;
  bool addCurr = false;
  bool addNext = false;

  bool continueLooping = true;

  while (continueLooping && (bIter != bIterEnd))
  {
    TSITravelSegMarkup& tsm = (*bIter);
    TSITravelSegMarkup& prevTsm = (*bIterPrev);

    if (checkFareBreaksOnly &&
        !((checkOrig && tsm.fareBreakAtOrigin()) || (checkDest && tsm.fareBreakAtDestination())))
    {
      bIterPrev = bIter;
      ++bIter;
      continue;
    }

    bool origMatch = false;
    bool destMatch = false;

    // process TSITravelSegments
    processTravelSegsSetupTSIMatch(
        trx, tsiData, mc, mc2, tsm, origMatch, destMatch, currMatch, prevMatch);

    checkLoopMatch(
        tsiData, prevMatch, currMatch, addPrev, addCurr, addNext, continueLooping, locSpecified);

    //  get segment wrapper for next
    if (addNext && (bIterPrev != bIter))
    {
      TravelSegWrapper* tsw = processTravelSegsAddNext(trx, prevTsm, destMatch, origSave);
      applTravelSegment.insert(applTravelSegment.begin(), tsw);
    }

    // get segment wrapper fo current
    if (addCurr)
    {
      TravelSegWrapper* tsw =
          processTravelSegsAddCurr(trx, tsm, origMatch, destMatch, origSave, destSave);
      applTravelSegment.insert(applTravelSegment.begin(), tsw);
    }

    // get segment wrapper for previous
    if (UNLIKELY(addPrev))
    {
      TSITravelSegMarkupRI it = bIter;
      ++it;
      if (it != bIterEnd)
      {
        TravelSegWrapper* tsw = processTravelSegsAddPrev(trx, *it, origMatch, destSave);
        applTravelSegment.insert(applTravelSegment.begin(), tsw);
      }
    }
    // increase iterator
    bIterPrev = bIter;
    ++bIter;
  }
  return true;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::getDirectionsFromLoop
//
// Description:
//
// function gets direction and farebreak from TSILoopDirection
//
// @param  trx                  - PricingTrx refference
// @param  tsiInfo              - TSIInfo structure
// @param  loopForward          - is direction forward
// @param  checkFareBreaksOnly  - is check fare break only
//
// @return true if succeded, otherwise false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::getDirectionsFromLoop(PricingTrx& trx,
                                   const TSIInfo& tsiInfo,
                                   bool& loopForward,
                                   bool& checkFareBreaksOnly)
{
  // just go thrue all posible values in the switch

  switch (RuleConst::TSILoopDirection(tsiInfo.loopDirection()))
  {
  case RuleConst::TSI_LOOP_FORWARD:
    loopForward = true;
    checkFareBreaksOnly = false;
    break;

  case RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL:
    loopForward = true;
    checkFareBreaksOnly = true;
    break;

  case RuleConst::TSI_LOOP_BACKWARD:
    loopForward = false;
    checkFareBreaksOnly = false;
    break;

  case RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART:
    loopForward = false;
    checkFareBreaksOnly = true;
    break;

  // Error!
  default:
    logErrMsg(trx, tsiInfo, "Bad TSI record. Unrecognized loopDirection.");
    return false;
  }
  return true;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::processTravelSegs
//
// Description:
//
// function process travel segment informations
//
// @param  trx                  - PricingTrx refference
// @param  tsiData              - TSIData structure
// @param  applTravelSegment    - returned vector of  TravelSegWrapper
// @param  tsMarkup             - vector of Travel segment markups
// @param  locSpecified         - is location specified
//
// @return true if succeded, otherwise false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::processTravelSegs(PricingTrx& trx,
                               const TSIData& tsiData,
                               TravelSegWrapperVector& applTravelSegment,
                               TSITravelSegMarkupContainer& tsMarkup,
                               bool locSpecified)
{
  // get TsiInfo
  const TSIInfo& tsiInfo = tsiData.tsiInfo();

  // get TsiGeoCheck
  RuleConst::TSIApplicationType geoType = (RuleConst::TSIApplicationType)tsiInfo.geoCheck();

  // set origin for CHECK_ORIG or CHECK_ORIG_DEST
  bool checkOrig =
      (RuleConst::TSI_APP_CHECK_ORIG == geoType) || (RuleConst::TSI_APP_CHECK_ORIG_DEST == geoType);
  // set destination for CHECK_DEST or CHECK_ORIG_DEST
  bool checkDest =
      (RuleConst::TSI_APP_CHECK_DEST == geoType) || (RuleConst::TSI_APP_CHECK_ORIG_DEST == geoType);

  bool origSave = false;
  bool destSave = false;

  if (UNLIKELY(!getTSIOrigDestCheck(&tsiInfo, origSave, destSave)))
    return false;

  //
  // Set up loop direction
  //
  bool loopForward, checkFareBreaksOnly;

  if (UNLIKELY(!getDirectionsFromLoop(trx, tsiInfo, loopForward, checkFareBreaksOnly)))
    return false;

  if (loopForward)
    return processTravelSegsForward(trx,
                                    tsiData,
                                    applTravelSegment,
                                    tsMarkup,
                                    locSpecified,
                                    checkOrig,
                                    checkDest,
                                    origSave,
                                    destSave,
                                    checkFareBreaksOnly);

  return processTravelSegsBackward(trx,
                                   tsiData,
                                   applTravelSegment,
                                   tsMarkup,
                                   locSpecified,
                                   checkOrig,
                                   checkDest,
                                   origSave,
                                   destSave,
                                   checkFareBreaksOnly);
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeaderGeoReqired
//
// Description:
//
// Function converts geoType into displayable string
//
// @param  geoReq                - RuleConst::TSIGeoRequired reo required
//
// @return representing string
//
// ----------------------------------------------------------------------------
std::string
RuleUtilTSI::writeTSIDiagHeaderGeoReqired(const char& geoReq)
{
  // initialize return value with 4 length empty string
  std::string ret("   ");
  // main switch
  switch (geoReq)
  {
  case RuleConst::TSI_GEO_BOTH_REQUIRED:
    ret = "REQ";
    break;

  case RuleConst::TSI_GEO_GEOTYPE_REQUIRED:
    ret = "TYP";
    break;

  case RuleConst::TSI_GEO_GET_FROM_ITIN:
    ret = "GET";
    break;

  case RuleConst::TSI_GEO_BLANK:
    ret = "   ";
    break;
    // no default, initialized to blank in constructor
  }
  // return created string
  return ret;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeaderGeoOut
//
// Description:
//
// Function converts geoOut into displayable string
//
// @param  geoOut                - RuleConst::TSIGeoOut reo required
//
// @return representing string
//
// ----------------------------------------------------------------------------
std::string
RuleUtilTSI::writeTSIDiagHeaderGeoOut(const char& geoOut)
{
  // initialize return value with 3 length empty string
  std::string ret("   ");
  // main switch
  switch (geoOut)
  {
  case RuleConst::TSI_GEO_OUT_INCLUDE:
    ret = "   ";
    break;

  case RuleConst::TSI_GEO_OUT_EXCLUDE:
    ret = "EXC";
    break;

    // no default, initialized to blank in constructor
  }
  // return created string
  return ret;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeaderGeoItinPart
//
// Description:
//
// Function converts geoItinPart into displayable string
//
// @param  geoItinPart                - RuleConst::TSIGeoItinPart itin part required
//
// @return representing string
//
// ----------------------------------------------------------------------------
std::string
RuleUtilTSI::writeTSIDiagHeaderGeoItinPart(const char& geoItinPart)
{
  // initialize return value with 4 length empty string
  std::string ret("    ");
  // main switch
  switch (geoItinPart)
  {
  case RuleConst::TSI_GEO_ITIN_PART_ORIG:
    ret = "ORIG";
    break;

  case RuleConst::TSI_GEO_ITIN_PART_DEST:
    ret = "DEST";
    break;

  case RuleConst::TSI_GEO_ITIN_PART_TURNAROUND:
    ret = "TURN";
    break;

  case RuleConst::TSI_GEO_ITIN_PART_BLANK:
    ret = "    ";
    break;
    // no default, initialized to blank in constructor
  }
  // return created string
  return ret;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeaderGeoGeoCheck
//
// Description:
//
// Function converts geocCheck into displayable string
//
// @param  geoCheck                - RuleConst::TSIApplicationType geocCheck
//
// @return representing string
//
// ----------------------------------------------------------------------------
std::string
RuleUtilTSI::writeTSIDiagHeaderGeoGeoCheck(const char& geoCheck)
{
  // initialize return value with 2 length empty string
  std::string ret("  ");
  // main switch
  switch (geoCheck)
  {
  case RuleConst::TSI_APP_CHECK_ORIG:
    ret = "O ";
    break;

  case RuleConst::TSI_APP_CHECK_DEST:
    ret = "D ";
    break;

  case RuleConst::TSI_APP_CHECK_ORIG_DEST:
    ret = "OD";
    break;
    // no default, initialized to blank in constructor
  }
  // return created string
  return ret;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeaderLoopDirection
//
// Description:
//
// Function converts loopDirection into displayable string
//
// @param  loopDirection                - RuleConst::TSILoopDirection loop direction
//
// @return representing string
//
// ----------------------------------------------------------------------------
std::string
RuleUtilTSI::writeTSIDiagHeaderLoopDirection(const char& loopDirection)
{
  // initialize return value with 4 length empty string
  std::string ret("    ");
  // main switch
  switch (loopDirection)
  {
  case RuleConst::TSI_LOOP_FORWARD:
    ret = "FRWD";
    break;

  case RuleConst::TSI_LOOP_BACKWARD:
    ret = "BKWD";
    break;

  case RuleConst::TSI_LOOP_FORWARD_BREAK_ARRIVAL:
    ret = "FWBA";
    break;

  case RuleConst::TSI_LOOP_BACKWARD_BREAK_DEPART:
    ret = "BKBD";
    break;
    // no default, initialized to blank in constructor
  }
  // return created string
  return ret;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeaderLoopToSet
//
// Description:
//
// Function converts loopToSet into displayable string
//
// @param  loopToSet                - RuleConst::TSILoopItemToSet loop to set
//
// @return representing string
//
// ----------------------------------------------------------------------------
std::string
RuleUtilTSI::writeTSIDiagHeaderLoopToSet(const int16_t& loopToSet)
{
  // initialize return value with 3 length empty string
  std::string ret("   ");
  // main switch
  switch (loopToSet)
  {
  case RuleConst::TSI_LOOP_SET_PREVIOUS:
    ret = " -1";
    break;

  case RuleConst::TSI_LOOP_SET_CURRENT:
    ret = "  0";
    break;

  case RuleConst::TSI_LOOP_SET_NEXT:
    ret = "  1";
    break;

  case RuleConst::TSI_LOOP_SET_CUR_NEXT:
    ret = "101";
    break;

  case RuleConst::TSI_LOOP_SET_CUR_PREV:
    ret = " 99";
    break;
    // no default, initialized to blank in constructor
  }
  // return created string
  return ret;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeaderLoopMatch
//
// Description:
//
// Function converts loopMatch into displayable string
//
// @param  loopMatch                - RuleConst::TSILoopMatch loop match
//
// @return representing string
//
// ----------------------------------------------------------------------------
std::string
RuleUtilTSI::writeTSIDiagHeaderLoopMatch(const char& loopMatch)
{
  // initialize return value with 5 length empty string
  std::string ret("     ");
  // main switch
  switch (loopMatch)
  {
  case RuleConst::TSI_MATCH_ALL:
    ret = "ALL  ";
    break;

  case RuleConst::TSI_MATCH_FIRST:
    ret = "FIRST";
    break;

  case RuleConst::TSI_MATCH_ONCE:
    ret = "ONCE ";
    break;

  case RuleConst::TSI_MATCH_LAST:
    ret = "LAST ";
    break;

  case RuleConst::TSI_MATCH_FIRST_ALL:
    ret = "1/ALL";
    break;

  case RuleConst::TSI_MATCH_SOFT_MATCH:
    ret = "SOFT ";
    break;

  case RuleConst::TSI_MATCH_SECOND_FIRST:
    ret = "2FRST";
    break;
    // no default, initialized to blank in constructor
  }
  // return created string
  return ret;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeaderScopeParam
//
// Description:
//
// Function converts defaultScope param  into displayable string
//
// @param  defaultScope              - RuleConst::TSIScopeParamType scope param
//
// @return representing string
//
// ----------------------------------------------------------------------------
std::string
RuleUtilTSI::writeTSIDiagHeaderScopeParam(const RuleConst::TSIScopeParamType& defaultScope)
{
  // initialize return value with 11 length empty string
  std::string ret("           ");
  // mani switch
  switch (defaultScope)
  {
  case RuleConst::TSI_SCOPE_PARAM_JOURNEY:
    ret = "JOURNEY";
    break;

  case RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY:
    ret = "SUB JOURNEY";
    break;

  case RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT:
    ret = "FARE";
    break;
    // no default, initialized to blank in constructor
  }
  // return created string
  return ret;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeaderScope
//
// Description:
//
// Function converts Scope into displayable string
//
// @param  scope              - RuleConst::TSIScopeType scope type
//
// @return representing string
//
// ----------------------------------------------------------------------------
std::string
RuleUtilTSI::writeTSIDiagHeaderScope(const char& scope)
{
  // initialize return value with 5 length empty string
  std::string ret("     ");
  // main switch
  switch (scope)
  {
  case RuleConst::TSI_SCOPE_JOURNEY:
    ret = "JURNY";
    break;

  case RuleConst::TSI_SCOPE_SUB_JOURNEY:
    ret = "SUBJY";
    break;

  case RuleConst::TSI_SCOPE_FARE_COMPONENT:
    ret = "FARE ";
    break;

  case RuleConst::TSI_SCOPE_SJ_AND_FC:
    ret = "SJ/FC";
    break;
    // no default, initialized to blank in constructor
  }
  // return created string
  return ret;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeaderProcScope
//
// Description:
//
// Function converts Scope into displayable string
//
// @param  processingScope              - RuleConst::TSIScopeType scope type
//
// @return representing string
//
// ----------------------------------------------------------------------------
std::string
RuleUtilTSI::writeTSIDiagHeaderProcScope(const RuleConst::TSIScopeType& processingScope)
{
  // initialize return value with 11 length empty string
  std::string ret("            ");
  // main switch
  switch (processingScope)
  {
  case RuleConst::TSI_SCOPE_JOURNEY:
    ret = "JOURNEY";
    break;

  case RuleConst::TSI_SCOPE_SUB_JOURNEY:
    ret = "SUB JOURNEY";
    break;

  case RuleConst::TSI_SCOPE_FARE_COMPONENT:
    ret = "FARE";
    break;

  case RuleConst::TSI_SCOPE_SJ_AND_FC:
    ret = "SJ/FC";
    break;
    // no default, initialized to blank in constructor
  }
  // return created string
  return ret;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeaderType
//
// Description:
//
// Function converts TSI type into displayable string
//
// @param  type              - RuleConst::TSIApplicationType type
//
// @return representing string
//
// ----------------------------------------------------------------------------
std::string
RuleUtilTSI::writeTSIDiagHeaderType(const char& type)
{
  // initialize return value with 2 length empty string
  std::string ret("  ");
  // main switch
  switch (type)
  {
  case RuleConst::TSI_APP_CHECK_ORIG:
    ret = "O ";
    break;

  case RuleConst::TSI_APP_CHECK_DEST:
    ret = "D ";
    break;

  case RuleConst::TSI_APP_CHECK_ORIG_DEST:
    ret = "OD";
    break;
    // no default, initialized to blank in constructor
  }
  // return created string
  return ret;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeaderMatchCriteria
//
// Description:
//
// Function converts match criteria into displayable string
//
// @param  mc              - TSIInfo::TSIMatchCriteria match criteria
//
// @return representing string
//
// ----------------------------------------------------------------------------
std::string
RuleUtilTSI::writeTSIDiagHeaderMatchCriteria(const TSIInfo::TSIMatchCriteria& mc)
{
  // initialize return value with ERROR string
  std::string ret("ERROR");
  // main switch
  switch (mc)
  {
  case TSIInfo::STOP_OVER:
    ret = RuleConst::MATCH_STOP_OVER_DESC;
    break;

  case TSIInfo::INBOUND:
    ret = RuleConst::MATCH_INBOUND_DESC;
    break;

  case TSIInfo::OUTBOUND:
    ret = RuleConst::MATCH_OUTBOUND_DESC;
    break;

  case TSIInfo::DOMESTIC:
    ret = RuleConst::MATCH_DOMESTIC_DESC;
    break;

  case TSIInfo::ONE_COUNTRY:
    ret = RuleConst::MATCH_ONE_COUNTRY_DESC;
    break;

  case TSIInfo::INTERNATIONAL:
    ret = RuleConst::MATCH_INTERNATIONAL_DESC;
    break;

  case TSIInfo::GATEWAY:
    ret = RuleConst::MATCH_GATEWAY_DESC;
    break;

  case TSIInfo::ORIG_GATEWAY:
    ret = RuleConst::MATCH_ORIG_GATEWAY_DESC;
    break;

  case TSIInfo::DEST_GATEWAY:
    ret = RuleConst::MATCH_DEST_GATEWAY_DESC;
    break;

  case TSIInfo::TRANS_ATLANTIC:
    ret = RuleConst::MATCH_TRANS_ATLANTIC_DESC;
    break;

  case TSIInfo::TRANS_PACIFIC:
    ret = RuleConst::MATCH_TRANS_PACIFIC_DESC;
    break;

  case TSIInfo::TRANS_OCEANIC:
    ret = RuleConst::MATCH_TRANS_OCEANIC_DESC;
    break;

  case TSIInfo::INTERCONTINENTAL:
    ret = RuleConst::MATCH_INTERCONTINENTAL_DESC;
    break;

  case TSIInfo::OVER_WATER:
    ret = RuleConst::MATCH_OVER_WATER_DESC;
    break;

  case TSIInfo::INTL_DOM_TRANSFER:
    ret = RuleConst::MATCH_INTL_DOM_TRANSFER_DESC;
    break;

  // this one was missing in original code, so diag displayed ERROR for FURTHEST
  case TSIInfo::FURTHEST:
    ret = RuleConst::MATCH_FURTHEST_DESC;
    break;
    // no default, initialized to blank in constructor
  }
  ret += " ";
  return ret;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::writeTSIDiagHeader
//
// Description:
//
// Function prints diagnostic header
//
// @param  diag                         - DiagCollector reference
// @param  tsiData                      - TSIData reference with data to be displayed
// @param  defaultScope                 - default TSI scope
// @param  processingScope              - processing TSI scope
// @param  allowJourneyScopeOverride    - is Journey override allowed
// @param  allowPUScopeOverride         - is Pricing Unit override allowed
// @param  allowFCScopeOverride         - is Fare componetnt override allowed
// @param  locKey1                      - geo location 1
// @param  locKey2                      - geo location 2
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::writeTSIDiagHeader(DiagCollector& diag,
                                const TSIData& tsiData,
                                const RuleConst::TSIScopeParamType& defaultScope,
                                const RuleConst::TSIScopeType& processingScope,
                                const bool allowJourneyScopeOverride,
                                const bool allowPUScopeOverride,
                                const bool allowFCScopeOverride,
                                const LocKey& locKey1,
                                const LocKey& locKey2)
{
  const TSIInfo& tsiInfo = tsiData.tsiInfo();

  //
  // Create string descriptions for the TSI data fields
  //
  std::string req = writeTSIDiagHeaderGeoReqired(tsiInfo.geoRequired());
  std::string out = writeTSIDiagHeaderGeoOut(tsiInfo.geoOut());
  std::string itin = writeTSIDiagHeaderGeoItinPart(tsiInfo.geoItinPart());
  std::string ck = writeTSIDiagHeaderGeoGeoCheck(tsiInfo.geoCheck());
  std::string dir = writeTSIDiagHeaderLoopDirection(tsiInfo.loopDirection());
  std::string set = writeTSIDiagHeaderLoopToSet(tsiInfo.loopToSet());
  std::string match = writeTSIDiagHeaderLoopMatch(tsiInfo.loopMatch());
  std::string passedScope = writeTSIDiagHeaderScopeParam(defaultScope);
  std::string scope = writeTSIDiagHeaderScope(tsiInfo.scope());
  std::string procScope = writeTSIDiagHeaderProcScope(processingScope);
  std::string type = writeTSIDiagHeaderType(tsiInfo.type());

  // write initial line
  writeTSIDiagLine(diag);

  // write tsi and description
  diag << "TSI DATA PASSED TO PROCESS" << std::endl << "TSI:" << std::setw(3) << std::right
       << tsiInfo.tsi() << " - " << tsiInfo.description() << std::endl;

  diag << " PASSED SCOPE: " << passedScope << std::endl << " PERMITTED SCOPE OVERRIDES: ";

  //  write override flags
  if (allowJourneyScopeOverride)
    diag << " JRNY-FC";
  if (allowPUScopeOverride)
    diag << " PU-FC";
  if (allowFCScopeOverride)
    diag << " FC-PU";

  // write TSI geo locations
  diag << std::endl << " PROCESSING TSI USING SCOPE: " << procScope << std::endl
       << " GEO TYPE-1: " << locKey1.locType() << " LOCALE-1: " << locKey1.loc()
       << " GEO TYPE-2: " << locKey2.locType() << " LOCALE-2: " << locKey2.loc() << std::endl << " "
       << std::endl;

  // write TSI data
  diag << "TSI DATA FROM TABLE" << std::endl
       << "--------GEO--------  -------LOOP-------  ---SAVE---" << std::endl
       << "REQ NOT OUT ITIN CK  DIR  OFF SET MATCH  SCOPE TYPE" << std::endl << req << "  "
       << tsiInfo.geoNotType() << "  " << out << " " << itin << " " << ck << "  " << dir << "   "
       << tsiInfo.loopOffset() << " " << set << " " << match << "  " << scope << "  " << type
       << std::endl << std::endl;

  // write all match criteria
  diag << "MATCH CRITERIA" << std::endl;

  std::vector<TSIInfo::TSIMatchCriteria>::const_iterator it = tsiInfo.matchCriteria().begin();
  std::vector<TSIInfo::TSIMatchCriteria>::const_iterator ie = tsiInfo.matchCriteria().end();

  for (; it != ie; ++it)
  {
    diag << writeTSIDiagHeaderMatchCriteria(*it);
  }
  // done
  diag << std::endl;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::scopeTSIGeoScopeSetup
//
// Description:
//
// Function identify the TSI scope and override it if required
//
// @param  tsiInfo                    - The TSIInfo structure
// @param  defaultScope               - Default scope for TSI
// @param  allowJourneyScopeOverride  - True if Journey scope TSI allowed to
//                                      override Category scope of FC
// @param  allowPUScopeOverride       - True if PU scope TSI allowed to override
//                                      Category scope of FC
// @param  allowFCScopeOverride       - True if FC scope TSI allowed to override
//                                      Category scope of PU
//
// @return TSI scope
//
// ----------------------------------------------------------------------------
RuleConst::TSIScopeType
RuleUtilTSI::scopeTSIGeoScopeSetup(const TSIInfo* tsiInfo,
                                   const RuleConst::TSIScopeParamType& defaultScope,
                                   const bool allowJourneyScopeOverride,
                                   const bool allowPUScopeOverride,
                                   const bool allowFCScopeOverride)
{
  // get scope from TSIInfo (db)
  RuleConst::TSIScopeType scope = RuleConst::TSIScopeType(tsiInfo->scope());

  if (RuleConst::TSI_SCOPE_JOURNEY == scope)
  {
    if ((RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT == defaultScope) && (!allowJourneyScopeOverride))
    {
      // Journey scope TSI not allowed to override fare component scope
      //  of caller. Use fare component scope to process the TSI.
      scope = RuleConst::TSIScopeType(defaultScope);
    }
  }
  else if (RuleConst::TSI_SCOPE_SUB_JOURNEY == scope)
  {
    if ((RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT == defaultScope) && (!allowPUScopeOverride))
    {
      // Sub journey scope TSI not allowed to override fare component scope
      //  of caller. Use fare component scope to process the TSI.
      scope = RuleConst::TSIScopeType(defaultScope);
    }
  }
  else if (RuleConst::TSI_SCOPE_FARE_COMPONENT == scope)
  {
    if ((RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY == defaultScope) && (!allowFCScopeOverride))
    {
      // Fare component scope TSI not allowed to override sub journey
      //  scope of caller. Use sub journey scope to process the TSI.
      scope = RuleConst::TSIScopeType(defaultScope);
    }
  }
  else if (LIKELY(RuleConst::TSI_SCOPE_SJ_AND_FC == scope))
  {
    // always return default scope
    scope = RuleConst::TSIScopeType(defaultScope);
  }
  return scope;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::scopeTSIGeoScopeValidate
//
// Description:
//
// Function validate if passed parameters are valid for current TSI scope validation
//
// @param  tsi                   - The TSI number
// @param  scope                 - TSI scope
// @param  trx                   - The Transaction object
// @param  farePath              - The Journey
// @param  itin                  - The Journey (if FarePath is not available)
// @param  pricingUnit           - The SubJourney
// @param  fareMarket            - The Fare component
//
// @return true passed arguments are valid
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::scopeTSIGeoScopeValidate(TSICode tsi,
                                      RuleConst::TSIScopeType scope,
                                      PricingTrx& trx,
                                      const FarePath* farePath,
                                      const Itin* itin,
                                      const PricingUnit* pu,
                                      const FareMarket* fm)
{
  // validate Journey scope
  if (RuleConst::TSI_SCOPE_JOURNEY == scope)
  {
    // one of them haveto be filled
    if (!farePath && !itin)
    {
      logDbgMsg(trx, tsi, "FarePath and Itin cannot both be null for Journey scope.");
      return false;
    }
    // if farepath is filled, it should also have Itin
    if (farePath && !farePath->itin())
    {
      logErrMsg(trx, tsi, "FarePath->itin() cannot be null for Journey scope.");
      return false;
    }
  }
  // sub journey
  else if (RuleConst::TSI_SCOPE_SUB_JOURNEY == scope)
  {
    // Pricing unit need to be passed
    if (!pu)
    {
      logDbgMsg(trx, tsi, "PricingUnit is null. Scope = SubJourney.");
      return false;
    }
  }
  // fare market need to be passed
  else if (LIKELY(RuleConst::TSI_SCOPE_FARE_COMPONENT == scope))
  {
    if (UNLIKELY(!fm))
    {
      logDbgMsg(trx, tsi, "FareMarket is null. Scope = Fare.");
      return false;
    }
  }
  return true;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::scopeTSIGeoCheckDMC
//
// Description:
//
// Function check if we should use reversed FareMarket, and if so, then create one
//
// @param  scope                 - The TSI scope
// @param  trx                   - The Transaction object
// @param  pricingUnit           - pricingUnit pointer
// @param  fareMarket            - Faremarket which will reverser
// @param  retFM                 - returned FareMarket
//
// @return true if reversing is not needed or done, if failed then false
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::scopeTSIGeoCheckDMC(RuleConst::TSIScopeType scope,
                                 PricingTrx& trx,
                                 const PricingUnit* pricingUnit,
                                 const FareMarket* fareMarket,
                                 const FareMarket*& retFM)
{
  // set resulting fare market
  retFM = fareMarket;
  // pricing unit must exist and have fareDirectionReversed set
  if (LIKELY(!pricingUnit || !pricingUnit->fareDirectionReversed()))
    return true;

  // scope can be only fare component
  if (RuleConst::TSI_SCOPE_FARE_COMPONENT != scope)
    return true;

  // Reuse allocated memory for most DMC cases (FM scope)
  if (fareMarket->reversedFareMarket())
  {
    retFM = fareMarket->reversedFareMarket();
    return true;
  }

  // create reversed market
  FMScopedLock g((const_cast<FareMarket*>(fareMarket))->fmReversedFMMutex());
  if (!fareMarket->reversedFareMarket())
  {
    FareMarket*& dmcFM = (const_cast<FareMarket*>(fareMarket))->reversedFareMarket();
    trx.dataHandle().get(dmcFM);

    // no memmory allocated - return false
    if (!dmcFM)
      return false;

    dmcFM->setGlobalDirection(fareMarket->getGlobalDirection());
    getReversedTravelSegs(trx.dataHandle(), dmcFM->travelSeg(), fareMarket->travelSeg());
  }
  // save new fare market
  retFM = fareMarket->reversedFareMarket();
  return true;
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::scopeTSIGeoCreateDiag
//
// Description:
//
// Create Geo diagostic if reqired, and write inital top lines
//
// @param  trx                   - The Transaction object
// @param  factory               - reference to DCFactory to be created
// @param  diag                  - reference to the diagnostic to be created
// @param  callerDiag            - Controls diagnostic display based on the
//                                  calling method. If the value does not match
//                                  the requested diagnostic number of the
//                                  transaction, then diagnostic output is
//                                  not produced.
// @param  tsiData               - reference to the TSIData structure
// @param  defaultScope          - Default processing scope
// @param  scope                 - processing scope
// @param  allowJourneyScopeOverride - True if Journey scope TSI allowed to
//                                      override Category scope of FC
// @param  allowPUScopeOverride  - True if PU scope TSI allowed to override
//                                  Category scope of FC
// @param  allowFCScopeOverride  - True if FC scope TSI allowed to override
//                                  Category scope of PU
// @param  locKey1               - GeoData
// @param  locKey2               - GeoData
//
// @return true diagnostic is required and is created
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::scopeTSIGeoCreateDiag(PricingTrx& trx,
                                   DCFactory*& factory,
                                   DiagCollector*& diag,
                                   const DiagnosticTypes& callerDiag,
                                   const TSIData& tsiData,
                                   const RuleConst::TSIScopeParamType& defaultScope,
                                   const RuleConst::TSIScopeType& scope,
                                   const bool allowJourneyScopeOverride,
                                   const bool allowPUScopeOverride,
                                   const bool allowFCScopeOverride,
                                   const LocKey& locKey1,
                                   const LocKey& locKey2)
{
  // get the diag type from transaction
  const DiagnosticTypes diagType = trx.diagnostic().diagnosticType();

  // Check if the diagnostic request included the /DDGEO option.
  if (LIKELY(trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) !=
      RuleConst::DIAGNOSTIC_INCLUDE_GEO))
    return false;

  // diagnostic only for rules diagnostics (300-350), and only if match on caller diag
  if ((callerDiag != DiagnosticNone) && (callerDiag == diagType) && (diagType >= Diagnostic300) &&
      (diagType <= Diagnostic350))
  {
    factory = DCFactory::instance();
    diag = factory->create(trx);

    diag->enable(diagType);

    //
    // Generate the top half of the diagnostic output
    //
    writeTSIDiagHeader(*diag,
                       tsiData,
                       defaultScope,
                       scope,
                       allowJourneyScopeOverride,
                       allowPUScopeOverride,
                       allowFCScopeOverride,
                       locKey1,
                       locKey2);

    writeTSIDiagLine(*diag);

    (*diag) << "ITINERARY LOOP ITEM BREAKDOWN" << std::endl;
    return true;
  }
  return false;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::scopeTSIGeoWriteMatch
//
// Description:
//
// Function writes to diagnostic processing status
//
// @param  diag                  - DiagCollector pointer
// @param  tsiInfo               - reference to the TSIInfo structure
// @param  scope                 - processing scope
// @param  matchFound            - is match found during trabvel segment processing
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::scopeTSIGeoWriteMatch(DiagCollector* diag,
                                   const TSIInfo* tsiInfo,
                                   RuleConst::TSIScopeType scope,
                                   bool matchFound)
{
  // get the loop match
  const RuleConst::TSILoopMatch loopMatch = RuleConst::TSILoopMatch(tsiInfo->loopMatch());

  // create 'FOUND' / 'NOT FOUND' string to apply
  std::string strFound = matchFound ? "FOUND" : "NOT FOUND";
  // main switch
  switch (loopMatch)
  {
  case RuleConst::TSI_MATCH_ALL:
    (*diag) << "ALL ITEMS CHECKED" << std::endl;
    break;

  case RuleConst::TSI_MATCH_FIRST:
    (*diag) << "FIRST MATCH " << strFound << std::endl;
    break;

  case RuleConst::TSI_MATCH_ONCE:
    (*diag) << "ONLY 1 ITEM CHECKED. MATCH " << strFound << std::endl;
    break;

  case RuleConst::TSI_MATCH_LAST:
    (*diag) << "LAST MATCH " << strFound << std::endl;
    break;

  case RuleConst::TSI_MATCH_FIRST_ALL:

    if (RuleConst::TSI_SCOPE_SUB_JOURNEY == scope)
      (*diag) << "ALL ITEMS CHECKED" << std::endl;
    else if (RuleConst::TSI_SCOPE_FARE_COMPONENT == scope)
      (*diag) << "FIRST MATCH " << strFound << std::endl;
    else
      (*diag) << "ERROR!! MATCH-FIRST-ALL NOT ALLOWED WITH JOURNEY SCOPE" << std::endl;
    break;

  case RuleConst::TSI_MATCH_SOFT_MATCH:
    (*diag) << "SOFT MATCH REQUESTED" << std::endl;
    break;

  case RuleConst::TSI_MATCH_SECOND_FIRST:
    (*diag) << "MATCH SECOND FIRST REQUESTED" << std::endl;
    break;
    // no default required
  }
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::scopeTSIGeo
//
// Description:
//
// Identify the travel segments in a Journey (FarePath), Sub Journey
//  (PricingUnit) or Fare Component (FareMarket) that a TSI applies to.
//
// @param  tsi                   - The TSI number
// @param  locKey1               - GeoData
// @param  locKey2               - GeoData
// @param  defaultScope          - Default processing scope
// @param  allowJourneyScopeOverride - True if Journey scope TSI allowed to
//                                      override Category scope of FC
// @param  allowPUScopeOverride  - True if PU scope TSI allowed to override
//                                  Category scope of FC
// @param  allowFCScopeOverride  - True if FC scope TSI allowed to override
//                                  Category scope of PU
// @param  trx                   - The Transaction object
// @param  farePath              - The Journey
// @param  itin                  - The Journey (if FarePath is not available)
// @param  pricingUnit           - The SubJourney
// @param  fareMarket            - The Fare component
// @param  ticketingDate         -
// @param  applTravelSegment     - The matching travel segments
// @param  vendorCode            - Needed for Zone lookup, default = "ATP"
// @param  callerDiag            - Controls diagnostic display based on the
//                                  calling method. If the value does not match
//                                  the requested diagnostic number of the
//                                  transaction, then diagnostic output is
//                                  not produced.
//
// @return true if the TSI processing was successful
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::scopeTSIGeo(const TSICode tsi,
                         const LocKey& locKey1,
                         const LocKey& locKey2,
                         const RuleConst::TSIScopeParamType& defaultScope,
                         const bool allowJourneyScopeOverride,
                         const bool allowPUScopeOverride,
                         const bool allowFCScopeOverride,
                         PricingTrx& trx,
                         const FarePath* farePath,
                         const Itin* itin,
                         const PricingUnit* pu,
                         const FareMarket* fm,
                         const DateTime& ticketingDate,
                         RuleUtilTSI::TravelSegWrapperVector& applTravelSegment,
                         const DiagnosticTypes& callerDiag,
                         const VendorCode& vendorCode,
                         const FareUsage* fareUsage)
{
  LOG4CXX_DEBUG(_tsiLogger, "Entering RuleUtilTSI::scopeTSIGeo()");

  const DiagnosticTypes diagType = trx.diagnostic().diagnosticType();

  DCFactory* factory = nullptr;
  DiagCollector* diag = nullptr;

  bool diagEnabled = false;

  // Clear the output vector
  applTravelSegment.clear();

  // Get the TSIInfo object from the dataHandle
  const TSIInfo* tsiInfo = trx.dataHandle().getTSI(tsi);

  if (UNLIKELY(!tsiInfo))
  {
    logErrMsg(trx, tsi, "Invalid TSI number");
    LOG4CXX_DEBUG(_tsiLogger, "Leaving RuleUtilTSI::scopeTSIGeo()");
    return false;
  }

  //
  // Get the Save Scope (Scope Type) of the TSI
  //
  RuleConst::TSIScopeType scope = scopeTSIGeoScopeSetup(
      tsiInfo, defaultScope, allowJourneyScopeOverride, allowPUScopeOverride, allowFCScopeOverride);

  //
  // Validate the function arguments based on the scope
  //
  if (!RuleUtilTSI::scopeTSIGeoScopeValidate(tsi, scope, trx, farePath, itin, pu, fm))
  {
    LOG4CXX_DEBUG(_tsiLogger, "Leaving RuleUtilTSI::scopeTSIGeo()");
    return false;
  }

  //
  // Create and initialize the TSIData Data Transfer object
  //
  const Itin* itinerary = itin;
  if (farePath && farePath->itin())
    itinerary = farePath->itin();

  const PricingUnit* pricingUnit(pu);
  const FareMarket* fareMarket = fm;

  // For DMC, we may need reversed travel
  // getDMC

  if (UNLIKELY(!scopeTSIGeoCheckDMC(scope, trx, pricingUnit, fm, fareMarket)))
    return false;

  // create TSIData
  TSIData tsiData(*tsiInfo, scope, vendorCode, farePath, itinerary, pricingUnit, fareMarket);

  //
  // Setup the GeoData for the TSI.
  //
  if (!getGeoData(trx, tsiData, locKey1, locKey2))
  {
    logErrMsg(trx, tsi, "Error getting GeoData.");
    LOG4CXX_DEBUG(_tsiLogger, "Leaving RuleUtilTSI::scopeTSIGeo()");
    return false;
  }
  //
  // Setup the map of TravelSegMarkup objects. This must be done prior
  //  to processing the travel segs for TSI matches.
  //
  TSITravelSegMarkupContainer tsMarkup;

  if (UNLIKELY(!setupTravelSegMarkup(trx, tsiData, fareUsage, tsMarkup)))
  {
    logErrMsg(trx, tsi, "setupTravelSegMarkup returned false.");
    LOG4CXX_DEBUG(_tsiLogger, "Leaving RuleUtilTSI::scopeTSIGeo()");
    return false;
  }

  //
  // Validate the save type
  //
  RuleConst::TSIApplicationType saveType = RuleConst::TSIApplicationType(tsiInfo->type());

  if (UNLIKELY(saveType != RuleConst::TSI_APP_CHECK_ORIG && saveType != RuleConst::TSI_APP_CHECK_DEST &&
      saveType != RuleConst::TSI_APP_CHECK_ORIG_DEST))
  {
    logErrMsg(trx, tsi, "Bad TSI record. Unrecognized saveType (type).");
    LOG4CXX_DEBUG(_tsiLogger, "Leaving RuleUtilTSI::scopeTSIGeo()");
    return false;
  }

  //

  // Check for Diagnostics 300 thru 350 then initialize the
  //  DCFactory and DiagCollector
  //
  diagEnabled = scopeTSIGeoCreateDiag(trx,
                                      factory,
                                      diag,
                                      callerDiag,
                                      tsiData,
                                      defaultScope,
                                      scope,
                                      allowJourneyScopeOverride,
                                      allowPUScopeOverride,
                                      allowFCScopeOverride,
                                      locKey1,
                                      locKey2);

  identifyIntlDomTransfers(tsiData, tsMarkup);

  bool locSpecified = ((LOCTYPE_NONE != locKey1.locType()) || (LOCTYPE_NONE != locKey2.locType()));

  //
  // Now process the travel segs for geo and criteria matches
  //
  if (UNLIKELY(!processTravelSegs(trx, tsiData, applTravelSegment, tsMarkup, locSpecified)))
  {
    logErrMsg(trx, tsi, "Error processing data.");
    LOG4CXX_DEBUG(_tsiLogger, "Leaving RuleUtilTSI::scopeTSIGeo()");
    // without diag created here would be seg fault??
    if (UNLIKELY(diagEnabled))
    {
      (*diag) << "**ERROR PROCESSING TRAVEL SEGMENTS**" << std::endl;
      diag->flushMsg();
      diag->disable(diagType);
    }
    return false;
  }

  if (UNLIKELY(diagEnabled))
  {
    writeTSIDiagDetails(*diag, tsiData, tsMarkup);
    scopeTSIGeoWriteMatch(diag, tsiInfo, scope, !applTravelSegment.empty());
    writeTSIDiagLine(*diag);

    diag->flushMsg();
    diag->disable(diagType);
  }

  LOG4CXX_DEBUG(_tsiLogger, "Leaving RuleUtilTSI::scopeTSIGeo()");

  return true;
}

// ----------------------------------------------------------------------------
//
// @function const std::vector<const TravelSeg*>& RuleUtilTSI::TSISetup::travelSeg
//
// Description:
//
// function return reference to travel segment vector for correct scope
//
// @param  scope                - TSI scope
// @param  tsiData              - TSIData sturcture reference
//
// @return const reference of travel segment vector
//
// ----------------------------------------------------------------------------
const std::vector<TravelSeg*>&
RuleUtilTSI::TSISetup::travelSeg(const RuleConst::TSIScopeType& scope, const TSIData& tsiData)
{
  // for journey use travel segments from itin
  if (RuleConst::TSI_SCOPE_JOURNEY == scope)
  {
    return tsiData.itin()->travelSeg();
  }
  // forsub journey use segments from pricing unit
  else if (RuleConst::TSI_SCOPE_SUB_JOURNEY == scope)
  {
    return tsiData.pricingUnit()->travelSeg();
  }
  // if not journey or subjourney then fare component - use from fare market
  return tsiData.fareMarket()->travelSeg();
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::TSISetupTurnAround::build
//
// Description:
//
// function build TSISetup used for setting TSISegmentMarkup vector
//
// @param  scope                - TSI scope
// @param  errMsg               - error message whisch is returned if build failed
// @param  tsiData              - TSIData sturcture reference
// @param  loopForward          - is loop direction forward
// @param  trx                  - PricingTrx reference
//
// @return nothing
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::TSISetupTurnAround::build(const RuleConst::TSIScopeType& scope,
                                       std::string& errMsg,
                                       const TSIData& tsiData,
                                       bool loopForward,
                                       PricingTrx& trx)
{
  // main switch
  if (RuleConst::TSI_SCOPE_JOURNEY == scope)
  {
    // Find the turn around point for the Journey. This is defined as
    // the furthest geographical point from the journey origin.
    // furthest point segment order start from 1 then -1 to get to
    // the right travel in the travelseg vector
    int16_t furthestPointSegmentOrder = tsiData.itin()->furthestPointSegmentOrder();

    _turnAroundPoint = nullptr;
    if (furthestPointSegmentOrder > 0)
      _turnAroundPoint = tsiData.itin()->travelSeg()[furthestPointSegmentOrder - 1];

    if (furthestPointSegmentOrder > 1 && _turnAroundPoint &&
        _turnAroundPoint->origin() ==
            (tsiData.itin()->travelSeg()[furthestPointSegmentOrder - 2])->destination())
    {
      _turnAroundPointAtDest = tsiData.itin()->travelSeg()[furthestPointSegmentOrder - 2];
    }
  }
  else if (RuleConst::TSI_SCOPE_SUB_JOURNEY == scope)
  {
    // Get the turnaround point for the Pricing Unit (Sub Journey)
    //
    _turnAroundPoint = tsiData.pricingUnit()->turnAroundPoint();
    if (_turnAroundPoint)
    {
      std::vector<TravelSeg*>::const_iterator it = tsiData.pricingUnit()->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator ite = tsiData.pricingUnit()->travelSeg().end();
      std::vector<TravelSeg*>::const_iterator itPrev = it;
      for (; it != ite; ++it)
      {
        // if match on turn around
        if (*it == _turnAroundPoint)
        {
          if (LIKELY(itPrev != it))
          {
            const Loc* loc1 = (*it)->origin();
            const Loc* loc2 = (*itPrev)->destination();

            bool turnAround = (tsiData.pricingUnit()->puType() == PricingUnit::Type::OPENJAW) ||
                              LocUtil::isSameCity(loc1->loc(), loc2->loc(), trx.dataHandle());

            if (LIKELY(turnAround))
              _turnAroundPointAtDest = *itPrev;
          }

          break;
        }
        itPrev = it;
      }
    }
  }
  else if (LIKELY(RuleConst::TSI_SCOPE_FARE_COMPONENT == scope))
  {
    // do nothing
  }
  // always success
  return true;
}

// ----------------------------------------------------------------------------
//
// @function void RuleUtilTSI::TSISetupTurnAround::process
//
// Description:
//
// function check TravelSegment and set correct flag in TSITravelSegMarkup
//
// @param  tvlSeg               - travel segment pointer
// @param  tsMarkup             - reference to travel seggment markup
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::TSISetupTurnAround::process(const TravelSeg* tvlSeg, TSITravelSegMarkup& tsMarkup)
{
  // check if this is the turnaround segment, if so  set flag
  if (tvlSeg == _turnAroundPoint)
    tsMarkup.isTurnAroundPoint() = true;
  // check if this is turnaroundatdes segment, if so set flag
  else if (tvlSeg == _turnAroundPointAtDest)
    tsMarkup.destIsTurnAroundPoint() = true;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::TSISetupGateway::build
//
// Description:
//
// function build TSISetup used for setting TSISegmentMarkup vector
//
// @param  scope                - TSI scope
// @param  errMsg               - error message whisch is returned if build failed
// @param  tsiData              - TSIData sturcture reference
// @param  loopForward          - is loop direction forward
// @param  trx                  - PricingTrx reference
//
// @return nothing
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::TSISetupGateway::build(const RuleConst::TSIScopeType& scope,
                                    std::string& errMsg,
                                    const TSIData& tsiData,
                                    bool loopForward,
                                    PricingTrx& trx)
{
  // get match crietaria
  const std::vector<TSIInfo::TSIMatchCriteria>& mc = tsiData.tsiInfo().matchCriteria();
  TSIGateway::MarkGWType markGWType = TSIGateway::MARK_NONE;

  //
  // TODO: is criterai ORIG_GATEWAY & DEST_GATEWAY together possible????
  //
  if (find(mc.begin(), mc.end(), TSIInfo::GATEWAY) != mc.end())
    markGWType = TSIGateway::MARK_ALL_GATEWAY;
  else if (find(mc.begin(), mc.end(), TSIInfo::ORIG_GATEWAY) != mc.end())
    markGWType = TSIGateway::MARK_ORIG_GATEWAY;
  else if (find(mc.begin(), mc.end(), TSIInfo::DEST_GATEWAY) != mc.end())
    markGWType = TSIGateway::MARK_DEST_GATEWAY;

  // should always be true
  if (LIKELY(markGWType != TSIGateway::MARK_NONE))
  {
    // switch all scopes
    if (UNLIKELY(RuleConst::TSI_SCOPE_JOURNEY == scope))
    {
      // @NOTE: We cannot find the gateways for the journey because
      //         we cannot always positively identify a single location
      //         as the destination of the journey. At this time there
      //         are no TSIs that use gateway with journey scope so
      //         we will never get here.
      errMsg = "GATEWAY not supported with scope=JOURNEY";
      return false;
    }
    else if (RuleConst::TSI_SCOPE_SUB_JOURNEY == scope)
    {
      std::vector<TravelSeg*> outSegs;
      std::vector<TravelSeg*> inSegs;
      collectOutInTvl(*tsiData.pricingUnit(), outSegs, inSegs);

      if (RtwUtil::isRtw(trx))
        _foundGateways = _tsiGW->markGwRtw(markGWType, outSegs);
      else
        _foundGateways = _tsiGW->markGW(markGWType, outSegs, inSegs);
    }
    else if (LIKELY(RuleConst::TSI_SCOPE_FARE_COMPONENT == scope))
    {
      if (UNLIKELY(RtwUtil::isRtw(trx)))
        _foundGateways = _tsiGW->markGwRtw(markGWType, tsiData.fareMarket()->travelSeg());
      else
        _foundGateways = _tsiGW->markGW(markGWType, tsiData.fareMarket()->travelSeg());
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
//
// @function void RuleUtilTSI::TSISetupGateway::process
//
// Description:
//
// function check TravelSegment and set correct flag in TSITravelSegMarkup
//
// @param  tvlSeg               - travel segment pointer
// @param  tsMarkup             - reference to travel seggment markup
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::TSISetupGateway::process(const TravelSeg* tvlSeg, TSITravelSegMarkup& tsMarkup)
{
  // if gateways found only
  if (!_foundGateways)
    return;

  // check if this is departure gateway segment
  if (_tsiGW->isDepartureFromGW(tvlSeg))
  {
    tsMarkup.departsOrigGateway() = true;
  }
  // check if this is arrival gateway segment
  if (_tsiGW->isArrivalOnGW(tvlSeg))
  {
    tsMarkup.arrivesDestGateway() = true;
  }
}
// ----------------------------------------------------------------------------
//
// @function void RuleUtilTSI::TSISetupGateway::collectOutInTvl
//
// Description:
//
// function check TravelSegment and set correct flag in TSITravelSegMarkup
//
// @param  pu                   - PricingUnit refference
// @param  outSegs              - returned vector of outbound travel segments
// @param  inSegs               - returned vector of inbound travel segments
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::TSISetupGateway::collectOutInTvl(const PricingUnit& pu,
                                              std::vector<TravelSeg*>& outSegs,
                                              std::vector<TravelSeg*>& inSegs)
{
  // get the fare usage vector
  const std::vector<FareUsage*>& puFu = pu.fareUsage();

  std::vector<FareUsage*>::const_iterator fuIter = puFu.begin();
  std::vector<FareUsage*>::const_iterator fuIterEnd = puFu.end();

  // for each fareusage
  for (; fuIter != fuIterEnd; ++fuIter)
  {
    const FareUsage* fu = *fuIter;
    const std::vector<TravelSeg*>& fuTs = fu->travelSeg();

    // if outbound the into outbound vector
    if (fu->isOutbound())
    {
      outSegs.insert(outSegs.end(), fuTs.begin(), fuTs.end());
    }
    // inbound into inbound vector
    else
    {
      inSegs.insert(inSegs.end(), fuTs.begin(), fuTs.end());
    }
  }
}

// ----------------------------------------------------------------------------
//
// @function void RuleUtilTSI::TSISetupGateway::initialize
//
// Description:
//
// function initialize TSIGAteway parameter of RuleUtilTSI::TSISetupGateway class
//   this is done in this place to avoid referencing TSIGateway class in header
//
// @param  trx                  - PricingTrx reference
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::TSISetupGateway::initialize(PricingTrx& trx)
{
  trx.dataHandle().get(_tsiGW);
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::TSISetupOverWater::build
//
// Description:
//
// function build TSISetup used for setting TSISegmentMarkup vector
//
// @param  scope                - TSI scope
// @param  errMsg               - error message whisch is returned if build failed
// @param  tsiData              - TSIData sturcture reference
// @param  loopForward          - is loop direction forward
// @param  trx                  - PricingTrx reference
//
// @return nothing
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::TSISetupOverWater::build(const RuleConst::TSIScopeType& scope,
                                      std::string& errMsg,
                                      const TSIData& tsiData,
                                      bool loopForward,
                                      PricingTrx& trx)
{
  // local vector
  std::vector<TravelSeg*> overWaterSegs;

  // get the vecot of travel segments for current scope and pass it
  // to LocUtil::getOverWaterSegments
  LocUtil::getOverWaterSegments(travelSeg(scope, tsiData), overWaterSegs);

  std::vector<TravelSeg*>::const_iterator owIter = overWaterSegs.begin();
  std::vector<TravelSeg*>::const_iterator owIterEnd = overWaterSegs.end();

  // copy data to result set
  for (; owIter != owIterEnd; ++owIter)
  {
    _overWaterSegments.insert(*owIter);
  }
  return true;
}

// ----------------------------------------------------------------------------
//
// @function void RuleUtilTSI::TSISetupOverWater::process
//
// Description:
//
// function check TravelSegment and set correct flag in TSITravelSegMarkup
//
// @param  tvlSeg               - travel segment pointer
// @param  tsMarkup             - reference to travel seggment markup
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::TSISetupOverWater::process(const TravelSeg* tvlSeg, TSITravelSegMarkup& tsMarkup)
{
  // if find segment between travel segments, mark this markup
  if (_overWaterSegments.find(tvlSeg) != _overWaterSegments.end())
    tsMarkup.isOverWater() = true;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::TSISetupInternational::build
//
// Description:
//
// function build TSISetup used for setting TSISegmentMarkup vector
//
// @param  scope                - TSI scope
// @param  errMsg               - error message whisch is returned if build failed
// @param  tsiData              - TSIData sturcture reference
// @param  loopForward          - is loop direction forward
// @param  trx                  - PricingTrx reference
//
// @return nothing
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::TSISetupInternational::build(const RuleConst::TSIScopeType& scope,
                                          std::string& errMsg,
                                          const TSIData& tsiData,
                                          bool loopForward,
                                          PricingTrx& trx)
{
  // Find the international segments
  //
  std::vector<TravelSeg*> intlSegs;

  // get match criterai
  const std::vector<TSIInfo::TSIMatchCriteria>& mc = tsiData.tsiInfo().matchCriteria();

  // get the travel seg vector
  const std::vector<TravelSeg*>& tsVec = travelSeg(scope, tsiData);

  if (tsiData.tsiInfo().tsi() == 18)
  {
    getInternationalSegmentsForTSI18(tsVec, intlSegs);
  }
  else if (std::find(mc.begin(), mc.end(), TSIInfo::STOP_OVER) != mc.end())
  {
    // The definition of International is different for Stopover TSIs.
    //
    if (LIKELY(!tsVec.empty()))
    {
      const Loc* referenceLoc = loopForward ? tsVec.front()->origin() : tsVec.back()->destination();

      // get the segments with stopover
      getInternationalSegmentsForStopoverTSI(*referenceLoc, tsVec, intlSegs);
    }
  }
  else
  {
    LocUtil::getInternationalSegments(tsVec, intlSegs);
  }

  std::vector<TravelSeg*>::const_iterator intIter = intlSegs.begin();
  std::vector<TravelSeg*>::const_iterator intIterEnd = intlSegs.end();

  for (; intIter != intIterEnd; ++intIter)
  {
    TravelSeg* ts = *intIter;

    // Travel between Russia and East Ural Russia is not considered
    //  international for TSI validation.
    //
    if (((ts->origin()->nation() == NATION_USSR) && (ts->destination()->nation() == NATION_EUR)) ||
        ((ts->origin()->nation() == NATION_EUR) && (ts->destination()->nation() == NATION_USSR)))
    {
      continue;
    }
    _intlSegments.insert(ts);
  }
  return true;
}

// ----------------------------------------------------------------------------
//
// @function void RuleUtilTSI::TSISetupInternational::process
//
// Description:
//
// function check TravelSegment and set correct flag in TSITravelSegMarkup
//
// @param  tvlSeg               - travel segment pointer
// @param  tsMarkup             - reference to travel seggment markup
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::TSISetupInternational::process(const TravelSeg* tvlSeg, TSITravelSegMarkup& tsMarkup)
{
  // if found segment among international segments, mark this markup as international
  if (_intlSegments.find(tvlSeg) != _intlSegments.end())
  {
    tsMarkup.isInternational() = true;
  }
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::TSISetupInternational::getInternationalSegmentsForTSI18
//
// Description:
//
// function get selects vectorof international segments from passed vector of
// segments for TSI18
//
// @param  inSeg                  - vector of segments from which International
//                                  segments will be selected
// @param  outSeg                - returned vector of international segments
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::TSISetupInternational::getInternationalSegmentsForTSI18(
    const std::vector<TravelSeg*>& inSeg, std::vector<TravelSeg*>& outSeg)
{
  std::vector<TravelSeg*>::const_iterator it = inSeg.begin();
  std::vector<TravelSeg*>::const_iterator ite = inSeg.end();
  for (; it != ite; ++it)
  {
    if ((*it)->origin()->nation() != (*it)->destination()->nation())
      outSeg.push_back(*it);
  }
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::TSISetupInternational::getInternationalSegmentsForStopoverTSI
//
// Description:
//
// function get selects vectorof international segments from passed vector of segments
//
// @param  referenceLoc           - referencing location used to check the segment
// @param  inSeg                  - vector of segments from which International
//                                  segments will be selected
// @param  LocUtil                - returned vector of international segments
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::TSISetupInternational::getInternationalSegmentsForStopoverTSI(
    const Loc& referenceLoc, const std::vector<TravelSeg*>& inSeg, std::vector<TravelSeg*>& outSeg)
{
  // For travel between US and Canada only, US <-> Canada is international <- copied from LocUtil
  //

  std::vector<TravelSeg*>::const_iterator it = inSeg.begin();
  std::vector<TravelSeg*>::const_iterator ite = inSeg.end();

  // check all segments to see if whole travel is within USCA
  bool isUSCanada = true;

  for (; it != ite; ++it)
  {
    const Loc* orig = (*it)->origin();
    const Loc* dest = (*it)->destination();

    if ((!LocUtil::isDomestic(*orig, *dest)) && (!LocUtil::isTransBorder(*orig, *dest)))
    {
      isUSCanada = false;
      break;
    }
  }
  // for TSI validation when travel is within USCA, there are no international segments
  if (UNLIKELY(isUSCanada))
    return;

  // if travel is outside USCA, even segments USCA are threded ad International
  for (it = inSeg.begin(); it != ite; ++it)
  {
    const Loc* dest = (*it)->destination();
    if (referenceLoc.nation() != dest->nation())
      //       && ( ! isUSCanada || ! LocUtil::isTransBorder(referenceLoc, *dest) ) )
      outSeg.push_back(*it);
  }
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::TSISetupChain::buildTSIChain
//
// Description:
//
// function goes trureu all travel segment markups, and mark stopover when needed
//
// @param  trx                    - PricingTrx reference
// @param  tsi                    - TSIData structure reference
// @param  loopForward            - is loop forward set
// @param  checkFareBreaksOnly    - is check only fare breaks
//
// @return true if setup chain si correctly set
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::TSISetupChain::buildTSIChain(PricingTrx& trx,
                                          const TSIData& tsi,
                                          bool loopForward,
                                          bool checkFareBreaksOnly)
{
  // clear returning vecotor
  _setupChain.clear();

  _checkFareBreaksOnly = checkFareBreaksOnly;

  // save geo check
  const RuleConst::TSIApplicationType geoCheck =
      RuleConst::TSIApplicationType(tsi.tsiInfo().geoCheck());
  // set origin for CHECK_ORIG or CHECK_ORIG_DEST
  _checkOrig = (RuleConst::TSI_APP_CHECK_ORIG == geoCheck) ||
               (RuleConst::TSI_APP_CHECK_ORIG_DEST == geoCheck);
  // set destination for CHECK_DEST or CHECK_ORIG_DEST
  _checkDest = (RuleConst::TSI_APP_CHECK_DEST == geoCheck) ||
               (RuleConst::TSI_APP_CHECK_ORIG_DEST == geoCheck);

  // turn around is always in vector
  TSISetupTurnAround* tsiTurn = nullptr;
  trx.dataHandle().get(tsiTurn);
  _setupChain.push_back(tsiTurn);

  // get the match criteria vector
  const std::vector<TSIInfo::TSIMatchCriteria>& mc = tsi.tsiInfo().matchCriteria();

  // check if need gateway
  if (find(mc.begin(), mc.end(), TSIInfo::GATEWAY) != mc.end() ||
      find(mc.begin(), mc.end(), TSIInfo::ORIG_GATEWAY) != mc.end() ||
      find(mc.begin(), mc.end(), TSIInfo::DEST_GATEWAY) != mc.end())
  {
    TSISetupGateway* tsiGtw = nullptr;
    trx.dataHandle().get(tsiGtw);
    _setupChain.push_back(tsiGtw);
  }

  // check if need over water
  if (find(mc.begin(), mc.end(), TSIInfo::OVER_WATER) != mc.end())
  {
    TSISetupOverWater* tsiOvw = nullptr;
    trx.dataHandle().get(tsiOvw);
    _setupChain.push_back(tsiOvw);
  }

  // check if need international
  if ((find(mc.begin(), mc.end(), TSIInfo::INTERNATIONAL) != mc.end()) ||
      (find(mc.begin(), mc.end(), TSIInfo::INTL_DOM_TRANSFER) != mc.end()))
  {
    TSISetupInternational* tsiInt = nullptr;
    trx.dataHandle().get(tsiInt);
    _setupChain.push_back(tsiInt);
  }

  // for each TSISetup item call build method
  std::vector<TSISetup*>::iterator it = _setupChain.begin();
  std::vector<TSISetup*>::iterator ie = _setupChain.end();

  const RuleConst::TSIScopeType& scope = tsi.scope();

  for (; it != ie; it++)
  {
    // initialize each eelement
    (*it)->initialize(trx);
    // if failed to build element, then fail whole function
    if (UNLIKELY(!(*it)->build(scope, _errMsg, tsi, loopForward, trx)))
      return false;
  }
  // if get here, then all items are builded
  return true;
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::TSISetupChain::passCheckFareBreakOnly
//
// Description:
//
// function check if travel segment markup is passing fare brek
//
// @param  tsm                    - TSITravelSegMarkup reference
//
// @return true segment markup pass fare break checking
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::TSISetupChain::passCheckFareBreakOnly(TSITravelSegMarkup& tsm)
{
  // if look for fare breaks, and no match on orig or destination, then return false
  if (_checkFareBreaksOnly &&
      !((_checkOrig && tsm.fareBreakAtOrigin()) || (_checkDest && tsm.fareBreakAtDestination())))
  {
    // set error msg
    tsm.noMatchReason() = "NOT FARE-BREAK";
    return false;
  }
  return true;
}
// ----------------------------------------------------------------------------
//
// @function void RuleUtilTSI::TSISetupChain::process
//
// Description:
//
// function goes trure all setup elements and call process on passed travel segment
//
// @param  seg                    - TravelSeg reference
// @param  tsm                    - TSITravelSegMarkup structure reference
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::TSISetupChain::process(const TravelSeg* seg, TSITravelSegMarkup& tsm)
{
  // creat helper object
  TSISetupProcess tsiProc(seg, tsm);
  // and use std::for_each
  for_each(_setupChain.begin(), _setupChain.end(), tsiProc);
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::setupTravelSegMarkupSetFurthest
//
// Description:
//
// function goes trureu all travel segment markups, and mark furthest when needed
//
// @param  scope                  - TSI scope
// @param  tsiData                - TSIData structure reference
// @param  stopoverExists         - was stopovers found during setup
// @param  tsMarkup               - TSI travel segment markup container
//
// @return true if matched on scope
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::setupTravelSegMarkupSetFurthest(const RuleConst::TSIScopeType& scope,
                                             const TSIData& tsiData,
                                             bool stopoverExists,
                                             TSITravelSegMarkupContainer& tsMarkup)
{
  // Mark the stopover point that is furthest (in mileage) from the Journey/PU
  //  origin. If no stopover points exist, then use fare breaks.
  //
  // only for Journey and sub journey
  if (scope != RuleConst::TSI_SCOPE_JOURNEY && scope != RuleConst::TSI_SCOPE_SUB_JOURNEY)
    return false;

  // is furthest checking needed
  const std::vector<TSIInfo::TSIMatchCriteria>& mc = tsiData.tsiInfo().matchCriteria();

  if (LIKELY(find(mc.begin(), mc.end(), TSIInfo::FURTHEST) == mc.end()))
    return true;

  TSITravelSegMarkupI iter = tsMarkup.begin();
  TSITravelSegMarkupI iterEnd = tsMarkup.end();
  TSITravelSegMarkupI iterFurthest = tsMarkup.end();

  // get the orig from itin or PU
  const Loc* orig = (RuleConst::TSI_SCOPE_JOURNEY == scope)
                        ? tsiData.itin()->travelSeg().front()->origin()
                        : tsiData.pricingUnit()->travelSeg().front()->origin();

  int32_t furthestMileage = 0;

  for (; iter != iterEnd; ++iter)
  {
    TSITravelSegMarkup& tsm = *iter;
    TravelSeg* ts = tsm.travelSeg();

    // check for stopovers
    if (stopoverExists)
    {
      if (!tsm.isStopover())
        continue;
    }
    // if no stopovers, check for fare breaks
    else
    {
      if (!tsm.fareBreakAtDestination())
        continue;
    }
    // get distance between locations
    int32_t mileage = TseUtil::greatCircleMiles(*orig, *(ts->destination()));

    if (mileage > furthestMileage)
    {
      iterFurthest = iter;
      furthestMileage = mileage;
    }
  }
  if (iterFurthest != tsMarkup.end())
  {
    (*iterFurthest).isFurthest() = true;
  }

  return true;
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::setupTravelSegMarkupSetStopover
//
// Description:
//
// function goes trureu all travel segment markups, and mark stopover when needed
//
// @param  scope                  - TSI scope
// @param  tsiData                - TSIData structure reference
// @param  stopoverExists         - was stopovers found during setup
// @param  tsMarkup               - TSI travel segment markup container
//
// @return true if matched on scope
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::setupTravelSegMarkupSetStopover(const RuleConst::TSIScopeType& scope,
                                             const TSIData& tsiData,
                                             const FareUsage* fareUsage,
                                             bool stopoverExists,
                                             TSITravelSegMarkupContainer& tsMarkup,
                                             const PricingTrx&  trx)
{
  // If the TSI has STOPOVER as a match criteria, make sure the
  //  Journey/PU has a stopover. If no stopover exists, then mark fare
  //  breaks as stopovers if a transfer occurs at the fare break.
  //
  // only for Journey and sub journey
  if (scope != RuleConst::TSI_SCOPE_JOURNEY && scope != RuleConst::TSI_SCOPE_SUB_JOURNEY)
    return false;

  // is stopover checking needed
  const std::vector<TSIInfo::TSIMatchCriteria>& mc = tsiData.tsiInfo().matchCriteria();

  if (stopoverExists || (find(mc.begin(), mc.end(), TSIInfo::STOP_OVER) == mc.end()))
    return true;

  TSITravelSegMarkupI iter = tsMarkup.begin();
  TSITravelSegMarkupI iterEnd = tsMarkup.end();

  for (; iter != iterEnd; ++iter)
  {
    TSITravelSegMarkup& tsm = *iter;
    TravelSeg* ts = tsm.travelSeg();
    TravelSeg* nextTs = tsm.nextTravelSeg();
    
    // APO-36040 SHM-1582: Every ctpu or a rtpu will have a turnaround point.
    // a turnaround point will qualify as a stopover in TSI processing.
    // So there is no need to set a transfer as a stopover here.

    if ( (fallback::apo36040Cat6TSI5Check(&trx)) )
    {
       if (tsm.fareBreakAtDestination() && ts && nextTs)
       {
          AirSeg* as = dynamic_cast<AirSeg*>(ts);
          AirSeg* nextAs = dynamic_cast<AirSeg*>(nextTs);

          if (as)
          {
             // if no next air seg, or carriers are different, or flight noubers are different
             if (!nextAs || (as->carrier() != nextAs->carrier()) ||
                  (as->flightNumber() != nextAs->flightNumber()))
             {
                tsm.isStopover() = true;
             }
          }
       }
    } //fallback
    // only for subjourney
    if (LIKELY(RuleConst::TSI_SCOPE_SUB_JOURNEY == scope))
    {
      if (!tsm.isStopover() && nextTs)
      {
        // check, if next travel segment is surface - or stopover
        // if considered as part of Pricing Unit (not whole journey)

        if (fallback::fixed::APO29538_StopoverMinStay())
        {
          tsm.isStopover() = nextTs->segmentType() == Arunk ||
                             nextTs->isStopOver(ts, tsiData.itin()->geoTravelType());
        }
        else
        {
          if (fareUsage && tsiData.itin())
          {
            tsm.isStopover() = (nextTs->segmentType() == Arunk) ||
                               nextTs->isStopOver(ts,
                                                  tsiData.itin()->geoTravelType(),
                                                  fareUsage->stopoverMinTime());
          }
        }
      }
    }
  }
  return true;
}

void
RuleUtilTSI::setupTravelSegMarkupRtwPostprocess(const PricingTrx& trx,
                                                const TSIData& tsiData,
                                                TSITravelSegMarkupContainer& tsiMarkups)
{
  if (LIKELY(!trx.getOptions() || !trx.getOptions()->isRtw() || !tsiData.itin()))
    return;

  TSITravelSegMarkupContainer::iterator tsiMarkupIt = tsiMarkups.begin();
  TSITravelSegMarkupContainer::iterator tsiMarkupEnd = tsiMarkups.end();

  for (; tsiMarkupIt != tsiMarkupEnd; ++tsiMarkupIt)
  {
    TSITravelSegMarkup& tsiMarkup = *tsiMarkupIt;
    const TravelSeg* ts = tsiMarkup.travelSeg();
    const bool isInbound = tsiData.itin()->segmentOrder(ts) >
                           tsiData.itin()->furthestPointSegmentOrder();

    setupTravelSegMarkupSetDirection(tsiMarkup, isInbound);
  }
}

// ----------------------------------------------------------------------------
//
// @function RuleUtilTSI::TSITravelSegMarkup& RuleUtilTSI::setupTravelSegMarkupCreateSegFM
//
// Description:
//
// function creates new TSITravelSegMarkup, put it in the container, and initialize some parameters
//
// @param  tsMarkup               - TSI markup container
// @param  ts                     - Travel segment pointer, whihc is used for iniltaization
// @param  loopForward            - loop direction
// @param  fm                     - FareMarket pointer used for initailzaition (can be null)
//
// @return reference to the created Travel segment markup
//
// ----------------------------------------------------------------------------
RuleUtilTSI::TSITravelSegMarkup&
RuleUtilTSI::setupTravelSegMarkupCreateSegFM(TSITravelSegMarkupContainer& tsMarkup,
                                             TravelSeg* ts,
                                             TravelSeg* nextTs,
                                             GeoTravelType geoTvlType,
                                             bool loopForward,
                                             const FareMarket* fm,
                                             const FareUsage* fareUsage)
{
  // create new segment markup, and initialize it
  TSITravelSegMarkup t;
  t.travelSeg() = ts;
  t.noMatchReason() = "NOT CHECKED";

  // if fare market si passed, use faremarket global direction
  if (fm)
    t.globalDirection() = fm->getGlobalDirection();

  if (fallback::fixed::APO29538_StopoverMinStay())
  {
    // check only if not forced connection
    if (LIKELY(!ts->isForcedConx()))
    {
      // set stopover
      t.isStopover() = ts->stopOver() || ts->isForcedStopOver();
      // if not stopover, and fare market exists, check if there is segment in stopoovers
      if (!t.isStopover() && fm)
      {
        if (UNLIKELY(find(fm->stopOverTravelSeg().begin(), fm->stopOverTravelSeg().end(), ts) !=
            fm->stopOverTravelSeg().end()))
          t.isStopover() = true;
      }
    }
  }
  else
  {
    if (LIKELY(!ts->isForcedConx()))
    {
      if (nextTs && fareUsage)
      {
        t.isStopover() = ts->isStopOver(nextTs, geoTvlType, fareUsage->stopoverMinTime()) ||
                         ts->isForcedStopOver();
      }
      else
      {
        // Use dafault logic, without overridden SO time values
        t.isStopover() = ts->stopOver() || ts->isForcedStopOver();

        if (!t.isStopover() && fm && !fareUsage)
        {
          if (find(fm->stopOverTravelSeg().begin(), fm->stopOverTravelSeg().end(), ts) !=
              fm->stopOverTravelSeg().end())
          {
            t.isStopover() = true;
          }
        }
      }
    }
  }

  // set correct nexttravel segment, and put in correct place in container
  if (loopForward)
  {
    if (!tsMarkup.empty())
      tsMarkup.back().nextTravelSeg() = ts;

    tsMarkup.push_back(t);
  }
  else
  {
    if (!tsMarkup.empty())
      t.nextTravelSeg() = tsMarkup.front().travelSeg();

    tsMarkup.push_front(t);
  }
  // return reference to created element
  return loopForward ? tsMarkup.back() : tsMarkup.front();
}
// ----------------------------------------------------------------------------
//
// @function RuleUtilTSI::TSITravelSegMarkup& RuleUtilTSI::setupTravelSegMarkupCreateSegPTF
//
// Description:
//
// function creates new TSITravelSegMarkup, put it in the container, and initialize some parameters
//
// @param  tsMarkup               - TSI markup container
// @param  ts                     - Travel segment pointer, whihc is used for iniltaization
// @param  loopForward            - loop direction
// @param  fm                     - PaxTypeFare pointer used for initailzaition (can be null)
//
// @return reference to the created Travel segment markup
//
// ----------------------------------------------------------------------------
RuleUtilTSI::TSITravelSegMarkup&
RuleUtilTSI::setupTravelSegMarkupCreateSegPTF(TSITravelSegMarkupContainer& tsMarkup,
                                              TravelSeg* ts,
                                              TravelSeg* nextTs,
                                              GeoTravelType geoTvlType,
                                              bool loopForward,
                                              const PaxTypeFare* ptf,
                                              const FareUsage* fareUsage)
{
  // check if PaxTypeFare is present
  if (LIKELY(ptf))
  {
    // if we have PaxTypeFare, then we can pass FareMarket
    TSITravelSegMarkup& t = setupTravelSegMarkupCreateSegFM(
        tsMarkup, ts, nextTs, geoTvlType, loopForward, ptf->fareMarket(), fareUsage);
    // override global Direction
    t.globalDirection() = ptf->globalDirection();
    return t;
  }
  // if no pax type fare is passed, then the same as without FareMarket
  return setupTravelSegMarkupCreateSegFM(
      tsMarkup, ts, nextTs, geoTvlType, loopForward, nullptr, fareUsage);
}
// ----------------------------------------------------------------------------
//
// @function void RuleUtilTSI::setupTravelSegMarkupSetDirection
//
// Description:
//
// function set direction in Travel Segment markup eleement
//
// @param  tsm                  - reference to TSITravelSegMarkup
// @param  isInbound            - is travel inbound
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::setupTravelSegMarkupSetDirection(TSITravelSegMarkup& tsm, bool isInbound)
{
  // if inbund
  if (isInbound)
    tsm.direction() = RuleConst::INBOUND;
  // otherwise outbound
  else
    tsm.direction() = RuleConst::OUTBOUND;
}
// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::setupTravelSegMarkupJour

// Description:
//
// function setup TravelSegmentMarkupContainer for scope journey
//
// @param  trx                  - Pricing transacion reference
// @param  tsiData              - TSIData structure reference
// @param  tsMarkup             - reference to travel segment markup container
// @param  loopForward          - loop direction
// @param  tsiChain             - initailzated TSISetupChain structure reference
// @param  stopoverExists       - will be set to true if any stopover exists
//
// @return true if all travel segment markups were correctly created
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::setupTravelSegMarkupJour(PricingTrx& trx,
                                      const FareUsage* fareUsage,
                                      const TSIData& tsiData,
                                      TSITravelSegMarkupContainer& tsMarkup,
                                      bool loopForward,
                                      TSISetupChain& tsiChain,
                                      bool& stopoverExists)
{
  // get FarePath and Itin from TSIData
  const FarePath* fp = tsiData.farePath();
  const Itin* itin = tsiData.itin(); // lint !e578

  if (fp)
  {
    const std::vector<PricingUnit*>& fpPu = fp->pricingUnit();

    const TravelSeg* lastSegment = itin->travelSeg().back();

    int32_t puCtr = setupTravelSegMarkupGetStart(loopForward, fpPu);
    int32_t puEnd = setupTravelSegMarkupGetEnd(loopForward, fpPu);

    for (; puCtr != puEnd; loopForward ? ++puCtr : --puCtr)
    {
      const std::vector<FareUsage*>& puFu = fpPu[puCtr]->fareUsage();

      int32_t fuCtr = setupTravelSegMarkupGetStart(loopForward, puFu);
      int32_t fuEnd = setupTravelSegMarkupGetEnd(loopForward, puFu);

      for (; fuCtr != fuEnd; loopForward ? ++fuCtr : --fuCtr)
      {
        const FareUsage* fu = puFu[fuCtr];

        // need PaxTypeFare
        const PaxTypeFare* ptFare = fu->paxTypeFare();
        if (!ptFare)
        {
          logErrMsg(trx, tsiData.tsiInfo(), "PaxTypeFare is NULL.");
          return false;
        }

        const std::vector<TravelSeg*>& fuTs = fu->travelSeg();

        int32_t tsCtr = setupTravelSegMarkupGetStart(loopForward, fuTs);
        int32_t tsEnd = setupTravelSegMarkupGetEnd(loopForward, fuTs);
        int32_t tsLast = fuTs.size() - 1;

        for (; tsCtr != tsEnd; loopForward ? ++tsCtr : --tsCtr)
        {
          TravelSeg* ts = fuTs[tsCtr];
          TravelSeg* nextTs = nullptr;

          if (!fallback::fixed::APO29538_StopoverMinStay())
          {
            int32_t nextIndex = tsCtr + 1;
            int32_t size = fuTs.size();

            if ((0 <= nextIndex) && (nextIndex < size))
            {
              nextTs = fuTs[nextIndex];
            }
          }

          GeoTravelType geoTravelType = GeoTravelType::UnknownGeoTravelType;
          if (tsiData.itin())
          {
            geoTravelType = tsiData.itin()->geoTravelType();
          }

          TSITravelSegMarkup& tsm = setupTravelSegMarkupCreateSegPTF(
              tsMarkup, ts, nextTs, geoTravelType, loopForward, ptFare, fareUsage);

          // Mark the first TravelSeg of FareUsage as FareBreak
          tsm.fareBreakAtOrigin() = (tsCtr == 0);
          // Mark the last TravelSeg of FareUsage as FareBreak
          tsm.fareBreakAtDestination() = (tsCtr == tsLast);

          if (ts == lastSegment)
          {
            tsm.isLastTravelSeg() = true;
          }
          else
          {
            // keep info if stopover exists
            stopoverExists |= tsm.isStopover();
          }

          if (!tsiChain.passCheckFareBreakOnly(tsm))
            continue;

          setupTravelSegMarkupSetDirection(tsm, fu->isInbound());
          tsiChain.process(ts, tsm);
        }
      }
    }
  }
  else
  {
    const std::vector<TravelSeg*>& tsVec = itin->travelSeg();
    int32_t tsCtr = setupTravelSegMarkupGetStart(loopForward, tsVec);
    int32_t tsEnd = setupTravelSegMarkupGetEnd(loopForward, tsVec);
    int32_t tsLast = tsVec.size() - 1;

    for (; tsCtr != tsEnd; loopForward ? ++tsCtr : --tsCtr)
    {
      TravelSeg* ts = tsVec[tsCtr];
      TravelSeg* nextTs = nullptr;

      if (!fallback::fixed::APO29538_StopoverMinStay())
      {
        int32_t nextIndex = tsCtr + 1;
        int32_t size = tsVec.size();

        if ((0 <= nextIndex) && (nextIndex < size))
        {
          nextTs = tsVec[nextIndex];
        }
      }

      GeoTravelType geoTravelType = GeoTravelType::UnknownGeoTravelType;
      if (LIKELY(tsiData.itin()))
      {
        geoTravelType = tsiData.itin()->geoTravelType();
      }

      TSITravelSegMarkup& tsm = setupTravelSegMarkupCreateSegFM(
          tsMarkup, ts, nextTs, geoTravelType, loopForward, nullptr, fareUsage);

      // Mark the first TravelSeg of FareMarket as FareBreak.
      tsm.fareBreakAtOrigin() = (tsCtr == 0);
      // Mark the last TravelSeg of FareMarket as FareBreak.
      tsm.fareBreakAtDestination() = (tsCtr == tsLast);

      if (tsCtr == tsLast)
      {
        tsm.isLastTravelSeg() = true;
      }
      else
      {
        // keep info if stopover exists
        stopoverExists |= tsm.isStopover();
      }

      if (UNLIKELY(!tsiChain.passCheckFareBreakOnly(tsm)))
        continue;

      setupTravelSegMarkupSetDirection(tsm, ts->segmentOrder() > itin->furthestPointSegmentOrder());
      tsiChain.process(ts, tsm);
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::setupTravelSegMarkupSubJour
//
// Description:
//
// function setup TravelSegmentMarkupContainer for scope sub journey
//
// @param  trx                  - Pricing transacion reference
// @param  tsiData              - TSIData structure reference
// @param  tsMarkup             - reference to travel segment markup container
// @param  loopForward          - loop direction
// @param  tsiChain             - initailzated TSISetupChain structure reference
// @param  stopoverExists       - will be set to true if any stopover exists
//
// @return true if all travel segment markups were correctly created
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::setupTravelSegMarkupSubJour(PricingTrx& trx,
                                         const FareUsage* fareUsage,
                                         const TSIData& tsiData,
                                         TSITravelSegMarkupContainer& tsMarkup,
                                         bool loopForward,
                                         TSISetupChain& tsiChain,
                                         bool& stopoverExists)
{
  // get PricingUnit form TSIData
  const PricingUnit* pu = tsiData.pricingUnit();
  const std::vector<FareUsage*>& puFu = pu->fareUsage();

  int32_t fuCtr = setupTravelSegMarkupGetStart(loopForward, puFu);
  int32_t fuEnd = setupTravelSegMarkupGetEnd(loopForward, puFu);
  ;
  int32_t fuLast = puFu.size() - 1;

  for (; fuCtr != fuEnd; loopForward ? ++fuCtr : --fuCtr)
  {
    const FareUsage* fu = puFu[fuCtr];

    const PaxTypeFare* ptFare = fu->paxTypeFare();
    if (UNLIKELY(!ptFare))
    {
      logErrMsg(trx, tsiData.tsiInfo(), "PaxTypeFare is NULL.");
      return false;
    }

    const std::vector<TravelSeg*>& fuTs = fu->travelSeg();

    int32_t tsCtr = setupTravelSegMarkupGetStart(loopForward, fuTs);
    int32_t tsEnd = setupTravelSegMarkupGetEnd(loopForward, fuTs);
    int32_t tsLast = fuTs.size() - 1;

    for (; tsCtr != tsEnd; loopForward ? ++tsCtr : --tsCtr)
    {
      TravelSeg* ts = fuTs[tsCtr];
      TravelSeg* nextTs = nullptr;

      if (!fallback::fixed::APO29538_StopoverMinStay())
      {
        int32_t nextIndex = tsCtr + 1;
        int32_t size = fuTs.size();

        if ((0 <= nextIndex) && (nextIndex < size))
        {
          nextTs = fuTs[nextIndex];
        }
      }

      GeoTravelType geoTravelType = GeoTravelType::UnknownGeoTravelType;
      if (tsiData.itin())
      {
        geoTravelType = tsiData.itin()->geoTravelType();
      }

      TSITravelSegMarkup& tsm = setupTravelSegMarkupCreateSegPTF(
          tsMarkup, ts, nextTs, geoTravelType, loopForward, ptFare, fareUsage);

      // Mark the first TravelSeg of FareUsage as FareBreak.
      tsm.fareBreakAtOrigin() = (tsCtr == 0);
      // Mark the last TravelSeg of FareUsage as FareBreak.
      tsm.fareBreakAtDestination() = (tsCtr == tsLast);

      if ((tsCtr == tsLast) && (fuCtr == fuLast))
      {
        tsm.isLastTravelSeg() = true;
      }
      else
      {
        // keep info if stopover exists
        stopoverExists |= tsm.isStopover();
      }

      if (!tsiChain.passCheckFareBreakOnly(tsm))
        continue;
      // APO-36040 [SHM-1583]: In the case of circle trip Pus,
      // as per atpco, all fcs following
      // the pu turnaround point shoud be considered inbound for tsi 
      // processing. 
      // If the pu turnaround tvl seg equals the fu starting seg,
      // then all tvlsegs in the fu are set as inbound
      if ( (!fallback::apo36040Cat6TSI5Check(&trx)) )
      {
          if ((pu->puType() == PricingUnit::Type::CIRCLETRIP) &&
              (pu->turnAroundPoint())  &&
              (pu->turnAroundPoint() == fuTs.front()) )
          { // set the direction as inbound for this tvlseg
             setupTravelSegMarkupSetDirection(tsm, true);
          }
          else
             setupTravelSegMarkupSetDirection(tsm, fu->isInbound());
      } //fallback
      else // can be removed when fallbacks are removed.
         setupTravelSegMarkupSetDirection(tsm, fu->isInbound());
      tsiChain.process(ts, tsm);
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::setupTravelSegMarkupFare
//
// Description:
//
// function setup TravelSegmentMarkupContainer for scope sub journey
//
// @param  trx                  - Pricing transacion reference
// @param  tsiData              - TSIData structure reference
// @param  tsMarkup             - reference to travel segment markup container
// @param  loopForward          - loop direction
// @param  tsiChain             - initailzated TSISetupChain structure reference
// @param  stopoverExists       - will be set to true if any stopover exists
//
// @return true if all travel segment markups were correctly created
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::setupTravelSegMarkupFare(PricingTrx& trx,
                                      const FareUsage* fareUsage,
                                      const TSIData& tsiData,
                                      TSITravelSegMarkupContainer& tsMarkup,
                                      bool loopForward,
                                      TSISetupChain& tsiChain,
                                      bool& stopoverExists)
{
  // get fare market from TSIData
  const FareMarket* fm = tsiData.fareMarket();
  const std::vector<TravelSeg*>& fmTs = fm->travelSeg();

  std::vector<TravelSeg*>::const_iterator it = fmTs.begin();
  std::vector<TravelSeg*>::const_iterator ite = fmTs.end();

  FMDirection dir = fm->direction();
  if (dir == FMDirection::UNKNOWN && tsiData.pricingUnit())
  {
    std::vector<FareUsage*>::const_iterator it = tsiData.pricingUnit()->fareUsage().begin();
    std::vector<FareUsage*>::const_iterator ie = tsiData.pricingUnit()->fareUsage().end();
    for (; it != ie; it++)
    {
      const PaxTypeFare* ptf = (*it)->paxTypeFare();
      if (UNLIKELY(!ptf))
        continue;

      const FareMarket* pfm = ptf->fareMarket();
      if (UNLIKELY(!pfm))
        continue;

      // fare market could be from base fare, where in FU we can have
      // validated fare faremarket, so compare travelSeg instead.
      if (fm->travelSeg() == pfm->travelSeg())
      {
        dir = (*it)->isInbound() ? FMDirection::INBOUND : FMDirection::OUTBOUND;
        break;
      }
    }
  }

  // for each travel segment form fare market
  for (; it != ite; ++it)
  {
    TravelSeg* ts = *it;
    TravelSeg* nextTs = nullptr;

    if (!fallback::fixed::APO29538_StopoverMinStay())
    {
      if ((it != ite) && ((it + 1) != ite))
      {
        nextTs = *(it + 1);
      }
    }

    GeoTravelType geoTravelType = GeoTravelType::UnknownGeoTravelType;
    if (tsiData.itin())
    {
      geoTravelType = tsiData.itin()->geoTravelType();
    }

    TSITravelSegMarkup& tsm =
        setupTravelSegMarkupCreateSegFM(tsMarkup, ts, nextTs, geoTravelType, true, fm, fareUsage);

    std::vector<TravelSeg*>::const_iterator itNext = it;
    ++itNext;
    if (itNext == ite)
      tsm.isLastTravelSeg() = true;

    // no fare breaks for fare scope

    // Set the inbound/outbound directionality for the
    //  TravelSegMarkup
    //
    setupTravelSegMarkupSetDirection(tsm, FMDirection::INBOUND == dir);
    tsiChain.process(ts, tsm);
  }
  return true;
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtilTSI::setupTravelSegMarkup
//
// Description:
//
// function setup TravelSegmentMarkupContainer
//
// @param  trx                  - Pricing transacion reference
// @param  tsiData              - TSIData structure reference
// @param  tsMarkup             - reference to travel segment markup container
//
// @return true if all travel segment markups were correctly created
//
// ----------------------------------------------------------------------------
bool
RuleUtilTSI::setupTravelSegMarkup(PricingTrx& trx,
                                  const TSIData& tsiData,
                                  const FareUsage* fareUsage,
                                  TSITravelSegMarkupContainer& tsMarkup)
{
  // get TSI scope
  const RuleConst::TSIScopeType& scope = tsiData.scope();

  // get directions
  bool loopForward, checkFareBreaksOnly;
  if (UNLIKELY(!getDirectionsFromLoop(trx, tsiData.tsiInfo(), loopForward, checkFareBreaksOnly)))
    return false;

  // container with seups for for match criteria
  TSISetupChain tsiChain;
  if (UNLIKELY(!tsiChain.buildTSIChain(trx, tsiData, loopForward, checkFareBreaksOnly)))
  {
    logErrMsg(trx, tsiData.tsiInfo(), tsiChain.errMsg().c_str());
    return false;
  }

  // Used for FURTHEST point calculation
  bool stopoverExists = false;

  // build markups for passed scope
  if (RuleConst::TSI_SCOPE_JOURNEY == scope)
  {
    if (!setupTravelSegMarkupJour(
            trx, fareUsage, tsiData, tsMarkup, loopForward, tsiChain, stopoverExists))
      return false;
  }
  else if (RuleConst::TSI_SCOPE_SUB_JOURNEY == scope)
  {
    if (UNLIKELY(!setupTravelSegMarkupSubJour(
            trx, fareUsage, tsiData, tsMarkup, loopForward, tsiChain, stopoverExists)))
      return false;
  }
  else if (LIKELY(RuleConst::TSI_SCOPE_FARE_COMPONENT == scope))
  {
    if (UNLIKELY(!setupTravelSegMarkupFare(
            trx, fareUsage, tsiData, tsMarkup, loopForward, tsiChain, stopoverExists)))
      return false;
  }

  // post initailization FURTHEST and STOPOVER
  setupTravelSegMarkupSetFurthest(scope, tsiData, stopoverExists, tsMarkup);
  setupTravelSegMarkupSetStopover(scope, tsiData, fareUsage, 
                                  stopoverExists, tsMarkup, trx);
  setupTravelSegMarkupRtwPostprocess(trx, tsiData, tsMarkup);

  return true;
}

// ----------------------------------------------------------------------------
//
// @function void RuleUtilTSI::writeTSIDiagTravelSeg
//
// Description:
//
// function witres TSITravel segment detailes information
//
// @param  diag                 - Diagnostic reference
// @param  puNumber             - pricing unit order
// @param  fcNumber             - FareUsage order
// @param  displayPuFc          - should ppricing unit information be displayed
// @param  tsm                  - reference to travel segment markup
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::writeTSIDiagTravelSeg(DiagCollector& diag,
                                   const int32_t puNumber,
                                   const int32_t fcNumber,
                                   const bool displayPuFc,
                                   const TSITravelSegMarkup& tsm)
{ // lint !e578

  // prepare PU header
  if (displayPuFc)
  {
    if (puNumber > 0)
    {
      diag << std::setw(4) << std::right << puNumber << " ";
    }
    else
    {
      diag << "   - ";
    }

    if (fcNumber > 0)
    {
      diag << std::setw(4) << std::right << fcNumber << " ";
    }
    else
    {
      diag << "   - ";
    }
  }
  else
  {
    // 10 chars
    diag << "          ";
  }

  // for stopover set O
  if (tsm.isStopover())
    diag << "O ";
  else
    diag << "  ";

  // travel segment details
  diag << std::setw(4) << std::left << tsm.travelSeg()->origin()->loc() << std::setw(7) << std::left
       << tsm.travelSeg()->destination()->loc();

  // if matched, then display details
  if (RuleConst::TSI_NOT_MATCH != tsm.match())
  {
    if (tsm.origMatch())
      diag << "O";
    else
      diag << "-";
    if (tsm.destMatch())
      diag << std::setw(6) << "D";
    else
      diag << std::setw(6) << "-";
  }
  else
  {
    diag << std::setw(7) << "--";
  }

  if (tsm.origSave() || tsm.destSave())
  {
    if (tsm.origSave())
      diag << "O";
    else
      diag << "-";
    if (tsm.destSave())
      diag << std::setw(5) << "D";
    else
      diag << std::setw(5) << "-";
  }
  else
  {
    diag << std::setw(6) << "--";
  }

  // if there is some string, display this
  if (!tsm.noMatchReason().empty())
  {
    diag << tsm.noMatchReason();
  }

  diag << std::endl;
}
// ----------------------------------------------------------------------------
//
// @function void RuleUtilTSI::writeTSIDiagLine
//
// Description:
//
// function witres empty line in diagnostic
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::writeTSIDiagLine(DiagCollector& diag)
{
  diag << " " << std::endl;
}
// ----------------------------------------------------------------------------
//
// @function void RuleUtilTSI::writeTSIDiagDetails
//
// Description:
//
// function witres TSITravel segment detailes information
//
// @param  diag                 - Diagnostic reference
// @param  tsiData              - TSIData structure reference
// @param  tsMarkup             - reference to travel segment markup container
//
// @return nothing
//
// ----------------------------------------------------------------------------
void
RuleUtilTSI::writeTSIDiagDetails(DiagCollector& diag,
                                 const TSIData& tsiData,
                                 TSITravelSegMarkupContainer& tsMarkup)
{
  const RuleConst::TSIScopeType& scope = tsiData.scope();

  // empty travel segment markup
  TSITravelSegMarkup tsmEmpty;
  tsmEmpty.noMatchReason() = "NOT CHECKED";

  // for journet
  if (RuleConst::TSI_SCOPE_JOURNEY == scope)
  {
    diag << "SCOPE: JOURNEY" << std::endl;
    diag << "  PU   FC             MATCH  SAVE  NO MATCH REASON" << std::endl;

    // if fare path exists
    if (tsiData.farePath())
    {
      const FarePath* fp = tsiData.farePath();
      const std::vector<PricingUnit*>& fpPu = fp->pricingUnit();

      int32_t puNumber = 1;
      std::vector<PricingUnit*>::const_iterator puIter = fpPu.begin();

      for (; puIter != fpPu.end(); ++puIter, ++puNumber)
      {
        const std::vector<FareUsage*>& puFu = (*puIter)->fareUsage();
        int32_t fuNumber = 1;
        std::vector<FareUsage*>::const_iterator fuIter = puFu.begin();

        for (; fuIter != puFu.end(); ++fuIter, ++fuNumber)
        {
          bool displayPuFc = true;
          const std::vector<TravelSeg*>& fuTs = (*fuIter)->travelSeg();
          std::vector<TravelSeg*>::const_iterator tsIter = fuTs.begin();

          for (; tsIter != fuTs.end(); ++tsIter)
          {
            tsmEmpty.travelSeg() = *tsIter;
            TSITravelSegMarkupI tsmIter = tsMarkup.find(*tsIter);

            // if not found travel segment markup, set to empty segment markup
            TSITravelSegMarkup& tsm = (tsmIter != tsMarkup.end()) ? *tsmIter : tsmEmpty;

            writeTSIDiagTravelSeg(diag, puNumber, fuNumber, displayPuFc, tsm);

            displayPuFc = false;
          }
        }
      }
    }
    // if not fare path then itin
    else if (tsiData.itin())
    {
      const Itin* itin = tsiData.itin();

      const std::vector<TravelSeg*>& ts = itin->travelSeg();
      std::vector<TravelSeg*>::const_iterator tsIter = ts.begin();
      std::vector<TravelSeg*>::const_iterator tsIterEnd = ts.end();

      for (; tsIter != tsIterEnd; ++tsIter)
      {
        tsmEmpty.travelSeg() = *tsIter;

        TSITravelSegMarkupI tsmIter = tsMarkup.find(*tsIter);

        // if not found travel segment markup, set to empty segment markup
        TSITravelSegMarkup& tsm = (tsmIter != tsMarkup.end()) ? *tsmIter : tsmEmpty;

        writeTSIDiagTravelSeg(diag, -1, -1, true, tsm);
      }
    }
  }
  // sub journey scope
  else if (RuleConst::TSI_SCOPE_SUB_JOURNEY == scope)
  {
    diag << "SCOPE: SUB JOURNEY" << std::endl;
    diag << "  PU   FC             MATCH  SAVE  NO MATCH REASON" << std::endl;

    const PricingUnit* pu = tsiData.pricingUnit();
    const std::vector<FareUsage*>& puFu = pu->fareUsage();

    int32_t fuNumber = 1;
    std::vector<FareUsage*>::const_iterator fuIter = puFu.begin();

    for (; fuIter != puFu.end(); ++fuIter, ++fuNumber)
    {
      bool displayPuFc = true;
      const std::vector<TravelSeg*>& fuTs = (*fuIter)->travelSeg();
      std::vector<TravelSeg*>::const_iterator tsIter = fuTs.begin();

      for (; tsIter != fuTs.end(); ++tsIter)
      {
        tsmEmpty.travelSeg() = *tsIter;

        TSITravelSegMarkupI tsmIter = tsMarkup.find(*tsIter);

        // if not found travel segment markup, set to empty segment markup
        TSITravelSegMarkup& tsm = (tsmIter != tsMarkup.end()) ? *tsmIter : tsmEmpty;

        writeTSIDiagTravelSeg(diag, 1, fuNumber, displayPuFc, tsm);
        displayPuFc = false;
      }
    }
  }
  // scope fare
  else if (RuleConst::TSI_SCOPE_FARE_COMPONENT == scope)
  {
    diag << "SCOPE: FARE COMPONENT" << std::endl;
    diag << "  PU   FC             MATCH  SAVE  NO MATCH REASON" << std::endl;

    const FareMarket* fm = tsiData.fareMarket();
    const std::vector<TravelSeg*>& fmTs = fm->travelSeg();

    std::vector<TravelSeg*>::const_iterator tsIter = fmTs.begin();

    for (; tsIter != fmTs.end(); ++tsIter)
    {
      tsmEmpty.travelSeg() = *tsIter;

      TSITravelSegMarkupI tsmIter = tsMarkup.find(*tsIter);

      // if not found travel segment markup, set to empty segment markup
      TSITravelSegMarkup& tsm = (tsmIter != tsMarkup.end()) ? *tsmIter : tsmEmpty;

      writeTSIDiagTravelSeg(diag, -1, -1, true, tsm);
    }
  }
}

} // tse
