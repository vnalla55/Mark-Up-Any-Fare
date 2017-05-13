//----------------------------------------------------------------------------
//
//  File:           FareMarketUtil.cpp
//  Created:        8/16/2004
//  Authors:        Mike Carroll
//
//  Description:    Common functions required for ATSE shopping/pricing.
//
//  Updates:
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

#include "Common/FareMarketUtil.h"

#include "Common/ClassOfService.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/MultiTransport.h"

#include <sstream>
#include <type_traits>

namespace tse
{

//----------------------------------------------------------------------------
// getDisplayString()
//----------------------------------------------------------------------------
std::string
FareMarketUtil::getDisplayString(const FareMarket& fareMarket)
{
  const std::vector<TravelSeg*>& trvlSeg = fareMarket.travelSeg();

  if (trvlSeg.empty())
    return "";

  std::ostringstream marketString;

  std::vector<TravelSeg*>::const_iterator i = trvlSeg.begin();
  std::vector<TravelSeg*>::const_iterator j = trvlSeg.end();

  TravelSeg* tvlSeg = *i;

  if (tvlSeg != nullptr)
    marketString << getBoardMultiCity(fareMarket, *tvlSeg);

  for (; i != j; ++i)
  {
    tvlSeg = *i;

    if (tvlSeg == nullptr)
      continue;

    const ArunkSeg* arunkSeg = dynamic_cast<const ArunkSeg*>(tvlSeg);
    if (arunkSeg != nullptr)
    {
      marketString << "-";
      marketString << getOffMultiCity(fareMarket, *arunkSeg); // destination()->loc();
    }
    else
    {
      AirSeg* airSeg = dynamic_cast<AirSeg*>(tvlSeg);
      if (LIKELY(airSeg != nullptr))
      {
        marketString << "-";
        marketString << airSeg->carrier();
        marketString << "-";
        marketString << getOffMultiCity(fareMarket, *airSeg); // destination()->loc();
      }
    }
  }
  return marketString.str();
}

//----------------------------------------------------------------------------
// getFullDisplayString()
//----------------------------------------------------------------------------
std::string
FareMarketUtil::getFullDisplayString(const FareMarket& fareMarket)
{
  std::ostringstream marketString;

  marketString << FareMarketUtil::getDisplayString(fareMarket);
  marketString << "    /CXR-" << fareMarket.governingCarrier() << "/";

  std::string globalDirStr;
  globalDirectionToStr(globalDirStr, fareMarket.getGlobalDirection());

  marketString << " #GI-" << globalDirStr << "#  " << fareMarket.getDirectionAsString();

  return marketString.str();
}

//----------------------------------------------------------------------------
// setMultiCities()
//----------------------------------------------------------------------------
bool
FareMarketUtil::setMultiCities(FareMarket& fareMarket, const DateTime& tvlDate)
{
  std::vector<TravelSeg*>::iterator i = fareMarket.travelSeg().begin();
  for (; i != fareMarket.travelSeg().end(); ++i)
  {
    const TravelSeg* travelSeg = *i;
    fareMarket.boardMultiCities()[travelSeg] = getMultiCity(fareMarket.governingCarrier(),
                                                            travelSeg->origAirport(),
                                                            fareMarket.geoTravelType(),
                                                            tvlDate);
    fareMarket.offMultiCities()[travelSeg] = getMultiCity(fareMarket.governingCarrier(),
                                                          travelSeg->destAirport(),
                                                          fareMarket.geoTravelType(),
                                                          tvlDate);
  }
  return true;
}

//----------------------------------------------------------------------------
// getBoardMultiCity()
//----------------------------------------------------------------------------
const LocCode&
FareMarketUtil::getBoardMultiCity(const FareMarket& fareMarket, const TravelSeg& travelSeg)
{
  const std::map<const TravelSeg*, LocCode>& multiCities = fareMarket.boardMultiCities();
  std::map<const TravelSeg*, LocCode>::const_iterator i = multiCities.find(&travelSeg);
  if (i != multiCities.end())
  {
    return i->second;
  }
  return travelSeg.origAirport();
}

//----------------------------------------------------------------------------
// getOffMultiCity()
//----------------------------------------------------------------------------
const LocCode&
FareMarketUtil::getOffMultiCity(const FareMarket& fareMarket, const TravelSeg& travelSeg)
{
  const std::map<const TravelSeg*, LocCode>& multiCities = fareMarket.offMultiCities();
  std::map<const TravelSeg*, LocCode>::const_iterator i = multiCities.find(&travelSeg);
  if (i != multiCities.end())
  {
    return i->second;
  }
  return travelSeg.destAirport();
}

//----------------------------------------------------------------------------
// getMultiCity()
//----------------------------------------------------------------------------

namespace
{
log4cxx::LoggerPtr
_logger(log4cxx::Logger::getLogger("atseintl.Common.FareMarketUtil"));
}
LocCode
FareMarketUtil::getMultiCity(const CarrierCode& carrier,
                             const LocCode& airport,
                             GeoTravelType geoTravelType,
                             const DateTime& tvlDate)
{
  LOG4CXX_DEBUG(_logger,
                " getMultiCity airport " << airport << " carrier " << carrier << " geoTravelType "
                                         << static_cast<int>(geoTravelType));

  LocCode ret = airport;

  if (carrier.empty())
    LOG4CXX_WARN(_logger, "Application error: Governing carrier is empty");

  LocCode multiCity = LocUtil::getMultiTransportCity(airport, carrier, geoTravelType, tvlDate);

  if (LIKELY(!multiCity.empty()))
    ret = multiCity;
  else
    LOG4CXX_DEBUG(_logger, "-> not multi transport");

  return ret;
}

void
FareMarketUtil::getParticipatingCarrier(const std::vector<TravelSeg*>& travelSegs,
                                        std::set<CarrierCode>& carriers)
{
  std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
  for (; i != travelSegs.end(); ++i)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*i);
    if (airSeg)
    {
      carriers.insert(airSeg->carrier());
    }
  }
}

