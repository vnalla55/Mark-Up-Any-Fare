///-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "ItinAnalyzer/ItinAnalyzerServiceWrapper.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/GoverningCarrier.h"
#include "Common/ItinUtil.h"
#include "Common/RtwUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag192Collector.h"
#include "ItinAnalyzer/FamilyLogicUtils.h"
#include "ItinAnalyzer/FareMarketBuilder.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"
#include "ItinAnalyzer/SimplifiedFareMarketsBuilder.h"
#include "Limitations/LimitationOnIndirectTravel.h"

#include <boost/tokenizer.hpp>

namespace tse
{
FALLBACK_DECL(fallbackSkipAcrossStopoverDomesticFM);
FALLBACK_DECL(fallbackFareSelction2016);
FALLBACK_DECL(segmentAttributesRefactor)
FALLBACK_DECL(simpleFareMarketBuilding)

namespace
{
ConfigurableValue<std::string>
acrossStopoverEnabledCarriers("SHOPPING_OPT", "ACROSS_STOPOVER_ENABLED_CARRIERS");
ConfigurableValue<bool>
excludeSidetripFm("SHOPPING_OPT", "EXCLUDE_SIDETRIP_FM", false);

bool isFMinMiddleOfLeg(const std::vector<SegmentAttributes>& segAttributes,
                       std::vector<SegmentAttributes>::const_iterator begin,
                       std::vector<SegmentAttributes>::const_iterator end)
{
  return (std::distance(begin, end) == 1 ||
          std::distance(begin, end) == 2 ) &&
         begin != segAttributes.begin() &&
         end != segAttributes.end() &&
         std::prev(begin)->tvlSeg->legId() == end->tvlSeg->legId();
}

bool isArunkFirst(std::vector<SegmentAttributes>::const_iterator begin)
{
  return begin->tvlSeg->segmentType() == Arunk;
}

bool isArunkLast(const std::vector<SegmentAttributes>& segAttributes,
                 std::vector<SegmentAttributes>::const_iterator end)
{
  return end != segAttributes.end() &&
         end->tvlSeg->segmentType() == Arunk;
}
}

ItinAnalyzerServiceWrapper::ItinAnalyzerServiceWrapper(ItinAnalyzerService& itinAnalyzer)
  : _itinAnalyzer(itinAnalyzer)
{
}

void
ItinAnalyzerServiceWrapper::initialize(PricingTrx& trx)
{
  if (fallback::fallbackSkipAcrossStopoverDomesticFM(&trx))
    return;

  if (trx.getTrxType() != PricingTrx::MIP_TRX)
    return;

  parseAcrossStopoverExcludedCarriersCities(acrossStopoverEnabledCarriers.getValue());

  if(!fallback::fallbackFareSelction2016(&trx) && trx.getTrxType()==PricingTrx::IS_TRX)
  {
    ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(trx);
    Itin& journeyItin = *shoppingTrx.journeyItin();

    Itin2SopInfoMap itinOrigSopInfoMap;
    for (ShoppingTrx::Leg& curLeg : shoppingTrx.legs())
    {
      itinOrigSopInfoMap.clear();
      itinOrigSopInfoMap = itinanalyzerutils::getSopInfoMap(journeyItin, curLeg);
      _itin2SopInfoMapVec.push_back(itinOrigSopInfoMap);
    }
  }
}

void
ItinAnalyzerServiceWrapper::processExchangeItin(ExchangePricingTrx& trx)
{
  _itinAnalyzer.processExchangeItin(trx);
}

void
ItinAnalyzerServiceWrapper::determineChanges(ExchangePricingTrx& trx)
{
  _itinAnalyzer.determineChanges(trx);
}

bool
ItinAnalyzerServiceWrapper::selectTicketingCarrier(PricingTrx& trx)
{
  return _itinAnalyzer.selectTicketingCarrier(trx);
}

void
ItinAnalyzerServiceWrapper::setRetransits(PricingTrx& trx)
{
  _itinAnalyzer.setRetransits(trx);
}

void
ItinAnalyzerServiceWrapper::setOpenSegFlag(PricingTrx& trx)
{
  _itinAnalyzer.setOpenSegFlag(trx);
}

void
ItinAnalyzerServiceWrapper::setTripCharacteristics(PricingTrx& trx)
{
  _itinAnalyzer.setTripCharacteristics(trx);
}

void
ItinAnalyzerServiceWrapper::checkJourneyActivation(PricingTrx& trx)
{
  _itinAnalyzer.checkJourneyActivation(trx);
}

void
ItinAnalyzerServiceWrapper::checkSoloActivation(PricingTrx& trx)
{
  _itinAnalyzer.checkSoloActivation(trx);
}

void
ItinAnalyzerServiceWrapper::setATAEContent(PricingTrx& trx)
{
  _itinAnalyzer.setATAEContent(trx);
  ItinUtil::removeAtaeMarkets(trx);
}

void
ItinAnalyzerServiceWrapper::setATAEAvailContent(PricingTrx& trx)
{
  _itinAnalyzer.setATAEAvailContent(trx);
  ItinUtil::removeAtaeMarkets(trx);
}
void
ItinAnalyzerServiceWrapper::setATAESchedContent(PricingTrx& trx)
{
  _itinAnalyzer.setATAESchedContent(trx);
}
void
ItinAnalyzerServiceWrapper::setItinRounding(PricingTrx& trx)
{
  _itinAnalyzer.setItinRounding(trx);
}

void
ItinAnalyzerServiceWrapper::setInfoForSimilarItins(PricingTrx& trx)
{
  FamilyLogicUtils::setInfoForSimilarItins(trx, &_itinAnalyzer);
}

bool
ItinAnalyzerServiceWrapper::buildFareMarket(PricingTrx& trx, Itin& itin)
{
  if (trx.getRequest()->isSimpleShoppingRQ() && trx.isMip() &&
      SimplifiedFareMarketsBuilder::isComplexItin(itin) &&
      !fallback::simpleFareMarketBuilding(&trx))
  {
    return simplifiedBuildFareMarket(trx, itin);
  }

  { // brackets are intentional, please don't remove

    // regular pricing FM set for avail call
    // should be constructed just like pricing, not like RTW
    RtwUtil::ScopedRtwDisabler rtwDisabler(trx);
    SegmentAttribs allSegmentsAtributes;

    // Set attributes of the segment (ARUNK, etc.)
    if (fallback::segmentAttributesRefactor(&trx))
      TravelSegUtil::setSegmentAttributes(itin.travelSeg(), allSegmentsAtributes);
    else
      allSegmentsAtributes = TravelSegUtil::calcSegmentAttributes(itin.travelSeg());

    int i = 0;

    for (SegmentAttribs::iterator beginIt = allSegmentsAtributes.begin();
         beginIt != allSegmentsAtributes.end();
         ++beginIt)
    {
      LOG4CXX_DEBUG(_itinAnalyzer._logger,
                    "SEG " << (i + 1) << " ATTRIBUTES:"
                           << " FORCED ST " << ((*beginIt).attributes & FORCED_SIDE_TRIP_ELEMENT)
                           << " ST BEG " << ((*beginIt).attributes & SIDE_TRIP_BEGIN) << " ST END "
                           << ((*beginIt).attributes & SIDE_TRIP_END) << " ST ELEM "
                           << ((*beginIt).attributes & SIDE_TRIP_ELEMENT));
      ++i;

      const bool isArunk = fallback::segmentAttributesRefactor(&trx)
                               ? ((*beginIt).attributes & ARUNK)
                               : (beginIt->tvlSeg->segmentType() == Arunk);

      if (isArunk)
      {
        createARUNKFareMarket(trx, itin, beginIt - 1, allSegmentsAtributes.end());
        continue;
      }

      std::vector<TravelSeg*> travelSegs;
      bool breakIndicator = false;

      // Check if next travel segment is forced side trip while current
      // segment is not forced side trip.
      // If yes, mark this fare market with this currect travel segment not
      // for combinability.
      if (UNLIKELY((beginIt + 1) != allSegmentsAtributes.end() &&
                    !((*beginIt).attributes & FORCED_SIDE_TRIP_ELEMENT) &&
                    ((*(beginIt + 1)).attributes & FORCED_SIDE_TRIP_ELEMENT)))
      {
        breakIndicator = true;
      }

      // create fm with only one travelSeg
      if (LIKELY(dynamic_cast<AirSeg*>((*beginIt).tvlSeg)))
      {
        travelSegs.push_back((*beginIt).tvlSeg);
        createFareMarket(trx, itin, beginIt + 1, travelSegs, breakIndicator, nullptr);
      }

      if (UNLIKELY((*beginIt).attributes & FORCED_SIDE_TRIP_ELEMENT))
        continue;

      SideTripsInfo sideTripsInfo;

      // create fareMarkets with travelSegs in <beginIt + 1, endIt)
      for (SegmentAttribs::iterator endIt = beginIt + 1; endIt != allSegmentsAtributes.end();
           ++endIt)
      {
        if (collectSegments(trx, itin, *endIt, travelSegs, sideTripsInfo))
        {
          createFareMarket(trx, itin, /*beginIt,*/ endIt + 1, travelSegs, false, &sideTripsInfo);
        }
      }
    }
  }

  if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
  {
    ItinAnalyzerService::IsPartButNotWholeDummyFare isPartButNotWholeDummyFare(
        (static_cast<ExchangePricingTrx&>(trx)).exchangeOverrides().dummyFCSegs());

    itin.fareMarket().erase(std::remove_if(itin.fareMarket().begin(),
                                           itin.fareMarket().end(),
                                           isPartButNotWholeDummyFare),
                            itin.fareMarket().end());
  }

  if (trx.getOptions()->isRtw())
  {
    createFareMarketForRtw(trx, itin);
  }

  return true;
}

bool
ItinAnalyzerServiceWrapper::collectSegments(PricingTrx& trx,
                                            Itin& itin,
                                            const SegmentAttributes& segAttr,
                                            std::vector<TravelSeg*>& travelSegs,
                                            SideTripsInfo& sideTripsInfo) const
{
  // Order of if and else if is very important. Be careful when changing!!!
  if ((segAttr.attributes & SIDE_TRIP_END) && sideTripsInfo._pendingSideTrips.size() > 0)
  {
    getSideTripEnd(itin, segAttr, &sideTripsInfo, travelSegs);
    return false;
  }
  else if (segAttr.attributes & SIDE_TRIP_BEGIN)
  {
    getSideTripBegin(trx.dataHandle(), segAttr, &sideTripsInfo);
  }
  else if ((segAttr.attributes & SIDE_TRIP_ELEMENT) && sideTripsInfo._pendingSideTrips.size() > 0)
  {
    getSideTripElement(segAttr, &sideTripsInfo);
  }
  else
  {
    if (UNLIKELY(segAttr.attributes & FORCED_SIDE_TRIP_ELEMENT))
      return false;

    travelSegs.push_back(segAttr.tvlSeg);
  }

  return true;
}

void
ItinAnalyzerServiceWrapper::createFareMarket(PricingTrx& trx,
                                             Itin& itin,
                                             SegmentAttribs::iterator endSegIt,
                                             std::vector<TravelSeg*>& travelSegs,
                                             bool breakIndicator,
                                             SideTripsInfo* const sideTripsInfo) const
{
  FareMarket* newFareMarket;
  trx.dataHandle().get(newFareMarket); // lint !e530
  if (UNLIKELY(breakIndicator))
    newFareMarket->setBreakIndicator(true);

  newFareMarket->travelSeg() = travelSegs;

  if (sideTripsInfo)
  {
    // Check if there is pending side trip that is forced
    PendingSideTripsForced::iterator forcedStIter =
        std::find(sideTripsInfo->_pendingSideTripsForced.begin(),
                  sideTripsInfo->_pendingSideTripsForced.end(),
                  true);
    if (UNLIKELY(forcedStIter != sideTripsInfo->_pendingSideTripsForced.end()))
      return;

    std::set<int16_t> stSegOrders;

    addStToFareMarket(itin, *newFareMarket, sideTripsInfo->_resultSideTrips, stSegOrders);

    // Add the travel segments in the non-forced pending side trip to the fare market
    addPendingStToFareMarket(itin, *newFareMarket, sideTripsInfo->_pendingSideTrips, stSegOrders);
  }

  if (UNLIKELY(newFareMarket->travelSeg().empty()))
    return;

  newFareMarket->origin() = newFareMarket->travelSeg().front()->origin();
  newFareMarket->destination() = newFareMarket->travelSeg().back()->destination();
  newFareMarket->legIndex() = newFareMarket->travelSeg().front()->legId();
  newFareMarket->travelDate() = itin.travelDate();
  // special case for ARUNK
  if (fallback::segmentAttributesRefactor(&trx))
  {
    if ((*(endSegIt - 1)).attributes & ARUNK)
      newFareMarket->destination() = (*endSegIt).tvlSeg->origin();
  }
  else
  {
    if ((endSegIt - 1)->tvlSeg->segmentType() == Arunk)
      newFareMarket->destination() = endSegIt->tvlSeg->origin();
  }

  setupAndStoreFareMarket(trx, itin, newFareMarket);

  for (SideTrips::iterator it = newFareMarket->sideTripTravelSeg().begin();
       it != newFareMarket->sideTripTravelSeg().end();
       ++it)
  {
    createSideTripCombination(trx, itin, newFareMarket, it);
  }

  // Create the fare market that does not contain any side trip.
  if (sideTripsInfo)
  {
    if (newFareMarket->sideTripTravelSeg().size() > 1 && !sideTripsInfo->_fmStForced)
    {
      createThruFareMarket(trx, itin, newFareMarket);
    }
  }
}

bool
ItinAnalyzerServiceWrapper::collectSegmentsForRtw(PricingTrx& trx,
                                                  Itin& itin,
                                                  std::vector<TravelSeg*>& travelSegs) const
{
  travelSegs = itin.travelSeg();

  if (travelSegs.empty())
    return false;

  if (travelSegs.front()->origAirport() == travelSegs.back()->destAirport())
    return true;

  const TravelSeg& tsFront = *travelSegs.front();
  const TravelSeg& tsBack = *travelSegs.back();

  ArunkSeg* arunkSeg = trx.dataHandle().create<ArunkSeg>();
  arunkSeg->segmentType() = Surface;
  arunkSeg->pnrSegment() = ARUNK_PNR_SEGMENT_ORDER;
  arunkSeg->setRtwDynamicSupplementalArunk(true);
  arunkSeg->segmentOrder() = tsBack.segmentOrder() + 1;
  arunkSeg->origin() = tsBack.destination();
  arunkSeg->origAirport() = tsBack.destAirport();
  arunkSeg->boardMultiCity() = tsBack.offMultiCity();
  arunkSeg->destination() = tsFront.origin();
  arunkSeg->destAirport() = tsFront.origAirport();
  arunkSeg->offMultiCity() = tsFront.boardMultiCity();
  arunkSeg->bookedCabin().setEconomyClass();
  arunkSeg->bookingDT() = tsFront.bookingDT();

  travelSegs.push_back(arunkSeg);
  itin.travelSeg().push_back(arunkSeg);
  return true;
}

void
ItinAnalyzerServiceWrapper::createFareMarketForRtw(PricingTrx& trx, Itin& itin) const
{
  FareMarket* newFm = trx.dataHandle().create<FareMarket>();

  if (!collectSegmentsForRtw(trx, itin, newFm->travelSeg()))
  {
    ItinUtil::roundTheWorldDiagOrThrow(trx, dynamic_cast<const ExcItin*>(&itin));
    return;
  }

  if (!setupAndStoreFareMarketForRtw(trx, itin, *newFm))
  {
    ItinUtil::roundTheWorldDiagOrThrow(trx, dynamic_cast<const ExcItin*>(&itin));
    return;
  }

  const std::vector<AirlineAllianceCarrierInfo*>& airlineAllianceCarrierInfo =
      trx.dataHandle().getAirlineAllianceCarrier(newFm->governingCarrier());

  if (airlineAllianceCarrierInfo.size() > 1 || !trx.isCustomerActivatedByFlag("RTW"))
    ItinUtil::roundTheWorldDiagOrThrow(trx, dynamic_cast<const ExcItin*>(&itin));
}

bool
ItinAnalyzerServiceWrapper::setupAndStoreFareMarketForRtw(PricingTrx& trx,
                                                          Itin& itin,
                                                          FareMarket& newFm) const
{
  const std::vector<TravelSeg*>& travelSegs = newFm.travelSeg();

  newFm.origin() = travelSegs.front()->origin();
  newFm.destination() = travelSegs.back()->destination();
  newFm.travelDate() = itin.travelDate();
  newFm.direction() = FMDirection::OUTBOUND;
  newFm.legIndex() = travelSegs.front()->legId();

  if (TrxUtil::newDiscountLogic(trx))
  {
    if (!TrxUtil::validateDiscountNew(trx, newFm) && !trx.diagnostic().isActive(Diagnostic199))
      return false;
  }
  else
  {
    if (!TrxUtil::validateDiscountOld(trx, newFm))
      return false;
  }

  FareMarketBuilder::setBreakIndicator(&newFm, &itin, trx);
  Boundary tvlboundary = _itinAnalyzer._tvlSegAnalysis.selectTravelBoundary(travelSegs);
  ItinUtil::setGeoTravelType(tvlboundary, newFm);

  std::vector<CarrierCode> govCxrOverrides;
  std::set<CarrierCode> participatingCxrs;
  getGovCxrOverrides(trx, newFm, govCxrOverrides, participatingCxrs);

  if (govCxrOverrides.size() > 1)
    newFm.setBreakIndicator(true);

  if (!govCxrOverrides.empty())
    newFm.governingCarrier() = govCxrOverrides.front();

  if (!setBoardAndOffPoints(trx, itin, newFm))
    return false;

  storeFareMarket(trx, &newFm, itin, false);
  return true;
}

void
ItinAnalyzerServiceWrapper::getGovCxrOverrides(PricingTrx& trx,
                                               const FareMarket& newFm,
                                               std::vector<CarrierCode>& govCxrOverrides,
                                               std::set<CarrierCode>& participatingCxrs) const
{
  if (UNLIKELY(FareMarketUtil::isYYOverride(trx, newFm.travelSeg())))
  {
    govCxrOverrides.push_back(INDUSTRY_CARRIER);
    participatingCxrs.insert(INDUSTRY_CARRIER);
  }
  else
  {
    FareMarketUtil::getGovCxrOverride(trx, newFm.travelSeg(), govCxrOverrides);
    FareMarketUtil::getParticipatingCarrier(newFm.travelSeg(), participatingCxrs);
  }
}

void
ItinAnalyzerServiceWrapper::cloneAndStoreFareMarket(
    PricingTrx& trx,
    FareMarket* const srcFareMarket,
    Itin& itin,
    TravelSeg* const primarySector,
    CarrierCode& govCxr,
    const std::vector<CarrierCode>& govCxrOverrides,
    const std::set<CarrierCode>& participatingCarriers,
    bool yyOverride,
    bool fmBreakSet,
    bool removeOutboundFares,
    bool highTPMGovCxr) const
{
  FareMarket* anotherFareMarket; // get an empty FareMarket
  trx.dataHandle().get(anotherFareMarket); // lint !e530

  _itinAnalyzer.cloneFareMarket(*srcFareMarket, *anotherFareMarket, govCxr);

  if (primarySector)
    anotherFareMarket->primarySector() = primarySector;

  if (highTPMGovCxr)
    anotherFareMarket->setHighTPMGoverningCxr(highTPMGovCxr);

  if (removeOutboundFares)
  {
    anotherFareMarket->setRemoveOutboundFares(true);
    anotherFareMarket->direction() = FMDirection::INBOUND;
  }

  if (setBoardAndOffPoints(trx, itin, *anotherFareMarket))
  {
    storeFareMarket(trx, anotherFareMarket, itin, false);

    if (fmBreakSet) // Original FM is set break for other reason.
    {
      anotherFareMarket->setBreakIndicator(true);
    }
    else
    {
      FareMarketBuilder::setBreakIndByCxrOverride(
          govCxrOverrides, participatingCarriers, *anotherFareMarket, yyOverride);
    }
  }
}

void
ItinAnalyzerServiceWrapper::addFareMarketDiffGovCXR(PricingTrx& trx,
                                                    Itin& itin,
                                                    FareMarket& fareMarket) const
{
  std::vector<TravelSeg*>& travelSeg = fareMarket.travelSeg();
  if (travelSeg.size() < 2)
  {
    return;
  }

  GoverningCarrier govCxrUtil(&trx);
  std::set<CarrierCode> govCXRsInb;
  govCxrUtil.getGoverningCarrier(travelSeg, govCXRsInb, FMDirection::INBOUND);
  if (govCXRsInb.empty())
  {
    return;
  }
  std::set<CarrierCode>::iterator igInb = govCXRsInb.begin();

  if (*igInb != fareMarket.governingCarrier())
  {
    CarrierCode inbCxr = *igInb;
    TravelSeg* primarySector(nullptr);
    FareMarket* anotherFareMarket; // get an empty FareMarket
    trx.dataHandle().get(anotherFareMarket); // lint !e530

    _itinAnalyzer.cloneFareMarket(fareMarket, *anotherFareMarket, inbCxr);

    anotherFareMarket->primarySector() = primarySector;
    anotherFareMarket->setBreakIndicator(fareMarket.breakIndicator());
    fareMarket.direction() = FMDirection::OUTBOUND;

    if (anotherFareMarket->geoTravelType() == GeoTravelType::International)
      anotherFareMarket->setRemoveOutboundFares(true);

    anotherFareMarket->direction() = FMDirection::INBOUND;

    if (setBoardAndOffPoints(trx, itin, *anotherFareMarket))
    {
      storeFareMarket(trx, anotherFareMarket, itin, false);
    }
  }
}

FareMarket*
ItinAnalyzerServiceWrapper::storeFareMarket(PricingTrx& trx,
                                            FareMarket* newFareMarket,
                                            Itin& itin,
                                            bool checkIfExist) const
{
  if (itin.isSimilarItin())
  {
    itin.fareMarket().push_back(newFareMarket);
    return newFareMarket;
  }

  if (trx.getOptions()->isCarnivalSumOfLocal())
    checkIfExist = false;

  if (trx.getRequest()->originBasedRTPricing())
  {
    if (newFareMarket->travelSeg().size() == 1 && newFareMarket->travelSeg().front()->isAir() &&
        newFareMarket->travelSeg().front()->toAirSeg()->isFake())
      checkIfExist = false;
  }

  bool checkExistingForFareSelection =
      (trx.isIataFareSelectionApplicable() && (trx.getTrxType() == PricingTrx::IS_TRX));

  FareMarket* existingFareMarket = nullptr;
  if (checkIfExist || checkExistingForFareSelection)
    existingFareMarket = getExistingFareMarket(*newFareMarket, trx, checkExistingForFareSelection);

  if (existingFareMarket)
  {
    itin.fareMarket().push_back(existingFareMarket);

    // check if any of the itins from which the existingFareMarket originates has a different
    // TFO than the current itin, if so mark this fare market so it wont be removed at
    // removeSoloFMs
    bool sameTFO = true;

    for (std::vector<Itin*>::const_iterator it = trx.itin().begin(); it != trx.itin().end(); ++it)
    {
      std::vector<FareMarket*>::const_iterator foundFmIter =
          std::find((*it)->fareMarket().begin(), (*it)->fareMarket().end(), existingFareMarket);
      if (foundFmIter != (*it)->fareMarket().end())
      {
        if ((*it)->isThruFareOnly() != itin.isThruFareOnly())
        {
          sameTFO = false;
          break;
        }
      }
    }

    if (!sameTFO)
      existingFareMarket->setKeepSoloFM(true);

    return existingFareMarket;
  }

  trx.fareMarket().push_back(newFareMarket);
  itin.fareMarket().push_back(newFareMarket);
  return newFareMarket;
}

void
ItinAnalyzerServiceWrapper::getSideTripEnd(Itin& itin,
                                           const SegmentAttributes& attribs,
                                           SideTripsInfo* const sideTripsInfo,
                                           std::vector<TravelSeg*>& travelSegs) const
{
  PendingSideTrips::iterator pendingIter = sideTripsInfo->_pendingSideTrips.begin();
  for (; pendingIter != sideTripsInfo->_pendingSideTrips.end(); pendingIter++)
    (*pendingIter)->push_back(attribs.tvlSeg);
  {
    PendingSideTrips::reverse_iterator pstIter = sideTripsInfo->_pendingSideTrips.rbegin();
    for (; pstIter != sideTripsInfo->_pendingSideTrips.rend(); pstIter++)
    {
      if (((*pstIter)->front())->boardMultiCity() == ((*pstIter)->back())->offMultiCity())
        break;
    }
    if (pstIter != sideTripsInfo->_pendingSideTrips.rend())
    {
      PendingSideTrips::iterator tIter(--(pstIter.base()));
      if (isValidSideTripCombination(itin, sideTripsInfo->_resultSideTrips, **pstIter))
        sideTripsInfo->_resultSideTrips.push_back(**pstIter);
      else
        copyFromInvalidSideTrip(sideTripsInfo->_resultSideTrips, **pstIter, travelSegs);
      if (attribs.attributes & FORCED_SIDE_TRIP_ELEMENT)
        sideTripsInfo->_fmStForced = true;

      int posB = (tIter - sideTripsInfo->_pendingSideTrips.begin());

      sideTripsInfo->_pendingSideTrips.erase(tIter);

      if (posB < static_cast<int>(sideTripsInfo->_pendingSideTripsForced.size()))
      {
        std::vector<bool>::iterator itForcedST =
            sideTripsInfo->_pendingSideTripsForced.begin() + posB;
        sideTripsInfo->_pendingSideTripsForced.erase(itForcedST);
      }
    }
  }
}

void
ItinAnalyzerServiceWrapper::copyFromInvalidSideTrip(std::vector<std::vector<TravelSeg*>>& sideTrips,
                                                    std::vector<TravelSeg*>& newSt,
                                                    std::vector<TravelSeg*>& travelSegs)
{
  std::set<TravelSeg*> allStSegments;
  std::vector<std::vector<TravelSeg*>>::iterator stIter = sideTrips.begin();
  for (; stIter != sideTrips.end(); stIter++)
  {
    std::vector<TravelSeg*>& tSegs = *stIter;
    for (const auto ts : tSegs)
    {
      allStSegments.insert(ts);
    }
  }

  std::set<TravelSeg*>::iterator sI = allStSegments.begin();
  for (; sI != allStSegments.end(); sI++)
  {
    TravelSeg* ts = *sI;

    std::vector<TravelSeg*>::iterator i = std::find(newSt.begin(), newSt.end(), ts);
    if (i != newSt.end())
      newSt.erase(i);
  }
  if (!newSt.empty())
    travelSegs.insert(travelSegs.end(), newSt.begin(), newSt.end());
}

bool
ItinAnalyzerServiceWrapper::isValidSideTripCombination(
    Itin& itin, std::vector<std::vector<TravelSeg*>>& sideTrips, std::vector<TravelSeg*>& newSt)
{
  int newStBegin = itin.segmentOrder(newSt.front());
  int newStEnd = itin.segmentOrder(newSt.back());

  std::vector<std::vector<TravelSeg*>>::iterator stIter = sideTrips.begin();
  for (; stIter != sideTrips.end(); stIter++)
  {
    std::vector<TravelSeg*>& travelSegs = *stIter;
    int curStBegin = itin.segmentOrder(travelSegs.front());
    int curStEnd = itin.segmentOrder(travelSegs.back());

    if (newStBegin > curStBegin && newStBegin <= curStEnd && newStEnd > curStEnd)
      return false;
  }
  return true;
}

void
ItinAnalyzerServiceWrapper::getSideTripBegin(DataHandle& dataHandle,
                                             const SegmentAttributes& attribs,
                                             SideTripsInfo* const sideTripsInfo) const
{
  std::vector<TravelSeg*>* sideTrip = nullptr;
  dataHandle.get(sideTrip); // lint !e530

  sideTripsInfo->_pendingSideTrips.push_back(sideTrip);
  sideTripsInfo->_pendingSideTripsForced.push_back(attribs.attributes & FORCED_SIDE_TRIP_ELEMENT);

  PendingSideTrips::iterator pendingIter = sideTripsInfo->_pendingSideTrips.begin();
  for (; pendingIter != sideTripsInfo->_pendingSideTrips.end(); pendingIter++)
    (*pendingIter)->push_back(attribs.tvlSeg);
}

void
ItinAnalyzerServiceWrapper::getSideTripElement(const SegmentAttributes& attribs,
                                               SideTripsInfo* const sideTripsInfo) const
{
  PendingSideTrips::iterator pendingIter = sideTripsInfo->_pendingSideTrips.begin();
  for (; pendingIter != sideTripsInfo->_pendingSideTrips.end(); pendingIter++)
    (*pendingIter)->push_back(attribs.tvlSeg);
}

bool
ItinAnalyzerServiceWrapper::setupAndStoreFareMarket(PricingTrx& trx,
                                                    Itin& itin,
                                                    FareMarket* newFareMarket,
                                                    uint32_t legIndex) const
{
  TSELatencyData metrics(trx, "ITIN setupAndStoreFareMarket");
  if (UNLIKELY(!newFareMarket))
    return false;

  std::vector<TravelSeg*>& travelSegs = newFareMarket->travelSeg();

  if (UNLIKELY(travelSegs.empty()))
    return false;

  if (LIKELY(!trx.getOptions()->isRtw()))
  {
    if (travelSegs.front()->origAirport() == travelSegs.back()->destAirport())
      return false;
  }

  // Inhibit creation of surface at fare break for all US/CA trips
  if ((itin.geoTravelType() == GeoTravelType::Domestic || itin.geoTravelType() == GeoTravelType::Transborder) &&
      (travelSegs.front()->segmentType() == Arunk || travelSegs.front()->segmentType() == Surface ||
       travelSegs.back()->segmentType() == Arunk || travelSegs.back()->segmentType() == Surface))
    return false;

  bool checkIfMarketAlreadyAdded = true;

  FlightFinderTrx* ffTrx = dynamic_cast<FlightFinderTrx*>(&trx);

  if (UNLIKELY((ffTrx && ffTrx->avlInS1S3Request()) || trx.getRequest()->isMultiTicketRequest()))
  {
    checkIfMarketAlreadyAdded = false;
  }

  // Skip fare market with sideTrip if server configured to do so
  if (UNLIKELY((!newFareMarket->sideTripTravelSeg().empty()) && excludeSidetripFm.getValue()))
  {
    return false;
  }

  // Limit side trips in fare market
  if (UNLIKELY(newFareMarket->sideTripTravelSeg().size() >
                ItinAnalyzerService::MAX_SIDETRIP_IN_FAREMARKET))
    return false;

  if (TrxUtil::newDiscountLogic(trx))
  {
    if (UNLIKELY(!TrxUtil::validateDiscountNew(trx, *newFareMarket) && !trx.diagnostic().isActive(Diagnostic199)))
      return false;
  }
  else
  {
    if (UNLIKELY(!TrxUtil::validateDiscountOld(trx, *newFareMarket)))
      return false;
  }

  FareMarketBuilder::setBreakIndicator(newFareMarket, &itin, trx);

  Boundary tvlboundary = _itinAnalyzer._tvlSegAnalysis.selectTravelBoundary(travelSegs);
  ItinUtil::setGeoTravelType(tvlboundary, *newFareMarket);

  if (!fallback::fallbackSkipAcrossStopoverDomesticFM(&trx) &&
      trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (isAcrossStopoverDomesticUSFareMarket(*newFareMarket))
      return false;
  }

  if (!TrxUtil::isAAAwardPricing(trx) && isJointUSCAFareMarket(newFareMarket))
    return false;

  _itinAnalyzer.setInboundOutbound(trx, itin, *newFareMarket);

  const bool yyOverride = FareMarketUtil::isYYOverride(trx, travelSegs);
  std::vector<CarrierCode> govCxrOverrides;
  std::set<CarrierCode> participatingCarriers;
  getGovCxrOverrides(trx, *newFareMarket, govCxrOverrides, participatingCarriers);

  if (trx.getTrxType() == PricingTrx::MIP_TRX)
    _itinAnalyzer.setRetransits(itin);

  // Check if break has been set for other reasons than carrier override
  bool fmBreakSet = newFareMarket->breakIndicator();

  // check for IS inbound SOP
  if (trx.getTrxType() == PricingTrx::IS_TRX && itin.simpleTrip() && itin.inboundSop())
  {
    newFareMarket->direction() = FMDirection::INBOUND;
  }

  bool fareMarketAdded = false;

  if (trx.isIataFareSelectionApplicable() && !TrxUtil::isFareSelectionForSubIata21Only(trx))
  { // IATA Fare Selection is active
    selectGovCxrs(trx,
                  itin,
                  newFareMarket,
                  checkIfMarketAlreadyAdded,
                  fareMarketAdded,
                  fmBreakSet,
                  govCxrOverrides,
                  participatingCarriers,
                  legIndex);
  }
  else
  {
    selectGovCxrsOld(trx,
                     itin,
                     newFareMarket,
                     checkIfMarketAlreadyAdded,
                     fareMarketAdded,
                     fmBreakSet,
                     govCxrOverrides,
                     participatingCarriers);
  }

  for (std::vector<CarrierCode>::iterator govCxrIter = govCxrOverrides.begin();
       govCxrIter != govCxrOverrides.end();
       ++govCxrIter)
  {
    if (((*govCxrIter) != newFareMarket->governingCarrier()) &&
        (std::find(participatingCarriers.begin(), participatingCarriers.end(), (*govCxrIter)) !=
         participatingCarriers.end()))
    {
      // Clone a new fare market with the governing carrier override
      cloneAndStoreFareMarket(trx,
                              newFareMarket,
                              itin,
                              nullptr,
                              *govCxrIter,
                              govCxrOverrides,
                              participatingCarriers,
                              yyOverride,
                              fmBreakSet,
                              false);
    }
  }

  return fareMarketAdded;
}

void
ItinAnalyzerServiceWrapper::selectGovCxrsOld(PricingTrx& trx,
                                             Itin& itin,
                                             FareMarket* newFareMarket,
                                             bool& checkIfMarketAlreadyAdded,
                                             bool& fareMarketAdded,
                                             bool& fmBreakSet,
                                             const std::vector<CarrierCode>& govCxrOverrides,
                                             const std::set<CarrierCode>& participatingCarriers)
    const
{
  std::vector<TravelSeg*>& travelSegs = newFareMarket->travelSeg();
  bool yyOverride = FareMarketUtil::isYYOverride(trx, travelSegs);
  SmallBitSet<uint8_t, FMTravelBoundary>& boundary = newFareMarket->travelBoundary();

  if (boundary.isSet(FMTravelBoundary::TravelWithinSameCountryExceptUSCA))
  {
    LOG4CXX_DEBUG(_itinAnalyzer._logger,
                  "Entered ItinAnalyzerServiceWrapper::TravelWithinSameCountryExceptUSCA");

    AirSeg* airSeg = dynamic_cast<AirSeg*>(travelSegs.front());
    if (airSeg == nullptr || airSeg->carrier().empty())
      newFareMarket->governingCarrier() = INDUSTRY_CARRIER;
    else
      newFareMarket->governingCarrier() = airSeg->carrier();

    newFareMarket->primarySector() = travelSegs.front();

    std::set<CarrierCode> setOfCarriers;
    setOfCarriers.insert(newFareMarket->governingCarrier());

    if (setBoardAndOffPoints(trx, itin, *newFareMarket))
    {
      storeFareMarket(trx, newFareMarket, itin, checkIfMarketAlreadyAdded);
      fareMarketAdded = true;

      if (!fmBreakSet)
        FareMarketBuilder::setBreakIndByCxrOverride(
            govCxrOverrides, participatingCarriers, *newFareMarket, yyOverride);
    }
    if (travelSegs.size() > 1)
    {
      for (std::vector<TravelSeg*>::iterator it = travelSegs.begin() + 1; it != travelSegs.end();
           ++it)
      {
        AirSeg* airSeg = dynamic_cast<AirSeg*>(*it);
        CarrierCode govCxr;
        if (airSeg == nullptr || airSeg->carrier().empty())
          govCxr = INDUSTRY_CARRIER;
        else
          govCxr = airSeg->carrier();

        std::set<CarrierCode>::iterator pos =
            find(setOfCarriers.begin(), setOfCarriers.end(), govCxr);

        if ((pos == setOfCarriers.end()) && (!govCxr.empty()))
        {
          setOfCarriers.insert(govCxr);

          cloneAndStoreFareMarket(trx,
                                  newFareMarket,
                                  itin,
                                  (*it),
                                  govCxr,
                                  govCxrOverrides,
                                  participatingCarriers,
                                  yyOverride,
                                  fmBreakSet,
                                  false);
        }
      }
    }
  }
  else if (boundary.isSet(FMTravelBoundary::TravelWithinSubIATA21))
  {
    LOG4CXX_DEBUG(_itinAnalyzer._logger,
                  "Entered ItnAnalyzerServiceWrapper::TravelWithinSubIATA21");

    // Set GovCxr by fare market direction
    newFareMarket->governingCarrier() =
        ((newFareMarket->direction() == FMDirection::INBOUND)
             ? TravelSegAnalysis::getLastIntlFlt(travelSegs, newFareMarket->primarySector())
             : TravelSegAnalysis::getFirstIntlFlt(travelSegs, newFareMarket->primarySector()));

    if (newFareMarket->governingCarrier() == INBOUND_CARRIER)
      newFareMarket->setRemoveOutboundFares(true);

    if (setBoardAndOffPoints(trx, itin, *newFareMarket))
    {
      storeFareMarket(trx, newFareMarket, itin, checkIfMarketAlreadyAdded);
      fareMarketAdded = true;

      FareMarketBuilder::setBreakIndByCxrOverride(
          govCxrOverrides, participatingCarriers, *newFareMarket, yyOverride);
    }

    if (travelSegs.size() > 1)
    {
      GoverningCarrier govC(&trx);
      CarrierCode govCxr = govC.getHighestTPMCarrierOld(travelSegs);
      if (((govCxr != BAD_CARRIER) && (!govCxr.empty())) &&
          (newFareMarket->governingCarrier() != govCxr))
      {
        cloneAndStoreFareMarket(trx,
                                newFareMarket,
                                itin,
                                nullptr,
                                govCxr,
                                govCxrOverrides,
                                participatingCarriers,
                                yyOverride,
                                fmBreakSet,
                                false);
      }

      // Clone for last intl sector carrier for unknown Fare Market
      if (newFareMarket->direction() == FMDirection::UNKNOWN)
      {
        TravelSeg* primarySector(nullptr);

        CarrierCode inboundGovCxr =
            TravelSegAnalysis::getLastIntlFlt(newFareMarket->travelSeg(), primarySector);

        if (inboundGovCxr != BAD_CARRIER && newFareMarket->governingCarrier() != inboundGovCxr &&
            (govCxr == BAD_CARRIER || govCxr.empty() || govCxr != inboundGovCxr))
        {
          cloneAndStoreFareMarket(trx,
                                  newFareMarket,
                                  itin,
                                  primarySector,
                                  inboundGovCxr,
                                  govCxrOverrides,
                                  participatingCarriers,
                                  yyOverride,
                                  fmBreakSet,
                                  true);
        }
      }
    }
  }
  else
  {
    if (setBoardAndOffPoints(trx, itin, *newFareMarket))
    {
      storeFareMarket(trx, newFareMarket, itin, checkIfMarketAlreadyAdded);
      fareMarketAdded = true;

      FareMarketBuilder::setBreakIndByCxrOverride(
          govCxrOverrides, participatingCarriers, *newFareMarket, yyOverride);
    }
  }

  if (!boundary.isSet(FMTravelBoundary::TravelWithinSubIATA21) &&
      newFareMarket->direction() == FMDirection::UNKNOWN)
  {
    addFareMarketDiffGovCXR(trx, itin, *newFareMarket);
  }
}

bool
ItinAnalyzerServiceWrapper::skipForShoppingIS(PricingTrx& trx, const FareMarket* fm) const
{
  if (trx.getTrxType() != PricingTrx::IS_TRX)
    return false;

  if (TrxUtil::isFareSelectionForSubIata21Only(trx) &&
      !fm->travelBoundary().isSet(FMTravelBoundary::TravelWithinSubIATA21))
    return true;

  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(trx);

  if (shoppingTrx.isSumOfLocalsProcessingEnabled())
    return true;

  return shoppingTrx.legs().at(fm->legIndex()).stopOverLegFlag();
}

void
ItinAnalyzerServiceWrapper::selectGovCxrs(PricingTrx& trx,
                                          Itin& itin,
                                          FareMarket* newFareMarket,
                                          bool& checkIfMarketAlreadyAdded,
                                          bool& fareMarketAdded,
                                          bool& fmBreakSet,
                                          const std::vector<CarrierCode>& govCxrOverrides,
                                          const std::set<CarrierCode>& participatingCarriers,
                                          uint32_t legIndex) const
{
  std::vector<TravelSeg*>& travelSegs = newFareMarket->travelSeg();
  bool yyOverride = FareMarketUtil::isYYOverride(trx, travelSegs);
  SmallBitSet<uint8_t, FMTravelBoundary>& boundary = newFareMarket->travelBoundary();

  if (boundary.isSet(FMTravelBoundary::TravelWithinSameCountryExceptUSCA))
  {
    LOG4CXX_DEBUG(_itinAnalyzer._logger,
                  "Entered ItinAnalyzerServiceWrapper::TravelWithinSameCountryExceptUSCA");

    GoverningCarrier govC(&trx);
    govC.process(*newFareMarket);

    std::set<CarrierCode> setOfCarriers;
    setOfCarriers.insert(newFareMarket->governingCarrier());

    if (setBoardAndOffPoints(trx, itin, *newFareMarket))
    {
      storeFareMarket(trx, newFareMarket, itin, checkIfMarketAlreadyAdded);
      fareMarketAdded = true;

      if (!fmBreakSet)
        FareMarketBuilder::setBreakIndByCxrOverride(
            govCxrOverrides, participatingCarriers, *newFareMarket, yyOverride);
    }
    if (travelSegs.size() > 1)
    {
      for (std::vector<TravelSeg*>::iterator it = travelSegs.begin() + 1; it != travelSegs.end();
           ++it)
      {
        AirSeg* airSeg = dynamic_cast<AirSeg*>(*it);
        CarrierCode govCxr;
        if (airSeg == nullptr || airSeg->carrier().empty())
          govCxr = INDUSTRY_CARRIER;
        else
          govCxr = airSeg->carrier();

        std::set<CarrierCode>::iterator pos =
            find(setOfCarriers.begin(), setOfCarriers.end(), govCxr);

        if ((pos == setOfCarriers.end()) && (!govCxr.empty()))
        {
          setOfCarriers.insert(govCxr);

          cloneAndStoreFareMarket(trx,
                                  newFareMarket,
                                  itin,
                                  (*it),
                                  govCxr,
                                  govCxrOverrides,
                                  participatingCarriers,
                                  yyOverride,
                                  fmBreakSet,
                                  false);
        }
      }
    }
  }
  else if (boundary.isSet(FMTravelBoundary::TravelWithinUSCA))
  {
    if (setBoardAndOffPoints(trx, itin, *newFareMarket))
    {
      storeFareMarket(trx, newFareMarket, itin, checkIfMarketAlreadyAdded);
      fareMarketAdded = true;

      FareMarketBuilder::setBreakIndByCxrOverride(
          govCxrOverrides, participatingCarriers, *newFareMarket, yyOverride);
    }
  }
  else
  {
    LOG4CXX_DEBUG(_itinAnalyzer._logger,
                  "Entered ItnAnalyzerServiceWrapper::all boundaries except any domestic");

    if (setBoardAndOffPoints(trx, itin, *newFareMarket))
    {
      storeFareMarket(trx, newFareMarket, itin, checkIfMarketAlreadyAdded);
      fareMarketAdded = true;

      FareMarketBuilder::setBreakIndByCxrOverride(
          govCxrOverrides, participatingCarriers, *newFareMarket, yyOverride);
    }

    if ((travelSegs.size() > 1) && !skipForShoppingIS(trx, newFareMarket))
    {
      CarrierCode tpmGovCxr;
      if (!isVctrCarrier(newFareMarket))
      {
        GoverningCarrier govC(&trx);
        TravelSeg* primarySector = nullptr;
        tpmGovCxr =
            govC.getHighestTPMCarrier(travelSegs, newFareMarket->direction(), primarySector);

        if (newFareMarket->governingCarrier() == tpmGovCxr)
        {
          newFareMarket->setFirstCrossingAndHighTpm(true);
          tpmGovCxr.clear();
        }
        else if (!tpmGovCxr.empty())
        {
           if(!fallback::fallbackFareSelction2016(&trx)&&
               trx.getTrxType() == PricingTrx::IS_TRX)
           {
             if (legIndex < _itin2SopInfoMapVec.size())
             {
               Itin2SopInfoMap::const_iterator it(_itin2SopInfoMapVec[legIndex].find(&itin));

               if(it != _itin2SopInfoMapVec[legIndex].end())
               {
                 SopInfo origSopInfo = it->second;
                 uint32_t origSopId =  origSopInfo.first;
                 ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(trx);
                 itinanalyzerutils::setSopWithHighestTPM(shoppingTrx, legIndex, origSopId);
               }
             }
           }
           else
           {
             cloneAndStoreFareMarket(trx,
                                     newFareMarket,
                                     itin,
                                     primarySector,
                                     tpmGovCxr,
                                     govCxrOverrides,
                                     participatingCarriers,
                                     yyOverride,
                                     fmBreakSet,
                                     false,
                                     true);
          }
        }
      }

      // Clone for last intl sector carrier for unknown Fare Market
      if (newFareMarket->direction() == FMDirection::UNKNOWN)
      {
        TravelSeg* primarySector(nullptr);

        CarrierCode inboundGovCxr =
            TravelSegAnalysis::getLastIntlFlt(newFareMarket->travelSeg(), primarySector);

        if (inboundGovCxr != BAD_CARRIER && newFareMarket->governingCarrier() != inboundGovCxr &&
            (tpmGovCxr.empty() || tpmGovCxr != inboundGovCxr))
        {
          cloneAndStoreFareMarket(trx,
                                  newFareMarket,
                                  itin,
                                  primarySector,
                                  inboundGovCxr,
                                  govCxrOverrides,
                                  participatingCarriers,
                                  yyOverride,
                                  fmBreakSet,
                                  true);
        }
      }
    }
  }

  if ((boundary.isSet(FMTravelBoundary::TravelWithinUSCA) ||
       boundary.isSet(FMTravelBoundary::TravelWithinSameCountryExceptUSCA)) &&
      newFareMarket->direction() == FMDirection::UNKNOWN)
  {
    addFareMarketDiffGovCXR(trx, itin, *newFareMarket);
  }
}

// This could only be the case for an exchange (REX) transaction where the
// VCTR is given.
bool
ItinAnalyzerServiceWrapper::isVctrCarrier(const FareMarket* faremarket) const
{
  const FareCompInfo* fci =
      (faremarket && faremarket->fareCompInfo()) ? faremarket->fareCompInfo() : nullptr;
  return (fci && fci->hasVCTR() && fci->VCTR().carrier() == faremarket->governingCarrier());
}

void
ItinAnalyzerServiceWrapper::createARUNKFareMarket(PricingTrx& trx,
                                                  Itin& itin,
                                                  SegmentAttribs::iterator beginSegIt,
                                                  SegmentAttribs::iterator endSegIt) const
{
  TSELatencyData metrics(trx, "ITIN BUILD ARUNK FMKT");

  FareMarket* fareMarket;
  trx.dataHandle().get(fareMarket); // lint !e530
  fareMarket->origin() = (*beginSegIt).tvlSeg->destination();

  // Next segment will be the ARUNK, we are guaranteed not to have
  // back-to-back ARUNKs
  for (SegmentAttribs::iterator i = beginSegIt + 2; i != endSegIt; ++i)
  {
    FareMarket* newFareMarket;
    trx.dataHandle().get(newFareMarket); // lint !e530

    newFareMarket->origin() = fareMarket->origin();
    newFareMarket->boardMultiCity() = fareMarket->boardMultiCity();
    newFareMarket->travelDate() = itin.travelDate();
    newFareMarket->travelSeg().push_back((*(beginSegIt + 1)).tvlSeg);
    // Push inclusive travel segments minus the one we're evaluating
    for (SegmentAttribs::iterator j = beginSegIt + 2; j != i; ++j)
    {
      newFareMarket->travelSeg().push_back((*j).tvlSeg);
    }

    // Another ARUNK?  We're guaranteed not to have an ARUNK
    // as the last segment
    const bool isArunk = fallback::segmentAttributesRefactor(&trx)
                             ? ((*i).attributes & ARUNK)
                             : (i->tvlSeg->segmentType() == Arunk);

    if (isArunk)
    {
      // Push the ARUNK
      newFareMarket->travelSeg().push_back((*i).tvlSeg);
      ++i;
      newFareMarket->destination() = (*i).tvlSeg->destination();
      newFareMarket->travelSeg().push_back((*i).tvlSeg);
      newFareMarket->legIndex() = newFareMarket->travelSeg().front()->legId();
      setupAndStoreFareMarket(trx, itin, newFareMarket);
    }
    else
    {
      if ((*i).tvlSeg->isAir())
      {
        newFareMarket->destination() = (*i).tvlSeg->destination();
        newFareMarket->travelSeg().push_back((*i).tvlSeg);
        newFareMarket->legIndex() = newFareMarket->travelSeg().front()->legId();
        setupAndStoreFareMarket(trx, itin, newFareMarket);
      }
    }
  }
}

void
ItinAnalyzerServiceWrapper::addStToFareMarket(const Itin& itin,
                                              FareMarket& newFareMarket,
                                              std::vector<std::vector<TravelSeg*>>& sideTrips,
                                              std::set<int16_t>& stSegOrders) const
{
  // Add side trips to fare market in reverse oder
  std::vector<std::vector<TravelSeg*>>::reverse_iterator stIter = sideTrips.rbegin();
  int lastStBegin = -1;
  int lastStEnd = -1;
  for (; stIter != sideTrips.rend(); stIter++)
  {
    std::vector<TravelSeg*>& stTvlSegs = *stIter;

    int curStBegin = itin.segmentOrder(stTvlSegs.front());
    int curStEnd = itin.segmentOrder(stTvlSegs.back());
    if (curStBegin > lastStBegin && curStEnd < lastStEnd)
    {
      // The current side trip is within another side trip.
      continue;
    }

    newFareMarket.sideTripTravelSeg().insert(newFareMarket.sideTripTravelSeg().begin(), stTvlSegs);
    for (int16_t i = curStBegin; i <= curStEnd; i++)
      stSegOrders.insert(i);

    lastStBegin = curStBegin;
    lastStEnd = curStEnd;
  }
}

void
ItinAnalyzerServiceWrapper::addPendingStToFareMarket(
    Itin& itin,
    FareMarket& newFareMarket,
    std::vector<std::vector<TravelSeg*>*>& pendingSideTrips,
    std::set<int16_t>& stSegOrders) const
{
  bool needResortFmTvlSeg = false;

  std::vector<std::vector<TravelSeg*>*>::iterator stIter = pendingSideTrips.begin();
  for (; stIter != pendingSideTrips.end(); stIter++)
  {
    std::vector<TravelSeg*>& travelSegs = **stIter;
    for (const auto travelSeg : travelSegs)
    {
      int16_t curSegOrder = itin.segmentOrder(travelSeg);
      if (stSegOrders.find(curSegOrder) != stSegOrders.end())
        continue;

      newFareMarket.travelSeg().push_back(travelSeg);
      needResortFmTvlSeg = true;
      stSegOrders.insert(curSegOrder);
    }
  }

  if (needResortFmTvlSeg)
  {
    std::sort(newFareMarket.travelSeg().begin(),
              newFareMarket.travelSeg().end(),
              CompareSegOrderBasedOnItin(&itin));
  }
}

void
ItinAnalyzerServiceWrapper::createSideTripCombination(PricingTrx& trx,
                                                      Itin& itin,
                                                      FareMarket*& fareMarket,
                                                      SideTrips::iterator& currentSideTripIt) const
{
  // Handle forced side trip
  std::vector<TravelSeg*>::iterator sourceIterBegin = (*currentSideTripIt).begin();
  std::vector<TravelSeg*>::iterator sourceIterEnd = (*currentSideTripIt).end();
  for (std::vector<TravelSeg*>::iterator sourceIter = sourceIterBegin; sourceIter != sourceIterEnd;
       sourceIter++)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*sourceIter);
    if (airSeg != nullptr && airSeg->forcedSideTrip() == 'T')
      return;
  }

  FareMarket* newFareMarket;
  trx.dataHandle().get(newFareMarket); // lint !e530

  newFareMarket->origin() = fareMarket->origin();
  newFareMarket->boardMultiCity() = fareMarket->boardMultiCity();
  newFareMarket->destination() = fareMarket->destination();
  newFareMarket->offMultiCity() = fareMarket->offMultiCity();
  newFareMarket->travelDate() = fareMarket->travelDate();
  newFareMarket->travelSeg() = fareMarket->travelSeg();

  // Copy the side trip travel segments as base travel segments
  std::vector<TravelSeg*>::iterator posIter = newFareMarket->travelSeg().begin();

  int16_t stSegStart = itin.segmentOrder(*sourceIterBegin);
  for (size_t j = 1; j < newFareMarket->travelSeg().size(); j++)
  {
    if (itin.segmentOrder(newFareMarket->travelSeg()[j]) < stSegStart)
    {
      continue;
    }
    else
    {
      posIter += j;
      break;
    }
  }
  newFareMarket->travelSeg().insert(posIter, sourceIterBegin, sourceIterEnd);

  // Keep remaining side trips as side trips
  for (SideTrips::iterator i = fareMarket->sideTripTravelSeg().begin();
       i != fareMarket->sideTripTravelSeg().end();
       ++i)
  {
    if (i != currentSideTripIt)
      newFareMarket->sideTripTravelSeg().push_back(*i);
  }

  // Save this fare market that removed this side trip.
  if (!getExistingFareMarket(*newFareMarket, &itin, trx))
    setupAndStoreFareMarket(trx, itin, newFareMarket);
}

