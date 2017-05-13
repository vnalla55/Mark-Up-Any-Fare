//-------------------------------------------------------------------
//
//  File:        FareMarket.cpp
//  Created:     March 8, 2004
//  Design:      Doug Steeb
//  Authors:
//
//  Description: One Fare Market
//
//  Updates:
//          03/08/04 - VN - file created.
//          04/18/04 - VN - PaxTypeBucket added.
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
#include "DataModel/FareMarket.h"

#include "Common/Assert.h"
#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/PaxTypeInfo.h"

#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/range/algorithm.hpp>

#include <algorithm>
#include <sstream>
#include <utility>

namespace tse
{
static Logger
logger("atseintl.DataModel.FareMarket");

FIXEDFALLBACK_DECL(fallbackFmBrandStatus)

bool
OWRTFilter::
operator()(PaxTypeFare* pxFare)
{
  if ((pxFare->owrt() == _selectedOWRT) || (pxFare->owrt() == ONE_WAY_MAY_BE_DOUBLED))
    return pxFare->isValid();
  return false;
}

bool
FareMarket::
operator==(const FareMarket& rhs) const
{
  return _origin == rhs._origin && _destination == rhs._destination &&
         _boardMultiCity == rhs._boardMultiCity && _offMultiCity == rhs._offMultiCity &&
         _globalDirection == rhs._globalDirection && _governingCarrier == rhs._governingCarrier &&
         _governingCarrierPref == rhs._governingCarrierPref && _travelSeg == rhs._travelSeg &&
         _sideTripTravelSeg == rhs._sideTripTravelSeg && _paxTypeCortege == rhs._paxTypeCortege &&
         hasAllMarriedSegs() == rhs.hasAllMarriedSegs() && flowMarket() == rhs.flowMarket() &&
         _availBreaks == rhs._availBreaks && _travelDate == rhs._travelDate;
}

std::string
FareMarket::fareRetrievalFlagToStr(FareRetrievalFlags flag)
{
  switch (static_cast<unsigned int>(flag))
  {
  case RetrievHistorical:
    return "H";
  case RetrievKeep:
  case RetrievExpndKeep | RetrievKeep:
    return "K";
  case RetrievTvlCommence:
    return "T";
  case RetrievCurrent:
    return "C";
  case RetrievCurrent | RetrievTvlCommence:
    return "*";
  case RetrievExpndKeep:
    return "E";
  case RetrievHistorical | RetrievKeep:
  case RetrievHistorical | RetrievExpndKeep:
  case RetrievHistorical | RetrievExpndKeep | RetrievKeep:
    return "-";
  case RetrievTvlCommence | RetrievKeep:
  case RetrievTvlCommence | RetrievExpndKeep:
  case RetrievTvlCommence | RetrievExpndKeep | RetrievKeep:
    return ".";
  case RetrievCurrent | RetrievTvlCommence | RetrievKeep:
  case RetrievCurrent | RetrievTvlCommence | RetrievExpndKeep:
  case RetrievCurrent | RetrievTvlCommence | RetrievExpndKeep | RetrievKeep:
    return "A";
  default:
    return " ";
  }
}

// these are used for speed when determining if a fare can be used for this FM
void
FareMarket::setCat19PaxFlags(PaxType* pt)
{
  if (LIKELY(pt != nullptr && pt->paxTypeInfo() != nullptr))
  {
    if (pt->paxTypeInfo()->isChild())
      setChildNeeded(true);
    if (pt->paxTypeInfo()->isInfant())
      setInfantNeeded(true);
  }
}

int
FareMarket::getValidForPOFaresCount() const
{
  return std::count_if(_allPaxTypeFare.cbegin(),
                       _allPaxTypeFare.cend(),
                       [](const auto& ptFare)
                       { return ptFare && ptFare->isValidForPO(); });
}

void
FareMarket::invalidateAllPaxTypeFaresForRetailer(PricingTrx& trx)
{
  for (auto ptf : _allPaxTypeFare)
  {
    if (ptf && !RuleUtil::isFareValidForRetailerCodeQualiferMatch(trx, *ptf))
      ptf->invalidateFare(PaxTypeFare::FD_Retailer_Code);
  }
}

FareMarket::SOL_FM_TYPE
FareMarket::getFmTypeSol() const
{
  return _solFmType;
}

void
FareMarket::setFmTypeSol(FareMarket::SOL_FM_TYPE type)
{
  _solFmType = type;
}

void
FareMarket::setSolComponentDirection(FareMarket::SOL_COMPONENT_DIRECTION direction)
{
  _solComponentDirection = direction;
}

FareMarket::SOL_COMPONENT_DIRECTION
FareMarket::getSolComponentDirection() const
{
  return _solComponentDirection;
}

bool
FareMarket::initialize(PricingTrx& trx)
{
  // fill _PaxTypeBucket vector

  _paxTypeCortege.resize(trx.paxType().size());

  std::vector<PaxTypeBucket>::iterator paxTypeCortege = _paxTypeCortege.begin();

  uint16_t paxTypeCnt = 0;

  std::vector<PaxType*>::iterator paxType = trx.paxType().begin();
  for (; paxType != trx.paxType().end(); ++paxType)
  {
    (*paxTypeCortege).requestedPaxType() = *paxType;

    (*paxTypeCortege).paxIndex() = paxTypeCnt;

    std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxTypeMap = (*paxType)->actualPaxType();

    //--------------------------------------------------------------
    // Populate actual pax type vector in the paxType cortege only
    // with the requested paxType when XO is input in the entry.
    //--------------------------------------------------------------
    /*    if (trx.getOptions()->isXoFares())
    {
       (*paxTypeCortege).actualPaxType().push_back( *paxType );
    }
    else
    {
*/ // actual PaxType for specified carrier

    std::vector<PaxType*>::iterator actualPT;
    std::map<CarrierCode, std::vector<PaxType*>*>::iterator actualPaxType;

    actualPaxType = actualPaxTypeMap.find(governingCarrier());
    if (UNLIKELY(actualPaxType != actualPaxTypeMap.end()))
    {
      actualPT = (*actualPaxType).second->begin();
      for (; actualPT != (*actualPaxType).second->end(); actualPT++)
      {
        if (!bypassCat19FlagsSet() && (!isChildNeeded() || !isInfantNeeded()))
          setCat19PaxFlags(*actualPT);

        // specifically for 'INF' type (free baby fare)
        if ((*actualPT)->paxType() == INFANT)
          setInfNeeded(true);

        (*paxTypeCortege).actualPaxType().push_back(*actualPT);
      }
    }

    // actual PaxType for any carrier

    actualPaxType = actualPaxTypeMap.find(ANY_CARRIER);
    if (LIKELY(actualPaxType != actualPaxTypeMap.end()))
    {
      actualPT = (*actualPaxType).second->begin();
      for (; actualPT != (*actualPaxType).second->end(); actualPT++)
      {
        if (!bypassCat19FlagsSet() && (!isChildNeeded() || !isInfantNeeded()))
          setCat19PaxFlags(*actualPT);

        // specifically for 'INF' type (free baby fare)
        if ((*actualPT)->paxType() == INFANT)
          setInfNeeded(true);

        (*paxTypeCortege).actualPaxType().push_back(*actualPT);
      }
    }

    if (UNLIKELY((*paxTypeCortege).actualPaxType().empty()))
    {
      LOG4CXX_ERROR(logger,
                    "Error: FareMarket " << origin()->loc() << "-" << destination()->loc()
                                         << ", requested PaxType #" << (paxTypeCnt + 1)
                                         << ": _actualPaxType vector is empty");
      return false;
    }

    paxTypeCortege++;
    paxTypeCnt++;
  };

  if (UNLIKELY(trx.getRequest()->fareGroupRequested()))
    setValidateFareGroup(true);

  return true;
}

PaxTypeBucket*
FareMarket::paxTypeCortege(const PaxType* requestedPaxType)
{
  std::vector<PaxTypeBucket>::iterator paxTypeCortege = _paxTypeCortege.begin();

  for (; paxTypeCortege != _paxTypeCortege.end(); ++paxTypeCortege)
  {
    if (comparePaxTypeCode())
    {
      if ((*paxTypeCortege).requestedPaxType()->paxType() == requestedPaxType->paxType())
        return &(*paxTypeCortege);
    }
    else
    {
      if ((*paxTypeCortege).requestedPaxType() == requestedPaxType)
        return &(*paxTypeCortege);
    }
  }

  return nullptr;
}

const PaxTypeBucket*
FareMarket::paxTypeCortege(const PaxType* requestedPaxType) const
{
  std::vector<PaxTypeBucket>::const_iterator paxTypeCortege = _paxTypeCortege.begin();

  for (; paxTypeCortege != _paxTypeCortege.end(); ++paxTypeCortege)
  {
    if (comparePaxTypeCode())
    {
      if ((*paxTypeCortege).requestedPaxType()->paxType() == requestedPaxType->paxType())
        return &(*paxTypeCortege);
    }
    else
    {
      if ((*paxTypeCortege).requestedPaxType() == requestedPaxType)
        return &(*paxTypeCortege);
    }
  }

  return nullptr;
}

uint16_t
FareMarket::getStartSegNum() const
{
  if (_travelSeg.empty())
  {
    // Error: should not be empty

    return 0;
  }

  return _travelSeg.front()->segmentOrder();
}

uint16_t
FareMarket::getEndSegNum() const
{
  if (_travelSeg.empty())
  {
    // Error: should not be empty

    return 0;
  }

  return _travelSeg.back()->segmentOrder();
}

const FareMarket::FareMarketSavedFnResult::Result*
FareMarket::savedFnResult(const uint32_t categoryNumber,
                          const PaxTypeFare& paxTypeFare,
                          FareMarketSavedFnResult::Results& results)
{
  Indicator indicator1 = ' ';
  if (UNLIKELY(categoryNumber == 6 || categoryNumber == 7))
  {
    indicator1 = paxTypeFare.owrt();
  }
  else if (categoryNumber == 15)
  {
    if (LIKELY(!paxTypeFare.isFareClassAppMissing()))
    {
      indicator1 = paxTypeFare.fcaDisplayCatType();
      // L/C/T can share result, all others can share result
      if (indicator1 == 'L' || indicator1 == 'C' || indicator1 == 'T')
        indicator1 = 'L';
      else
        indicator1 = ' ';
    }
  }
  // do not care, unless owrt for MinStay or MaxStay,
  //                     displayCatType for FareByRule cat15

  const Directionality& directionality = paxTypeFare.directionality();
  const FareMarket::FareMarketSavedFnResult::Result* result = nullptr;

  PaxTypeFareResultKey key(
      indicator1,
      (results.directional()) ? directionality : (static_cast<Directionality>(BOTH)),
      (results.chkPsgType() && paxTypeFare.paxType()) ? paxTypeFare.paxType()->paxType() : "",
      (results.chkPsgAge() && paxTypeFare.paxType()) ? paxTypeFare.paxType()->age() : 0);

  if (UNLIKELY(results.chkPsgType() && categoryNumber != 13))
    key.paxType() += paxTypeFare.fcasPaxType();

  FMScopedLock guard(results.resultMapMutex());
  FareMarketSavedFnResult::ResultMap::const_iterator resultI = results.resultMap().find(key);

  if (resultI != results.resultMap().end())
  {
    result = resultI->second;
  }
  return result;
}

bool
FareMarket::saveFnResult(DataHandle& dataHandle,
                         const uint32_t categoryNumber,
                         const PaxTypeFare& paxTypeFare,
                         const FareMarketSavedFnResult::Result* result,
                         FareMarketSavedFnResult::Results& results)
{
  PaxTypeFareResultKey key;

  key.directionality() = (results.directional()) ? paxTypeFare.directionality() : BOTH;
  if (UNLIKELY(results.chkPsgType()))
  {
    if (paxTypeFare.paxType())
      key.paxType() = paxTypeFare.paxType()->paxType();
    if (categoryNumber != 13)
      key.paxType() += paxTypeFare.fcasPaxType();
  }
  key.psgAge() = (results.chkPsgAge() && paxTypeFare.paxType()) ? paxTypeFare.paxType()->age() : 0;

  Indicator indicator1 = ' ';
  if (UNLIKELY(categoryNumber == 6 || categoryNumber == 7))
  {
    indicator1 = paxTypeFare.owrt();
  }
  else if (categoryNumber == 15)
  {
    if (LIKELY(!paxTypeFare.isFareClassAppMissing()))
    {
      indicator1 = paxTypeFare.fcaDisplayCatType();
      // L/C/T can share result, all others can share result
      if (indicator1 == 'L' || indicator1 == 'C' || indicator1 == 'T')
        indicator1 = 'L';
      else
        indicator1 = ' ';
    }
  }
  key.indicator1() = indicator1;

  FMScopedLock guard(results.resultMapMutex());
  results.resultMap()[key] = result;

  return true;
}

const FareMarket::FareMarketSavedGfrResult::Result*
FareMarket::savedGfrResult(const uint32_t categoryNumber,
                           const PaxTypeFare& paxTypeFare,
                           FareMarketSavedGfrResult::Results& results)
{
  Indicator indicator1 = ' ';
  if (categoryNumber == 6 || categoryNumber == 7)
  {
    indicator1 = paxTypeFare.owrt();
  }
  else if (categoryNumber == 15)
  {
    if (LIKELY(!paxTypeFare.isFareClassAppMissing()))
    {
      indicator1 = paxTypeFare.fcaDisplayCatType();
      // L/C/T can share result, all others can share result
      if (UNLIKELY(indicator1 == 'L' || indicator1 == 'C' || indicator1 == 'T'))
        indicator1 = 'L';
      else
        indicator1 = ' ';
    }
  }
  // do not care, unless owrt for MinStay or MaxStay,
  //                     displayCatType for FareByRule cat15

  Directionality directionality = paxTypeFare.directionality();
  const FareMarket::FareMarketSavedGfrResult::Result* result = nullptr;

  PaxTypeFareResultKey key(
      indicator1,
      (results.directional()) ? directionality : BOTH,
      (results.chkPsgType() && paxTypeFare.paxType()) ? paxTypeFare.paxType()->paxType() : "",
      (results.chkPsgAge() && paxTypeFare.paxType()) ? paxTypeFare.paxType()->age() : 0);

  if (results.chkPsgType() && categoryNumber != 13)
    key.paxType() += paxTypeFare.fcasPaxType();

  FMScopedLock guard(results.resultMapMutex());
  FareMarketSavedGfrResult::ResultMap::const_iterator resultI = results.resultMap().find(key);

  if (resultI != results.resultMap().end())
  {
    result = resultI->second;
  }
  return result;
}

bool
FareMarket::saveGfrResult(DataHandle& dataHandle,
                          const uint32_t categoryNumber,
                          const PaxTypeFare& paxTypeFare,
                          const FareMarketSavedGfrResult::Result* result,
                          FareMarketSavedGfrResult::Results& results)
{
  PaxTypeFareResultKey key;

  key.directionality() = (results.directional()) ? paxTypeFare.directionality() : BOTH;
  if (results.chkPsgType())
  {
    if (paxTypeFare.paxType())
      key.paxType() = paxTypeFare.paxType()->paxType();
    if (categoryNumber != 13)
      key.paxType() += paxTypeFare.fcasPaxType();
  }
  key.psgAge() = (results.chkPsgAge() && paxTypeFare.paxType()) ? paxTypeFare.paxType()->age() : 0;

  Indicator indicator1 = ' ';
  if (categoryNumber == 6 || categoryNumber == 7)
  {
    indicator1 = paxTypeFare.owrt();
  }
  else if (categoryNumber == 15)
  {
    if (LIKELY(!paxTypeFare.isFareClassAppMissing()))
    {
      indicator1 = paxTypeFare.fcaDisplayCatType();
      // L/C/T can share result, all others can share result
      if (UNLIKELY(indicator1 == 'L' || indicator1 == 'C' || indicator1 == 'T'))
        indicator1 = 'L';
      else
        indicator1 = ' ';
    }
  }
  // do not care, unless owrt for MinStay or MaxStay,
  //                     displayCatType for FareByRule cat15
  key.indicator1() = indicator1;

  FMScopedLock guard(results.resultMapMutex());
  results.resultMap()[key] = result;

  return true;
}

void
FareMarket::resetResultMap(const uint32_t categoryNumber)
{
  resetGfrResultUMap(categoryNumber);
  resetFnResultUMap(categoryNumber);
}

int
FareMarket::setFCChangeStatus(const int16_t& pointOfChgSegOrder)
{
  if (changeStatus() == tse::UK)
    changeStatus() = asBookChangeStatus() =
        ExchangeUtil::getChangeStatus(_travelSeg, pointOfChgSegOrder);

  return changeStatus();
}

int
FareMarket::setFCChangeStatus(const int16_t& pointOfChgFirst, const int16_t& pointOfChgSecond)
{
  if (changeStatus() == tse::UK)
    changeStatus() = asBookChangeStatus() =
        ExchangeUtil::getChangeStatus(_travelSeg, pointOfChgFirst, pointOfChgSecond);

  return changeStatus();
}

bool
FareMarket::isChanged() const
{
  return (changeStatus() == tse::UC);
}

bool
FareMarket::isFlown() const
{
  return (changeStatus() == tse::FL);
}

bool
FareMarket::isLocalJourneyFare() const
{
  bool result = false;
  if (nullptr == _governingCarrierPref)
  {
    LOG4CXX_ERROR(logger, "FareMarket::isLocalJourneyFare: uninitialized _governingCarrierPref");
  }
  else
  {
    result = (YES == _governingCarrierPref->localMktJourneyType());
  }
  return result;
}

bool
FareMarket::isFlowJourneyFare() const
{
  bool result = false;
  if (nullptr == _governingCarrierPref)
  {
    LOG4CXX_ERROR(logger, "FareMarket::isFlowJourneyFare: uninitialized _governingCarrierPref");
  }
  else
  {
    result = (YES == _governingCarrierPref->flowMktJourneyType());
  }
  return result;
}

bool
FareMarket::existValidFares() const
{
  return std::any_of(_allPaxTypeFare.begin(),
                     _allPaxTypeFare.end(),
                     [](const PaxTypeFare* ptf)
                     { return ptf->isValid(); });
}

bool
FareMarket::existRequestedFareBasisValidFares() const
{
  return std::any_of(_allPaxTypeFare.begin(),
                     _allPaxTypeFare.end(),
                     [](const PaxTypeFare* ptf)
                     { return !ptf->isRequestedFareBasisInvalid(); });
}

bool
FareMarket::isRoutingNotProcessed() const
{
  return std::none_of(_allPaxTypeFare.begin(),
                      _allPaxTypeFare.end(),
                      [](PaxTypeFare* ptf)
                      { return ptf->isRoutingProcessed(); });
}

namespace
{
PaxTypeFare::CategoryBitSet
cmdPricingCat("11000101111111100"); // cat: 2-9,11,15,16
}

bool
FareMarket::isCategoryNotProcessed() const
{
  return std::none_of(_allPaxTypeFare.begin(),
                      _allPaxTypeFare.end(),
                      [](PaxTypeFare* ptf)
                      { return ptf->areAnyCategoriesProcessed(cmdPricingCat); });
}

bool
FareMarket::isBookingCodeNotProcessed() const
{
  return std::none_of(_allPaxTypeFare.begin(),
                      _allPaxTypeFare.end(),
                      [](PaxTypeFare* ptf)
                      { return ptf->isBookingCodeProcessed(); });
}

bool
FareMarket::areAllFaresFailingOnRules() const
{
  if (isCategoryNotProcessed())
  {
    return true;
  }

  for (PaxTypeFare* ptf : _allPaxTypeFare)
  {
    if (ptf->isRequestedFareBasisInvalid() || !ptf->areAnyCategoriesProcessed(cmdPricingCat))
      continue;

    if (((ptf->areAllCategoryValid() && ptf->fare() && !ptf->fare()->isMissingFootnote()) ||
         !ptf->isMatchFareFocusRule() || !ptf->isValidForBranding()) && ptf->isValid())
    {
      return false;
    }
  }
  return true;
}

bool
FareMarket::areAllFaresFailingOnRouting() const
{
  if (isRoutingNotProcessed())
  {
    return true;
  }

  for (PaxTypeFare* ptf : _allPaxTypeFare)
  {
    if (ptf->isRequestedFareBasisInvalid() || !ptf->isRoutingProcessed())
      continue;

    if (ptf->isRoutingValid())
    {
      return false;
    }
  }
  return true;
}

bool
FareMarket::areAllFaresFailingOnBookingCodes() const
{
  if (isBookingCodeNotProcessed())
  {
    return true;
  }

  for (PaxTypeFare* ptf : _allPaxTypeFare)
  {
    if (ptf->isRequestedFareBasisInvalid() || !ptf->isBookingCodeProcessed())
      continue;

    if (ptf->isBookingCodePass())
    {
      return false;
    }
  }
  return true;
}

bool
FareMarket::isTravelSegPresent(const TravelSeg* seg) const
{
  return std::any_of(_travelSeg.begin(),
                     _travelSeg.end(),
                     [&seg](const TravelSeg* ts)
                     { return seg->segmentOrder() == ts->segmentOrder(); });
}

void
FareMarket::clone(FareMarket& cloneObj) const
{
  cloneObj._allPaxTypeFare = _allPaxTypeFare;
  cloneObj._availBreaks = _availBreaks;
  cloneObj._boardMultiCities = _boardMultiCities;
  cloneObj._boardMultiCity = _boardMultiCity;
  cloneObj.setBreakIndicator(breakIndicator());
  cloneObj.setBypassCat19FlagsSet(bypassCat19FlagsSet());
  cloneObj._changeStatus = _changeStatus;
  cloneObj._classOfServiceVec = _classOfServiceVec;
  cloneObj.setCombineSameCxr(combineSameCxr());
  cloneObj._currencies = _currencies;
  cloneObj._destination = _destination;
  cloneObj._direction = _direction;
  cloneObj._failCode = _failCode;
  cloneObj._fareBasisCode = _fareBasisCode;
  cloneObj._multiPaxUniqueFareBasisCodes = _multiPaxUniqueFareBasisCodes;
  cloneObj._fbcUsage = _fbcUsage;
  cloneObj._fareCalcFareAmt = _fareCalcFareAmt;
  cloneObj.setFlowMarket(flowMarket());
  cloneObj.setFltTrkIndicator(fltTrkIndicator());
  cloneObj._fmSavedFnUMap = _fmSavedFnUMap;
  cloneObj._fmSavedGfrUMap = _fmSavedGfrUMap;
  cloneObj._geoTravelType = _geoTravelType;
  cloneObj._globalDirection = _globalDirection;
  cloneObj._governingCarrier = _governingCarrier;
  cloneObj._governingCarrierPref = _governingCarrierPref;
  cloneObj.setHasAllMarriedSegs(hasAllMarriedSegs());
  cloneObj._inBoundAseanCurrencies = _inBoundAseanCurrencies;
  cloneObj._indirectEquivAmtCurrencyCode = _indirectEquivAmtCurrencyCode;
  cloneObj.setChildNeeded(isChildNeeded());
  cloneObj.setInfantNeeded(isInfantNeeded());
  cloneObj.setInfNeeded(isInfNeeded());
  cloneObj._legIndex = _legIndex;
  cloneObj._mInfo = _mInfo;
  cloneObj._mInfoOut = _mInfoOut;
  cloneObj._offMultiCities = _offMultiCities;
  cloneObj._offMultiCity = _offMultiCity;
  cloneObj._origin = _origin;
  cloneObj._outBoundAseanCurrencies = _outBoundAseanCurrencies;
  cloneObj._paxTypeCortege = _paxTypeCortege;
  cloneObj._primarySector = _primarySector;
  cloneObj._retrievalInfo = _retrievalInfo;
  cloneObj._reversedFareMarket = _reversedFareMarket;
  cloneObj._serviceStatus = _serviceStatus;
  cloneObj.setThru(isThru());
  cloneObj._sideTripTravelSeg = _sideTripTravelSeg;
  cloneObj._stopOverTravelSeg = _stopOverTravelSeg;
  cloneObj._travelBoundary = _travelBoundary;
  cloneObj._travelDate = _travelDate;
  cloneObj._ruleApplicationDate = _ruleApplicationDate;
  cloneObj._travelSeg = _travelSeg;
  cloneObj.setValidateFareGroup(validateFareGroup());
  cloneObj._webFareMatchSet = _webFareMatchSet;
  cloneObj.setSpecialRtgFound(specialRtgFound());
  cloneObj._fareCompInfo = _fareCompInfo;
  cloneObj.setRemoveOutboundFares(removeOutboundFares());
  cloneObj.setMergedAvailability(getMergedAvailability());
  cloneObj.setDualGoverningFlag(isDualGoverning());
  cloneObj._solComponentDirection = _solComponentDirection;
  cloneObj._solFmType = _solFmType;
  cloneObj._multiAirportInfo = _multiAirportInfo;
  cloneObj._validatingCarriers = _validatingCarriers;
  cloneObj._brandProgramIndexVec = _brandProgramIndexVec;
  cloneObj._brandAvailability = _brandAvailability;
  cloneObj._brandAvailabilityHardPassesOnly = _brandAvailabilityHardPassesOnly;
  cloneObj._hardPassAvailabilityOnly = _hardPassAvailabilityOnly;
  cloneObj._originsRequested = _originsRequested;
  cloneObj._destinationsRequested = _destinationsRequested;
}

void
FareMarket::addProcessedPTF(PaxTypeFare* ptfPtr)
{
  if (LIKELY(ptfPtr))
  {
    if (LIKELY(_allPaxTypeFareProcessedForCarrier))
    {
      std::vector<PaxTypeFare*>::iterator pTFIter =
          find(_allPaxTypeFareProcessedForCarrier->begin(),
               _allPaxTypeFareProcessedForCarrier->end(),
               ptfPtr);
      if (pTFIter != _allPaxTypeFareProcessedForCarrier->end())
        return;
      _allPaxTypeFareProcessedForCarrier->push_back(ptfPtr);
    }
    else
    {
      std::vector<PaxTypeFare*>::iterator pTFIter =
          find(_allPaxTypeFareProcessed.begin(), _allPaxTypeFareProcessed.end(), ptfPtr);
      if (pTFIter != _allPaxTypeFareProcessed.end())
        return;
      _allPaxTypeFareProcessed.push_back(ptfPtr);
    }
  }
}

inline static FareMarket::FareTypeGroupState
ftGroupToBitmask(NoPNRPricingTrx::FareTypes::FTGroup ftg)
{
  return FareMarket::FareTypeGroupState(1u << (ftg - 1));
}

void
FareMarket::setFareTypeGroupValid(NoPNRPricingTrx::FareTypes::FTGroup ftg)
{
  using FT = NoPNRPricingTrx::FareTypes;
  if (ftg > FT::FTG_NONE && ftg < FT::FTG_MAX)
    _fareTypeGroupStatus.set(ftGroupToBitmask(ftg));
}

bool
FareMarket::isFareTypeGroupValid(NoPNRPricingTrx::FareTypes::FTGroup ftg) const
{
  using FT = NoPNRPricingTrx::FareTypes;
  if (ftg > FT::FTG_NONE && ftg < FT::FTG_MAX)
    return _fareTypeGroupStatus.isSet(ftGroupToBitmask(ftg));
  return false;
}

FCChangeStatus&
FareMarket::changeStatus()
{
  TSE_ASSERT(getExcItinIndex() < _changeStatus.size());
  return _changeStatus[getExcItinIndex()];
}
const FCChangeStatus&
FareMarket::changeStatus() const
{
  TSE_ASSERT(getExcItinIndex() < _changeStatus.size());
  return _changeStatus[getExcItinIndex()];
}

FCChangeStatus&
FareMarket::asBookChangeStatus()
{
  TSE_ASSERT(getExcItinIndex() < _asBookChangeStatus.size());
  return _asBookChangeStatus[getExcItinIndex()];
}

const FCChangeStatus&
FareMarket::asBookChangeStatus() const
{
  TSE_ASSERT(getExcItinIndex() < _asBookChangeStatus.size());
  return _asBookChangeStatus[getExcItinIndex()];
}

FareMarket::RetrievalInfo*
FareMarket::RetrievalInfo::construct(const PricingTrx& trx,
                                     const DateTime& date,
                                     const FareMarket::FareRetrievalFlags& flag)
{
  RetrievalInfo* info = nullptr;
  trx.dataHandle().get(info);
  info->_date = date;
  info->_flag = flag;
  return info;
}

MileageInfo*&
FareMarket::mileageInfo(bool isOutbound)
{
  return (isOutbound ? _mInfoOut : _mInfo);
}

const MileageInfo*
FareMarket::mileageInfo(bool isOutbound) const
{
  return (isOutbound ? _mInfoOut : _mInfo);
}

namespace
{
struct Cloner
{
  Cloner(DataHandle& dh) : _dh(dh) {}