bool
FareMarketUtil::isParticipatingCarrier(const std::set<CarrierCode>& participatingCarriers,
                                       const CarrierCode& carrier)
{
  return (std::find(participatingCarriers.begin(), participatingCarriers.end(), carrier) !=
          participatingCarriers.end());
}

bool
FareMarketUtil::isYYOverride(PricingTrx& trx, const std::vector<TravelSeg*>& travelSegs)
{
  PricingRequest* request = trx.getRequest();

  if (UNLIKELY(request && request->isIndustryFareOverrideEntry())) // C-YY with segment select
  {
    std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
    for (; i != travelSegs.end(); ++i)
    {
      if (request->industryFareOverride((*i)->segmentOrder()))
      {
        return true;
      }
    }
  }

  return false;
}

void
FareMarketUtil::getGovCxrOverride(PricingTrx& trx,
                                  const std::vector<TravelSeg*>& travelSegs,
                                  std::vector<CarrierCode>& govCxrOverride)
{
  PricingRequest* request = trx.getRequest();

  if (UNLIKELY(request && request->isGoverningCarrierOverrideEntry())) // C-xx with segment select
  {
    std::vector<TravelSeg*>::const_iterator i = travelSegs.begin();
    for (; i != travelSegs.end(); ++i)
    {
      CarrierCode govCxr = request->governingCarrierOverride((*i)->segmentOrder());
      if (!govCxr.empty() &&
          (std::find(govCxrOverride.begin(), govCxrOverride.end(), govCxr) == govCxrOverride.end()))
      {
        govCxrOverride.push_back(govCxr); // Order is important
      }
    }
  }
}

size_t
FareMarketUtil::numFlts(const Itin& itin, const FareMarket& fmkt, const size_t startIndex)
{
  const size_t numSegs = fmkt.travelSeg().size();
  if (startIndex >= numSegs)
    return 0;
  std::vector<TravelSeg*>::const_iterator tvlSegIter = fmkt.travelSeg().begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegEnd = fmkt.travelSeg().end();
  size_t i = 0;
  size_t fltsCovered = 0;
  AirSeg* currSeg = nullptr;
  AirSeg* nextSeg = nullptr;

  for (; tvlSegIter != tvlSegEnd; tvlSegIter++, i++)
  {
    if (i < startIndex)
      continue;
    currSeg = dynamic_cast<AirSeg*>(*tvlSegIter);
    if (i == startIndex)
      fltsCovered = 1;
    if (currSeg == nullptr)
    {
      if (i != startIndex)
        ++fltsCovered;
      if (fltsCovered == 3)
        break;
      else
        continue;
    }
    if (i + 1 < numSegs)
    {
      nextSeg = dynamic_cast<AirSeg*>(fmkt.travelSeg()[i + 1]);
      if (nextSeg != nullptr)
      {
        if (nextSeg->carrier() != currSeg->carrier())
          break;
        if (!ItinUtil::journeyConnection(itin, nextSeg, currSeg, fltsCovered + 1))
          break;
        ++fltsCovered;
      }
    }
    if (fltsCovered == 3)
      break;
  }
  return fltsCovered;
}