void
ItinAnalyzerServiceWrapper::createThruFareMarket(PricingTrx& trx,
                                                 Itin& itin,
                                                 const FareMarket* const fareMarket) const
{
  FareMarket* thruFareMarket = nullptr;
  trx.dataHandle().get(thruFareMarket); // lint --e{413}

  thruFareMarket->origin() = fareMarket->origin();
  thruFareMarket->boardMultiCity() = fareMarket->boardMultiCity();
  thruFareMarket->destination() = fareMarket->destination();
  thruFareMarket->offMultiCity() = fareMarket->offMultiCity();
  thruFareMarket->travelDate() = itin.travelDate();

  int16_t startPos = itin.segmentOrder(fareMarket->travelSeg().front());
  int16_t endPos = itin.segmentOrder(fareMarket->travelSeg().back());
  for (int16_t pos = startPos; pos <= endPos; pos++)
    thruFareMarket->travelSeg().push_back(itin.travelSeg()[pos - 1]);

  if (!getExistingFareMarket(*thruFareMarket, &itin, trx))
    setupAndStoreFareMarket(trx, itin, thruFareMarket);
}

bool
ItinAnalyzerServiceWrapper::isJointUSCAFareMarket(const FareMarket* const fareMarket) const
{
  if (fareMarket->geoTravelType() != GeoTravelType::Domestic && fareMarket->geoTravelType() != GeoTravelType::Transborder)
    return false;

  CarrierCode firstCxr;
  for (const auto elem : fareMarket->travelSeg())
  {
    const AirSeg* seg = elem->toAirSeg();
    if (!seg)
      continue;
    if (firstCxr.empty())
    {
      firstCxr = seg->carrier();
      continue;
    }
    if (seg->carrier() != firstCxr)
      return true;
  }
  return false;
}