  TravelSeg* operator()(TravelSeg* seg) { return seg->clone(_dh); }

protected:
  DataHandle& _dh;
};
}

void
FareMarket::recreateTravelSegments(DataHandle& dh)
{
  std::vector<TravelSeg*> segs(_travelSeg.size());

  std::transform(_travelSeg.begin(), _travelSeg.end(), segs.begin(), Cloner(dh));
  _travelSeg.swap(segs);
}

void
FareMarket::setCarrierInAirSegments(const CarrierCode& carrier)
{
  for (const auto elem : _travelSeg)
  {
    if (AirSeg* airSeg = elem->toAirSeg())
    {
      airSeg->setMarketingCarrierCode(carrier);
      airSeg->setOperatingCarrierCode(carrier);
    }
  }
}

namespace
{
bool
findCxr(const CarrierCode& carrier, const std::vector<CarrierApplicationInfo*>& carrierApplList)
{
  if (carrier.empty())
    return true;

  return std::any_of(carrierApplList.begin(),
                     carrierApplList.end(),
                     [carrier](const CarrierApplicationInfo* const cai)
                     { return carrier == cai->carrier(); });
}
}

bool
FareMarket::findFMCxrInTbl(PricingTrx::TrxType trxType,
                           const std::vector<CarrierApplicationInfo*>& carrierApplList) const
{
  if (carrierApplList.empty())
    return false;

  if (trxType == PricingTrx::FAREDISPLAY_TRX)
  {
    return findCxr(governingCarrier(), carrierApplList);
  }

  if (primarySector() != nullptr)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(primarySector());

    if (!airSeg)
      return false;

    return findCxr(airSeg->carrier(), carrierApplList);
  }