//----------------------------------------------------------------------------
// group3
//----------------------------------------------------------------------------
void
FareMarketUtil::group3(Itin& itin,
                       FareMarket& fm,
                       size_t startIndex,
                       size_t fltsCovered,
                       PricingTrx& trx,
                       std::vector<FareMarket*>& processedFM,
                       bool processCOS)
{
  fm.availBreaks()[startIndex + fltsCovered - 1] = true;
  AirSeg* airSeg = nullptr;
  size_t i = 0;
  std::vector<TravelSeg*>::iterator tvlSegIter = fm.travelSeg().begin();
  const std::vector<TravelSeg*>::iterator tvlSegEnd = fm.travelSeg().end();
  std::vector<TravelSeg*>::iterator groupStartTvlSeg;
  std::vector<TravelSeg*>::iterator nextGroupStartTvlSeg;

  // this loop will setp up the starting tvlSegIter
  // for this group of flights
  for (; tvlSegIter != tvlSegEnd; tvlSegIter++)
  {
    if (i == startIndex)
      break;
    i++;
  }
  groupStartTvlSeg = tvlSegIter;
  nextGroupStartTvlSeg = groupStartTvlSeg;

  size_t numArunk = 0; // if no ARUNK in this froup then numArunk == 0
  // if first flight in ARUNK then numArunk == 1 .. so on
  // max value possible = 3

  // now see which flight is an ARUNK
  i = 0;
  for (; tvlSegIter != tvlSegEnd && i < fltsCovered; tvlSegIter++, i++)
  {
    airSeg = dynamic_cast<AirSeg*>(*tvlSegIter);
    if (airSeg == nullptr)
    {
      numArunk = i + 1;
      break;
    }
  }

  switch (fltsCovered)
  {
  case 3:
  {
    switch (numArunk)
    {
    case 0:
    {
      AirSeg* airSegTemp1 = dynamic_cast<AirSeg*>(fm.travelSeg()[startIndex]);
      AirSeg* airSegTemp2 = dynamic_cast<AirSeg*>(fm.travelSeg()[startIndex + 1]);
      AirSeg* airSegTemp3 = dynamic_cast<AirSeg*>(fm.travelSeg()[startIndex + 2]);
      // bool all3CxrSame = false;
      bool all3CxrDiff = false;
      bool first2Same = false;
      bool last2Same = false;
      if (airSegTemp1->carrier() == airSegTemp2->carrier())
      {
        first2Same = true;
        std::vector<TravelSeg*> journeySegs;
        journeySegs.push_back(fm.travelSeg()[startIndex + 1]);
        journeySegs.push_back(fm.travelSeg()[startIndex + 2]);
        FareMarket* journeyFm = ItinUtil::findMarket(itin, journeySegs);
        if (journeyFm && journeyFm->flowMarket())
        {
          first2Same = false;
          last2Same = true;
        }
      }
      else
      {
        if (airSegTemp2->carrier() == airSegTemp3->carrier())
          last2Same = true;
        else
          all3CxrDiff = true;
      }
      if (all3CxrDiff)
      {
        buildMarket(itin, fm, groupStartTvlSeg, trx, startIndex, 3, processedFM, processCOS);
      }
      else
      {
        if (first2Same)
        {
          buildMarket(itin, fm, groupStartTvlSeg, trx, startIndex, 2, processedFM, processCOS);
          fm.availBreaks()[startIndex + 1] = true;
          // now do the 3rd flight
          nextGroupStartTvlSeg = nextGroupStartTvlSeg + 2;
          // groupStartTvlSeg = groupStartTvlSeg + 2;
          buildMarket(
              itin, fm, nextGroupStartTvlSeg, trx, startIndex + 2, 1, processedFM, processCOS);
        }
        else if (last2Same)
        {
          buildMarket(itin, fm, groupStartTvlSeg, trx, startIndex, 1, processedFM, processCOS);
          fm.availBreaks()[startIndex] = true;
          // now do the 2nd and 3rd flight
          nextGroupStartTvlSeg = nextGroupStartTvlSeg + 1;
          // groupStartTvlSeg = groupStartTvlSeg + 1;
          buildMarket(
              itin, fm, nextGroupStartTvlSeg, trx, startIndex + 1, 2, processedFM, processCOS);
        }
      }
      break;
    }
    case 1:
    {
      pushBackArunkCOS(trx, fm, processCOS);

      fm.availBreaks()[startIndex] = true;

      // now do the rest 2 flights
      ++groupStartTvlSeg;
      buildMarket(itin, fm, groupStartTvlSeg, trx, startIndex + 1, 2, processedFM, processCOS);
      break;
    }
    case 2:
    {
      // 2nd flight in this froup is ARUNK
      buildMarket(itin, fm, groupStartTvlSeg, trx, startIndex, 1, processedFM, processCOS);

      pushBackArunkCOS(trx, fm, processCOS);

      fm.availBreaks()[startIndex + 1] = true;
      // now do the 3rd flight
      groupStartTvlSeg = groupStartTvlSeg + 2;
      buildMarket(itin, fm, groupStartTvlSeg, trx, startIndex + 2, 1, processedFM, processCOS);
      break;
    }
    case 3:
    {
      // 3rd flight in this group is ARUNK

      // process the first 2 flights first
      buildMarket(itin, fm, groupStartTvlSeg, trx, startIndex, 2, processedFM, processCOS);

      pushBackArunkCOS(trx, fm, processCOS);

      fm.availBreaks()[startIndex + 2] = true;
      break;
    }
    default:
      break;
    } // switch(numArunk)
    break;
  } // case3 : for the switch(fltsCovered)
  case 2:
  {
    // total number of flights in this group == 2
    switch (numArunk)
    {
    case 0:
    {
      // no flight in this group is an ARUNK
      buildMarket(itin, fm, groupStartTvlSeg, trx, startIndex, 2, processedFM, processCOS);
      break;
    }
    case 1:
    {
      pushBackArunkCOS(trx, fm, processCOS);

      fm.availBreaks()[startIndex] = true;

      // now do the rest 2 flights
      ++groupStartTvlSeg;
      buildMarket(itin, fm, groupStartTvlSeg, trx, startIndex + 1, 1, processedFM, processCOS);
      break;
    }
    case 2:
    {
      // 2nd flight in this froup is ARUNK
      buildMarket(itin, fm, groupStartTvlSeg, trx, startIndex, 1, processedFM, processCOS);

      pushBackArunkCOS(trx, fm, processCOS);

      fm.availBreaks()[startIndex + 1] = true;
      break;
    }
    default:
      break;
    } // switch(numArunk)
    break;
  } // case2 : for the switch(fltsCovered)
  case 1:
  {
    // total number of flights in this group == 1
    if (numArunk == 0)
    {
      buildMarket(itin, fm, groupStartTvlSeg, trx, startIndex, 1, processedFM, processCOS);
    }
    else
    {
      pushBackArunkCOS(trx, fm, processCOS);
      fm.availBreaks()[startIndex] = true;
    }
    break;
  }
  } // switch(fltsCovered)
}

