// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#ifndef AIRSEGBUILDER_H
#define AIRSEGBUILDER_H

#include <boost/assign/std/vector.hpp>

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Loc.h"
#include "DataModel/AirSeg.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using boost::assign::operator+=;

class AirSegBuilder
{
  AirSeg* _airSeg;
  TestMemHandle* _memHandle;
  Loc* _originLoc;
  Loc* _destinationLoc;

public:
  AirSegBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _airSeg = _memHandle->create<AirSeg>();
    _originLoc = _memHandle->create<Loc>();
    _destinationLoc = _memHandle->create<Loc>();
  }

  ~AirSegBuilder() {}

  AirSegBuilder& withArea(const IATAAreaCode& orig, const IATAAreaCode& dest)
  {
    _originLoc->area() = orig;
    _destinationLoc->area() = dest;
    return *this;
  }

  AirSegBuilder& withSubArea(const IATASubAreaCode& orig, const IATASubAreaCode& dest)
  {
    _originLoc->subarea() = orig;
    _destinationLoc->subarea() = dest;
    return *this;
  }

  AirSegBuilder& withNations(const NationCode& orig, const NationCode& dest)
  {
    _originLoc->nation() = orig;
    _destinationLoc->nation() = dest;
    return *this;
  }

  AirSegBuilder& withCarriers(const std::string& marketing, const std::string& operating)
  {
    _airSeg->setMarketingCarrierCode(marketing);
    _airSeg->setOperatingCarrierCode(operating);
    return *this;
  }

  AirSegBuilder& withMarketingCarrier(const std::string& carrier)
  {
    _airSeg->setMarketingCarrierCode(carrier);
    return *this;
  }

  AirSegBuilder& withOperatingCarrier(const std::string& carrier)
  {
    _airSeg->setOperatingCarrierCode(carrier);
    return *this;
  }

  AirSegBuilder& setType(const TravelSegType& type)
  {
    _airSeg->segmentType() = type;
    return *this;
  }

  AirSegBuilder& withSegmentOrder(int16_t order)
  {
    _airSeg->segmentOrder() = order;
    return *this;
  }

  AirSegBuilder& withFlightNo(const FlightNumber& flight)
  {
    _airSeg->flightNumber() = flight;
    return *this;
  }

  AirSegBuilder& withOrigin(const LocCode& origAirport)
  {
    _originLoc->loc() = origAirport;
    return *this;
  }

  AirSegBuilder& withDestination(const LocCode& destAirport)
  {
    _destinationLoc->loc() = destAirport;
    return *this;
  }

  AirSegBuilder& withLocations(const LocCode& origAirport, const LocCode& destAirport = "")
  {
    _originLoc->loc() = origAirport;
    _destinationLoc->loc() = destAirport;
    return *this;
  }

  AirSegBuilder& withAirports(const LocCode& origAirport, const LocCode& destAirport)
  {
    _airSeg->origAirport() = origAirport;
    _airSeg->destAirport() = destAirport;

    return *this;
  }

  AirSegBuilder& withMulticity(const LocCode& board, const LocCode& off)
  {
    _airSeg->boardMultiCity() = board;
    _airSeg->offMultiCity() = off;

    return *this;
  }

  AirSegBuilder& withPnr(int16_t segment)
  {
    _airSeg->pnrSegment() = segment;
    return *this;
  }

  AirSegBuilder& withUnflown(bool unflown)
  {
    _airSeg->unflown() = unflown;
    return *this;
  }

  AirSegBuilder& withDepartureDT(const DateTime& dt)
  {
    _airSeg->departureDT() = dt;
    return *this;
  }

  AirSegBuilder& withArrivalDT(const DateTime& dt)
  {
    _airSeg->arrivalDT() = dt;
    return *this;
  }

  AirSegBuilder& withPssDepartureTime(const PSSTime& time)
  {
    _airSeg->pssDepartureTime() = time;
    return *this;
  }

  AirSegBuilder& withPssDepartureDate(const PSSDate& date)
  {
    _airSeg->pssDepartureDate() = date;
    return *this;
  }

  AirSegBuilder& withHiddenStop(const LocCode& code)
  {
    Loc* loc = _memHandle->create<Loc>();
    loc->loc() = code;
    _airSeg->hiddenStops().push_back(loc);
    return *this;
  }

  AirSegBuilder& withEquipmentType(const EquipmentType& eqType)
  {
    _airSeg->equipmentType() = eqType;
    return *this;
  }

  AirSegBuilder& withEquipmentTypes(const EquipmentType& eqType)
  {
    _airSeg->equipmentTypes().push_back(eqType);
    return *this;
  }

  AirSegBuilder& withForcedConxStopOver(char fconx, char fstopover)
  {
    _airSeg->forcedConx() = fconx;
    _airSeg->forcedStopOver() = fstopover;
    return *this;
  }

  AirSeg* build()
  {
    _airSeg->origin() = _originLoc;
    _airSeg->destination() = _destinationLoc;
    return _airSeg;
  }

  AirSeg& buildRef()
  {
    _airSeg->origin() = _originLoc;
    _airSeg->destination() = _destinationLoc;
    return *_airSeg;
  }

  void buildVec(std::vector<TravelSeg*>::const_iterator& beginI,
                std::vector<TravelSeg*>::const_iterator& endI)
  {
    _airSeg->origin() = _originLoc;
    _airSeg->destination() = _destinationLoc;
    std::vector<TravelSeg*>* travelSegs = _memHandle->create<std::vector<TravelSeg*> >();
    *travelSegs += _airSeg;
    beginI = travelSegs->begin();
    endI = travelSegs->end();
  }
};
} // tse
#endif // AIRSEGBUILDER_H
