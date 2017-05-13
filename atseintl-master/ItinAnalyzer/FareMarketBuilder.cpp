//----------------------------------------------------------------------------
//  Copyright Sabre 2006
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "ItinAnalyzer/FareMarketBuilder.h"

#include "Common/Logger.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"

namespace tse
{

static Logger
logger("atseintl.ItinAnalyzer.FareMarketBuilder");

void
FareMarketBuilder::setBreakIndicator(FareMarket* fareMarket, Itin* itin, PricingTrx& trx)
{
  if (UNLIKELY(fareMarket->breakIndicator())) // It may have been set in other functions.
    return;

  if (UNLIKELY(trx.isPbbRequest() == PBB_RQ_PROCESS_BRANDS &&
      !fareMarket->isApplicableForPbb()))
  {
    fareMarket->setBreakIndicator(true);
    return;
  }

  // Will always have at least one travel seg
  int numTvlSegs = fareMarket->travelSeg().size();

  // Assume false
  bool breakIndicator = false;
  bool firstFbcDetermined = false;
  std::string firstFbc(EMPTY_STRING());
  if (UNLIKELY(setFareCalcAmtForDummyFareFM(trx, itin, *fareMarket)))
    return;

  const bool ignoreFareBasisCode =
      (trx.excTrxType() == PricingTrx::AR_EXC_TRX) && (nullptr == dynamic_cast<ExcItin*>(itin));

  for (int i = 0; i < numTvlSegs; i++)
  {
    TravelSeg* airSeg = fareMarket->travelSeg()[i];
    if (UNLIKELY(airSeg == nullptr))
      continue;

    // If segments have conflict selected FareBasisCode, break this fare market.
    // If segments have conflict selected FareAmt, break this fare market.
    if (UNLIKELY(!ignoreFareBasisCode && trx.getOptions()->fbcSelected() &&
        !(airSeg->segmentType() == Surface || airSeg->segmentType() == Arunk)))
    {
      if (fareMarket->fareBasisCode().empty())
      {
        if (!airSeg->fareBasisCode().empty())
        {
          fareMarket->fareBasisCode() = airSeg->fareBasisCode();
          fareMarket->fbcUsage() = airSeg->fbcUsage();
        }
      }
      else
      {
        if (fareMarket->fareBasisCode() != airSeg->fareBasisCode() ||
            fareMarket->fbcUsage() != airSeg->fbcUsage())
        {
          breakIndicator = true;
          break;
        }
      }

      if (!firstFbcDetermined)
      {
        firstFbc = airSeg->fareBasisCode();
        firstFbcDetermined = true;
      }
      else
      {
        if (firstFbc != airSeg->fareBasisCode())
        {
          breakIndicator = true;
          break;
        }
      }
    }

    if (UNLIKELY(airSeg->isForcedFareBrk()))
    {
      if (fareMarket->destination() != airSeg->destination())
      {
        breakIndicator = true;
        break;
      }
    }
    else if (UNLIKELY(airSeg->isForcedNoFareBrk()))
    {
      if (fareMarket->destination() == airSeg->destination())
      {
        breakIndicator = true;
        break;
      }
    }
  }

  // Back search to ensure the air segment before the
  // fare market is not NB
  int fmStartSegOrder = itin->segmentOrder(fareMarket->travelSeg().front());

  int itinTvlSegs = itin->travelSeg().size();

  // if THRU fare parameter is found and fm is not stopover at Destination
  //  then fm can not be used

  const bool thruFare =
      (trx.getOptions()->isThruFares()) && (trx.getTrxType() == PricingTrx::MIP_TRX);
  if ((thruFare) && (!(fareMarket->travelSeg().back()->stopOver())))
    breakIndicator = true;

  if (fmStartSegOrder > 1 && fmStartSegOrder <= itinTvlSegs)
  {
    TravelSeg* lastAirSeg = itin->travelSeg()[fmStartSegOrder - 2];

    // fix https://jira.sabre.com/browse/SCI-48.
    // Open Jaw contains ArunkSeg. We must skip
    // this segment and test segment before this one:
    // ITIN: 1
    // 1  CY  346  H 8 20DEC  T LCA  LHR 1000A  105P  321  0  /E
    // 2  CY 1822  S 8 20DEC  T LHR  LAX  315P  655P  346  0  /E
    // 3  ARUNK
    // 4  CY 1827  L 8 14JAN  M JFK  LHR 1015P 1030A  346  0  /E
    // 5  CY  327  U 8 15JAN  T LHR  LCA 1000P  435A  320  0  /E
    if (dynamic_cast<ArunkSeg*>(lastAirSeg))
    {
      if (fmStartSegOrder > 2)
      {
        lastAirSeg = itin->travelSeg().at(fmStartSegOrder - 3);
      }
      else
      {
        lastAirSeg = nullptr;
      }
    }

    if (UNLIKELY(lastAirSeg != nullptr && lastAirSeg->isForcedNoFareBrk()))
      breakIndicator = true;

    // if THRU fare parameter is found and fm is not stopover at Origin
    //  then fm can not be used
    if ((thruFare) && (lastAirSeg != nullptr) && (!(lastAirSeg->stopOver())))
      breakIndicator = true;
  }

  fareMarket->setBreakIndicator(breakIndicator);
  if (fareMarket->breakIndicator())
    fareMarket->fareBasisCode() = "";
}

void
FareMarketBuilder::setBreakIndByCxrOverride(const std::vector<CarrierCode>& govCxrOverrides,
                                            const std::set<CarrierCode>& participatingCarriers,
                                            FareMarket& fareMarket,
                                            bool yyOverride)
{
  if (UNLIKELY(yyOverride))
  {
    if (fareMarket.governingCarrier() != INDUSTRY_CARRIER)
    {
      fareMarket.setBreakIndicator(true); // GovCxr is not YY. Fare market is not for primary
                                          // pricing
    }
  }
  else if (UNLIKELY(!govCxrOverrides.empty()))
  {
    if ((govCxrOverrides.size() > 1) || // Conflict GovCxr Overrides
        ((fareMarket.governingCarrier() != govCxrOverrides.front()) &&
         (participatingCarriers.find(govCxrOverrides.front()) != participatingCarriers.end())))
    {
      // CxrOverride is participating carrier but different from GovCxr. Fare market is not for
      // primary pricing
      fareMarket.setBreakIndicator(true);
    }
  }
}

void
CollectStopOverTravelSeg::
operator()(FareMarket* fm)
{
  if (UNLIKELY(fm->travelSeg().empty()))
    return;

  bool hasSideTrip = (!fm->sideTripTravelSeg().empty());

  std::vector<TravelSeg*>& tvlSegs = fm->travelSeg();
  std::vector<TravelSeg*>::iterator tvlSegPrevI = tvlSegs.begin();
  std::vector<TravelSeg*>::iterator tvlSegI = (tvlSegPrevI + 1);

  for (; tvlSegI != tvlSegs.end(); ++tvlSegPrevI, ++tvlSegI)
  {
    TravelSeg& prevTvlSeg = **tvlSegPrevI;
    TravelSeg& curTvlSeg = **tvlSegI;

    if (UNLIKELY(prevTvlSeg.isForcedConx()))
      continue;

    if (prevTvlSeg.isForcedStopOver() || (!hasSideTrip && prevTvlSeg.stopOver()) ||
        (hasSideTrip && curTvlSeg.isStopOver(&prevTvlSeg, _itin.geoTravelType())))
    {
      fm->stopOverTravelSeg().insert(&prevTvlSeg);
    }
  }

  // Check the end travel seg (No Side Trip happens at fare break point)
  TravelSeg& lastTvlSeg = **tvlSegPrevI;
  if (lastTvlSeg.isForcedStopOver() || (!lastTvlSeg.isForcedConx() && lastTvlSeg.stopOver()))
  {
    fm->stopOverTravelSeg().insert(&lastTvlSeg);
  }
}

bool
FareMarketBuilder::setFareCalcAmtForDummyFareFM(const PricingTrx& trx,
                                                const Itin* const& itin,
                                                FareMarket& fm)
{
  if (LIKELY(itin == nullptr || trx.excTrxType() != PricingTrx::PORT_EXC_TRX ||
      fm.travelSeg().back()->fareCalcFareAmt().empty()))
    return false;

  const ExchangePricingTrx& excTrx = static_cast<const ExchangePricingTrx&>(trx);

  if (excTrx.exchangeOverrides().dummyFCSegs().empty())
    return false;

  const std::map<const TravelSeg*, uint16_t>::const_iterator dFCSegIEnd =
      excTrx.exchangeOverrides().dummyFCSegs().end();

  std::map<const TravelSeg*, uint16_t>::const_iterator dFCBeginIter =
      excTrx.exchangeOverrides().dummyFCSegs().find(fm.travelSeg().front());
  if (dFCBeginIter == dFCSegIEnd)
    return false;

  const uint16_t dummyFCNum = dFCBeginIter->second;

  std::map<const TravelSeg*, uint16_t>::const_iterator dFCEndIter =
      excTrx.exchangeOverrides().dummyFCSegs().find(fm.travelSeg().back());
  if (dFCEndIter == dFCSegIEnd || (dummyFCNum != dFCEndIter->second))
    return false;

  // make sure the number of travel seg in dummy FC is same as number of
  // travel seg in FareMarket
  uint16_t dummyFCSegCnt = 0;
  std::map<const TravelSeg*, uint16_t>::const_iterator dFCSegI =
      excTrx.exchangeOverrides().dummyFCSegs().begin();
  for (; dFCSegI != dFCSegIEnd; dFCSegI++)
  {
    if (dFCSegI->second == dummyFCNum)
      dummyFCSegCnt++;
  }
  if (dummyFCSegCnt != fm.travelSeg().size())
    return false;

  fm.fareBasisCode() = fm.travelSeg().back()->fareBasisCode();
  fm.fareCalcFareAmt() = fm.travelSeg().back()->fareCalcFareAmt();

  return true;
}
}