void
FareMarketUtil::pushBackArunkCOS(PricingTrx& trx, FareMarket& fm, bool processCOS)
{
  if (processCOS)
  {
    std::vector<ClassOfService*>* cosVec = nullptr;
    trx.dataHandle().get(cosVec);
    fm.classOfServiceVec().push_back(cosVec);
  }
}

//----------------------------------------------------------------------------
// buildMarket
//----------------------------------------------------------------------------
void
FareMarketUtil::buildMarket(Itin& itin,
                            FareMarket& fm,
                            std::vector<TravelSeg*>::iterator& groupStartTvlSeg,
                            PricingTrx& trx,
                            size_t startIndex,
                            size_t fltsCovered,
                            std::vector<FareMarket*>& processedFM,
                            bool processCOS)
{
  std::vector<TravelSeg*> tvlSegs;

  std::vector<TravelSeg*>::iterator tvlI = groupStartTvlSeg;
  const std::vector<TravelSeg*>::iterator tvlE = fm.travelSeg().end();
  for (size_t iTvlSeg = 0; (iTvlSeg < fltsCovered) && (tvlI != tvlE); tvlI++, iTvlSeg++)
  {
    tvlSegs.push_back((*tvlI));
  }

  FareMarket* srcFm = ItinUtil::findMarket(itin, tvlSegs);
  if (srcFm != nullptr)
  {
    if (processCOS)
    {
      if (srcFm->classOfServiceVec().empty())
        srcFm = nullptr;
    }
    else
    {
      if (fmWithoutCOS(*srcFm, trx, srcFm, processedFM))
        srcFm = nullptr;
    }
  }

  if (tvlSegs.empty())
    return;

  if (srcFm == nullptr)
  {
    // if we could not find this market, build it from locals
    buildUsingLocal(fm, trx, fltsCovered, tvlSegs[0], processCOS);
    return;
  }

  size_t numVec;
  if (processCOS)
    numVec = srcFm->classOfServiceVec().size();
  else
    numVec = srcFm->availBreaks().size();

  for (size_t iVec = 0; iVec < numVec; iVec++, startIndex++)
  {
    fm.availBreaks()[startIndex] = srcFm->availBreaks()[iVec];

    if (LIKELY(!processCOS))
      continue;

    std::vector<ClassOfService*>* srcCosVec = srcFm->classOfServiceVec()[iVec];
    std::vector<ClassOfService*>* cosVec = nullptr;
    trx.dataHandle().get(cosVec);
    fm.classOfServiceVec().push_back(cosVec);

    const size_t numCos = srcCosVec->size();
    for (size_t iCos = 0; iCos < numCos; iCos++)
    {
      ClassOfService& srcCos = *((*srcCosVec)[iCos]);
      ClassOfService* cs = nullptr;
      trx.dataHandle().get(cs);
      if (cs != nullptr)
      {
        cs->bookingCode() = srcCos.bookingCode();
        cs->numSeats() = srcCos.numSeats();
        cs->cabin() = srcCos.cabin();
        cosVec->push_back(cs);
      }
    }
  }
}

