//-------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#include "Common/ShoppingAltDateUtil.h"

#include "Common/AirlineShoppingUtils.h"
#include "Common/Assert.h"
#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/TravelSegUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "Rules/RuleConst.h"

#include <boost/date_time/gregorian/gregorian.hpp>

#include <algorithm>

namespace tse
{

bool
ShoppingAltDateUtil::validateAltDatePair(const ShoppingTrx& trx, DatePair datePair)
{
  return validateDuration(trx, datePair);
}

bool
ShoppingAltDateUtil::validateDuration(const ShoppingTrx& trx, DatePair datePair)
{
  const long minStay = trx.minDuration();
  const long maxStay = trx.maxDuration();

  if (minStay == -1 || maxStay == -1)
    return true;

  const boost::gregorian::date arrDate = datePair.first.date();
  const boost::gregorian::date depDate = datePair.second.date();
  const boost::gregorian::date_duration durDate = depDate - arrDate;
  const long lengthOfStay = durDate.days();

  return (lengthOfStay >= minStay && lengthOfStay <= maxStay);
}

DatePair
ShoppingAltDateUtil::getTravelDateRange(const ShoppingTrx& trx)
{
  DateTime minDate(DateTime::emptyDate());
  DateTime maxDate(DateTime::emptyDate());
  TravelSeg* airSegPtr = trx.legs()[0].sop()[0].itin()->travelSeg().front();
  TSE_ASSERT(airSegPtr);

  const DateTime& depDT = airSegPtr->departureDT();
  minDate = depDT;
  maxDate = depDT;

  for (unsigned int i = 1; i != trx.legs()[0].sop().size(); ++i)
  {
    airSegPtr = trx.legs()[0].sop()[i].itin()->travelSeg().front();
    TSE_ASSERT(airSegPtr);

    const DateTime& depDT = airSegPtr->departureDT();

    if (minDate > depDT)
      minDate = depDT;

    if (maxDate < depDT)
      maxDate = depDT;
  }

  DatePair datePair(minDate, maxDate);
  return datePair;
}

DateTime
ShoppingAltDateUtil::getDateSop(const ShoppingTrx::SchedulingOption& sop)
{
  return dateOnly(sop.itin()->firstTravelSeg()->departureDT());
}

DateTime
ShoppingAltDateUtil::getDateSop(const ShoppingTrx& trx, int leg, int sop)
{
  TSE_ASSERT(leg < int(trx.legs().size()));
  TSE_ASSERT(sop < int(trx.legs()[leg].sop().size()));
  const DateTime& dep = trx.legs()[leg].sop()[sop].itin()->travelSeg().front()->departureDT();
  return dateOnly(dep);
}

DatePair
ShoppingAltDateUtil::getDatePairSops(const ShoppingTrx& trx, const SopIdVec& sops)
{
  TSE_ASSERT(sops.size() <= trx.legs().size());
  DatePair res;

  for (size_t leg = 0; leg != sops.size() && leg != 2; ++leg)
  {
    const Itin& i = *trx.legs()[leg].sop()[sops[leg]].itin();
    const DateTime& dep = i.travelSeg().front()->departureDT();

    if (leg == 0)
    {
      res.first = dep.date();
    }
    else
    {
      res.second = dep.date();
    }
  }

  return res;
}

DatePair
ShoppingAltDateUtil::getDatePairSops(const ShoppingTrx::SchedulingOption* sop1,
                                     const ShoppingTrx::SchedulingOption* sop2)
{
  DatePair res;

  TSE_ASSERT(sop1 != nullptr);
  res.first = sop1->itin()->travelSeg().front()->departureDT().date();
  if (sop2)
    res.second = sop2->itin()->travelSeg().front()->departureDT().date();

  return res;
}

bool
ShoppingAltDateUtil::getDatePair(const PricingTrx::AltDatePairs& altDatePairs,
                                 const DateTime& date,
                                 uint8_t leg,
                                 DatePair& datePair)
{
  for (const auto& altDatePair : altDatePairs)
  {
    datePair = altDatePair.first;

    if (leg == 0)
    {
      if (datePair.first.get64BitRepDateOnly() == date.get64BitRepDateOnly())
        return true;
    }
    else
    {
      if (datePair.second.get64BitRepDateOnly() == date.get64BitRepDateOnly())
        return true;
    }
  }

  return false;
}

Itin*
ShoppingAltDateUtil::getJourneyItin(const PricingTrx::AltDatePairs& altDatePairs,
                                    const DatePair& datePair)
{
  return (altDatePairs.find(datePair))->second->journeyItin;
}

bool
ShoppingAltDateUtil::checkEffExpDate(const ShoppingTrx& trx,
                                     const PaxTypeFare& ptFare,
                                     uint8_t& ret)
{
  DatePair myPair = ShoppingAltDateUtil::getTravelDateRange(trx);

  if (!checkEffDisc(myPair.first, ptFare.fare()->fareInfo()->effInterval(), ret))
  {
    return false;
  }

  if (!checkEffDisc(myPair.second, ptFare.fare()->fareInfo()->effInterval(), ret))
  {
    return false;
  }

  return true;
}

bool
ShoppingAltDateUtil::checkEffDisc(const DateTime& travelDate,
                                  const TSEDateInterval& range,
                                  uint8_t& ret)
{
  return checkDateRange(travelDate, range.effDate(), range.discDate(), ret);
}

bool
ShoppingAltDateUtil::checkDateRange(const DateTime& travelDate,
                                    const DateTime& effDate,
                                    const DateTime& discDate,
                                    uint8_t& ret)
{
  if (travelDate.isValid())
  {
    if (effDate.isValid() && effDate > travelDate)
    {
      ret = RuleConst::EFFECTIVE_DATE_FAIL;
      return false;
    }

    if (discDate.isValid() && discDate < travelDate)
    {
      ret = RuleConst::EXPIRED_DATE_FAIL;
      return false;
    }
  }

  return true;
}

void
ShoppingAltDateUtil::setAltDateFltBit(PaxTypeFare* ptFare, const int bitIndex)
{
  if (ptFare->isFlightInvalid(bitIndex))
  {
    if (*(ptFare->getFlightBit(bitIndex)) == RuleConst::BOOKINGCODE_FAIL)
    {
      ptFare->setAltDateFltBitGLBProcessed(bitIndex);
      ptFare->setAltDateFltBitRTGProcessed(bitIndex);
      ptFare->setAltDateFltBitBKCProcessed(bitIndex);
      ptFare->setAltDateFltBitBKCFailed(bitIndex);
    }
    else if (*(ptFare->getFlightBit(bitIndex)) == RuleConst::ROUTING_FAIL)
    {
      ptFare->setAltDateFltBitGLBProcessed(bitIndex);
      ptFare->setAltDateFltBitRTGProcessed(bitIndex);
      ptFare->setAltDateFltBitRTGFailed(bitIndex);
    }
    else if (*(ptFare->getFlightBit(bitIndex)) == RuleConst::GLOBALDIR_FAIL)
    {
      ptFare->setAltDateFltBitGLBProcessed(bitIndex);
      ptFare->setAltDateFltBitGLBFailed(bitIndex);
    }
    else
    {
      ptFare->setAltDateFltBitGLBProcessed(bitIndex);
    }
  }
  else
  {
    ptFare->setAltDateFltBitGLBProcessed(bitIndex);
    ptFare->setAltDateFltBitBKCProcessed(bitIndex);
    ptFare->setAltDateFltBitRTGProcessed(bitIndex);
  }
}

void
ShoppingAltDateUtil::cloneAltDates(const ShoppingTrx& trx,
                                   PricingTrx::AltDatePairs& altDatePairsCopy,
                                   std::deque<Itin>& journeyItins)
{
  altDatePairsCopy.clear();
  journeyItins.clear();
  const PricingTrx::AltDatePairs& altDatePairs = trx.altDatePairs();

  for (const auto& altDatePair : altDatePairs)
  {
    journeyItins.push_back(*(altDatePair.second)->journeyItin);
    PricingTrx::AltDateInfo*& info = altDatePairsCopy[altDatePair.first];
    info = trx.dataHandle().create<PricingTrx::AltDateInfo>();
    info->journeyItin = &journeyItins.back();
  }

  return;
}

void
ShoppingAltDateUtil::setJrnItinFM(FareMarket& fm, AirSeg* seg)
{
  fm.travelSeg().push_back(seg);
  fm.origin() = seg->origin();
  fm.destination() = seg->destination();
  fm.boardMultiCity() = seg->boardMultiCity();
  fm.offMultiCity() = seg->offMultiCity();
}

void
ShoppingAltDateUtil::generateJourneySegAndFM(DataHandle& dataHandle,
                                             Itin& journeyItin,
                                             DateTime departureDT,
                                             LocCode boardCity,
                                             LocCode offCity,
                                             CarrierCode cxrCode,
                                             int16_t pnrSegment)
{
  AirSeg* segment(nullptr);
  dataHandle.get(segment);
  TravelSegUtil::setupItinerarySegment(
      dataHandle, segment, departureDT, boardCity, offCity, cxrCode, pnrSegment);
  journeyItin.travelSeg().push_back(segment);
  // Setup FM
  FareMarket* journeyFM(nullptr);
  dataHandle.get(journeyFM);
  ShoppingAltDateUtil::setJrnItinFM(*journeyFM, segment);
  journeyItin.fareMarket().push_back(journeyFM);
}

DatePair
ShoppingAltDateUtil::getMIPTravelDateRange(const PricingTrx& trx)
{
  PricingTrx::AltDatePairs::const_iterator it = trx.altDatePairs().begin();
  PricingTrx::AltDatePairs::const_iterator itEnd = trx.altDatePairs().end();
  DateTime minDate((it->first).first);
  DateTime maxDate((it->first).first);

  for (; it != itEnd; ++it)
  {
    const DateTime& travelDate = (it->first).first;

    if (minDate > travelDate)
      minDate = travelDate;

    if (maxDate < travelDate)
      maxDate = travelDate;
  }

  return DatePair(minDate, maxDate);
}

uint64_t
ShoppingAltDateUtil::getDuration(const ShoppingTrx& trx, const SopIdVec& sops)
{
  DatePair datePair = getDatePairSops(trx, sops);
  uint64_t duration = getDuration(datePair);
  TSE_ASSERT(trx.durationAltDatePairs().find(duration) != trx.durationAltDatePairs().end());

  return duration;
}

uint64_t
ShoppingAltDateUtil::getNoOfDays(const uint64_t duration)
{
  return ((duration / 1000000) / HOURS_PER_DAY) / SECONDS_PER_HOUR;
}

uint64_t
ShoppingAltDateUtil::getDuration(const DatePair& datePair)
{
  return datePair.second.get64BitRepDateOnly() - datePair.first.get64BitRepDateOnly();
}

bool
ShoppingAltDateUtil::cutOffAltDate(PricingTrx& trx,
                                   const FarePath* farePath,
                                   const uint16_t altDateItinPriceJumpFactor,
                                   const uint16_t altDateCutOffNucThreshold)
{
  if (!trx.isAltDates())
  {
    // Not Alt-Date Trx
    return false;
  }

  if (trx.altDatePairs().size() <= 1)
  {
    // Single Date Alt-Date Trx
    return false;
  }

  if (trx.getRequest()->owPricingRTTaxProcess() ||
      AirlineShoppingUtils::enableItinLevelThreading(trx))
  {
    // turn off cutoff for ASE multithreading
    return false;
  }

  if (trx.altDateCutOffNucThreshold() <= 0)
  {
    // This is the cheapest Itin
    ShoppingAltDateUtil::setAltDateCutOffNucThreshold(
        trx, farePath, altDateItinPriceJumpFactor, altDateCutOffNucThreshold);
    return false;
  }

  if (farePath->getNUCAmountScore() > trx.altDateCutOffNucThreshold())
  {
    // no need to continue too expensive Alt-Date
    trx.setCutOffReached();
    return true;
  }

  return false;
}

void
ShoppingAltDateUtil::setAltDateCutOffNucThreshold(PricingTrx& trx,
                                                  const FarePath* farePath,
                                                  const uint16_t altDateItinPriceJumpFactor,
                                                  const uint16_t altDateCutOffNucThreshold)
{
  // PriceJumpFactor below is only used by MIP Alternate Dates Request

  if (trx.getOptions()->altDateMIPCutOffRequest() == 1)
  {
    trx.getOptions()->altDateMIPCutOffRequest() = 0;
  }

  if (trx.getOptions()->altDateMIPCutOffRequest() < 0)
  {
    trx.getOptions()->altDateMIPCutOffRequest() = altDateItinPriceJumpFactor;
  }

  if (altDateCutOffNucThreshold > farePath->getNUCAmountScore())
  {

    trx.altDateCutOffNucThreshold() =
        altDateCutOffNucThreshold * trx.getOptions()->altDateMIPCutOffRequest();
  }
  else
  {
    trx.altDateCutOffNucThreshold() =
        farePath->getNUCAmountScore() * trx.getOptions()->altDateMIPCutOffRequest();
  }
}

//----------------------------------------------------------------------------
// set fail category number for each alt date in the date pair vector
//----------------------------------------------------------------------------
void
ShoppingAltDateUtil::setAltDateStatus(PaxTypeFare* paxTypeFare,
                                      const DatePair datePair,
                                      const uint32_t legId)
{
  if (!paxTypeFare->isCategoryValid(RuleConst::DAY_TIME_RULE))
  {
    paxTypeFare->setAltDateStatus(datePair, RuleConst::CAT2_FAIL);
    paxTypeFare->setCategoryValid(RuleConst::DAY_TIME_RULE);
    ShoppingAltDateUtil::applyStatustoSimilarDatePair(paxTypeFare, datePair, RuleConst::DAY_TIME_RULE, legId);
  }
  else if (!paxTypeFare->isCategoryValid(RuleConst::SEASONAL_RULE))
  {
    paxTypeFare->setAltDateStatus(datePair, RuleConst::CAT3_FAIL);
    paxTypeFare->setCategoryValid(RuleConst::SEASONAL_RULE);
    ShoppingAltDateUtil::applyStatustoSimilarDatePair(paxTypeFare, datePair, RuleConst::SEASONAL_RULE, legId);
  }
  else if (!paxTypeFare->isCategoryValid(RuleConst::ADVANCE_RESERVATION_RULE))
  {
    paxTypeFare->setAltDateStatus(datePair, RuleConst::CAT5_FAIL);
    paxTypeFare->setCategoryValid(RuleConst::ADVANCE_RESERVATION_RULE);
    ShoppingAltDateUtil::applyStatustoSimilarDatePair(paxTypeFare, datePair, RuleConst::ADVANCE_RESERVATION_RULE, legId);
  }
  else if (!paxTypeFare->isCategoryValid(RuleConst::MINIMUM_STAY_RULE))
  {
    paxTypeFare->setAltDateStatus(datePair, RuleConst::CAT6_FAIL);
    paxTypeFare->setCategoryValid(RuleConst::MINIMUM_STAY_RULE);
    ShoppingAltDateUtil::applyStatustoSimilarDatePair(paxTypeFare, datePair, RuleConst::MINIMUM_STAY_RULE, legId);
  }
  else if (!paxTypeFare->isCategoryValid(RuleConst::MAXIMUM_STAY_RULE))
  {
    paxTypeFare->setAltDateStatus(datePair, RuleConst::CAT7_FAIL);
    paxTypeFare->setCategoryValid(RuleConst::MAXIMUM_STAY_RULE);
  }
  else if (!paxTypeFare->isCategoryValid(RuleConst::BLACKOUTS_RULE))
  {
    paxTypeFare->setAltDateStatus(datePair, RuleConst::CAT11_FAIL);
    paxTypeFare->setCategoryValid(RuleConst::BLACKOUTS_RULE);
  }
  else if (!paxTypeFare->isCategoryValid(RuleConst::TRAVEL_RESTRICTIONS_RULE))
  {
    paxTypeFare->setAltDateStatus(datePair, RuleConst::CAT14_FAIL);
    paxTypeFare->setCategoryValid(RuleConst::TRAVEL_RESTRICTIONS_RULE);
  }
  else if (!paxTypeFare->isCategoryValid(RuleConst::SALE_RESTRICTIONS_RULE))
  {
    paxTypeFare->setAltDateStatus(datePair, RuleConst::CAT15_FAIL);
    paxTypeFare->setCategoryValid(RuleConst::SALE_RESTRICTIONS_RULE);
  }
}

//-------------------------------------------------------------------
// set failed status for other date pair if it's applied
// cat2 - apply if it's the same travel date and DOW
//-------------------------------------------------------------------
void
ShoppingAltDateUtil::applyStatustoSimilarDatePair(PaxTypeFare* paxTypeFare,
                                                  const DatePair datePair,
                                                  const unsigned int category,
                                                  const uint32_t legId)
{
  uint32_t travelDOW(0);
  if (legId == 0)
    travelDOW = datePair.first.date().day_of_week().as_number();

  else
    travelDOW = datePair.second.date().day_of_week().as_number();

  int64_t duration = ShoppingAltDateUtil::getDuration(datePair);
  VecMap<DatePair, uint8_t>::reverse_iterator iter = paxTypeFare->altDateStatus().rbegin();
  // go to the rest of date pair, try to find the similar date pair to apply the same failed result
  bool startSearch = false;

  for (; iter != paxTypeFare->altDateStatus().rend(); iter++)
  {
    if (iter->first == datePair)
    {
      startSearch = true;
      continue;
    }

    if (!startSearch)
      continue;

    if (category == RuleConst::DAY_TIME_RULE)
    {
      // if same travel date  or same day of week then reuse result
      if ((legId == 0 && (iter->first.first == datePair.first ||
                          travelDOW == iter->first.first.date().day_of_week().as_number())) ||
          (legId == 1 && (iter->first.second == datePair.second ||
                          travelDOW == iter->first.second.date().day_of_week().as_number())))
      {
        paxTypeFare->setAltDateStatus(iter->first, RuleConst::CAT2_FAIL);
      }
    }
    else if (category == RuleConst::SEASONAL_RULE)
    {
      // Season use fare component base if assumptionoverride == x or no geo 995.
      if (legId == 1)
        continue;

      // outbound date = outbound processed date
      if (iter->first.first == datePair.first)
        paxTypeFare->setAltDateStatus(iter->first, RuleConst::CAT3_FAIL);
    }
    else if (category == RuleConst::MINIMUM_STAY_RULE)
    {
      // outbound date <= outbound processed date and inbound date <= processed date pair and
      // duration <= prodees datepair duration then reuse result
      if (iter->first.first <= datePair.first && iter->first.second <= datePair.second &&
          (iter->first.second.get64BitRepDateOnly() - iter->first.first.get64BitRepDateOnly()) <=
              duration)
      {
        paxTypeFare->setAltDateStatus(iter->first, RuleConst::CAT6_FAIL);
      }
    }
    else if (LIKELY(category == RuleConst::ADVANCE_RESERVATION_RULE))
    {
      // outbound date <= outbound processed date
      if (LIKELY(iter->first.first <= datePair.first))
        paxTypeFare->setAltDateStatus(iter->first, RuleConst::CAT5_FAIL);
    }
  }
}

//----------------------------------------------------------------------------
// prepare paxtype fare validation for each date pair
//----------------------------------------------------------------------------
void
ShoppingAltDateUtil::cleanUpCategoryProcess(PaxTypeFare* paxTypeFare)
{
  //#######################################################
  //#Setting this flag effectively disables any call      #
  //#to PaxTypeFare::isValid( )                           #
  //# This flag will be reset after bitmap validation     #
  //#######################################################
  paxTypeFare->setIsShoppingFare();

  setCategoriesUnprocessed(paxTypeFare);

  if (paxTypeFare->isFareByRule())
  {
    PaxTypeFare* baseFare = paxTypeFare->getBaseFare(RuleConst::FARE_BY_RULE);
    if (baseFare)
      setCategoriesUnprocessed(baseFare);
  }
}

void
ShoppingAltDateUtil::setCategoriesUnprocessed(PaxTypeFare* paxTypeFare)
{
  paxTypeFare->setCategoryProcessed(RuleConst::DAY_TIME_RULE, false);
  paxTypeFare->setCategoryProcessed(RuleConst::SEASONAL_RULE, false);
  paxTypeFare->setCategoryProcessed(RuleConst::ADVANCE_RESERVATION_RULE, false);
  paxTypeFare->setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, false);
  paxTypeFare->setCategoryProcessed(RuleConst::MAXIMUM_STAY_RULE, false);
  paxTypeFare->setCategoryProcessed(RuleConst::BLACKOUTS_RULE, false);
  paxTypeFare->setCategoryProcessed(RuleConst::TRAVEL_RESTRICTIONS_RULE, false);
  paxTypeFare->setCategoryProcessed(RuleConst::SALE_RESTRICTIONS_RULE, false);
}

}