bool
ItinAnalyzerServiceWrapper::setBoardAndOffPoints(PricingTrx& trx,
                                                 const Itin& itin,
                                                 FareMarket& fareMarket) const
{
  TSELatencyData metrics(trx, "ITIN SETBRDANDOFFPNTS");

  if (fareMarket.governingCarrier().empty())
  {
    GoverningCarrier govCxrService(&trx);
    govCxrService.process(fareMarket);
  }

  FareMarketUtil::setMultiCities(fareMarket, trx.travelDate());
  fareMarket.boardMultiCity() =
      FareMarketUtil::getBoardMultiCity(fareMarket, *fareMarket.travelSeg().front());
  fareMarket.offMultiCity() =
      FareMarketUtil::getOffMultiCity(fareMarket, *fareMarket.travelSeg().back());

  if (LIKELY(!trx.getOptions()->isRtw()))
  {
    if (fareMarket.boardMultiCity() == fareMarket.offMultiCity())
      return false;

    // Check if retransit FM board or off point after GovCxr is set
    LimitationOnIndirectTravel lim(trx, itin);
    if (lim.retransitFareComponentBoardOffPoint(fareMarket))
      return false;
  }

  return true;
}

FareMarket*
ItinAnalyzerServiceWrapper::getExistingFareMarket(const FareMarket& fareMarket,
                                                  Itin* const itin,
                                                  PricingTrx& trx) const
{
  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    // Set retransit points
    _itinAnalyzer.setRetransits(*itin);
  }
  std::vector<FareMarket*>::const_iterator fareMarketsIter = itin->fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fareMarketsIterEnd = itin->fareMarket().end();
  for (; fareMarketsIter != fareMarketsIterEnd; fareMarketsIter++)
  {
    if ((*fareMarketsIter)->origin() != fareMarket.origin())
      continue;
    if ((*fareMarketsIter)->destination() != fareMarket.destination())
      continue;
    if ((*fareMarketsIter)->getGlobalDirection() != fareMarket.getGlobalDirection())
      continue;
    if ((*fareMarketsIter)->travelSeg() != fareMarket.travelSeg())
      continue;
    if ((*fareMarketsIter)->sideTripTravelSeg() != fareMarket.sideTripTravelSeg())
      continue;
    if ((*fareMarketsIter)->travelDate().date() != fareMarket.travelDate().date())
      continue;
    if ((*fareMarketsIter)->direction() != fareMarket.direction())
      continue;
    if ((*fareMarketsIter)->breakIndicator() != fareMarket.breakIndicator())
      continue;

    LOG4CXX_INFO(_itinAnalyzer._logger,
                 "FareMarket: " << fareMarket.origin()->loc() << "-"
                                << fareMarket.destination()->loc() << " already exists, REUSING");
    LOG4CXX_DEBUG(_itinAnalyzer._logger,
                  "  fareMarket == existing fareMarket: " << (**fareMarketsIter == fareMarket));
    return (*fareMarketsIter);
  }
  return nullptr;
}