  // get all carriers in the fare market
  std::vector<TravelSeg*>::const_iterator TvlSegI = travelSeg().begin();
  const std::vector<TravelSeg*>::const_iterator TvlSegIEnd = travelSeg().end();

  for (; TvlSegI != TvlSegIEnd; TvlSegI++)
  {
    const AirSeg* airSeg = dynamic_cast<AirSeg*>(*TvlSegI);
    if (!airSeg)
      continue;

    if (findCxr(airSeg->carrier(), carrierApplList))
      return true;
  }

  return false;
}

// After sort, reassigning
void
FareMarket::setOrigDestByTvlSegs()
{
  _origin = _travelSeg.front()->origin();
  _boardMultiCity = _travelSeg.front()->boardMultiCity();
  _destination = _travelSeg.back()->destination();
  _offMultiCity = _travelSeg.back()->offMultiCity();
}

void
FareMarket::setMarketCurrencyPresent(PricingTrx& trx)
{
  for (PaxTypeBucket& ptc : _paxTypeCortege)
  {
    ptc.setMarketCurrencyPresent(trx, *this);
  }
}

namespace
{
struct IsPrivateFare
{
  bool operator()(const PaxTypeFare* ptf) const { return 1 == ptf->tcrTariffCat(); }
};

struct IsTCRRuleTariffZero
{
  bool operator()(const PaxTypeFare* ptf) const { return 0 == ptf->tcrRuleTariff(); }
};

struct TCRRuleTariffLess
{
  TCRRuleTariffLess(TariffNumber value = -1) : _value(value) {}

