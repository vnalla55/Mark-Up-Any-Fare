//-------------------------------------------------------------------------------
//  CalcTotals.cpp
//
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "FareCalc/CalcTotals.h"

#include "Common/Assert.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TravelSegUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FcDispFareUsage.h"

namespace tse
{
FIXEDFALLBACK_DECL(APO29538_StopoverMinStay);

static Logger
CalcTotals_logger("atseintl.FareCalc.CalcTotals");

bool
CalcTotals::getDispConnectionInd(const PricingTrx& trx,
                                 const TravelSeg* ts,
                                 Indicator connectionInd) const
{
  if (UNLIKELY(!ts))
    return false;

  std::map<const TravelSeg*, const FareUsage*>::const_iterator i = fareUsages.find(ts);

  if (UNLIKELY(i == fareUsages.end()))
    return false;

  const FareUsage* fareUsage = i->second;
  if (UNLIKELY(!fareUsage))
    return false;

  bool isConnection = isConnectionPoint(ts, fareUsage);

  const TravelSeg* lastTvlSeg = fareUsage->travelSeg().back();
  if (connectionInd == FareCalcConsts::FC_TWO)
  {
    // FCC - do not display X at fare break
    isConnection &= (isConnection && (ts != lastTvlSeg));
  }

  if (isConnection)
  {
    const TravelSeg* lastAirSeg = TravelSegUtil::lastAirSeg(fareUsage->travelSeg());

    // Do not display X indicator if the last air seg is combined
    // with the last open segment of the fare component.

    isConnection &= (isConnection && (connectionInd == FareCalcConsts::FC_ONE || ts != lastAirSeg ||
                                      lastAirSeg == lastTvlSeg));

    // Do not display X indicator if Abacus point-to-point return
    // trip travel within 24-hour
    isConnection &=
        ts->isForcedConx() || // if it is a force conx, that overrides the
        // point-to-point turnaround logic
        (isConnection && (!trx.getRequest()->ticketingAgent()->abacusUser() || !isPointToPoint()));
  }

  return isConnection;
}

bool
CalcTotals::isConnectionPoint(const TravelSeg* travelSeg, const FareUsage* fareUsage) const
{
  if (UNLIKELY(fareUsage == nullptr))
  {
    std::map<const TravelSeg*, const FareUsage*>::const_iterator i = fareUsages.find(travelSeg);

    if (i == fareUsages.end())
    {
      LOG4CXX_WARN(CalcTotals_logger, "Logic error - TS/FU");
      return false; // error - raise exception?
    }

    fareUsage = i->second;
  }

  if (UNLIKELY(travelSeg->isForcedConx()))
    return true; // came from input entry

  if (UNLIKELY(travelSeg->isForcedStopOver()))
    return false; // came from input entry

  //////////////////////////////////////////////////////////////////
  const std::vector<TravelSeg*>& tvlSeg = farePath->itin()->travelSeg();

  std::vector<TravelSeg*>::const_iterator k = std::find(tvlSeg.begin(), tvlSeg.end(), travelSeg);

  if (UNLIKELY(k == tvlSeg.end()))
  {
    LOG4CXX_WARN(CalcTotals_logger, "Logic error - ITIN/TS");
    throw ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }

  if ((*k) == tvlSeg.back())
    return false; // last segment for fare

  // if we are droping multi-city segment:
  {
    bool same_mcity = true;
    for (std::vector<TravelSeg*>::const_iterator iter = k; ++iter != tvlSeg.end() && same_mcity;)
    {
      const TravelSeg* next_ts = *iter;
      LocCode next_orig = FcDispFareUsage::getDisplayLoc(*fcConfig,
                                                         farePath->itin()->geoTravelType(),
                                                         next_ts->boardMultiCity(),
                                                         next_ts->origAirport(),
                                                         fareUsage->paxTypeFare()->carrier(),
                                                         next_ts->departureDT());

      LocCode next_dest = FcDispFareUsage::getDisplayLoc(*fcConfig,
                                                         farePath->itin()->geoTravelType(),
                                                         next_ts->offMultiCity(),
                                                         next_ts->destAirport(),
                                                         fareUsage->paxTypeFare()->carrier(),
                                                         next_ts->departureDT());

      same_mcity = (next_orig == next_dest);
      if (UNLIKELY(same_mcity && next_ts == tvlSeg.back() &&
                    (dynamic_cast<const AirSeg*>(next_ts) == nullptr)))
        return false;
    }
  }

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*k);
  const AirSeg* airSegTo = airSeg;
  for (std::vector<TravelSeg*>::const_iterator i = k; ++i != tvlSeg.end();)
  {
    airSegTo = dynamic_cast<const AirSeg*>(*i);

    if (airSegTo)
      break;
  }