FareMarket*
ItinAnalyzerServiceWrapper::getExistingFareMarket(const FareMarket& fareMarket,
                                                  PricingTrx& trx,
                                                  bool checkExistingForFareSelection) const
{
  TSELatencyData metrics(trx, "ITIN GETEXISTFAREMARKET");
  std::vector<FareMarket*>::const_iterator fareMarketsIter = trx.fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fareMarketsIterEnd = trx.fareMarket().end();
  for (; fareMarketsIter != fareMarketsIterEnd; fareMarketsIter++)
  {
    if ((*fareMarketsIter)->origin() != fareMarket.origin())
      continue;
    if ((*fareMarketsIter)->destination() != fareMarket.destination())
      continue;

    if (LIKELY(!checkExistingForFareSelection))
    {
      if ((*fareMarketsIter)->getGlobalDirection() != fareMarket.getGlobalDirection())
        continue;
      if ((*fareMarketsIter)->sideTripTravelSeg() != fareMarket.sideTripTravelSeg())
        continue;
      if ((*fareMarketsIter)->travelSeg() != fareMarket.travelSeg())
        continue;
    }
    else
    {
      if ((*fareMarketsIter)->governingCarrier() != fareMarket.governingCarrier())
        continue;
    }

    if ((*fareMarketsIter)->direction() != fareMarket.direction())
      continue;
    if (UNLIKELY((*fareMarketsIter)->breakIndicator() != fareMarket.breakIndicator()))
      continue;
    if (UNLIKELY((*fareMarketsIter)->travelDate().date() != fareMarket.travelDate().date()))
      continue;

    return (*fareMarketsIter);
  }

  return nullptr;
}