  bool operator()(const PaxTypeFare* first, const PaxTypeFare* second) const
  {
    if (first != nullptr && second != nullptr)
    {
      return first->tcrRuleTariff() < second->tcrRuleTariff();
    }
    else if (nullptr == first)
    {
      return _value < second->tcrRuleTariff();
    }
    else if (LIKELY(nullptr == second))
    {
      return first->tcrRuleTariff() < _value;
    }
    return false;
  }
  TariffNumber _value;
};

} // namespace

class PTFRangeImpl
{
public:
  PTFRangeImpl() : _fareMarket(nullptr) {}

  ~PTFRangeImpl()
  {
    if (LIKELY(_fareMarket))
    {
      _fareMarket->resetPTFRangeImpl();
    }
  }

  void initialize(FareMarket* fareMarket, std::vector<PaxTypeFare*>& allPaxTypeFare)
  {
    if (LIKELY(nullptr == _fareMarket))
    {
      _fareMarket = fareMarket;
      boost::remove_copy_if(allPaxTypeFare, std::back_inserter(_publicFares), IsPrivateFare());
      boost::remove_copy_if(
          allPaxTypeFare, std::back_inserter(_sortedFares), IsTCRRuleTariffZero());
      TCRRuleTariffLess pred;
      boost::stable_sort(_sortedFares, pred);
    }
  }