  for (std::vector<TravelSeg*>::const_iterator i = k;; --i)
  {
    airSeg = dynamic_cast<const AirSeg*>(*i);

    if (airSeg)
      break;

    if (i == tvlSeg.begin())
      break;
  }

  if (UNLIKELY(!airSeg || !airSegTo))
    return false;

  //////////////////////////////////////////////////////////////////
  //
  // Work around for the PSS inconsistency between X and O override
  // when the override involve arunk segment.
  //
  if (LIKELY(std::distance(k, tvlSeg.end()) > 1))
  {
    const ArunkSeg* nextTvlSeg = dynamic_cast<const ArunkSeg*>(*(k + 1));
    if (UNLIKELY(nextTvlSeg && nextTvlSeg->isForcedStopOver()))
    {
      return false;
    }
  }
  //////////////////////////////////////////////////////////////////

  bool isStopOver = true;
  if (fallback::fixed::APO29538_StopoverMinStay())
  {
    isStopOver = airSegTo->isStopOver(
        airSeg, fareUsage->paxTypeFare()->fareMarket()->geoTravelType(), TravelSeg::OTHER);
  }
  else
  {
    if (LIKELY(fareUsage))
    {
      std::vector<TravelSeg*>::const_iterator iter =
          std::find(fareUsage->travelSeg().begin(), fareUsage->travelSeg().end(), airSegTo);

      if (iter != fareUsage->travelSeg().end())
      {
        isStopOver = airSegTo->isStopOver(airSeg,
                                          fareUsage->paxTypeFare()->fareMarket()->geoTravelType(),
                                          fareUsage->stopoverMinTime());
      }
      else
      {
        isStopOver = airSegTo->isStopOver(
            airSeg, fareUsage->paxTypeFare()->fareMarket()->geoTravelType(), TravelSeg::OTHER);
      }
    }
  }

  if (fareUsage && fareUsage->paxTypeFare() && fareUsage->paxTypeFare()->fareMarket() && isStopOver)
  {
    return false;
  }

  if (fareUsage->stopovers().find(travelSeg) != fareUsage->stopovers().end())
    return false; // stopovers rule override existed

  if (fareUsage->transfers().find(travelSeg) != fareUsage->transfers().end())
    return true; // transfers rule override existed

  return true;
}

std::string
CalcTotals::getFareBasisCode(PricingTrx& trx,
                             const TravelSeg* travelSeg,
                             char tktDesLength,
                             char childInfantCode,
                             std::string::size_type maxLenAll,
                             std::string::size_type maxLenFB) const
{
  if (UNLIKELY(dynamic_cast<const AirSeg*>(travelSeg) == nullptr))
  {
    // Arunk segment may not have specified FBC or specified
    // TktDesignator populated
    return "";
  }

  const FareUsage* fareUsage = getFareUsage(travelSeg);
  if (UNLIKELY(fareUsage == nullptr))
  {
    throw ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }

  const PaxTypeFare* paxTypeFare = fareUsage->paxTypeFare(); // lint !e530
  std::string ret;
  std::string designator;
  std::string append;

  ret = paxTypeFare->createFareBasis(&trx);

  // overwrite with specified basis and/or des
  if (UNLIKELY(!travelSeg->specifiedFbc().empty()))
  {
    std::string::size_type pos = ret.find("/");
    ret.replace(0, pos, travelSeg->specifiedFbc());
  }

  PricingRequest* request = trx.getRequest();

  if (UNLIKELY(request->isSpecifiedTktDesignatorEntry()))
    designator = request->specifiedTktDesignator(travelSeg->segmentOrder()).c_str();

  if (UNLIKELY(!designator.empty()))
  {
    std::string::size_type pos = ret.find("/");
    if (pos != std::string::npos)
    {
      ret.erase(pos);
    }
    ret += ("/" + designator);
  }

  // The specified FBC override takes precedence over
  // the industry fare basis and the restriction code logic
  if (LIKELY(travelSeg->specifiedFbc().empty()))
  {
    //   Check for industry fare and override fare basis code if needed
    if (paxTypeFare->fare()->isIndustry())
      paxTypeFare->insertBookingCode(ret);

    // the appendage code, if the restriction code is less than the highest restriction code for
    // the pricing unit, plus...
    std::map<const FareUsage*, const PricingUnit*>::const_iterator j = pricingUnits.find(fareUsage);
    if (j != pricingUnits.end())
    {
      const PricingUnit* pricingUnit = j->second;

      std::map<const PricingUnit*, Indicator>::const_iterator p = maxRestCode.find(pricingUnit);
      if (LIKELY(p != maxRestCode.end()))
      {
        if (fareUsage->penaltyRestInd() < p->second)
        {
          std::map<const PricingUnit*, AppendageCode>::const_iterator q =
              appendageCode.find(pricingUnit);
          LOG4CXX_DEBUG(CalcTotals_logger, "appendage code" << q->second);
          ret += q->second;
        }
      }
    }
  }

  if (UNLIKELY(ret.size() > maxLenAll))
  {
    ret = ret.substr(0, maxLenAll);
  }

  if (!paxTypeFare->isDiscounted())
  {
    LOG4CXX_DEBUG(CalcTotals_logger, "not discounted");
    return ret;
  }

  return ret;
}