bool
ItinAnalyzerServiceWrapper::isAcrossStopoverDomesticUSFareMarket(const FareMarket& fareMarket) const
{
  if (fareMarket.geoTravelType() == GeoTravelType::Domestic)
  {
    // exclude Hawaii and Alaska
    if (LocUtil::isHawaii(*fareMarket.origin()) || LocUtil::isHawaii(*fareMarket.destination()) ||
        LocUtil::isAlaska(*fareMarket.origin()) || LocUtil::isAlaska(*fareMarket.destination()))
    {
      return false;
    }
    size_t tvlSegSize = fareMarket.travelSeg().size();

    for (size_t i = 0; i < (tvlSegSize - 1); ++i) // do not check last segment
    {
      if (fareMarket.travelSeg()[i]->stopOver())
      {
        // its across stopover, now check exceptions
        // exclude carriers/cities specified in config
        std::map<std::string, std::set<std::string>>::const_iterator foundCarrier =
            _acrossStopoverEnabledCarriersCities.find(fareMarket.governingCarrier());

        if (foundCarrier != _acrossStopoverEnabledCarriersCities.end())
        {
          const std::set<std::string>& citiesList = foundCarrier->second;

          if (citiesList.find("*") != citiesList.end() || // all cities for this carrier
              citiesList.find(fareMarket.destination()->city()) !=
                  citiesList.end()) // or specified destination
          {
            return false;
          }
        }

        return true;
      }
    }
  }

  return false;
}

