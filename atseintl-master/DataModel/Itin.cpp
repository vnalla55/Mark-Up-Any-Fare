//-------------------------------------------------------------------
//
//  File:        Itin.cpp
//  Created:     March 8, 2004
//  Authors:
//
//  Description: The Itinerary class object represents one
//               itinerary from incoming shopping/pricing request.
//
//  Updates:
//          03/08/04 - VN - file created.
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
#include "DataModel/Itin.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ESVOptions.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PrecalcBaggageData.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "ServiceFees/ServiceFeesGroup.h"

#include <boost/tokenizer.hpp>

#include <algorithm>
#include <iterator>
#include <utility>

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp)
int16_t
Itin::segmentOrder(const TravelSeg* segment) const
{
  const auto segIt = std::find(_travelSeg.cbegin(), _travelSeg.cend(), segment);
  if (segIt == _travelSeg.end())
  {
    const auto iter = std::find_if(_travelSeg.cbegin(),
                                   _travelSeg.cend(),
                                   [segment](const TravelSeg* tvlSeg)
                                   {
      return ((tvlSeg->boardMultiCity() == segment->boardMultiCity()) &&
              (tvlSeg->offMultiCity() == segment->offMultiCity()) &&
              (tvlSeg->departureDT().get64BitRepDateOnly() ==
               segment->departureDT().get64BitRepDateOnly()));
    });
    if (iter != _travelSeg.end())
      return (std::distance(_travelSeg.begin(), iter) + 1);
    return -1;
  }
  return (std::distance(_travelSeg.begin(), segIt) + 1);
}

bool
Itin::hasTvlSegInFlowFareMarkets(const TravelSeg* segment) const
{
  return std::any_of(_flowFareMarket.cbegin(),
                     _flowFareMarket.cend(),
                     [segment](const FareMarket* fareMarket)
                     { return fareMarket->hasTravelSeg(segment); });
}

int16_t
Itin::segmentPnrOrder(const TravelSeg* segment) const
{
  return (segmentOrder(segment) == -1) ? int16_t(-1) : segment->pnrSegment();
}

bool
Itin::segmentFollows(const TravelSeg* a, const TravelSeg* b) const
{
  const std::vector<TravelSeg*>::const_iterator itor =
      std::find(_travelSeg.begin(), _travelSeg.end(), a);
  if (_travelSeg.end() - itor < 2)
  {
    return false;
  }
  else
  {
    return *(itor + 1) == b;
  }
}

bool
Itin::segmentFollowsAfterArunk(const TravelSeg* a, const TravelSeg* b) const
{
  const std::vector<TravelSeg*>::const_iterator itor =
      std::find(_travelSeg.begin(), _travelSeg.end(), a);
  if (_travelSeg.end() - itor < 3)
  {
    return false;
  }
  else
  {
    return dynamic_cast<const ArunkSeg*>(*(itor + 1)) != nullptr && *(itor + 2) == b;
  }
}

void
Itin::setItinBranding(skipper::ItinBranding* itinBranding)
{
  TSE_ASSERT(itinBranding != nullptr);
  TSE_ASSERT(_itinBranding == nullptr);
  _itinBranding = itinBranding;
}

size_t
Itin::getTODBucket(const std::vector<std::pair<uint16_t, uint16_t> >& todRanges) const
{
  int32_t departMin = getDepartTime().hours() * 60 + getDepartTime().minutes();
  for (size_t i = 0; i < todRanges.size(); ++i)
    if (departMin >= todRanges[i].first && departMin <= todRanges[i].second)
      return i;
  return 0;
}

void
Itin::calculateMileage(PricingTrx* trx)
{
  _mileage = 0;

  bool fsActiveForShoping = trx->isIataFareSelectionApplicable();
  for (const auto travelSeg : _travelSeg)
  {
    if (UNLIKELY(fsActiveForShoping))
    {
      if (travelSeg->mileage())
      {
        _mileage += travelSeg->mileage();
        continue;
      }
    }

    GlobalDirection gd;
    GlobalDirectionFinderV2Adapter::getGlobalDirection(
        trx, travelSeg->departureDT(), *travelSeg, gd);
    uint32_t segMileage = LocUtil::getTPM(*travelSeg->origin(),
                                          *travelSeg->destination(),
                                          gd,
                                          travelSeg->departureDT(),
                                          trx->dataHandle());
    _mileage += segMileage;

    if (UNLIKELY(fsActiveForShoping))
    {
      travelSeg->mileage() = segMileage;
    }
  }
}

