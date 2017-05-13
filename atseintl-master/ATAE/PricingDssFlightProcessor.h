//----------------------------------------------------------------------------
//
//  File   :  PricingDssFlightProcessor.h
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
//---------------------------------------------------------------------------
#pragma once

#include "ATAE/PricingDssFlightKey.h"

#include "Common/HiddenStopDetails.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include "DataModel/AncillaryPricingTrx.h"

#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/util/XercesDefs.hpp>

namespace tse
{
class PricingTrx;
class AirSeg;
class ClassOfService;

class PricingDssFlightProcessor final
{
protected:
  friend class PricingDssFlightProcessorTest;
public:
  PricingDssFlightProcessor(PricingTrx& trx)
    : _trx(trx),
      _isAncillaryPricingTrx(dynamic_cast<const AncillaryPricingTrx*>(&trx) != nullptr)
  {
  }

  PricingDssFlightMap::iterator
  findAirSegs(const PricingDssFlight& flight, PricingDssFlightMap& flightsToProcessMap) const;

  void findCabin(PricingDssFlightMap& flightsToProcessMap) const;

  void populateAirSegs(const PricingDssFlight& currentFlight,
                       std::vector<AirSeg*>::iterator begin,
                       std::vector<AirSeg*>::iterator end) const;

private:
  void populateCabin(const std::vector<BookingCode>& offeredBookingCodes, AirSeg& airSeg) const;
  ClassOfService* getCos(const BookingCode& bookingCode, const CarrierCode& carrier,
                         const DateTime& departureDt, const FlightNumber& flightNumber) const;

  void addEquipmentTypeForHiddenStop(const PricingDssFlight& flight, AirSeg& airSeg) const;
  void populateAirSeg(const PricingDssFlight& flight, AirSeg& airSeg) const;
  void getCabin(AirSeg& airSeg) const;

private:
  PricingTrx& _trx;
  const bool _isAncillaryPricingTrx;
};
} // end namespace tse