//-----------------------------------------------------------------
// parse config entry in a form of
// CARRIER1,CITY1,CITY2|CARRIER2,CITY3...
// ie. SY,*|HA,*|F9,*|US,DEN
// "*" means all cities
//-----------------------------------------------------------------
void
ItinAnalyzerServiceWrapper::parseAcrossStopoverExcludedCarriersCities(
    const std::string& acrossStopoverEnabledCarriers)
{
  typedef boost::char_separator<char> separatorType;
  typedef boost::tokenizer<separatorType> tokenizerType;

  if (acrossStopoverEnabledCarriers.empty())
    return;

  separatorType listSeparator("|");
  tokenizerType listTokens(acrossStopoverEnabledCarriers, listSeparator);

  for (tokenizerType::iterator listTokensIter = listTokens.begin();
       listTokensIter != listTokens.end();
       ++listTokensIter)
  {
    separatorType carrierCitySeparator("^");
    tokenizerType carrierCityTokens(*listTokensIter, carrierCitySeparator);

    bool gotCarrier = false;

    std::string carrier;
    std::set<std::string> cities;

    for (tokenizerType::iterator carrierCityTokensIter = carrierCityTokens.begin();
         carrierCityTokensIter != carrierCityTokens.end();
         ++carrierCityTokensIter)
    {
      if (!gotCarrier) // first is the carrier
      {
        carrier = *carrierCityTokensIter;
        gotCarrier = true;
        continue;
      }

      cities.insert(*carrierCityTokensIter);
      if (*carrierCityTokensIter == "*") // all cities
      {
        TSE_ASSERT(cities.size() == 1);
        break;
      }
    }

    TSE_ASSERT(gotCarrier && !carrier.empty() && !cities.empty());
    _acrossStopoverEnabledCarriersCities.insert(std::make_pair(carrier, cities));
  }
}