  PTFRange getRange(TariffNumber tariff) const
  {
    if (0 == tariff)
    {
      return PTFRange(_publicFares.begin(), _publicFares.end());
    }
    else
    {
      TCRRuleTariffLess pred(tariff);
      return boost::equal_range(_sortedFares, static_cast<PaxTypeFare*>(nullptr), pred);
    }
  }

private:
  FareMarket* _fareMarket;
  std::vector<PaxTypeFare*> _publicFares;
  std::vector<PaxTypeFare*> _sortedFares;
};

PTFRange
FareMarket::getPTFRange(TariffNumber tariff, DataHandle& dataHandle)
{
  if (nullptr == _ptfRangeImpl)
  {
    dataHandle.get(_ptfRangeImpl);
    _ptfRangeImpl->initialize(this, _allPaxTypeFare);
  }
  return _ptfRangeImpl->getRange(tariff);
}

void
FareMarket::resetPTFRangeImpl()
{
  _ptfRangeImpl = nullptr;
}

void
FareMarket::addFMItinMap(Itin* itin, bool spanishFamilyDiscount)
{
  boost::lock_guard<boost::mutex> g(_mutexFMItinMap);

  // map's insert() checks whether the element to be inserted
  // has a key equivalent to any one of the elements already
  // exists in the container before the insertion
  _fmItinMap.insert(std::pair<Itin*, bool>(itin, spanishFamilyDiscount));
}