const FareUsage*
CalcTotals::getFareUsage(const TravelSeg* travelSeg) const
{
  std::map<const TravelSeg*, const FareUsage*>::const_iterator i = fareUsages.find(travelSeg);
  if (i == fareUsages.end())
  {
    LOG4CXX_WARN(CalcTotals_logger, "Logic error");
    return nullptr;
  }

  return i->second;
}

const FareUsage*
CalcTotals::getFareUsage(const TravelSeg* travelSeg, uint16_t& travelSegIndex) const
{
  const FareUsage* fu = getFareUsage(travelSeg);
  if (fu != nullptr)
  {
    std::vector<TravelSeg*>::const_iterator iter =
        std::find(fu->travelSeg().begin(), fu->travelSeg().end(), travelSeg);

    if (iter != fu->travelSeg().end())
    {
      travelSegIndex = std::distance(fu->travelSeg().begin(), iter);
    }
  }

  return fu;
}

const FareBreakPointInfo*
CalcTotals::getFareBreakPointInfo(const FareUsage* fareUsage) const
{
  std::map<const FareUsage*, FareBreakPointInfo>::const_iterator i =
      fareBreakPointInfo.find(fareUsage);

  if (UNLIKELY(i == fareBreakPointInfo.end()))
  {
    return nullptr;
  }

  return &i->second;
}

std::string
CalcTotals::getDifferentialFbc(const TravelSeg* travelSeg) const
{
  std::string diffFbc;
  std::map<const TravelSeg*, std::string>::const_iterator i = differentialFareBasis.find(travelSeg);

  if (i != differentialFareBasis.end())
  {
    diffFbc = i->second;
  }

  return diffFbc;
}

FareBreakPointInfo&
CalcTotals::getFareBreakPointInfo(const FareUsage* fareUsage)
{
  std::map<const FareUsage*, FareBreakPointInfo>::iterator i = fareBreakPointInfo.find(fareUsage);
  if (i == fareBreakPointInfo.end())
  {
    fareBreakPointInfo[fareUsage] = FareBreakPointInfo();
    return fareBreakPointInfo[fareUsage];
  }

  return i->second;
}

// TODO: this function should be deprecated - QT.
// Determines if any stopover charges are present by checking for StopoverSurcharge
// objects in the stopoverSurcharges data member with amount > 0
bool
CalcTotals::getStopoverSummary(uint16_t& stopoverCount,
                               MoneyAmount& stopoverCharges,
                               CurrencyCode& pubCurr) const
{
  stopoverCount = stopOverSurcharge.count - stopOverSurchargeByOverride.count;
  stopoverCharges = stopOverSurcharge.total - stopOverSurchargeByOverride.total;
  if (stopOverSurcharge.pubCurrency.size() == 1)
  {
    pubCurr = *(stopOverSurcharge.pubCurrency.begin());
  }

  return stopoverCount > 0;
}

bool
CalcTotals::getTransferSummary(uint16_t& transferCount,
                               MoneyAmount& transferCharges,
                               CurrencyCode& pubCurr) const
{
  transferCount = transferSurcharge.count;
  transferCharges = transferSurcharge.total;
  if (transferSurcharge.pubCurrency.size() == 1)
  {
    pubCurr = *(transferSurcharge.pubCurrency.begin());
  }

  return transferCount > 0;
}

