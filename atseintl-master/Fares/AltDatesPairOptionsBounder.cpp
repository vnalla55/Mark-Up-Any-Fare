//-------------------------------------------------------------------
//
//  File:        AltDatesPairOptionsBounder.cpp
//  Created:     Mar 05, 2008
//  Authors:     Miroslaw Bartyna
//
//  Description: uses PQ for sort and reduce number of alt dates options
//
//
//  Copyright Sabre 2008
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

#include "Fares/AltDatesPairOptionsBounder.h"

#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{

// data has to be prioritize by one value so:
// outbound distance from requested date will be in 1000 - biggest priority
// favorization of "day after" from "day before" for outbound is on 100
// inbound distance from requested date will be in 10
// favorization of "day after" from "day before" for inbound is on 1
//
// in that case outbound distance (max. 7 or -7) will be multiple by 1000
// if two dates will have the same distance (ie. 2 and abs(-2) - for that first, will be 2*1000-100)
//                                                              for second, abs(-2)*1000
// for inbound it's the same but with another weight (for ie. 3, it will be 3*10-1)
// any weight wont cross with other till distance is up to 7 days
uint64_t
AltDatesPairOptionsBounder::countValue(const std::pair<DateTime, DateTime>& originalPair,
                                       const std::pair<DateTime, DateTime>& nextPair)
{
  uint64_t value = 0;

  if (originalPair.second.isEmptyDate() || nextPair.second.isEmptyDate())
  {
    value = countValueWithWeight(originalPair.first, nextPair.first, 1000);
  }
  else
  {
    value = countValueWithWeight(originalPair.first, nextPair.first, 1000) +
            countValueWithWeight(originalPair.second, nextPair.second, 10);
  }

  return value;
}

uint64_t
AltDatesPairOptionsBounder::countValueWithWeight(const DateTime& originalDate,
                                                 const DateTime& nextData,
                                                 const uint16_t weight)
{
  uint64_t value = 0;

  int16_t tmpValue = ((originalDate.date() - nextData.date()).days()) * weight;
  if (tmpValue >= 0)
  {
    value = tmpValue;
  }
  else
  {
    // favorization of "day after" from "day before" for outbound is on 100
    // favorization of "day after" from "day before" for inbound is on 1
    value = abs(tmpValue) - weight / 10;
  }

  return value;
}

void
AltDatesPairOptionsBounder::buildPQ(
    const std::pair<DateTime, DateTime>& originalPair,
    const std::map<DatePair, FlightFinderTrx::FlightBitInfo>& combinedAltDateStatus,
    AltDatePairsPQ& pairsPQ)
{
  std::map<DatePair, FlightFinderTrx::FlightBitInfo>::const_iterator elemIt =
      combinedAltDateStatus.begin();
  for (; elemIt != combinedAltDateStatus.end(); ++elemIt)
  {
    if (elemIt->second.flightBitStatus == 0)
    {
      PQElem newElem;
      newElem.second = *elemIt;
      newElem.first = countValue(originalPair, elemIt->first);
      pairsPQ.push(newElem);
    }
  }
}

void
AltDatesPairOptionsBounder::buildReturnMap(
    const uint8_t desiredNumberOfOutboundDates,
    const uint8_t desiredNumberOfInboundDates,
    std::vector<std::pair<DatePair, FlightFinderTrx::FlightBitInfo> >& boundedCombinedAltDateStatus,
    AltDatePairsPQ& pairsPQ)
{
  uint8_t outboundDateCounter = 0;
  uint8_t inboundDateCounter = 0;
  DateTime prevOutboundDate(DateTime::emptyDate());

  while (!pairsPQ.empty())
  {
    if (outboundDateCounter < desiredNumberOfOutboundDates + 1)
    {
      if (pairsPQ.top().second.first.first.date() == prevOutboundDate.date())
      {
        if (inboundDateCounter < desiredNumberOfInboundDates)
        {
          ++inboundDateCounter;
          boundedCombinedAltDateStatus.push_back(pairsPQ.top().second);
        }
        pairsPQ.pop();
      }
      else
      {
        inboundDateCounter = 0;
        prevOutboundDate = pairsPQ.top().second.first.first;
        ++outboundDateCounter;
      }
    }
    else
    {
      break;
    }
  }
}

void
AltDatesPairOptionsBounder::pickUpDesiredNumberOfPairs(
    uint8_t desiredNumberOfOutboundDates,
    uint8_t desiredNumberOfInboundDates,
    const std::pair<DateTime, DateTime>& originalPair,
    const std::map<DatePair, FlightFinderTrx::FlightBitInfo>& combinedAltDateStatus,
    std::vector<std::pair<DatePair, FlightFinderTrx::FlightBitInfo> >& boundedCombinedAltDateStatus,
    PricingTrx& trx)
{
  AltDatePairsPQ pairsPQ;

  buildPQ(originalPair, combinedAltDateStatus, pairsPQ);

  if (trx.diagnostic().diagnosticType() == Diagnostic969 &&
      trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALTDATESPQ")
  {
    showDiag969AltDatesPQ(originalPair, pairsPQ, trx);
  }
  else
  {
    buildReturnMap(desiredNumberOfOutboundDates,
                   desiredNumberOfInboundDates,
                   boundedCombinedAltDateStatus,
                   pairsPQ);
  }
}

void
AltDatesPairOptionsBounder::showDiag969AltDatesPQ(const std::pair<DateTime, DateTime>& originalPair,
                                                  AltDatePairsPQ& pairsPQ,
                                                  PricingTrx& trx)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);
  DiagCollector& diag = *diagPtr;
  bool isOneWay = originalPair.second.isEmptyDate();

  diag.enable(Diagnostic969);

  diag << "***************************************************" << std::endl;
  diag << "Diagnostic 969 : ALTDATES PQ BEGIN" << std::endl;
  diag << "***************************************************" << std::endl;

  if (originalPair.second.isEmptyDate())
  {
    diag << "Requesed outbound " << originalPair.first.date() << std::endl << std::endl;
  }
  else
  {
    diag << "Requesed outbound " << originalPair.first.date() << " Requesed inbound "
         << originalPair.second.date() << std::endl << std::endl;
  }

  if (pairsPQ.empty())
  {
    diag << "PQ is empty" << std::endl;
  }
  else
  {
    while (!pairsPQ.empty())
    {
      if (isOneWay)
      {
        diag << "OUTBOUND - " << pairsPQ.top().second.first.first.date() << " VALUE - "
             << pairsPQ.top().first << "\n";
      }
      else
      {
        diag << "OUTBOUND - " << pairsPQ.top().second.first.first.date() << " INBOUND - "
             << pairsPQ.top().second.first.second.date() << " VALUE - " << pairsPQ.top().first
             << "\n";
      }
      pairsPQ.pop();
    }
  }

  diag << "***************************************************" << std::endl;
  diag << "Diagnostic 969 : ALTDATES PQ END" << std::endl;
  diag << "***************************************************" << std::endl;
  diag << std::endl;

  diag.flushMsg();
  diag.disable(Diagnostic969);
}

} // tse