void
FareMarketUtil::buildUsingLocal(FareMarket& fm,
                                PricingTrx& trx,
                                size_t fltsCovered,
                                const TravelSeg* startTvlSeg,
                                bool processCOS)
{
  std::vector<TravelSeg*>::iterator tvlSegEnd = fm.travelSeg().end();

  std::vector<TravelSeg*>::iterator tvlSegIter =
      find(fm.travelSeg().begin(), tvlSegEnd, startTvlSeg);

  if (tvlSegIter == tvlSegEnd)
    return;

  for (size_t i = 0; (tvlSegIter != tvlSegEnd) && (i < fltsCovered); tvlSegIter++, i++)
  {
    fm.availBreaks()[i] = true;

    if (!processCOS)
      continue;

    AirSeg* const airSeg = dynamic_cast<AirSeg*>(*tvlSegIter);

    std::vector<ClassOfService*>* cosVec = nullptr;
    trx.dataHandle().get(cosVec);
    fm.classOfServiceVec().push_back(cosVec);

    if (airSeg == nullptr)
      continue;

    const size_t numCos = airSeg->classOfService().size();
    for (size_t iCos = 0; iCos < numCos; iCos++)
    {
      ClassOfService& fltCos = *(airSeg->classOfService()[iCos]);

      // get a new pointer
      ClassOfService* cs = nullptr;
      trx.dataHandle().get(cs);
      if (cs != nullptr)
      {
        cs->bookingCode() = fltCos.bookingCode();
        cs->numSeats() = fltCos.numSeats();
        cs->cabin() = fltCos.cabin();
        cosVec->push_back(cs);
      }
    }
  }
}

bool
FareMarketUtil::fmWithoutCOS(FareMarket& fm,
                             PricingTrx& trx,
                             FareMarket* srcFm,
                             std::vector<FareMarket*>& processedFM)
{
  if (srcFm && std::find(processedFM.begin(), processedFM.end(), srcFm) != processedFM.end())
    return false; // fm already processed

  AvailabilityMap::iterator availIt =
      trx.availabilityMap().find(ShoppingUtil::buildAvlKey(fm.travelSeg()));

  if ((fm.travelSeg().size() && !fm.travelSeg()[0]->unflown()) ||
      availIt == trx.availabilityMap().end())
    return true;

  return false;
}

std::vector<Itin*>
FareMarketUtil::collectOwningItins(const FareMarket& fm, const std::vector<Itin*>& collection)
{
  std::vector<Itin*> result;
  for (Itin* itin : collection)
  {
    if (itin &&
        std::find(itin->fareMarket().begin(), itin->fareMarket().end(), &fm) !=
            itin->fareMarket().end())
    {
      result.push_back(itin);
    }
  }
  return result;
}

inline static bool
findClassOfService(const std::vector<ClassOfService*>& cos, BookingCode bkc, uint16_t numSeats)
{
  return std::any_of(cos.begin(),
                     cos.end(),
                     [bkc, numSeats](const ClassOfService* cs)
                     { return cs->isAvailable(bkc, numSeats); });
}

bool
FareMarketUtil::checkAvailability(const FareMarket& fm, BookingCode bkc, uint16_t numSeats)
{
  if (UNLIKELY(fm.travelSeg().size() > fm.classOfServiceVec().size()))
    return false;

  for (const std::vector<ClassOfService*>* cosVec : fm.classOfServiceVec())
  {
    if (UNLIKELY(!cosVec))
      return false;

    if (!findClassOfService(*cosVec, bkc, numSeats))
      return false;
  }

  return true;
}

} // tse
