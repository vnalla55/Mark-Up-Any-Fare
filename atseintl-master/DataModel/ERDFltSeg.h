//-------------------------------------------------------------------
//
//  File:           ERDFltSeg.h
//  Created:        29 September 2008
//  Authors:        Konrad Koch
//
//  Description:    Wrapper to SEG section data from WPRD request.
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class FareDisplayTrx;
class ERDFareComp;

class ERDFltSeg
{
public:
  class MatchSegmentNumber : public std::unary_function<uint16_t, bool>
  {
  public:
    MatchSegmentNumber(uint16_t segmentNumber) { _segmentNumber = segmentNumber; }

    bool operator()(const ERDFltSeg* erdFltSeg) const
    {
      return _segmentNumber == erdFltSeg->itinSegNumber();
    }

  private:
    uint16_t _segmentNumber;
  };

  ERDFltSeg();

private:
  ERDFltSeg(const ERDFltSeg&);
  ERDFltSeg& operator=(const ERDFltSeg&);

public:
  //--------------------------------------------------------------------------
  // Accessors
  //--------------------------------------------------------------------------
  ERDFareComp*& fareComponent() { return _fareComponent; }
  const ERDFareComp* fareComponent() const { return _fareComponent; }

  uint16_t& itinSegNumber() { return _itinSegNumber; }
  const uint16_t itinSegNumber() const { return _itinSegNumber; }

  bool& surface() { return _surface; }
  const bool surface() const { return _surface; }

  LocCode& departureAirport() { return _departureAirport; }
  const LocCode& departureAirport() const { return _departureAirport; }

  LocCode& arrivalAirport() { return _arrivalAirport; }
  const LocCode& arrivalAirport() const { return _arrivalAirport; }

  LocCode& departureCity() { return _departureCity; }
  const LocCode& departureCity() const { return _departureCity; }

  LocCode& arrivalCity() { return _arrivalCity; }
  const LocCode& arrivalCity() const { return _arrivalCity; }

  GeoTravelType& geoTravelType() { return _geoTravelType; }
  const GeoTravelType& geoTravelType() const { return _geoTravelType; }

  void select(FareDisplayTrx& trx);
  bool isOriginAndDestinationInRussia(FareDisplayTrx& trx);

private:
  ERDFareComp* _fareComponent;

  LocCode _departureAirport; // SEG:C6I
  LocCode _arrivalAirport; // SEG:A02
  LocCode _departureCity; // SEG:A11
  LocCode _arrivalCity; // SEG:A12
  uint16_t _itinSegNumber; // SEG:Q0Z
  bool _surface; // SEG:S10
  GeoTravelType _geoTravelType; // SEG:C13
};

inline ERDFltSeg::ERDFltSeg()
  : _fareComponent(nullptr), _itinSegNumber(0), _surface(false), _geoTravelType(GeoTravelType::UnknownGeoTravelType)
{
}
}

