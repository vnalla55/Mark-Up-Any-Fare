//----------------------------------------------------------------------------
//  Copyright Sabre 2007
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

#include "ItinAnalyzer/ExcItinUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexNewItin.h"
#include "DBAccess/Loc.h"
#include "ItinAnalyzer/CouponMatcher.h"

#define areDifferent(attr, s1, s2) ((s1)->attr() != (s2)->attr())

namespace tse
{

namespace
{
typedef std::vector<TravelSeg*>::const_iterator TravelSegConstIterator;
typedef std::vector<TravelSeg*>::iterator TravelSegIterator;

void
FindExcOpenSegSection(TravelSegConstIterator& iter,
                      TravelSegConstIterator& endI,
                      TravelSegConstIterator& excEndI)
{
  TravelSegConstIterator beginI = iter;
  for (; iter != excEndI; iter++)
  {
    if (areDifferent(segmentType, *beginI, *iter))
      break;
  }

  iter -= 1;
  endI = iter;
}

bool
FindExcSegmentsInNewItin(TravelSegConstIterator beginInExcI,
                         TravelSegConstIterator endInExcI,
                         const std::vector<TravelSeg*>& newItinV,
                         std::vector<bool>& usedNewItinSegs,
                         TravelSegConstIterator& beginInNewI,
                         TravelSegConstIterator& endInNewI)
{
  TravelSegConstIterator iter = newItinV.begin();
  TravelSegConstIterator iterE = newItinV.end();

  iter = beginInNewI;
  iterE = endInNewI;

  beginInNewI = newItinV.end();
  endInNewI = newItinV.end();

  for (; iter != iterE; ++iter)
  {
    if (usedNewItinSegs[iter - newItinV.begin()])
      continue;

    if (!(*iter)->unflown())
    {
      (*iter)->changeStatus() = TravelSeg::UNCHANGED;
      usedNewItinSegs[iter - newItinV.begin()] = true;
      continue;
    }

    if ((*beginInExcI)->boardMultiCity() == (*iter)->boardMultiCity())
      beginInNewI = iter;

    if (beginInNewI != newItinV.end() && (*endInExcI)->offMultiCity() == (*iter)->offMultiCity())
    {
      endInNewI = iter;
      return true;
    }
  }

  return false;
}

TravelSeg::ChangeStatus
DetermineChangeStatus(TravelSegConstIterator beginInExcI,
                      TravelSegConstIterator endInExcI,
                      TravelSegConstIterator beginInNewI,
                      TravelSegConstIterator endInNewI,
                      bool& isCabinChanged)
{
  isCabinChanged = false;
  TravelSeg::ChangeStatus retVal = TravelSeg::UNCHANGED;

  TravelSegConstIterator iterExcI, iterNewI;
  AirSeg* curExcAirSeg, *curNewAirSeg;

  for (iterExcI = beginInExcI; iterExcI <= endInExcI; iterExcI++)
  {
    curExcAirSeg = dynamic_cast<AirSeg*>(*iterExcI);

    for (iterNewI = beginInNewI; iterNewI <= endInNewI; iterNewI++)
    {
      bool OpenSegConfirmed = (*iterNewI)->segmentType() == Air;

      if (!OpenSegConfirmed && areDifferent(segmentType, *iterExcI, *iterNewI))
        return TravelSeg::CHANGED;

      curNewAirSeg = dynamic_cast<AirSeg*>(*iterNewI);

      if (areDifferent(carrier, curExcAirSeg, curNewAirSeg))
        return TravelSeg::CHANGED;

      if ((retVal != TravelSeg::INVENTORYCHANGED) &&
          (OpenSegConfirmed || areDifferent(departureDT().date, curExcAirSeg, curNewAirSeg)))
        retVal = TravelSeg::CONFIRMOPENSEGMENT;

      if (areDifferent(bookedCabin, curExcAirSeg, curNewAirSeg))
      {
        isCabinChanged = true;
        return TravelSeg::CHANGED;
      }

      if (areDifferent(getBookingCode, curExcAirSeg, curNewAirSeg))
        retVal = TravelSeg::INVENTORYCHANGED;
    }
  }

  return (retVal == TravelSeg::UNCHANGED &&
          ((beginInExcI != endInExcI) || (beginInNewI != endInNewI)))
             ? TravelSeg::CONFIRMOPENSEGMENT
             : retVal;
}

inline
MoneyAmount
stringToMoneyAmount(const std::string& amount)
{
  try
  {
    return boost::lexical_cast<MoneyAmount>(amount);
  }
  catch(...)
  {
    return -1.0;
  }
}

} // namespace

void
ExcItinUtil::setUnchangedStatus(TravelSeg& excTvlSeg, TravelSeg& newTvlSeg, RexNewItin* rexNewItin)
{
  excTvlSeg.changeStatus() = newTvlSeg.changeStatus() = TravelSeg::UNCHANGED;
  if (rexNewItin)
    rexNewItin->excToNewUnchangedAndInventoryChangedTravelSegmentMap().insert(
        std::make_pair(&excTvlSeg, &newTvlSeg));
}

void
ExcItinUtil::markUnchangedSegmentsOnly(ExcItin* excItin, Itin* newItin, TvlSegPairV& tvlSegPairs)
{
  TravelSegIterator excSegIt = excItin->travelSeg().begin();
  TravelSegConstIterator startExcTvlSegIt = excSegIt;
  TravelSegIterator currNewSegIt = newItin->travelSeg().begin();

  for (; excSegIt != excItin->travelSeg().end() && currNewSegIt != newItin->travelSeg().end();
       ++excSegIt)
  {
    TravelSeg* excTvlSeg = *excSegIt;

    if (!excTvlSeg->unflown())
    {
      excTvlSeg->changeStatus() = TravelSeg::UNCHANGED;
      continue;
    }

    if (excTvlSeg->segmentType() == Open)
      continue;

    TravelSegIterator newSegIt = currNewSegIt;

    for (; newSegIt != newItin->travelSeg().end(); ++newSegIt)
    {
      TravelSeg* newTvlSeg = *newSegIt;
      AirSeg* newAirSeg = dynamic_cast<AirSeg*>(*newSegIt);

      if (!newTvlSeg->unflown())
      {
        newTvlSeg->changeStatus() = TravelSeg::UNCHANGED;
        continue;
      }

      bool skipCondition = newTvlSeg->changeStatus() == TravelSeg::UNCHANGED ||
                        areDifferent(origAirport, excTvlSeg, newTvlSeg) ||
                        areDifferent(destAirport, excTvlSeg, newTvlSeg) ||
                        areDifferent(segmentType, newTvlSeg, excTvlSeg) || !newAirSeg ||
                        areDifferent(carrier, static_cast<AirSeg*>(excTvlSeg), newAirSeg) ||
                        areDifferent(departureDT().date, static_cast<AirSeg*>(excTvlSeg), newAirSeg) ||
                        areDifferent(flightNumber, static_cast<AirSeg*>(excTvlSeg), newAirSeg) ||
                        areDifferent(bookedCabin, static_cast<AirSeg*>(excTvlSeg), newAirSeg) ||
                        areDifferent(getBookingCode, static_cast<AirSeg*>(excTvlSeg), newAirSeg);

      if (skipCondition)
        continue;

      excTvlSeg->newTravelUsedToSetChangeStatus().push_back(newTvlSeg);
      newTvlSeg->newTravelUsedToSetChangeStatus().push_back(excTvlSeg);

      ExcItinUtil::setUnchangedStatus(*excTvlSeg, *newTvlSeg, dynamic_cast<RexNewItin*>(newItin));

      if (startExcTvlSegIt != excSegIt && currNewSegIt != newSegIt)
        tvlSegPairs.push_back(TvlSegPairV::value_type(TvlSegPair(startExcTvlSegIt, excSegIt),
                                                      TvlSegPair(currNewSegIt, newSegIt)));

      startExcTvlSegIt = excSegIt + 1;
      currNewSegIt = newSegIt + 1;

      break;
    }
  }
  if (startExcTvlSegIt != excItin->travelSeg().end() && currNewSegIt != newItin->travelSeg().end())
    tvlSegPairs.push_back(
        TvlSegPairV::value_type(TvlSegPair(startExcTvlSegIt, excItin->travelSeg().end()),
                                TvlSegPair(currNewSegIt, newItin->travelSeg().end())));
}

void
ExcItinUtil::calculateMaxPriceForFlownOrNotShopped(RexPricingTrx& trx, Itin& itin)
{
  RexExchangeTrx* rexExchangeTrx = static_cast<RexExchangeTrx*>(&trx);
  for (const FareMarket* newFM : itin.fareMarket())
  {
    if(newFM->isFlown() || !newFM->isShopped())
    {
      for (const FareMarket* excFM : trx.exchangeItin().front()->fareMarket())
      {
        if(excFM->origin()->loc() == newFM->origin()->loc() &&
           excFM->destination()->loc() == newFM->destination()->loc())
        {
          MoneyAmount maxPrice =
                          stringToMoneyAmount(excFM->travelSeg().back()->fareCalcFareAmt()) *
                              (static_cast<double>(rexExchangeTrx->getMaxPriceJump())/100.0);
          if(maxPrice < 0)
          {
            //logic disabled, all fare's prices are available
            maxPrice = std::numeric_limits<MoneyAmount>::max();
          }
          rexExchangeTrx->addNewFMtoMaxPrice(newFM, maxPrice);
          break;
        }
      }
    }
  }
}

void
ExcItinUtil::DetermineChanges(ExcItin* excItin, Itin* newItin)
{
  const std::vector<TravelSeg*>& excTvlSegV = excItin->travelSeg();
  const std::vector<TravelSeg*>& newTvlSegV = newItin->travelSeg();

  RexNewItin* rexNewItin = dynamic_cast<RexNewItin*>(newItin);

  // to eliminate matching few times the same new segment with exc one
  bool b = false;
  std::vector<bool> usedNewItinSegs(newTvlSegV.size(), b);

  TvlSegPairV tvlSegPairs;
  ExcItinUtil::markUnchangedSegmentsOnly(excItin, newItin, tvlSegPairs);

  if (tvlSegPairs.empty())
    tvlSegPairs.push_back(TvlSegPairV::value_type(
        TvlSegPair(excItin->travelSeg().begin(), excItin->travelSeg().end()),
        TvlSegPair(newItin->travelSeg().begin(), newItin->travelSeg().end())));

  TvlSegPairV::const_iterator tvlSegPairsIt = tvlSegPairs.begin();

  for (; tvlSegPairsIt != tvlSegPairs.end(); ++tvlSegPairsIt)
  {
    TravelSegConstIterator excI = excTvlSegV.begin();
    TravelSegConstIterator excE = excTvlSegV.end();

    excI = tvlSegPairsIt->first.first;
    excE = tvlSegPairsIt->first.second;

    for (; excI != excE; ++excI)
    {
      if (!(*excI)->unflown())
      {
        (*excI)->changeStatus() = TravelSeg::UNCHANGED;
        continue;
      }

      if ((*excI)->segmentType() == Open)
      {
        TravelSegConstIterator beginInExcI = excI;
        TravelSegConstIterator endInExcI = excI;
        TravelSegConstIterator beginInNewI;
        TravelSegConstIterator endInNewI;

        beginInNewI = tvlSegPairsIt->second.first;
        endInNewI = tvlSegPairsIt->second.second;

        if (!FindExcSegmentsInNewItin(
                beginInExcI, endInExcI, newTvlSegV, usedNewItinSegs, beginInNewI, endInNewI))
        {
          FindExcOpenSegSection(excI, endInExcI, excE);
          beginInNewI = tvlSegPairsIt->second.first;
          endInNewI = tvlSegPairsIt->second.second;

          if (!FindExcSegmentsInNewItin(
                  beginInExcI, endInExcI, newTvlSegV, usedNewItinSegs, beginInNewI, endInNewI))
          {
            continue;
          }
        }

        bool isCabinChanged = false;
        TravelSeg::ChangeStatus retVal =
            DetermineChangeStatus(beginInExcI, endInExcI, beginInNewI, endInNewI, isCabinChanged);
        TravelSegConstIterator iter;

        for (iter = beginInExcI; iter <= endInExcI; iter++)
        {
          (*iter)->isCabinChanged() = isCabinChanged;
          (*iter)->changeStatus() = retVal;

          std::copy(
              beginInNewI, endInNewI, back_inserter((*iter)->newTravelUsedToSetChangeStatus()));
        }

        for (iter = beginInNewI; iter <= endInNewI; iter++)
        {
          (*iter)->changeStatus() = retVal;
          usedNewItinSegs[iter - newTvlSegV.begin()] = true;

          std::copy(
              beginInExcI, endInExcI, back_inserter((*iter)->newTravelUsedToSetChangeStatus()));
        }

        continue;
      }

      AirSeg* excAirSeg = dynamic_cast<AirSeg*>(*excI);

      TravelSegConstIterator newI = newTvlSegV.begin();
      TravelSegConstIterator newE = newTvlSegV.end();

      newI = tvlSegPairsIt->second.first;
      newE = tvlSegPairsIt->second.second;

      for (newI = newTvlSegV.begin(); newI != newE; ++newI)
      {
        if (usedNewItinSegs[newI - newTvlSegV.begin()])
          continue;

        if (!(*newI)->unflown())
        {
          (*newI)->changeStatus() = TravelSeg::UNCHANGED;
          usedNewItinSegs[newI - newTvlSegV.begin()] = true;
          continue;
        }

        if (areDifferent(origAirport, *excI, *newI))
          continue;
        if (areDifferent(destAirport, *excI, *newI))
          continue;

        usedNewItinSegs[newI - newTvlSegV.begin()] = true;
        (*excI)->newTravelUsedToSetChangeStatus().push_back(*newI);
        (*newI)->newTravelUsedToSetChangeStatus().push_back(*excI);

        AirSeg* newAirSeg = dynamic_cast<AirSeg*>(*newI);

        if (areDifferent(segmentType, *newI, *excI))
          break;

        if (newAirSeg) //  not ArunkSeg or SurfaceSeg
        {
          if (areDifferent(carrier, excAirSeg, newAirSeg))
            break;

          if (areDifferent(departureDT().date, excAirSeg, newAirSeg))
            break;

          if (areDifferent(flightNumber, excAirSeg, newAirSeg))
            break;

          if (areDifferent(bookedCabin, excAirSeg, newAirSeg))
          {
            excAirSeg->isCabinChanged() = true;
            break;
          }

          if (areDifferent(getBookingCode, excAirSeg, newAirSeg))
          {
            excAirSeg->changeStatus() = newAirSeg->changeStatus() = TravelSeg::INVENTORYCHANGED;
            if (rexNewItin)
              rexNewItin->excToNewUnchangedAndInventoryChangedTravelSegmentMap().insert(
                  std::make_pair(excAirSeg, newAirSeg));
            break;
          }
        }

        ExcItinUtil::setUnchangedStatus(**excI, **newI, rexNewItin);
        break;
      }
    }
  }
}

void
ExcItinUtil::findStopOvers(const Itin* itin, std::vector<LocCode>& stopOvers)
{
  TravelSegConstIterator currTvlSegI = itin->travelSeg().begin();
  const TravelSegConstIterator lastTvlSegI = itin->travelSeg().end();
  TravelSegConstIterator prevTvlSegI = currTvlSegI++;

  // Check first segment for stopover
  if ((*prevTvlSegI)->stopOver())
    stopOvers.push_back((*prevTvlSegI)->offMultiCity());

  // Check rest of the segments
  for (; currTvlSegI != lastTvlSegI; currTvlSegI++)
  {
    if ((*currTvlSegI)->boardMultiCity() != (*prevTvlSegI)->offMultiCity())
      stopOvers.push_back((*currTvlSegI)->boardMultiCity());

    // Check current segment for stopover
    if ((*currTvlSegI)->stopOver())
      stopOvers.push_back((*currTvlSegI)->offMultiCity());

    // Set previous segment
    prevTvlSegI = currTvlSegI;
  }
  // Last segment is always stopover
  stopOvers.push_back(itin->travelSeg().back()->offMultiCity());
}

void
ExcItinUtil::IsStopOverChange(ExcItin* exchangeItin, Itin* newItin)
{
  std::vector<LocCode> excItinStopOvers;
  std::vector<LocCode> newItinStopOvers;

  // Find stopovers in exchange and new itinerary
  findStopOvers(exchangeItin, excItinStopOvers);
  findStopOvers(newItin, newItinStopOvers);

  // Check for differeces in stopovers and mark stopover flag in exchange itinerary
  exchangeItin->stopOverChange() = (excItinStopOvers != newItinStopOvers);
}

void
ExcItinUtil::matchCoupon(ExcItin* exchangeItin, Itin* newItin)
{
  exchangeItin->domesticCouponChange() = !CouponMatcher().match(exchangeItin->travelSeg(),
                                                                newItin->travelSeg());
}

int
ExcItinUtil::findPointOfChange(const Itin* itin)
{
  TravelSeg* pointOfChangeTravelSeg = nullptr;
  TravelSegConstIterator travelSegIter = itin->travelSeg().begin();

  for (; travelSegIter != itin->travelSeg().end(); ++travelSegIter)
  {
    if ((*travelSegIter)->changeStatus() == TravelSeg::CHANGED ||
        (*travelSegIter)->changeStatus() == TravelSeg::INVENTORYCHANGED)
    {
      pointOfChangeTravelSeg = *travelSegIter;
      break;
    }
  }
  return (pointOfChangeTravelSeg != nullptr) ? pointOfChangeTravelSeg->pnrSegment() : -1;
}

void
ExcItinUtil::SetFareMarketChangeStatus(Itin* itinFirst, const Itin* itinSecond)
{
  if (itinFirst == nullptr || itinSecond == nullptr || itinFirst->travelSeg().empty() ||
      itinSecond->travelSeg().empty())
    return;

  int pointOfChangeFirst = findPointOfChange(itinFirst);
  int pointOfChangeSecond = findPointOfChange(itinSecond);

  std::vector<FareMarket*>::iterator fmIter = itinFirst->fareMarket().begin();

  for (; fmIter != itinFirst->fareMarket().end(); ++fmIter)
  {
    (*fmIter)->setFCChangeStatus(pointOfChangeFirst, pointOfChangeSecond);
  }
}

class checkChanged
{
public:
  checkChanged(TravelSeg::ChangeStatus status) : _status(status) {}
  bool operator()(const TravelSeg* travelSeg) { return (travelSeg->changeStatus() == _status); }

protected:
  TravelSeg::ChangeStatus _status;
};

bool
ExcItinUtil::isChanged(const std::vector<TravelSeg*>& travelSegs,
                       const TravelSeg::ChangeStatus status)
{
  return (std::find_if(travelSegs.begin(), travelSegs.end(), checkChanged(status)) !=
          travelSegs.end());
}

void
ExcItinUtil::CheckSegmentsStatus(ExcItin* excItin, Itin* newItin)
{
  if (excItin == nullptr || newItin == nullptr)
    return;

  excItin->someSegmentsChanged() = isChanged(excItin->travelSeg(), TravelSeg::CHANGED) ||
                                   isChanged(newItin->travelSeg(), TravelSeg::CHANGED);

  excItin->someSegmentsConfirmed() = isChanged(excItin->travelSeg(), TravelSeg::CONFIRMOPENSEGMENT);

  excItin->sameSegmentsInventoryChanged() =
      isChanged(excItin->travelSeg(), TravelSeg::INVENTORYCHANGED);
}
}
