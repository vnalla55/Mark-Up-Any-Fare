//----------------------------------------------------------------------------
//
//  File   :  PricingDssFlightProcessor.cpp
//
//  Author :  Janusz Jagodzinski
//
//  Copyright Sabre 2015
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s) have
//          been supplied.
//
//-----------------------------------------------------------------------

#include "ATAE/PricingDssFlightProcessor.h"
#include "ATAE/PricingDssResponseHandler.h"
#include "ATAE/PricingDssFlightKey.h"

#include "Common/ClassOfService.h"
#include "Common/FallbackUtil.h"
#include "Common/HiddenStopDetails.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Loc.h"
#include "Util/IteratorRange.h"

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include <sstream>

namespace tse
{
FALLBACK_DECL(dateAdjustTaxFix);
FALLBACK_DECL(fallbackMoveGetCabinToDSS);

static Logger
logger("atseintl.Xform.PricingDssFlightProcessor");

void
PricingDssFlightProcessor::populateCabin(const std::vector<BookingCode>& offeredBookingCodes, AirSeg& airSeg) const
{
  if(!TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(_trx))
  {
    const size_t vecSize = offeredBookingCodes.size();
    for (size_t j = 0; j < vecSize; j++)
    {
      ClassOfService* cos = getCos(offeredBookingCodes[j], airSeg.carrier(), airSeg.departureDT(), airSeg.flightNumber());
      if(cos != nullptr)
        airSeg.classOfService().push_back(cos);
    }
  }
  else
  {
    RBDByCabinUtil rbdCabin(_trx, DSS_RSP);
    rbdCabin.getCabinsByRbd(airSeg, offeredBookingCodes);
  }
}

ClassOfService*
PricingDssFlightProcessor::getCos(const BookingCode& bookingCode, const CarrierCode& carrier,
                                  const DateTime& departureDt, const FlightNumber& flightNumber) const
{
  DataHandle& dataHandle = _trx.dataHandle();
  ClassOfService* cos = nullptr;
  dataHandle.get(cos);
  if (cos == nullptr)
    return nullptr;
  const Cabin* const aCabin = dataHandle.getCabin(carrier, bookingCode, departureDt);
  if (aCabin == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "PricingDssResponseHandler::getCos() CABIN TABLE ERROR FLIGHT:"
                      << carrier << flightNumber << " " << bookingCode
                      << departureDt.dateToString(DDMMMYYYY, ""));
    return nullptr;
  }

  cos->bookingCode() = bookingCode;
  cos->numSeats() = 0;
  cos->cabin() = aCabin->cabin();
  return cos;
}

void
PricingDssFlightProcessor::findCabin(PricingDssFlightMap& flightMap) const
{
  for(auto it = flightMap.begin(); it != flightMap.end(); ++it)
  {
    for(auto airSeg : it->second)
    {
      if(!airSeg->bookedCabin().isValidCabin())
        getCabin(*airSeg);
    }
  }
}

void
PricingDssFlightProcessor::getCabin(AirSeg& airSeg) const
{
  if (!_isAncillaryPricingTrx &&
      TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(_trx) &&
      !fallback::fallbackMoveGetCabinToDSS(const_cast<PricingTrx*>(&_trx)))
  {
    // Store booked cabin
    DateTime tempAfDate;
    if (PricingTrx::AF_EXC_TRX == _trx.excTrxType())
    {
      tempAfDate = _trx.dataHandle().ticketDate();
      _trx.dataHandle().setTicketDate(airSeg.departureDT());
    }
    RBDByCabinUtil rbdUtil(_trx, PRICING_RQ);
    rbdUtil.getCabinByRBD(airSeg);
    if (PricingTrx::AF_EXC_TRX == _trx.excTrxType())
    {
      _trx.dataHandle().setTicketDate(tempAfDate);
    }
  }
}

void
PricingDssFlightProcessor::populateAirSegs(const PricingDssFlight& currentFlight,
                                                std::vector<AirSeg*>::iterator begin,
                                                std::vector<AirSeg*>::iterator end) const
{
  for(; begin != end; ++begin)
  {
    AirSeg* airSeg = *begin;
    populateAirSeg(currentFlight, *airSeg);
    airSeg->hiddenStopsDetails() = currentFlight._hiddenStopsDetails;
  }
}

// populate the data returned by DSS to appropriate Travel segment

