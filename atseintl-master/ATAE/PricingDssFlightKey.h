#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/HiddenStopDetails.h"

#include <map>

namespace tse
{
class AirSeg;

struct PricingDssFlight
{
  LocCode _origAirport;
  LocCode _destAirport;
  CarrierCode _marketingCarrierCode;
  FlightNumber _marketingFlightNumber = 0;
  CarrierCode _operatingCarrierCode;
  FlightNumber _operatingFlightNumber = 0;
  EquipmentType _equipmentCode;
  std::vector<BookingCode> _offeredBookingCodes;
  std::vector<LocCode> _hiddenStops;
  bool _bbrCarrier = false;
  int8_t _arrivalDayAdjust = 0;
  std::string _localArrivalTime;
  EquipmentType _equipTypeFirstLeg;
  EquipmentType _equipTypeLastLeg;
  std::vector<HiddenStopDetails> _hiddenStopsDetails;
};

class PricingDssFlightKey
{
public:
  DateTime _flightDate;
  DateTime _dssFlightDate;
  LocCode _origin;
  LocCode _destination;
  CarrierCode _carrier;
  FlightNumber _flightNumber;
  bool _unflown;

  PricingDssFlightKey(const DateTime& flightDate, const DateTime& dssFlightDate, const LocCode& origin, const LocCode& destination,
                      const CarrierCode& carrier, FlightNumber flightNumber, bool unflown)
    : _flightDate(flightDate), _dssFlightDate(dssFlightDate), _origin(origin), _destination(destination),
      _carrier(carrier), _flightNumber(flightNumber), _unflown(unflown)
  {
  }
  bool operator<(const PricingDssFlightKey& other) const
  {
    if(_flightDate != other._flightDate)
      return _flightDate < other._flightDate;
    if(_origin != other._origin)
      return _origin < other._origin;
    if(_destination != other._destination)
      return _destination < other._destination;
    if(_carrier != other._carrier)
      return _carrier < other._carrier;
    if(_unflown != other._unflown)
      return _unflown < other._unflown;
    return _flightNumber < other._flightNumber;
  }
};

using PricingDssFlightMap = std::map<PricingDssFlightKey, std::vector<AirSeg*>>;
}