void
FareMarket::forceStatusForBrand(const BrandCode& brandCode, Direction direction, IbfErrorMessage status,
  bool backwardCompabilityOnly)
{
  boost::unique_lock<boost::shared_mutex> lck(_mutexBrandAvailability);
  if (fallback::fixed::fallbackFmBrandStatus())
  {
    _brandAvailability[brandCode].forceStatus(status);
    _brandAvailabilityHardPassesOnly[brandCode].forceStatus(status);
  }
  else
  {
    if (direction == Direction::BOTHWAYS)
    {
      if (!backwardCompabilityOnly)
      {
        _brandDirectionAvailability[skipper::BrandCodeDirection(brandCode, Direction::ORIGINAL)]
            .forceStatus(status);
        _brandDirectionAvailabilityHardPassesOnly[
          skipper::BrandCodeDirection(brandCode, Direction::ORIGINAL)]
            .forceStatus(status);
        _brandDirectionAvailability[skipper::BrandCodeDirection(brandCode, Direction::REVERSED)]
            .forceStatus(status);
        _brandDirectionAvailabilityHardPassesOnly[
          skipper::BrandCodeDirection(brandCode, Direction::REVERSED)]
            .forceStatus(status);
      }
      //fill old vector too (only for bothways to not override status of one direction with the other)
      _brandAvailability[brandCode].forceStatus(status);
      _brandAvailabilityHardPassesOnly[brandCode].forceStatus(status);
    }
    else
    {
      _brandDirectionAvailability[skipper::BrandCodeDirection(brandCode, direction)]
         .forceStatus(status);
      _brandDirectionAvailabilityHardPassesOnly[skipper::BrandCodeDirection(brandCode, direction)]
         .forceStatus(status);
    }
  }
}

IbfErrorMessage
FareMarket::getStatusForBrandWithoutDirection(const BrandCode& brandCode) const
{
  {
    // scope for lock -- do not remove
    boost::shared_lock<boost::shared_mutex> lck(_mutexBrandAvailability);
    BrandAvailability::const_iterator found;
    bool gotIt = false;
    if (useHardPassBrandAvailabilityOnly(BrandCodeDirection(brandCode, Direction::BOTHWAYS)))
    {
      found = _brandAvailabilityHardPassesOnly.find(brandCode);
      gotIt = (found != _brandAvailabilityHardPassesOnly.end());
    }
    else
    {
      found = _brandAvailability.find(brandCode);
      gotIt = (found != _brandAvailability.end());
    }
    if (gotIt)
    {
      const IbfErrorMessage response = found->second.getStatus();
      LOG4CXX_DEBUG(logger,
                    "FM (" << origin()->loc() << "-" << destination()->loc()
                           << "), brand = " << brandCode << ", returning " << response);
      return response;
    }
  }

  // Get the default value for this type
  const IbfErrorMessage default_response = IbfErrorMessageFmAcc().getStatus();
  LOG4CXX_DEBUG(logger,
                "FM (" << origin()->loc() << "-" << destination()->loc()
                       << "), no status for brand " << brandCode << ", returning "
                       << default_response);
  return default_response;
}

IbfErrorMessage
FareMarket::getStatusForBrand(const BrandCode& brandCode, Direction direction) const
{
  if (fallback::fixed::fallbackFmBrandStatus())
    return getStatusForBrandWithoutDirection(brandCode);

  if (direction == Direction::BOTHWAYS)
    //if direction is BOTHWAYS use old status
    return getStatusForBrandWithoutDirection(brandCode);

  {
    // scope for lock -- do not remove
    boost::shared_lock<boost::shared_mutex> lck(_mutexBrandAvailability);
    {
      BrandDirectionAvailability::const_iterator found;
      bool gotIt = false;
      if (useHardPassBrandAvailabilityOnly(BrandCodeDirection(brandCode, direction)))
      {
        found = _brandDirectionAvailabilityHardPassesOnly.find(
            skipper::BrandCodeDirection(brandCode, direction));
        gotIt = (found != _brandDirectionAvailabilityHardPassesOnly.end());
      }
      else
      {
        found = _brandDirectionAvailability.find(skipper::BrandCodeDirection(brandCode, direction));
        gotIt = (found != _brandDirectionAvailability.end());
      }
      if (gotIt)
      {
        const IbfErrorMessage response = found->second.getStatus();
        LOG4CXX_DEBUG(logger,
                      "FM (" << origin()->loc() << "-" << destination()->loc()
                             << "), brand = " << brandCode << ", direction = "
                             << directionToString(direction) << " returning " << response);
        return response;
      }
    }
  }

  // Get the default value for this type
  const IbfErrorMessage default_response = IbfErrorMessageFmAcc().getStatus();
  LOG4CXX_DEBUG(logger,
                "FM (" << origin()->loc() << "-" << destination()->loc()
                       << "), no status for brand " << brandCode << " in direction "
                       << directionToString(direction) << ", returning "
                       << default_response);
  return default_response;
}