void
CalcTotals::getNetRemitFareUsage(const FarePath* origFp,
                                 const FareUsage* origFu,
                                 FareUsage*& netRemitFu1,
                                 FareUsage*& netRemitFu2)
{
  if (origFu->tktNetRemitFare() == nullptr)
    return;

  NetRemitFarePath* netRemitFp = const_cast<NetRemitFarePath*>(origFp->netRemitFarePath());
  if (netRemitFp != nullptr)
  {
    netRemitFp->getNetRemitFareUsage(const_cast<FareUsage*>(origFu), netRemitFu1, netRemitFu2);
  }
}

const FareUsage*
CalcTotals::getNetFareUsage(const FarePath* origFp, const FareUsage* origFu)
{
  FareUsage* netFu = nullptr;
  if (nullptr != origFp && nullptr != origFu)
  {
    NetFarePath* netFp = const_cast<NetFarePath*>(origFp->netFarePath());
    if (netFp != nullptr)
    {
      netFu = netFp->getNetFareUsage(const_cast<FareUsage*>(origFu));
    }
  }
  return netFu;
}

bool
CalcTotals::isPointToPoint() const
{
  // Check point to point return trip same day or within 24-hour travel.

  // There is two fare components (FareUsage) and two fly segments.
  if (pricingUnits.size() != 2)
  {
    return false;
  }

  const std::vector<TravelSeg*>& tvlSeg = farePath->itin()->travelSeg();

  if (tvlSeg.empty())
    return false;

  unsigned flySegCnt = 0;
  for (const auto ts : tvlSeg)
  {
    if (ts->segmentType() == Air || ts->segmentType() == Open)
      flySegCnt++;
  }
  if (flySegCnt != 2)
    return false;
  // Return to the same city (or multi-city)
  if (tvlSeg.front()->origin()->loc() != tvlSeg.back()->destination()->loc())
  {
    CarrierCode carrier;
    {
      std::map<const TravelSeg*, const FareUsage*>::const_iterator i =
          fareUsages.find(tvlSeg.front());
      if (i != fareUsages.end())
      {
        carrier = i->second->paxTypeFare()->carrier();
      }
    }

    LocCode s1_orig = FcDispFareUsage::getDisplayLoc(*fcConfig,
                                                     farePath->itin()->geoTravelType(),
                                                     tvlSeg.front()->boardMultiCity(),
                                                     tvlSeg.front()->origAirport(),
                                                     carrier,
                                                     tvlSeg.front()->departureDT());

    LocCode s2_dest = FcDispFareUsage::getDisplayLoc(*fcConfig,
                                                     farePath->itin()->geoTravelType(),
                                                     tvlSeg.back()->offMultiCity(),
                                                     tvlSeg.back()->destAirport(),
                                                     carrier,
                                                     tvlSeg.back()->departureDT());

    if (s1_orig != s2_dest)
    {
      return false;
    }
  }

  return true;
}

int
CalcTotals::getTotalMileage() const
{
  TSE_ASSERT(farePath);

  int totalMileage = 0;
  for (const PricingUnit* pu : farePath->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      totalMileage += fu->paxTypeFare()->mileage();
    }
  }
  return totalMileage;
}

MoneyAmount
CalcTotals::getTotalAmountPerPax() const
{
  MoneyAmount totalFareAmount;
  if (equivFareAmount == 0 && convertedBaseFareCurrencyCode != equivCurrencyCode)
    totalFareAmount = taxAmount();
  else if (convertedBaseFareCurrencyCode == equivCurrencyCode)
    totalFareAmount = convertedBaseFare + taxAmount();
  else
    totalFareAmount = equivFareAmount + taxAmount();

  return totalFareAmount;
}

MoneyAmount
CalcTotals::getTotalFareAmount(const CurrencyCode& currencyOverride) const
{
  MoneyAmount totalFareAmount = 0;

  if (currencyOverride.empty() || currencyOverride == convertedBaseFareCurrencyCode)
  {
    // Process Base Amount
    totalFareAmount = convertedBaseFare;
  }
  else
  {
    // Process Equivalent Amount
    if (convertedBaseFareCurrencyCode == equivCurrencyCode)
    {
      totalFareAmount = convertedBaseFare;
    }
    else
    {
      totalFareAmount = equivFareAmount;
    }
  }

  return totalFareAmount;

  if (currencyOverride.empty() || currencyOverride == convertedBaseFareCurrencyCode ||
      convertedBaseFareCurrencyCode == equivCurrencyCode)
    return convertedBaseFare;

  return equivFareAmount;
}
} // namespace tse
