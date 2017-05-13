//----------------------------------------------------------------------------
//  File:        DominatedFlights.cpp
//  Created:     2009-04-27
//
//  Description: Class used to find and remove dominated flights
//
//  Updates:
//
//  Copyright Sabre 2009
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

#include "Pricing/DominatedFlights.h"

namespace tse
{

namespace
{
inline bool
isDominatedFlight(ShoppingTrx::SchedulingOption* sop)
{
  return sop->itin()->dominatedFlight();
}
}

DominatedFlights::DominatedFlights(const ShoppingTrx& trx) : _trx(trx)
{
  bool oneWay = (_trx.legs().size() == 1);

  const int noOfOutbounds = oneWay ? _trx.visOptions()->valueBasedItinSelection().noOfOutboundsOW()
                                   : _trx.visOptions()->valueBasedItinSelection().noOfOutboundsRT();

  const int noOfInbounds = _trx.visOptions()->valueBasedItinSelection().noOfInboundsRT();

  _minOutFlightsCount = 2 * noOfOutbounds;

  _minInFlightsCount = 2 * noOfInbounds;
}

void
DominatedFlights::findDominatedFlights(
    std::vector<ShoppingTrx::SchedulingOption*>& outboundFlightsVec,
    std::vector<ShoppingTrx::SchedulingOption*>& inboundFlightsVec)
{
  std::sort(outboundFlightsVec.begin(), outboundFlightsVec.end(), ElapsedTimeSopComparator());
  std::sort(inboundFlightsVec.begin(), inboundFlightsVec.end(), ElapsedTimeSopComparator());

  prepareFlightsInfo(outboundFlightsVec);
  prepareFlightsInfo(inboundFlightsVec, true);

  markDominatedFlights(outboundFlightsVec);
  markDominatedFlights(inboundFlightsVec, true);
}

void
DominatedFlights::removeDominatedFlights(
    std::vector<ShoppingTrx::SchedulingOption*>& outboundFlightsVec,
    std::vector<ShoppingTrx::SchedulingOption*>& inboundFlightsVec)
{
  removeDominatedFlights(outboundFlightsVec);
  removeDominatedFlights(inboundFlightsVec);
}

void
DominatedFlights::removeDominatedFlights(std::vector<ShoppingTrx::SchedulingOption*>& flightsVec)
{
  flightsVec.erase(std::remove_if(flightsVec.begin(), flightsVec.end(), isDominatedFlight),
                   flightsVec.end());
}

void
DominatedFlights::prepareFlightsInfo(const std::vector<ShoppingTrx::SchedulingOption*>& flightsVec,
                                     bool isInbound)
{
  std::vector<ShoppingTrx::SchedulingOption*>::const_iterator sopIter;

  for (sopIter = flightsVec.begin(); sopIter != flightsVec.end(); ++sopIter)
  {
    DominatedFlightsInfo dfInfo;

    const ShoppingTrx::SchedulingOption* sop = (*sopIter);

    // Get cheapest ESV values for this flight
    ShoppingUtil::getCheapestESVValues(_trx, sop->itin(), dfInfo.esvValues);

    // Get carriers sequence for given flight
    CarrierCode lastCarrier = "";

    for (const auto ts : sop->itin()->travelSeg())
    {
      AirSeg* airSeg = ts->toAirSeg();

      if (("" == lastCarrier) || (airSeg->carrier() != lastCarrier))
      {
        lastCarrier = airSeg->carrier();
        dfInfo.carriersSequence += airSeg->carrier();
      }
    }

    // Calculate elapsed time
    dfInfo.elapsedTime = sop->itin()->getFlightTimeMinutes();

    // Add dominated flight info to vector
    if (isInbound)
    {
      _inboundInfo.push_back(dfInfo);
    }
    else
    {
      _outboundInfo.push_back(dfInfo);
    }
  }
}

void
DominatedFlights::markDominatedFlights(std::vector<ShoppingTrx::SchedulingOption*>& flightsVec,
                                       bool isInbound)
{
  std::vector<DominatedFlightsInfo>* dfInfoVec;
  int32_t maxDominatedFlightsCount = 0;

  if (flightsVec.size() < 2)
  {
    return;
  }

  if (isInbound)
  {
    dfInfoVec = &_inboundInfo;

    if (flightsVec.size() > _minInFlightsCount)
    {
      maxDominatedFlightsCount = flightsVec.size() - _minInFlightsCount;
    }
    else
    {
      // Do not mark any flight as dominated
      return;
    }
  }
  else
  {
    dfInfoVec = &_outboundInfo;

    if (flightsVec.size() > _minOutFlightsCount)
    {
      maxDominatedFlightsCount = flightsVec.size() - _minOutFlightsCount;
    }
    else
    {
      // Do not mark any flight as dominated
      return;
    }
  }

  for (int i = flightsVec.size() - 2; i >= 0; --i)
  {
    ShoppingTrx::SchedulingOption* sopFirst = flightsVec[i];
    DominatedFlightsInfo& dfInfoFirst = dfInfoVec->at(i);

    // If current flight is already dominated skip processing it
    if (sopFirst->itin()->dominatedFlight())
    {
      continue;
    }

    for (int j = flightsVec.size() - 1; j > i; --j)
    {
      ShoppingTrx::SchedulingOption* sopSecond = flightsVec[j];
      DominatedFlightsInfo& dfInfoSecond = dfInfoVec->at(j);

      // Maximum number of dominated flights was already found
      if (maxDominatedFlightsCount <= 0)
      {
        return;
      }

      // If current flight is already dominated skip processing it
      if (sopSecond->itin()->dominatedFlight())
      {
        continue;
      }

      // Check if we've got same carrier sequence on both flights
      if (dfInfoFirst.carriersSequence != dfInfoSecond.carriersSequence)
      {
        continue;
      }

      // Check if we've got same departure or arrival date
      if ((!checkSameDepartureDate(sopFirst, sopSecond)) &&
          (!checkSameArrivalDate(sopFirst, sopSecond)))
      {
        continue;
      }

      // Check elapsed time
      bool firstElapsedTimeShorter = (dfInfoFirst.elapsedTime < dfInfoSecond.elapsedTime);
      bool secondElapsedTimeShorter = (dfInfoFirst.elapsedTime > dfInfoSecond.elapsedTime);

      if (firstElapsedTimeShorter)
      {
        // Check if first flight is not more expensive than second
        if (isNotMoreExpensive(dfInfoFirst.esvValues, dfInfoSecond.esvValues))
        {
          sopSecond->itin()->dominatedFlight() = true;
          --maxDominatedFlightsCount;
        }
      }

      if (secondElapsedTimeShorter)
      {
        // Check if second flight is not more expensive than first
        if (isNotMoreExpensive(dfInfoSecond.esvValues, dfInfoFirst.esvValues))
        {
          sopFirst->itin()->dominatedFlight() = true;
          --maxDominatedFlightsCount;
          break;
        }
      }
    }
  }
}

bool
DominatedFlights::checkSameDepartureDate(const ShoppingTrx::SchedulingOption* sopFirst,
                                         const ShoppingTrx::SchedulingOption* sopSecond)
{
  if (sopFirst->itin()->travelSeg()[0]->departureDT() ==
      sopSecond->itin()->travelSeg()[0]->departureDT())
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool
DominatedFlights::checkSameArrivalDate(const ShoppingTrx::SchedulingOption* sopFirst,
                                       const ShoppingTrx::SchedulingOption* sopSecond)
{
  uint32_t firstSize = sopFirst->itin()->travelSeg().size();
  uint32_t secondSize = sopSecond->itin()->travelSeg().size();

  if (sopFirst->itin()->travelSeg()[firstSize - 1]->arrivalDT() ==
      sopSecond->itin()->travelSeg()[secondSize - 1]->arrivalDT())
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool
DominatedFlights::isNotMoreExpensive(const std::vector<MoneyAmount>& esvValuesFirst,
                                     const std::vector<MoneyAmount>& esvValuesSecond)
{
  for (uint32_t i = 0; i < 4; ++i)
  {
    if (esvValuesFirst[i] != -1.0)
    {
      if ((esvValuesSecond[i] != -1.0) && (esvValuesFirst[i] > esvValuesSecond[i]))
      {
        return false;
      }
    }
    else
    {
      if (esvValuesSecond[i] != -1.0)
      {
        return false;
      }
    }
  }

  return true;
}

} // tse