void
FareMarket::updateStatusForBrand(const BrandCode& brandCode,
                                 Direction direction,
                                 IbfErrorMessage status,
                                 bool isHardPass)
{
  if (status == IbfErrorMessage::IBF_EM_NOT_SET)
  {
    LOG4CXX_WARN(logger, "Expected error status, got " << status);
    return;
  }
  boost::unique_lock<boost::shared_mutex> lck(_mutexBrandAvailability);

  if (fallback::fixed::fallbackFmBrandStatus())
  {
    _brandAvailability[brandCode].updateStatus(status);
    if (isHardPass) // also update hard pass only availability
      _brandAvailabilityHardPassesOnly[brandCode].updateStatus(status);
  }
  else
  {
    if (direction == Direction::BOTHWAYS)
    {
      _brandDirectionAvailability[skipper::BrandCodeDirection(brandCode, Direction::ORIGINAL)]
        .updateStatus(status);
      _brandDirectionAvailability[skipper::BrandCodeDirection(brandCode, Direction::REVERSED)]
        .updateStatus(status);
    }
    else
      _brandDirectionAvailability[skipper::BrandCodeDirection(brandCode, direction)]
        .updateStatus(status);
    //fill old vector too
    _brandAvailability[brandCode].updateStatus(status);

    if (isHardPass) // also update hard pass only availability
    {
      if (direction == Direction::BOTHWAYS)
      {
        _brandDirectionAvailabilityHardPassesOnly[
          skipper::BrandCodeDirection(brandCode, Direction::ORIGINAL)].updateStatus(status);
        _brandDirectionAvailabilityHardPassesOnly[
          skipper::BrandCodeDirection(brandCode, Direction::REVERSED)].updateStatus(status);
      }
      else
        _brandDirectionAvailabilityHardPassesOnly[skipper::BrandCodeDirection(brandCode, direction)]
          .updateStatus(status);
      //fill old vector too
      _brandAvailabilityHardPassesOnly[brandCode].updateStatus(status);
    }
  }
}

std::string
FareMarket::toString() const
{
  std::ostringstream out;
  out << boardMultiCity() << "-" << governingCarrier() << "-" << offMultiCity();
  return out.str();
}

template <typename KEY, typename Value>
struct CategoryEqual
{
  typedef std::pair<KEY, Value> Pair;

  CategoryEqual(uint32_t category) : _category(category) {}

  bool operator()(const Pair& pair) const { return pair.first.categoryNumber() == _category; }

private:
  uint32_t _category;
};

std::vector<FareMarket::FareMarketSavedGfrResult::Results>*
FareMarket::saveGfrUMList(DataHandle& dataHandle,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const TariffNumber& tcrRuleTariff,
                          const RuleNumber& ruleNumber,
                          uint32_t categoryNumber,
                          const DateTime& travelDate,
                          const DateTime& returnDate,
                          const DateTime& ticketDate,
                          const GeneralFareRuleInfoVec* gfrList)
{
  FareMarketSavedGfrResult* gfrResult(nullptr);
  dataHandle.get(gfrResult);

  if (UNLIKELY(!gfrResult))
    return nullptr;

  GfrRecord2Key key(vendor,
                    carrier,
                    tcrRuleTariff,
                    ruleNumber,
                    categoryNumber,
                    travelDate,
                    returnDate,
                    ticketDate);

  gfrResult->gfrList() = const_cast<GeneralFareRuleInfoVec*>(gfrList);
  gfrResult->resultVec().resize(gfrList->size());

  FMScopedLock guard(fmSavedGfrMapMutex());
  _fmSavedGfrUMap[key] = gfrResult;

  return &(gfrResult->resultVec());
}

GeneralFareRuleInfoVec*
FareMarket::getGfrList(const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& tcrRuleTariff,
                       const RuleNumber& ruleNumber,
                       uint32_t categoryNumber,
                       const DateTime& travelDate,
                       const DateTime& returnDate,
                       const DateTime& ticketDate,
                       std::vector<FareMarket::FareMarketSavedGfrResult::Results>*& resultVec)
{
  GeneralFareRuleInfoVec* gfrList(nullptr);

  GfrRecord2Key key(vendor,
                    carrier,
                    tcrRuleTariff,
                    ruleNumber,
                    categoryNumber,
                    travelDate,
                    returnDate,
                    ticketDate);

  FMScopedLock guard(fmSavedGfrMapMutex());
  FMSavedGfrUMap::iterator it(_fmSavedGfrUMap.find(key));
  if (it != _fmSavedGfrUMap.end())
  {
    FareMarketSavedGfrResult* savedGfrResult(it->second);
    gfrList = savedGfrResult->gfrList();
    resultVec = &savedGfrResult->resultVec();
  }

  return gfrList;
}

void
FareMarket::resetGfrResultUMap(uint32_t categoryNumber)
{
  FMScopedLock guard(fmSavedGfrMapMutex());
  FMSavedGfrUMap::iterator gfrResultMapI(_fmSavedGfrUMap.begin());
  const FMSavedGfrUMap::iterator gfrResultMapIEnd(_fmSavedGfrUMap.end());
  CategoryEqual<GfrRecord2Key, FareMarketSavedGfrResult*> pred(categoryNumber);
  while ((gfrResultMapI = std::find_if(gfrResultMapI, gfrResultMapIEnd, pred)) != gfrResultMapIEnd)
  {
    FareMarketSavedGfrResult* const resultByVCTR(gfrResultMapI->second);
    std::vector<FareMarket::FareMarketSavedGfrResult::Results>::iterator resultIter =
        resultByVCTR->resultVec().begin();
    const std::vector<FareMarket::FareMarketSavedGfrResult::Results>::iterator resultIterEnd =
        resultByVCTR->resultVec().end();

    for (; resultIter != resultIterEnd; ++resultIter)
    {
      FareMarketSavedGfrResult::Results& results = *resultIter;
      FMScopedLock guard(results.resultMapMutex());
      FareMarketSavedGfrResult::ResultMap& resultByKey = results.resultMap();

      FareMarketSavedGfrResult::ResultMap::iterator iter = resultByKey.begin();
      const FareMarketSavedGfrResult::ResultMap::iterator iterEnd = resultByKey.end();

      for (; iter != iterEnd; ++iter)
      {
        FareMarketSavedGfrResult::Result& result =
            const_cast<FareMarketSavedGfrResult::Result&>(*(iter->second));
        result._ret = NOTPROCESSED;
      }
    }
    ++gfrResultMapI;
  }
}