// ******************************** ESV/VIS  ******************************** //

void
Itin::updateTotalPenalty(const ESVOptions* esvOptions, const DateTime& reqDepDateTime)
{
  int stopPenalty = esvOptions->perStopPenalty() * 100; // in dollars
  int travDurPenalty = esvOptions->travelDurationPenalty(); // in cents
  int depDevPenalty = esvOptions->departureTimeDeviationPenalty(); // in cents

  MoneyAmount totalStopPenalty = ((float)(ShoppingUtil::totalStopPenalty(this, stopPenalty)) / 100);

  MoneyAmount totalTravDurPenalty =
      ((float)(ShoppingUtil::totalTravDurPenalty(this, travDurPenalty)) / 100);

  MoneyAmount totalDepTimeDevPenalty =
      ((float)(ShoppingUtil::totalDepTimeDevPenalty(this, depDevPenalty, reqDepDateTime)) / 100);

  _totalPenalty = totalStopPenalty + totalDepTimeDevPenalty + totalTravDurPenalty;
}

void
Itin::calculateFlightTimeMinutes(const int32_t& hoursODOffset, const int32_t& legIdx)
{
  const DateTime& depTime = getDepartTime();
  const DateTime& arrTime = getArrivalTime();

  boost::posix_time::time_duration diff = arrTime - depTime;
  boost::posix_time::hours offset(hoursODOffset);

  if (0 == legIdx)
  {
    diff += offset;
  }
  else
  {
    diff -= offset;
  }

  _flightTimeMinutes = diff.hours() * 60 + diff.minutes();
}

const DateTime&
Itin::getDepartTime() const
{
  return firstTravelSeg()->departureDT();
}

const DateTime&
Itin::getArrivalTime() const
{
  return lastTravelSeg()->arrivalDT();
}

void
Itin::duplicate(const Itin& itin, DataHandle& dataHandle)
{
  duplicateBase(itin, dataHandle);

  _ticketingCarrierPref = const_cast<CarrierPreference*>(itin._ticketingCarrierPref);

  std::copy(itin._fareMarket.begin(), itin._fareMarket.end(), std::back_inserter(_fareMarket));
}

void
Itin::duplicate(const Itin& itin, const PaxTypeFare& ptf, DataHandle& dataHandle)
{
  duplicateBase(itin, dataHandle);

  FareMarket* fm = const_cast<FareMarket*>(ptf.fareMarket());
  _fareMarket.push_back(fm);
}

void
Itin::invalidateAllPaxTypeFaresForRetailer(PricingTrx& trx)
{
  for (const auto fareMarket : _fareMarket)
  {
    if (UNLIKELY(!fareMarket))
      continue;
    fareMarket->invalidateAllPaxTypeFaresForRetailer(trx);
  }
}

void
Itin::duplicateBase(const Itin& itin, DataHandle& dataHandle)
{
  _geoTravelType = itin._geoTravelType;
  _ticketingCarrier = itin._ticketingCarrier;
  _validatingCarrier = itin._validatingCarrier;

  _sequenceNumber = itin._sequenceNumber;
  _salesNationRestr = itin._salesNationRestr;
  _errResponseCode = itin._errResponseCode;
  _errResponseMsg = itin._errResponseMsg;

  std::copy(itin._travelSeg.begin(), itin._travelSeg.end(), std::back_inserter(_travelSeg));
  std::copy(itin._taxResponse.begin(), itin._taxResponse.end(), std::back_inserter(_taxResponse));

  _intlSalesIndicator = itin._intlSalesIndicator;
  _tripCharacteristics = itin._tripCharacteristics;
  _calculationCurrency = itin._calculationCurrency;
  _originationCurrency = itin._originationCurrency;

  std::copy(itin._paxGroup.begin(), itin._paxGroup.end(), std::back_inserter(_paxGroup));

  _simpleTrip = itin._simpleTrip;
  setTravelDate(itin._travelDate);
  _bookingDate = itin._bookingDate;
  _maxDepartureDT = itin._maxDepartureDT;
  _useInternationalRounding = itin._useInternationalRounding;

  _itinBranding = itin._itinBranding;

  std::copy(itin._legID.begin(), itin._legID.end(), std::back_inserter(_legID));
  std::copy(itin._csTextMessages.begin(),
            itin._csTextMessages.end(),
            std::back_inserter(_csTextMessages));

  if (itin._datePair)
  {
    dataHandle.get(_datePair);
    *_datePair = *itin._datePair;
  }
}