bool
ItinAnalyzerServiceWrapper::simplifiedBuildFareMarket(PricingTrx& trx, Itin& itin)
{
  using FareMarketParams = SimplifiedFareMarketsBuilder::FareMarketParams;

  std::vector<SegmentAttributes> segAttributes;
  const std::vector<FareMarketParams>& fmParams =
                                      SimplifiedFareMarketsBuilder(trx, itin).build(segAttributes);

  for (const FareMarketParams& param : fmParams)
  {
    if (std::distance(param.begin, param.end))
    {
      std::vector<TravelSeg*> tvlSegs;

      std::transform(param.begin, param.end, std::back_inserter(tvlSegs),
                     [](const auto segAttr) { return segAttr.tvlSeg; });

      if (!isFMNotAllowed(segAttributes, param.begin, param.end))
        createFareMarket(trx, itin, param.end, tvlSegs, false, nullptr);
    }
  }
  return true;
}

bool
ItinAnalyzerServiceWrapper::isFMNotAllowed(const std::vector<SegmentAttributes>& segAttributes,
                                           std::vector<SegmentAttributes>::const_iterator begin,
                                           std::vector<SegmentAttributes>::const_iterator end) const
{
  return isFMinMiddleOfLeg(segAttributes, begin, end) ||
         isArunkFirst(begin) ||
         isArunkLast(segAttributes, end);
}
}
