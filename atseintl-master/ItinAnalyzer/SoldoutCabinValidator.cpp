///-------------------------------------------------------------------------------
//  Code extracted directly from ItinAnalyzerService.cpp
//
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "ItinAnalyzer/SoldoutCabinValidator.h"

#include "Common/ClassOfService.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Cabin.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag996Collector.h"

namespace tse
{

namespace iadetail
{

namespace // anonymous
{

//-------------------------------------------------------------------------------
bool
allBrandsEmpty(const PricingTrx& trx, Diag996Collector* diag)
{
  uint16_t brandCount = trx.getRequest()->getBrandedFareSize();
  bool allBrandsEmpty = true;
  for (size_t brandIndex = 0; brandIndex < brandCount; ++brandIndex)
  {
    if (!trx.getRequest()->brandedFareBookingCode(brandIndex).empty())
    {
      allBrandsEmpty = false;
      break;
    }
  }

  if (allBrandsEmpty && diag)
    diag->printAllBrandsEmptyMsg();

  return allBrandsEmpty;
}

//-------------------------------------------------------------------------------
bool
isBookingCodeAvailable(const PricingTrx& trx,
                       const int totalNumSeats,
                       const TravelSeg* travelSeg,
                       const BookingCode& bkg)
{
  PricingTrx::ClassOfServiceKey cosKey;
  cosKey.push_back(const_cast<TravelSeg*>(travelSeg));

  const AvailabilityMap::const_iterator availIt =
      trx.availabilityMap().find(ShoppingUtil::buildAvlKey(cosKey));

  if (availIt == trx.availabilityMap().end())
  {
    return false;
  }

  const std::vector<std::vector<ClassOfService*> >* cosList = availIt->second;

  const ClassOfService* cos = ShoppingUtil::getCOS(cosList->front(), bkg);

  if (cos && cos->numSeats() >= totalNumSeats)
    return true;

  return false;
}

//-------------------------------------------------------------------------------
bool
isBrandBookingCodeAvailable(const PricingTrx& trx,
                            const int totalNumSeats,
                            const TravelSeg* travelSeg,
                            const std::vector<BookingCode>& bkgVec)
{
  for (const BookingCode& bkg : bkgVec)
  {
    if (isBookingCodeAvailable(trx, totalNumSeats, travelSeg, bkg))
      return true;
  }
  return false;
}

//-------------------------------------------------------------------------------
bool
isBrandBookingCodeAvailableForThroughAvl(const PricingTrx& trx,
                                         const int totalNumSeats,
                                         const Itin* itin,
                                         const std::vector<BookingCode>& bkgVec)
{
  PricingTrx::ClassOfServiceKey cosKey;

  // Build the key using all related travelSeg
  for (TravelSeg* travelSeg : itin->travelSeg())
  {
    const AirSeg* airSeg = static_cast<const AirSeg*>(travelSeg);
    if (!airSeg->isFake())
    {
      cosKey.push_back(travelSeg);
    }
  }

  const AvailabilityMap::const_iterator availIt =
      trx.availabilityMap().find(ShoppingUtil::buildAvlKey(cosKey));
  if (availIt == trx.availabilityMap().end())
  {
    return false;
  }

  const std::vector<std::vector<ClassOfService*> >* cosList = availIt->second;
  std::vector<std::vector<ClassOfService*> >::const_iterator classOfServiceIter = cosList->begin();
  const std::vector<std::vector<ClassOfService*> >::const_iterator classOfServiceIterEnd =
      cosList->end();
  bool validThruPath = true;

  for (; (classOfServiceIter != classOfServiceIterEnd) && validThruPath; classOfServiceIter++)
  {
    std::vector<BookingCode>::const_iterator it = bkgVec.begin();
    std::vector<BookingCode>::const_iterator itEnd = bkgVec.end();
    bool validTravelSeg = false;
    for (; (it != itEnd) && !validTravelSeg; ++it)
    {
      const ClassOfService* cos = ShoppingUtil::getCOS(*classOfServiceIter, *it);
      if (cos && cos->numSeats() >= totalNumSeats)
        validTravelSeg = true;
    }

    validThruPath &= validTravelSeg;
  }

  return validThruPath;
}

//-------------------------------------------------------------------------------
bool
isBrandBookingCodeAvailable(const PricingTrx& trx,
                            Diag996Collector* diag996,
                            const int totalNumSeats,
                            const Itin* itin,
                            const std::vector<BookingCode>& bkgVec)
{
  bool validItin = true;
  std::vector<TravelSeg*>::const_iterator segIter = itin->travelSeg().begin();
  const std::vector<TravelSeg*>::const_iterator segIterEnd = itin->travelSeg().end();
  for (; (segIter != segIterEnd) && validItin; ++segIter)
  {
    const TravelSeg* travelSeg = *segIter;
    if (diag996)
    {
      const AirSeg* aSegPtr = dynamic_cast<const AirSeg*>(*segIter);
      if (aSegPtr)
        diag996->printSeg(aSegPtr);
    }
    validItin &= isBrandBookingCodeAvailable(trx, totalNumSeats, travelSeg, bkgVec);
  }

  if (false == validItin)
  {
    // Check if the brand is available via through AVL
    validItin = isBrandBookingCodeAvailableForThroughAvl(trx, totalNumSeats, itin, bkgVec);
  }

  return validItin;
}

//-------------------------------------------------------------------------------
// For caching already processed flights per date - calendar shopping
//-------------------------------------------------------------------------------
void
setProcessedItinsMapStatus(AirSeg* airSeg,
                           std::map<std::pair<DateTime, FlightNumber>, bool>& statusMap,
                           bool status)
{
  DateTime depDate = airSeg->departureDT();
  DateTime depDateOnlyDay(depDate.date(), boost::posix_time::time_duration(0, 0, 0));
  statusMap[std::make_pair(depDateOnlyDay, airSeg->flightNumber())] = status;
}

const Cabin* getCabin(PricingTrx& trx,
                      const AirSeg& air,
                      const ClassOfService& cos)
{
  if(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx))
  {
    RBDByCabinUtil rbdCabin(trx, SOLD_OUT);
    return rbdCabin.getCabinByRBD(
         air.carrier(), cos.bookingCode(), air);
  }
  else
  {
    return trx.dataHandle().getCabin(
          air.carrier(), cos.bookingCode(), air.departureDT());
  }
  return nullptr;
}
//-------------------------------------------------------------------------------
// Checks available cabin for itinerary - calendar shopping
//-------------------------------------------------------------------------------
bool
checkAvailableCabin(PricingTrx& trx,
                    AirSeg* curAirSeg,
                    TravelSeg* travelSeg,
                    CabinType& preferredCabinClass)
{
  PricingTrx::ClassOfServiceKey cosKey;
  cosKey.push_back(travelSeg);

  AvailabilityMap::iterator availIt =
      trx.availabilityMap().find(ShoppingUtil::buildAvlKey(cosKey));

  bool cabinAvailable = false;

  if (availIt == trx.availabilityMap().end())
  {
    return false;
  }
  std::vector<std::vector<ClassOfService*> >* cosList = availIt->second;
  std::vector<ClassOfService*>& classOfService = cosList->front();

  std::vector<ClassOfService*>::iterator cosIter = classOfService.begin();
  std::vector<ClassOfService*>::iterator cosEndIter = classOfService.end();

  ClassOfService* cos = nullptr;

  uint16_t totalSeat = PaxTypeUtil::totalNumSeats(trx);
  for (; cosIter != cosEndIter; ++cosIter)
  {
    cos = *cosIter;

    if (!cos)
    {
      continue;
    }
    if (cos->cabin().isUndefinedClass())
    {
      const Cabin* aCabin = getCabin(trx, *curAirSeg, *cos);
      if (aCabin != nullptr)
      {
        cos->cabin() = aCabin->cabin();
      }
    }
    if (cos->cabin() == preferredCabinClass && (cos->numSeats() >= totalSeat))
    {
      cabinAvailable = true;
      break;
    }
  }

  return cabinAvailable;
}

//-------------------------------------------------------------------------------
// Checks available cabin for itinerary - calendar shopping
//-------------------------------------------------------------------------------
bool checkIsCabinAvailable(PricingTrx& trx,
                           CabinType& preferredCabinClass,
                           Itin& curItin,
                           std::map<std::pair<DateTime, FlightNumber>, bool>& statusMap)
{
  std::map<std::pair<DateTime, FlightNumber>, bool>::const_iterator processedIt;
  bool cabinAvailable = false;
  bool allSegWithSeats = true;

  std::vector<TravelSeg*>::const_iterator trvSegIter = curItin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator trvSegEndIter = curItin.travelSeg().end();

  if (trx.getRequest()->originBasedRTPricing())
  {
    if (trx.outboundDepartureDate() != DateTime::emptyDate())
    {
      // don't check fake outbount (first segment)
      ++trvSegIter;
    }
    else
    {
      // don't check fake inbound (last segment)
      --trvSegEndIter;
    }
  }

  for (; trvSegIter != trvSegEndIter; ++trvSegIter)
  {
    AirSeg* curAirSeg = dynamic_cast<AirSeg*>(*trvSegIter);

    if (curAirSeg != nullptr)
    {
      DateTime depDate = curAirSeg->departureDT();
      DateTime depDateOnlyDay(depDate.date(), boost::posix_time::time_duration(0, 0, 0));

      processedIt = statusMap.find(std::make_pair(depDateOnlyDay, curAirSeg->flightNumber()));

      if (processedIt != statusMap.end() && processedIt->second)
      {
        continue;
      }
      else if (processedIt != statusMap.end() && !processedIt->second)
      {
        allSegWithSeats = false;
        continue;
      }

      cabinAvailable =
          checkAvailableCabin(trx,
                              curAirSeg,
                              *trvSegIter,
                              (trx.awardRequest() && trx.isAltDates()) ? curAirSeg->bookedCabin()
                                                                       : preferredCabinClass);
      if (cabinAvailable == false)
      {
        allSegWithSeats = false;
      }

      setProcessedItinsMapStatus(curAirSeg, statusMap, cabinAvailable);
    } // end if (curAirSeg != NULL)
  } // for trvlSegs

  if (allSegWithSeats)
  {
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------
// Checks available seats, in case of not available seats stop processing - calendar shopping
//-------------------------------------------------------------------------------
bool
stopProcessingDueToLackOfSeats(PricingTrx& trx)
{
  if (trx.getRequest()->brandedFareEntry())
  {
    if (trx.validCalendarBrandIdMap().empty())
    {
      return false;
    }
    else
    {
      uint16_t brandCount = trx.getRequest()->getBrandedFareSize();
      for (int i = 0; i < brandCount; ++i)
      {
        size_t count = trx.validCalendarBrandIdMap().count(trx.getRequest()->brandId(i));
        if (count < trx.altDatePairs().size())
        {
          return false;
        }
      }
    }
    return true;
  }
  for (const auto& elem : trx.altDatePairs())
  {
    if (elem.second->goodItinForDatePairFound)
    {
      return false;
    }
  }
  return true;
}

//-------------------------------------------------------------------------------
Diag996Collector*
createDiag996(const PricingTrx& trx)
{
  if (trx.diagnostic().diagnosticType() != Diagnostic996)
    return nullptr;

  DCFactory* factory = DCFactory::instance();
  Diag996Collector* diag996 =
      dynamic_cast<Diag996Collector*>(factory->create(const_cast<PricingTrx&>(trx)));
  if (diag996)
    diag996->enable(Diagnostic996);

  return diag996;
}

//-------------------------------------------------------------------------------
void
collectValidBrandId(const PricingTrx& trx)
{
  Diag996Collector* diag996 = createDiag996(trx);

  if (allBrandsEmpty(trx, diag996))
    return;

  uint16_t brandCount = trx.getRequest()->getBrandedFareSize();
  const int totalNumSeats = PaxTypeUtil::totalNumSeats(trx);
  for (int brandIndex = 0; brandIndex < brandCount; ++brandIndex)
  {
    bool validAtLeastOneItin = false;
    std::vector<BookingCode> bkgVec;
    trx.getRequest()->brandedFareAllBookingCode(brandIndex, bkgVec);
    std::vector<Itin*>::const_iterator itinIt = trx.itin().begin();
    const std::vector<Itin*>::const_iterator itinItEnd = trx.itin().end();
    if (diag996)
      diag996->printBrandedFareBookingCodes(trx.getRequest()->brandId(brandIndex), bkgVec);

    if (bkgVec.empty())
    {
      validAtLeastOneItin = true;
    }
    for (uint16_t id = 1; (itinIt != itinItEnd) && (!validAtLeastOneItin); ++itinIt, ++id)
    {
      const Itin* itin = *itinIt;
      if (itin->legID().front().first == 0)
      {
        if (trx.getRequest()->processingDirection() == ProcessingDirection::ROUNDTRIP_INBOUND)
        {
          if (diag996)
            diag996->printItinDirectionIsReverse(id);

          continue;
        }
      }
      if (itin->legID().front().first == 1)
      {
        if (trx.getRequest()->processingDirection() == ProcessingDirection::ROUNDTRIP_OUTBOUND)
        {
          if (diag996)
            diag996->printItinDirectionIsReverse(id);

          continue;
        }
      }

      if (trx.diagnostic().diagnosticType() == Diagnostic996)
        diag996->printItinId(id);

      validAtLeastOneItin |= isBrandBookingCodeAvailable(trx, diag996, totalNumSeats, itin, bkgVec);

      if (diag996)
        diag996->printPassFailMsg(validAtLeastOneItin);
    }

    const_cast<std::vector<bool>&>(trx.validBrandIdVec()).push_back(validAtLeastOneItin);

    if (diag996)
      diag996->printSoldOutBrandId(!validAtLeastOneItin, trx.getRequest()->brandId(brandIndex));
  }

  if (diag996)
  {
    diag996->flushMsg();
  }
}

//-------------------------------------------------------------------------------
void
collectValidBrandIdCalendar(PricingTrx& trx)
{
  Diag996Collector* diag996 = createDiag996(trx);

  TSELatencyData metrics(trx, "ITIN VALIDATE CALENDAR FLCABIN");

  std::map<std::pair<DateTime, FlightNumber>, bool> processedItinsMap;

  PricingTrx::AltDatePairs::iterator altDateIt = trx.altDatePairs().begin();
  PricingTrx::AltDatePairs::iterator altDateItEnd = trx.altDatePairs().end();

  const bool originBasedRTPricing = trx.getRequest()->originBasedRTPricing();
  const bool reqAwardAltDatesRT =
      (trx.awardRequest() && trx.isAltDates() && !trx.altDatePairs().empty() &&
       !trx.altDatePairs().begin()->first.second.isEmptyDate());

  if (allBrandsEmpty(trx, diag996))
    return;

  uint16_t brandCount = trx.getRequest()->getBrandedFareSize();

  const int totalNumSeats = PaxTypeUtil::totalNumSeats(trx);
  for (; altDateIt != altDateItEnd; ++altDateIt)
  {
    DateTime depAltDate((((*altDateIt).first).first).date(),
                        boost::posix_time::time_duration(0, 0, 0));
    DateTime depInboundAltDate((((*altDateIt).first).second).date(),
                               boost::posix_time::time_duration(0, 0, 0));
    if (diag996)
      diag996->printDateTime(depAltDate);

    for (int brandIndex = 0; brandIndex < brandCount; ++brandIndex)
    {
      bool isValidItin = false;
      bool isRealItin = false;

      std::vector<BookingCode> bkgVec;
      trx.getRequest()->brandedFareAllBookingCode(brandIndex, bkgVec);

      std::vector<Itin*>::const_iterator itinIt = trx.itin().begin();
      std::vector<Itin*>::const_iterator itinItEnd = trx.itin().end();
      if (diag996)
        diag996->printBrandedFareBookingCodes(trx.getRequest()->brandId(brandIndex), bkgVec);

      if (bkgVec.empty())
      {
        continue;
      }

      for (uint16_t id = 1; (itinIt != itinItEnd) && !isValidItin; ++itinIt, ++id)
      {
        Itin* itin = *itinIt;
        if (itin->legID().front().first == 0)
        {
          if (trx.getRequest()->processingDirection() == ProcessingDirection::ROUNDTRIP_INBOUND)
          {
            if (diag996)
              diag996->printItinDirectionIsReverse(id);
            continue;
          }
        }
        if (itin->legID().front().first == 1)
        {
          if (trx.getRequest()->processingDirection() == ProcessingDirection::ROUNDTRIP_OUTBOUND)
          {
            if (diag996)
              diag996->printItinDirectionIsReverse(id);
            continue;
          }
        }
        if (diag996)
          diag996->flushMsg();

        const DateTime depDate(itin->travelSeg().front()->departureDT().date(),
                               boost::posix_time::time_duration(0, 0, 0));
        const DateTime depInboundDate(itin->datePair()->second,
                                      boost::posix_time::time_duration(0, 0, 0));

        const bool depDateEq = (depDate == depAltDate);
        const bool depInDateEq = (depInboundDate == depInboundAltDate);

        if (depDateEq)
        {
          if ((!originBasedRTPricing && !reqAwardAltDatesRT) ||
              ((originBasedRTPricing || reqAwardAltDatesRT) && depInDateEq))
          {
            if (diag996)
              diag996->printItinId(id);

            isRealItin = true;
            isValidItin = isBrandBookingCodeAvailable(trx, diag996, totalNumSeats, itin, bkgVec);
            if (!isValidItin)
            {
              if (diag996)
                diag996->printPassFailMsg(false);
            }
          }
        }
      } // for itinIt

      if (!isValidItin && isRealItin)
      {
        DatePair datePair(depAltDate, depInboundAltDate);
        std::pair<std::string, DatePair> mapItem(trx.getRequest()->brandId(brandIndex), datePair);
        trx.validCalendarBrandIdMap().insert(mapItem);
        if (diag996)
          diag996->printPassFailMsg(false);
      }
      else
      {
        if (diag996)
          diag996->printPassFailMsg(true);
      }
    } // for brands
  } // for altDateIt

  if (diag996)
  {
    diag996->flushMsg();
  }
  return;
}

} // namespace anonymous

//----------------------------------------------------------------------------
// validateFlightCabin()
// validate booking code for each flight with requested cabin.
// pass itin if :
// 1. requested cabin is offered and available.
// 2. requested cabin is not offered. the first lower level is offered and
//    available.
// Then remove all class of service of cabin less than the cabin that is offered
// and available.
// fail itin if :*
// 1. requested cabin is offered but not available.
// 2. requested cabin is not offered. First lower level cabin that is offered

//    but not available.
//----------------------------------------------------------------------------
void SoldoutCabinValidator::validateFlightCabin(PricingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN VALIDATE FLCABIN");

  CabinType preferredCabinClass = trx.calendarRequestedCabin();
  StatusMap processedItinsMap;
  bool isSingleDateItinAvailable = false;

  if (trx.altDatePairs().empty())
    isSingleDateItinAvailable = validateEmptyAltDatePairs(trx, preferredCabinClass, processedItinsMap);
  else
    validateAltDatePairs(trx, preferredCabinClass, processedItinsMap);

  if (!isSingleDateItinAvailable)
  {
    if (stopProcessingDueToLackOfSeats(trx))
    {
      if (trx.awardRequest())
        throw ErrorResponseException(ErrorResponseException::SOLD_OUT, nullptr);

      throw ErrorResponseException(ErrorResponseException::ATAE_RETURNED_NO_BOOKING_CODES,
                                   "NO REQUESTED SEAT FOR CALENDAR SHOPPING AVAILABLE");
    }
  }
}

void
SoldoutCabinValidator::validateSoldouts(PricingTrx& trx)
{
  if (trx.getRequest()->brandedFareEntry() && !trx.isAltDates())
  {
    collectValidBrandId(trx);
  }
  else if (trx.calendarSoldOut() || (trx.awardRequest() && trx.isAltDates()))
  {
    validateFlightCabin(trx);
  }
  else if (trx.getRequest()->brandedFareEntry() && trx.isAltDates())
  {
    collectValidBrandIdCalendar(trx);
  }
}

bool SoldoutCabinValidator::isCabinAvailable(PricingTrx& trx,
                                             CabinType& preferredCabinClass,
                                             Itin& curItin,
                                             StatusMap& statusMap)
{
  return checkIsCabinAvailable(trx, preferredCabinClass, curItin, statusMap);
}

bool SoldoutCabinValidator::validateEmptyAltDatePairs(PricingTrx& trx, CabinType& preferredCabinClass,
                                                      StatusMap& processedItinsMap)
{
  for (Itin* itin : trx.itin())
    if (isCabinAvailable(trx, preferredCabinClass, *itin, processedItinsMap))
      return true;

  return false;
}

bool SoldoutCabinValidator::isReqAwardAltDatesRT(PricingTrx& trx)
{
  return (trx.awardRequest() && trx.isAltDates() && !trx.altDatePairs().empty() &&
          !trx.altDatePairs().begin()->first.second.isEmptyDate());
}

void SoldoutCabinValidator::validateAltDatePairs(PricingTrx& trx, CabinType& preferredCabinClass,
                                                 StatusMap& processedItinsMap)
{
  const bool originBasedRTPricing = trx.getRequest()->originBasedRTPricing();
  const bool reqAwardAltDatesRT = isReqAwardAltDatesRT(trx);

  for (AltDatePair altDateIt : trx.altDatePairs())
    validateItin(trx, preferredCabinClass, altDateIt, processedItinsMap,
                 originBasedRTPricing, reqAwardAltDatesRT);
}

void SoldoutCabinValidator::validateItin(PricingTrx& trx, CabinType& preferredCabinClass,
                                         AltDatePair& altDateIt,
                                         StatusMap& processedItinsMap,
                                         const bool originBasedRTPricing,
                                         const bool reqAwardAltDatesRT)
{
  DateTime depAltDate(((altDateIt.first).first).date(),
                      boost::posix_time::time_duration(0, 0, 0));
  DateTime depInboundAltDate(((altDateIt.first).second).date(),
                             boost::posix_time::time_duration(0, 0, 0));

  for (Itin* itin : trx.itin())
  {
    const DateTime depDate(itin->travelSeg().front()->departureDT().date(),
                           boost::posix_time::time_duration(0, 0, 0));
    const bool depDateEq = (depDate == depAltDate);

    if (depDateEq)
    {
      const DateTime depInboundDate(itin->datePair()->second.date(),
                                    boost::posix_time::time_duration(0, 0, 0));
      const bool depInDateEq = (depInboundDate == depInboundAltDate);

      if (isOriginBasedOrReqAwardAltDatesRT(originBasedRTPricing, reqAwardAltDatesRT, depInDateEq))
      {
        if (isCabinAvailable(trx, preferredCabinClass, *itin, processedItinsMap))
        {
          (altDateIt.second)->goodItinForDatePairFound = true;
          break;
        }
      }
    }

  }
}

bool SoldoutCabinValidator::isOriginBasedOrReqAwardAltDatesRT(const bool originBasedRTPricing,
                                                              const bool reqAwardAltDatesRT,
                                                              const bool depInDateEq)
{
  return (!originBasedRTPricing && !reqAwardAltDatesRT) ||
         ((originBasedRTPricing || reqAwardAltDatesRT) && depInDateEq);
}

} // iadetail
} // tse