void
Itin::addSimilarItin(Itin* itin)
{
  _similarItinsData.emplace_back(itin);
}

void
Itin::addSimilarItins(std::vector<SimilarItinData>::const_iterator begin,
                      std::vector<SimilarItinData>::const_iterator end)
{
  _similarItinsData.insert(_similarItinsData.end(), begin, end);
}

bool
Itin::eraseSimilarItin(Itin* itin)
{
  const auto pos = std::find_if(_similarItinsData.begin(),
                                _similarItinsData.end(),
                                [itin](const SimilarItinData& data)
                                { return data.itin == itin; });
  if (pos == _similarItinsData.end())
    return false;

  _similarItinsData.erase(pos);
  return true;
}

bool
Itin::rotateHeadOfSimilarItins(Itin* newHead)
{
  if (!eraseSimilarItin(newHead))
    return false;

  TSE_ASSERT(newHead);
  TSE_ASSERT(newHead->_similarItinsData.empty());
  newHead->_similarItinsData.swap(_similarItinsData);

  newHead->addSimilarItin(this);

  return true;
}

std::vector<SimilarItinData>
Itin::clearSimilarItins()
{
  std::vector<SimilarItinData> result;
  result.swap(_similarItinsData);
  return result;
}

void
Itin::swapSimilarItins(std::vector<SimilarItinData>& itins)
{
  _similarItinsData.swap(itins);
}

const SimilarItinData*
Itin::getSimilarItinData(const Itin& similarItin) const
{
  auto it = std::find_if(_similarItinsData.cbegin(),
                         _similarItinsData.cend(),
                         [similarItinPtr = &similarItin](const auto& data)
                         { return data.itin == similarItinPtr; });
  return it != _similarItinsData.cend() ? &(*it) : nullptr;
}

void
Itin::addFareMarketJustForRexPlusUps(const FareMarket* fm)
{
  _fmsJustForRexPlusUps.insert(fm);
}

bool
Itin::isFareMarketJustForRexPlusUps(const FareMarket* fm) const
{
  return _fmsJustForRexPlusUps.find(fm) != _fmsJustForRexPlusUps.end();
}

void
Itin::getValidatingCarriers(const PricingTrx& trx, std::vector<CarrierCode>& result) const
{
  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    if (!_spValidatingCxrGsaDataMap)
      return;

    for (const auto& item : *_spValidatingCxrGsaDataMap)
      if (item.second)
        getValidatingCarriers(*item.second, result);
  }
  else
  {
    if (!_validatingCxrGsaData)
      return;
    getValidatingCarriers(*_validatingCxrGsaData, result);
  }
}

void
Itin::getValidatingCarriers(const ValidatingCxrGSAData& valCxrGsaData,
                            std::vector<CarrierCode>& result) const
{
  for (const auto&  item : valCxrGsaData.validatingCarriersData())
    if (std::find(result.begin(), result.end(), item.first) == result.end())
      result.push_back(item.first);
}

bool
Itin::hasFareMarket(const FareMarket* searchedFareMarket) const
{
  return std::any_of(_fareMarket.cbegin(),
                     _fareMarket.cend(),
                     [searchedFareMarket](const FareMarket* fareMarket)
                     { return searchedFareMarket == fareMarket; });
}

void
Itin::removeNonDefaultValidatingCarriers(CarrierCode & vcr) const
{
  if (!_validatingCxrGsaData)
    return;

  ValidatingCxrDataMap::iterator it = _validatingCxrGsaData->validatingCarriersData().begin();
  while ( it != _validatingCxrGsaData->validatingCarriersData().end() )
  {
    if (vcr != it->first)
    {
         _validatingCxrGsaData->validatingCarriersData().erase(it++);
    }
    else
     it++;
  }
}

