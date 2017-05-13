///-------------------------------------------------------------------------------
//  Code extracted directly from ItinAnalyzerService.cpp
//
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "ItinAnalyzer/JourneyPrepHelper.h"

#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/ItinUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/SimilarItinData.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"

namespace tse
{
FIXEDFALLBACK_DECL(fallbackAvailBreakInterlineJourney)

namespace iadetail
{
void
JourneyPrepHelper::prepareForJourney(PricingTrx& trx)
{
  markMarriedMarkets(trx);

  prepareItinForJourney(trx.itin().begin(), trx.itin().end(), trx);
}

void
JourneyPrepHelper::updateCarrierPreferences(PricingTrx& trx)
{
  updateCarrierPreferences(trx.itin().begin(), trx.itin().end(), trx);

  for (Itin* itin : trx.itin())
    updateCarrierPreferences(itin->getSimilarItins().begin(), itin->getSimilarItins().end(), trx);
}

//----------------------------------------------------------------------------
// updateCarrierPreferences - set CarrierPreference pointer in Travel segs
//----------------------------------------------------------------------------
void
JourneyPrepHelper::updateCarrierPreferences(std::vector<Itin*>::const_iterator itinBegin,
                                            std::vector<Itin*>::const_iterator itinEnd,
                                            PricingTrx& trx)
{
  for (; itinBegin != itinEnd; ++itinBegin)
    updateCarrierPreferences(trx, **itinBegin);
}

//----------------------------------------------------------------------------
// updateCarrierPreferences - set CarrierPreference pointer in Travel segs
//----------------------------------------------------------------------------
void
JourneyPrepHelper::updateCarrierPreferences(std::vector<SimilarItinData>::const_iterator itinBegin,
                                            std::vector<SimilarItinData>::const_iterator itinEnd,
                                            PricingTrx& trx)
{
  for (; itinBegin != itinEnd; ++itinBegin)
    updateCarrierPreferences(trx, *itinBegin->itin);
}

void
JourneyPrepHelper::updateCarrierPreferences(PricingTrx& trx, Itin& itin)
{
  for (TravelSeg* segment : itin.travelSeg())
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(segment);
    if (airSeg == nullptr)
      continue;
    const CarrierCode& carrier = airSeg->carrier();
    if (UNLIKELY(carrier.empty()))
      continue;

    const DateTime& travelDate = trx.adjustedTravelDate(airSeg->departureDT());
    const CarrierPreference* cp = getCarrierPref(trx, carrier, travelDate);
    airSeg->carrierPref() = cp;
  }
}

//----------------------------------------------------------------------------
// prepareItinForJourney -  get journeys FMs for regular Itins
//----------------------------------------------------------------------------
void
JourneyPrepHelper::prepareItinForJourney(std::vector<Itin*>::const_iterator itinBegin,
                                         std::vector<Itin*>::const_iterator itinEnd,
                                         PricingTrx& trx)
{
  const bool initAvailBreakForRepricing = shouldInitAvailBreakForRepricing(trx);

  for (; itinBegin != itinEnd; ++itinBegin)
    prepareItinForJourney(initAvailBreakForRepricing, **itinBegin, trx);
}

//----------------------------------------------------------------------------
// prepareItinForJourney -  get journeys FMs for Similar Itins
//----------------------------------------------------------------------------
void
JourneyPrepHelper::prepareItinForJourney(std::vector<SimilarItinData>::const_iterator itinBegin,
                                         std::vector<SimilarItinData>::const_iterator itinEnd,
                                         PricingTrx& trx)
{
  const bool initAvailBreakForRepricing = shouldInitAvailBreakForRepricing(trx);

  for (; itinBegin != itinEnd; ++itinBegin)
    prepareItinForJourney(initAvailBreakForRepricing, *itinBegin->itin, trx);
}

//----------------------------------------------------------------------------
// prepareItinForJourney -  get journeys FMs for single Itin
//----------------------------------------------------------------------------
void
JourneyPrepHelper::prepareItinForJourney(const bool initAvailBreakForRepricing,
                                         Itin& itin,
                                         PricingTrx& trx)
{
  // find and mark the Fare Markets in this itin as flow FareMarkets
  if (JourneyUtil::journeyActivated(trx))
    markFlowMarkets(trx, itin);

  // initialize avail Breaks in each fare market
  std::vector<FareMarket*> processedFM;

  bool ataeSecondCall =
      trx.getOptions()->callToAvailability() == 'T' || itin.dcaSecondCall(); // active 2nd wash

  for (FareMarket* fm : itin.fareMarket())
  {
    if (trx.getTrxType() == PricingTrx::MIP_TRX || initAvailBreakForRepricing)
    {
      if (FareMarketUtil::fmWithoutCOS(*fm, trx, nullptr, processedFM))
        initAvailBreak(trx, *fm); // initialize avail Breaks
      else
        initAvailBreakForMIP(trx, *fm, itin); // initialize avail Breaks
    }
    if (UNLIKELY(trx.getTrxType() == PricingTrx::PRICING_TRX))
      initAvailBreak(trx, *fm); // initialize avail Breaks

    if ((trx.getTrxType() == PricingTrx::MIP_TRX || initAvailBreakForRepricing) &&
        !ataeSecondCall) // availBreaks will be set after 2nd wash
    {
      initAvailSmallFMWithoutCOS(*fm, itin, trx, processedFM);
    }
  }

  if ((trx.getTrxType() == PricingTrx::MIP_TRX || initAvailBreakForRepricing) &&
      !ataeSecondCall) // availBreaks will be set after 2nd wash
  {
    for (FareMarket* fm : itin.fareMarket())
      initAvailWithoutCOS(*fm, itin, trx, processedFM);
  }
}

void
JourneyPrepHelper::markFlowMarkets(PricingTrx& trx, Itin& itin)
{
  std::vector<FareMarket*> jnyMarkets;
  ItinUtil::journeys(itin, trx, jnyMarkets);
  for (const auto fareMarket : jnyMarkets)
  {
    if (fareMarket->travelSeg().size() > 1)
    {
      fareMarket->setFlowMarket(true);
      trx.getOptions()->applyJourneyLogic() = true;
      itin.flowFareMarket().push_back(fareMarket);
    }
  }
}

void
JourneyPrepHelper::markMarriedMarkets(PricingTrx& trx)
{
  if (!JourneyUtil::useJourneyLogic(trx) || trx.itin().empty())
    return;

  Itin* itin = trx.itin().front();

  std::vector<FareMarket*>::iterator fmIt = itin->fareMarket().begin();
  std::vector<FareMarket*>::iterator fmItE = itin->fareMarket().end();

  std::vector<FareMarket*> marriedFm;

  for (; fmIt != fmItE; ++fmIt)
  {
    FareMarket& fm = *(*fmIt);

    std::map<const TravelSeg*, bool> marrriedSegs;
    bool journeyByMarriage, outOfSeqMarriage;

    bool segsMarried =
        JourneyUtil::getMarriedSegments(trx, fm, marrriedSegs, journeyByMarriage, outOfSeqMarriage);
    if (!segsMarried)
      continue;

    if (duplicateFareMarket(marriedFm, *fmIt))
      continue;

    marriedFm.push_back(*fmIt);

    fm.setHasAllMarriedSegs(true);

    if (journeyByMarriage)
      JourneyUtil::addOAndDMarket(trx, *itin, nullptr, &fm, false, &marrriedSegs);
  }
}

bool
JourneyPrepHelper::duplicateFareMarket(std::vector<FareMarket*>& marriedFm, FareMarket* fm)
{
  if (marriedFm.empty())
    return false;

  std::vector<FareMarket*>::iterator fmB = marriedFm.begin();
  std::vector<FareMarket*>::iterator fmE = marriedFm.end();
  std::vector<FareMarket*>::iterator fmI = find(fmB, fmE, fm);

  if (fmI != fmE)
    return true;

  for (fmI = fmB; fmI != fmE; fmI++)
  {
    const FareMarket& fmInVec = *(*fmI);
    if (fmInVec.travelSeg() == fm->travelSeg())
      return true;
  }

  return false;
}

void
JourneyPrepHelper::initAvailWithoutCOS(FareMarket& fm,
                                       Itin& itin,
                                       PricingTrx& trx,
                                       std::vector<FareMarket*>& processedFM)
{
  if (FareMarketUtil::fmWithoutCOS(fm, trx, nullptr, processedFM))
  {
    if (fm.travelSeg().size() > 2)

    {
      // again start the loop to process markets with 3
      // or more flights in them
      size_t startIndex = 0;
      const size_t totalFlt = fm.travelSeg().size();
      while (true)
      {
        if (startIndex >= totalFlt)
          break;
        const size_t fltsCovered = FareMarketUtil::numFlts(itin, fm, startIndex);
        FareMarketUtil::group3(itin, fm, startIndex, fltsCovered, trx, processedFM, false);
        startIndex += fltsCovered;
      }

      processedFM.push_back(&fm);
    }
  }
}

void
JourneyPrepHelper::initAvailSmallFMWithoutCOS(FareMarket& fm,
                                              Itin& itin,
                                              PricingTrx& trx,
                                              std::vector<FareMarket*>& processedFM)
{
  if (FareMarketUtil::fmWithoutCOS(fm, trx, nullptr, processedFM))
  {
    if (fm.travelSeg().size() < 3)
    {
      fm.availBreaks()[0] = true;
      if (fm.travelSeg().size() == 2)
        fm.availBreaks()[1] = true;

      processedFM.push_back(&fm);
    }
  }
}

void
JourneyPrepHelper::initAvailBreak(const PricingTrx& trx, FareMarket& fm)
{
  if (fm.availBreaks().size() == 0 || fm.availBreaks().size() != fm.travelSeg().size())
  {
    const auto& tvlSegs = fm.travelSeg();
    const size_t numTvlSegs = tvlSegs.size();

    fm.availBreaks().resize(numTvlSegs);

    // The availability should always break at last travel Seg
    // If last is ARUNK then also break at last valid AirSeg
    if (numTvlSegs > 0)
    {
      fm.availBreaks()[numTvlSegs - 1] = true;

      if (numTvlSegs == 1)
        return;

      if (TrxUtil::isControlConnectIndicatorActive(trx))
      {
         auto rit = std::find_if(tvlSegs.crbegin(), tvlSegs.crend(),
                                 [](const TravelSeg* tvl){ return tvl->isAir(); });

         if (rit == tvlSegs.crend())
            return;

         const size_t lastAirSegIdx = rit.base() - std::begin(tvlSegs) - 1;
         fm.availBreaks()[lastAirSegIdx] = true;

         for (size_t idx = 0; idx < lastAirSegIdx; idx++)
         {
           const AirSeg* airSeg = tvlSegs[idx]->toAirSeg();
           const AirSeg* nextAirSeg = tvlSegs[idx+1]->toAirSeg();

           if (airSeg && nextAirSeg)
           {
             if (airSeg->carrier() == nextAirSeg->carrier())
             {
               if (isSoloCarrierAvailabilityApply(trx, airSeg))
                 fm.availBreaks()[idx] = true;
             }
           }
         }
      }
      else
      {
        uint16_t i = numTvlSegs - 1;
        AirSeg* airSeg = nullptr;
        std::vector<TravelSeg*>::iterator tvlI = fm.travelSeg().end();
        std::vector<TravelSeg*>::iterator tvlB = fm.travelSeg().begin();
        --tvlI;
        for (; tvlI != tvlB; tvlI--, i--)
        {
          airSeg = dynamic_cast<AirSeg*>(*tvlI);
          if (airSeg != nullptr)
            break;
        }
        fm.availBreaks()[i] = true;
      }
    }
  }
}

void
JourneyPrepHelper::initAvailBreakForMIP(PricingTrx& trx, FareMarket& fm, Itin& itin)
{
  if (fm.availBreaks().size() == 0 || fm.availBreaks().size() != fm.travelSeg().size())
  {
    const size_t numTvlSegs = fm.travelSeg().size();

    fm.availBreaks().resize(numTvlSegs);

    // The availability should always break at last travel Seg
    // If last is ARUNK then also break at last valid AirSeg
    if (LIKELY(numTvlSegs > 0))
    {
      fm.availBreaks()[numTvlSegs - 1] = true;

      if (numTvlSegs == 1)
        return;

      AirSeg* airSeg = nullptr;
      AirSeg* nextAirSeg = nullptr;
      std::vector<TravelSeg*>::iterator tvlI = fm.travelSeg().begin();
      std::vector<TravelSeg*>::iterator tvlE = fm.travelSeg().end() - 1;
      bool availBreakFound = false;
      size_t availFltCount = 0;
      for (size_t i = 0; tvlI != tvlE; tvlI++, i++)
      {
        airSeg = dynamic_cast<AirSeg*>(*tvlI);

        if (airSeg == nullptr) // ARUNK
          continue;

        ++availFltCount;
        if (availBreakFound)
        {
          availFltCount = 0;
          availBreakFound = false;
        }

        nextAirSeg = dynamic_cast<AirSeg*>(*(tvlI + 1));

        if (availFltCount == 3) // always break if 3 flight connection already
        {
          fm.availBreaks()[i] = true;
        }
        else
        {
          if (fallback::fixed::fallbackAvailBreakInterlineJourney())
          {
            if ((nextAirSeg == nullptr) || // ARUNK
                (airSeg->carrier() != nextAirSeg->carrier()))
            {
              fm.availBreaks()[i] = true;
              availBreakFound = true;
            }
            else
            {
              if (!ItinUtil::journeyConnection(itin, nextAirSeg, airSeg, availFltCount + 1))
              {
                fm.availBreaks()[i] = true;
                availBreakFound = true;
              }
            }
          }
          else
          {
            availBreakFound = isAvailBreak(trx, itin, availFltCount, airSeg, nextAirSeg);
            if (availBreakFound)
              fm.availBreaks()[i] = true;
          }
        }
      }
    }
  }
}

bool JourneyPrepHelper::isAvailBreak(const PricingTrx& trx,
                                     const Itin& itin,
                                     const size_t availFltCount,
                                     const AirSeg* airSeg,
                                     const AirSeg* nextAirSeg)
{
  if (nextAirSeg == nullptr)  // ARUNK
    return true;

  if (!ItinUtil::journeyConnection(itin, nextAirSeg, airSeg, availFltCount + 1))
    return true;

  if (airSeg && nextAirSeg)
  {
    if (airSeg->carrier() != nextAirSeg->carrier())
    {
      if (!TrxUtil::intralineAvailabilityApply(trx, airSeg->carrier(), nextAirSeg->carrier()))
        return true;
    }
    else
    {
      if (TrxUtil::isControlConnectIndicatorActive(trx) &&
          isSoloCarrierAvailabilityApply(trx, airSeg))
        return true;
    }
  }

  return false;
}

// TODO This probably belongs in some utils class
const CarrierPreference*
JourneyPrepHelper::getCarrierPref(PricingTrx& trx, const CarrierCode& carrier, const DateTime& date)
{
  return trx.dataHandle().getCarrierPreference(carrier, date);
}

bool
JourneyPrepHelper::shouldInitAvailBreakForRepricing(PricingTrx& trx)
{
  if (trx.getTrxType() == PricingTrx::REPRICING_TRX)
  {
    RepricingTrx* reTrx = dynamic_cast<RepricingTrx*>(&trx);
    if (reTrx && reTrx->getOriginTrxType() == PricingTrx::MIP_TRX)
      return true;
  }

  return false;
}

bool
JourneyPrepHelper::isSoloCarrierAvailabilityApply(const PricingTrx& trx, const AirSeg* airSeg)
{
  if (trx.getTrxType() == PricingTrx::MIP_TRX &&  airSeg->isSoloShoppingCarrier())
    return true;

  if (trx.getRequest()->isLowFareRequested() && airSeg->isSoloPricingCarrier())
    return true;

  return false;
}

} // iadetail
} // tse