std::vector<FareMarket::FareMarketSavedFnResult::Results>*
FareMarket::saveFnCtlUMLst(DataHandle& dataHandle,
                           const VendorCode& vendor,
                           const CarrierCode& carrier,
                           const TariffNumber& tcrRuleTariff,
                           const Footnote& footnote,
                           uint32_t categoryNumber,
                           const DateTime& travelDate,
                           const DateTime& returnDate,
                           const DateTime& ticketDate,
                           const FootNoteCtrlInfoVec* fnCtrlList)
{
  FareMarketSavedFnResult* fnResult(nullptr);
  dataHandle.get(fnResult);

  if (UNLIKELY(!fnResult))
    return nullptr;

  FnRecord2Key key(
      vendor, carrier, tcrRuleTariff, footnote, categoryNumber, travelDate, returnDate, ticketDate);

  fnResult->fnCtrlList() = const_cast<FootNoteCtrlInfoVec*>(fnCtrlList);
  fnResult->resultVec().resize(fnCtrlList->size());

  FMScopedLock guard(fmSavedFnMapMutex());
  _fmSavedFnUMap[key] = fnResult;

  return &(fnResult->resultVec());
}

FootNoteCtrlInfoVec*
FareMarket::getFnCtrlList(const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const TariffNumber& tcrRuleTariff,
                          const Footnote& footnote,
                          uint32_t categoryNumber,
                          const DateTime& travelDate,
                          const DateTime& returnDate,
                          const DateTime& ticketDate,
                          std::vector<FareMarket::FareMarketSavedFnResult::Results>*& resultVec)
{
  FootNoteCtrlInfoVec* fnCtrlList(nullptr);

  FnRecord2Key key(
      vendor, carrier, tcrRuleTariff, footnote, categoryNumber, travelDate, returnDate, ticketDate);
  FMScopedLock guard(fmSavedFnMapMutex());
  FMSavedFnUMap::iterator savedFnMapI(_fmSavedFnUMap.find(key));
  if (savedFnMapI != _fmSavedFnUMap.end())
  {
    FareMarketSavedFnResult* savedFnResult(savedFnMapI->second);
    fnCtrlList = savedFnResult->fnCtrlList();
    resultVec = &savedFnResult->resultVec();
  }

  return fnCtrlList;
}

void
FareMarket::resetFnResultUMap(uint32_t categoryNumber)
{
  FMScopedLock guard(fmSavedFnMapMutex());
  FMSavedFnUMap::iterator fnResultMapI(_fmSavedFnUMap.begin());
  const FMSavedFnUMap::iterator fnResultMapIEnd(_fmSavedFnUMap.end());

  CategoryEqual<FnRecord2Key, FareMarketSavedFnResult*> pred(categoryNumber);
  while ((fnResultMapI = std::find_if(fnResultMapI, fnResultMapIEnd, pred)) != fnResultMapIEnd)
  {
    FareMarketSavedFnResult* const resultByVCTR(fnResultMapI->second);
    std::vector<FareMarket::FareMarketSavedFnResult::Results>::iterator resultIter =
        resultByVCTR->resultVec().begin();
    const std::vector<FareMarket::FareMarketSavedFnResult::Results>::iterator resultIterEnd =
        resultByVCTR->resultVec().end();

    for (; resultIter != resultIterEnd; ++resultIter)
    {
      FareMarketSavedFnResult::Results& results = *resultIter;
      FMScopedLock guard(results.resultMapMutex());
      FareMarketSavedFnResult::ResultMap& resultByKey = results.resultMap();

      FareMarketSavedFnResult::ResultMap::iterator iter = resultByKey.begin();
      const FareMarketSavedFnResult::ResultMap::iterator iterEnd = resultByKey.end();

      for (; iter != iterEnd; ++iter)
      {
        FareMarketSavedFnResult::Result& result =
            const_cast<FareMarketSavedFnResult::Result&>(*(iter->second));
        result._ret = NOTPROCESSED;
      }
    }
    ++fnResultMapI;
  }
}

bool
FareMarket::isApplicableForPbb() const
{
  TSE_ASSERT(!travelSeg().empty());
  SegmentBrandCodeComparator brandCoseComp(travelSeg().front());

  return boost::algorithm::all_of(travelSeg().begin() + 1, travelSeg().end(), brandCoseComp);
}

bool
FareMarket::hasBrandCode() const
{
  if (isApplicableForPbb() && !travelSeg().front()->getBrandCode().empty())
    return true;

  return false;
}

BrandCode
FareMarket::getBrandCode() const
{
  if (hasBrandCode())
    return travelSeg().front()->getBrandCode();

  return BrandCode();
}

std::string
FareMarket::getHashKey() const
{
  return _travelSeg.front()->departureDT().toSimpleString() + _origin->loc() + _destination->loc() +
         _governingCarrier;
}

bool
FareMarket::isFullyFlownFareMarket() const
{
  return std::none_of(_travelSeg.cbegin(),
                      _travelSeg.cend(),
                      [](const TravelSeg* const seg)
                      { return seg->unflown(); });
}

bool
FareMarket::doesSpanMoreThanOneLeg() const
{
  return !_travelSeg.empty() && _travelSeg.front()->legId() != _travelSeg.back()->legId();
}

bool
FareMarket::hasTravelSeg(const TravelSeg* const tvlSeg) const
{
  return std::find(_travelSeg.cbegin(), _travelSeg.cend(), tvlSeg) != _travelSeg.cend();
}

void
FareMarket::setBKSNotApplicapleIfBKSNotPASS()
{
  if (!(_failCode == ErrorResponseException::NO_ERROR ||
        _failCode == ErrorResponseException::NO_FARE_FOR_CLASS ||
        _failCode == ErrorResponseException::NO_FARE_FOR_CLASS_USED))
    return;

  for (PaxTypeFare* paxTypeFare : _allPaxTypeFare)
    paxTypeFare->setBKSNotApplicapleIfBKSNotPASS();

  _serviceStatus.set(FareMarket::FareValidator, false);
  _failCode = ErrorResponseException::NO_ERROR;
}

bool
FareMarket::hasAnyFareValid() const
{
  return std::any_of(_paxTypeCortege.cbegin(),
                     _paxTypeCortege.cend(),
                     [](const auto& paxTypeCortege)
                     { return paxTypeCortege.hasAnyFareValid(); });
}

bool
FareMarket::isCmdPricing() const
{
  return ((!fareBasisCode().empty() || isMultiPaxUniqueFareBasisCodes()) &&
          fbcUsage() == COMMAND_PRICE_FBC && fareCalcFareAmt().empty());
}

} // tse