// Remove alternates and if no validating carriers left in ValidatingCxrGsaData
// then remove it from spValidatingCxrGsaDataMap
void
Itin::removeAlternateValidatingCarriers()
{
  if (!_spValidatingCxrGsaDataMap || _validatingCarrier.empty())
    return;

  auto itSpM = _spValidatingCxrGsaDataMap->begin();
  while (itSpM != _spValidatingCxrGsaDataMap->end())
  {
    if (!itSpM->second)
    {
      ++itSpM;
      continue;
    }

    ValidatingCxrGSAData& valCxrGsaData = *(const_cast<ValidatingCxrGSAData*>(itSpM->second));
    auto itValCxrM = valCxrGsaData.validatingCarriersData().begin();
    while (itValCxrM != valCxrGsaData.validatingCarriersData().end())
    {
      if (_validatingCarrier != itValCxrM->first)
        valCxrGsaData.validatingCarriersData().erase(itValCxrM++);
      else
        ++itValCxrM;
    }

    if (valCxrGsaData.validatingCarriersData().empty())
      _spValidatingCxrGsaDataMap->erase(itSpM++);
    else
      ++itSpM;
  }
}

bool
Itin::isValidatingCxrGsaDataForMultiSp() const
{
  if (!_spValidatingCxrGsaDataMap)
    return false;

  for (const auto& item : *_spValidatingCxrGsaDataMap)
  {
    if (item.second)
      return true;
  }

  return false;
}

bool
Itin::hasNeutralValidatingCarrier(const SettlementPlanType& sp) const
{
  if (!_spValidatingCxrGsaDataMap)
    return false;

  auto it = _spValidatingCxrGsaDataMap->find(sp);
  return (it != _spValidatingCxrGsaDataMap->end() &&
      it->second &&
      it->second->isNeutralValCxr());
}

const GSASwapMap&
Itin::gsaSwapMap(const SettlementPlanType& sp) const
{
  static std::map< CarrierCode, std::set<CarrierCode> > gswapMap;
  if (_spValidatingCxrGsaDataMap)
  {
    auto it = _spValidatingCxrGsaDataMap->find(sp);
    if (it != _spValidatingCxrGsaDataMap->end() && it->second)
      return it->second->gsaSwapMap();
  }
  return gswapMap;
}

bool
Itin::getSwapCarriers(const CarrierCode& carrier,
    std::set<CarrierCode>& result, const SettlementPlanType& sp) const
{
  if (!_spValidatingCxrGsaDataMap)
    return false;
  auto it = _spValidatingCxrGsaDataMap->find(sp);
  if (it != _spValidatingCxrGsaDataMap->end() && it->second)
    return it->second->getSwapCarriers(carrier, result);
  return false;
}

bool
Itin::isGeoConsistent(const Itin& itinToCompare) const
{
  const auto areSegmentsSimilar = [](const TravelSeg* const lhs, const TravelSeg* const rhs)
  {
    return lhs->legId() == rhs->legId() && lhs->isAir() == rhs->isAir() &&
           lhs->boardMultiCity() == rhs->boardMultiCity() &&
           lhs->offMultiCity() == rhs->offMultiCity();
  };

  return _travelSeg.size() == itinToCompare._travelSeg.size() &&
         std::equal(_travelSeg.cbegin(),
                    _travelSeg.cend(),
                    itinToCompare._travelSeg.cbegin(),
                    areSegmentsSimilar);
}

const PrecalcBaggage::PaxData&
Itin::getPrecalcBagPaxData(const PaxType& pt) const
{
  TSE_ASSERT(_precalcBaggageData);
  const auto paxDataIt = _precalcBaggageData->paxData.find(&pt);
  TSE_ASSERT(paxDataIt != _precalcBaggageData->paxData.end());
  return paxDataIt->second;
}

size_t
Itin::countPaxTypeInFarePaths(const PaxType& pt) const
{
  return std::count_if(_farePath.cbegin(),
                       _farePath.cend(),
                       [&pt](const FarePath* const rhs)
                       { return rhs && *rhs->paxType() == pt; });
}

std::set<uint16_t>
Itin::getPricedBrandCombinationIndexes() const
{
  std::set<uint16_t> pricedBrandCombIdxs;

  for (const FarePath* farePath : _farePath)
     pricedBrandCombIdxs.insert(farePath->brandIndex());

  return pricedBrandCombIdxs;
}

bool
Itin::hasAtLeastOneEmptyFMForGovCxr(CarrierCode cxr) const
{
  return std::any_of(_fareMarket.cbegin(),
                     _fareMarket.cend(),
                     [cxr](const FareMarket* const fm)
                     { return fm->governingCarrier() == cxr && fm->allPaxTypeFare().empty(); });
}
} //tse