void
PricingDssFlightProcessor::populateAirSeg(const PricingDssFlight& flight, AirSeg& airSeg) const
{
  if (_isAncillaryPricingTrx && airSeg.arrivalDT().isEmptyDate())
  {
    static const boost::regex expression("^(\\d{2}):(\\d{2})$");
    boost::cmatch what;

    if (boost::regex_match(flight._localArrivalTime.c_str(), what, expression))
    {
      const DateTime adjustedWithDays =  (!fallback::dateAdjustTaxFix(&_trx) && flight._arrivalDayAdjust < 0) ?
          airSeg.departureDT().subtractDays(-flight._arrivalDayAdjust) :
          airSeg.departureDT().addDays(flight._arrivalDayAdjust);

      const DateTime arrivalDT(adjustedWithDays.date(),
                         std::atoi(what[1].first),
                         std::atoi(what[2].first),
                         0);

      airSeg.arrivalDT() = arrivalDT;
    }
  }

  airSeg.setArrivalDayAdjust(flight._arrivalDayAdjust);

  const bool isA2Trx = (PricingTrx::PORT_EXC_TRX <= _trx.excTrxType() &&
                  _trx.excTrxType() <= PricingTrx::ME_DIAG_TRX);

  if ((PricingTrx::AR_EXC_TRX || isA2Trx) && !airSeg.unflown())
  {
    airSeg.equipmentType() = flight._equipmentCode;
    getCabin(airSeg);
  }
  else
  {
    airSeg.setOperatingCarrierCode(flight._operatingCarrierCode);
    airSeg.operatingFlightNumber() = flight._operatingFlightNumber;

    const Agent* const agent = _trx.getRequest()->ticketingAgent();
    if (agent != nullptr && agent->infiniUser())
    {
      if (airSeg.equipmentType().empty())
        airSeg.equipmentType() = flight._equipmentCode;
    }
    else
    {
      airSeg.equipmentType() = flight._equipmentCode;
    }

    getCabin(airSeg);

    airSeg.bbrCarrier() = flight._bbrCarrier;

    if (!_isAncillaryPricingTrx && !(flight._offeredBookingCodes.empty()))
    {
      populateCabin(flight._offeredBookingCodes, airSeg);
    }
  }

  if (!flight._hiddenStops.empty())
  {
    const size_t vecSize = flight._hiddenStops.size();
    DataHandle& dataHandle = _trx.dataHandle();
    const Loc* loc = nullptr;

    for (size_t j = 0; j < vecSize; j++)
    {
      loc = dataHandle.getLoc(flight._hiddenStops[j], airSeg.departureDT());

      if (loc != nullptr)
        airSeg.hiddenStops().push_back(loc);
    }

    addEquipmentTypeForHiddenStop(flight, airSeg);
  }
}

PricingDssFlightMap::iterator
PricingDssFlightProcessor::findAirSegs(const PricingDssFlight& flight, PricingDssFlightMap& flightsToProcessMap) const
{
  for(auto it = flightsToProcessMap.begin(); it != flightsToProcessMap.end(); ++it)
  {
    const PricingDssFlightKey& key = it->first;
    if (key._origin == flight._origAirport &&
        key._destination == flight._destAirport &&
        key._carrier == flight._marketingCarrierCode &&
        key._flightNumber == flight._marketingFlightNumber)
    {
      return it;
    }
  }
  return flightsToProcessMap.end();
}

void
PricingDssFlightProcessor::addEquipmentTypeForHiddenStop(const PricingDssFlight& flight,
                                                         AirSeg& airSeg) const
{
  const size_t hiddenStopTotal = airSeg.hiddenStops().size();
  if (hiddenStopTotal == 0)
    return;

  const std::vector<const Loc*>& hiddenStops = airSeg.hiddenStops();
  const bool isInternational = airSeg.origin()->nation() != hiddenStops[0]->nation();
  bool addLastLeg = false;

  for (uint32_t i = 0; i < hiddenStopTotal; i++)
  {
    if (!addLastLeg)
    {
      airSeg.equipmentTypes().push_back(flight._equipTypeFirstLeg);

      // last hidden stop
      if (hiddenStopTotal == (i + 1))
        break;

      if (isInternational)
      {
        if (hiddenStops[i]->nation() == hiddenStops[i + 1]->nation())
          addLastLeg = true;
      }
      else
      {
        if (hiddenStops[i]->nation() != hiddenStops[i + 1]->nation())
          addLastLeg = true;
      }
    }
    else
      airSeg.equipmentTypes().push_back(flight._equipTypeLastLeg);
  }

  airSeg.equipmentTypes().push_back(flight._equipTypeLastLeg);
}
}
