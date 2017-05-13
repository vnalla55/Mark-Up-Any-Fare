#pragma once

#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ExcItin.h"
#include "DBAccess/Loc.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include <string>

namespace tse
{

class MockAirSeg : public AirSeg
{
public:
  MockAirSeg()
  {
    origin() = &_originLoc;
    destination() = &_destinationLoc;
  }

  Loc _originLoc;
  Loc _destinationLoc;
};

inline AirSeg*
createAirSegment(const LocCode& board,
                 const NationCode& boardNation,
                 const LocCode& off,
                 const NationCode& offNation,
                 const GeoTravelType& geoTravelType,
                 bool unflown = true)
{
  MockAirSeg* seg = new MockAirSeg;

  if (boardNation != "")
  {
    seg->_originLoc.nation() = boardNation;
    seg->_originLoc.loc() = ((LocCode&)(board));
  }
  if (offNation != "")
  {
    seg->_destinationLoc.nation() = offNation;
    seg->_destinationLoc.loc() = ((LocCode&)(off));
  }

  seg->origAirport() = board;
  seg->destAirport() = off;
  seg->boardMultiCity() = board;
  seg->offMultiCity() = off;
  seg->segmentType() = Air;

  seg->geoTravelType() = geoTravelType;
  seg->unflown() = unflown;

  return seg;
}

inline AirSeg*
createAirSegment(const LocCode& board,
                 const LocCode& off,
                 const CarrierCode& carrier,
                 const std::string& depDate,
                 const std::string& depTime = "14:25",
                 const FlightNumber& flight = 70,
                 const BookingCode& bookingCode = "Y",
                 CabinType::CabinTypeNew cabin = CabinType::ECONOMY_CLASS)
{
  AirSeg* seg = createAirSegment(board, "", off, "", GeoTravelType::UnknownGeoTravelType);

  seg->carrier() = carrier;
  seg->pssDepartureDate() = depDate;
  seg->pssDepartureTime() = depTime;
  seg->flightNumber() = flight;
  seg->setBookingCode(bookingCode);
  seg->bookedCabin().setClass(cabin);
  std::string dateTimeStr = depTime.empty() ? depDate + " 12:00" : depDate + " " + depTime;
  seg->departureDT() = DateTime(dateTimeStr);

  return seg;
}

inline AirSeg*
createOpenSegment(const LocCode& board,
                  const LocCode& off,
                  const CarrierCode& carrier,
                  const std::string& depDate = "",
                  const std::string& depTime = "14:25",
                  const FlightNumber& flight = 70,
                  const BookingCode& bookingCode = "Y",
                  CabinType::CabinTypeNew cabin = CabinType::ECONOMY_CLASS)
{
  AirSeg* seg = createAirSegment(board, off, carrier, depDate, depTime, flight, bookingCode, cabin);
  seg->segmentType() = Open;
  return seg;
}

inline ArunkSeg*
createArunkSegment(const LocCode& board, const LocCode& off)
{
  ArunkSeg* seg = new ArunkSeg;
  seg->origAirport() = board;
  seg->destAirport() = off;
  seg->boardMultiCity() = board;
  seg->offMultiCity() = off;
  seg->segmentType() = Arunk;
  return seg;
}

inline void
releaseTravelSegments(std::vector<TravelSeg*>& tvlSeg)
{
  for (std::vector<TravelSeg*>::iterator i = tvlSeg.begin(); i != tvlSeg.end(); i++)
    delete *i;
  tvlSeg.clear();
}

namespace
{
struct GetChangeStatus
{
  TravelSeg::ChangeStatus operator()(const TravelSeg* seg) const { return seg->changeStatus(); }
};
}

inline std::vector<TravelSeg::ChangeStatus>
getTravelSegmentsStatus(const std::vector<TravelSeg*>& tvlSeg)
{
  std::vector<TravelSeg::ChangeStatus> status(tvlSeg.size());
  std::transform(tvlSeg.begin(), tvlSeg.end(), status.begin(), GetChangeStatus());
  return status;
}

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
  std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(os, " "));
  return os;
}
}
