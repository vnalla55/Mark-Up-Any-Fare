// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/TaxName.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "Rules/SpecialTrips.h"
#include "test/include/CppUnitHelperMacros.h"

#include <memory>

namespace tax
{

class SpecialTripsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SpecialTripsTest);
  CPPUNIT_TEST(testUsRoundTrip);
  CPPUNIT_TEST(testIsOverlapped);
  CPPUNIT_TEST(testRemoveOverlappedTrips);
  CPPUNIT_TEST_SUITE_END();

  std::unique_ptr<GeoPath> _geoPath;
  std::unique_ptr<TaxName> _taxName;
  boost::ptr_vector<FlightUsage> _flightUsages;
  boost::ptr_vector<Flight> _flights;

public:

  Geo createArrivalGeo(const type::AirportCode& cityCode, const type::Nation& nation)
  {
    Geo result;
    result.loc().cityCode().convertFrom(cityCode);
    result.loc().code() = cityCode;
    result.loc().nation() = type::Nation(nation);
    result.loc().tag() = type::TaxPointTag::Arrival;

    return result;
  }

  Geo createDepartureGeo(const type::AirportCode& cityCode, const type::Nation& nation)
  {
    Geo result;
    result.loc().cityCode().convertFrom(cityCode);
    result.loc().code() = cityCode;
    result.loc().nation() = type::Nation(nation);
    result.loc().tag() = type::TaxPointTag::Departure;

    return result;
  }

  void addFlight(std::vector<Geo>& geos,
      const type::AirportCode& from, const type::Nation& fromNation,
      const type::AirportCode& to, const type::Nation& toNation)
  {
    Geo fromGeo = createDepartureGeo(from, fromNation);
    fromGeo.id() = geos.size();
    geos.push_back(fromGeo);

    Geo toGeo = createArrivalGeo(to, toNation);
    toGeo.id() = geos.size();
    geos.push_back(toGeo);
  }

  void prepareFlights2()
  {
    std::vector<Geo>& geos = _geoPath->geos();

    _flights.clear();

    for (int flight = 0; flight < 8; flight++)
    {
      _flightUsages.push_back(new FlightUsage());
      _flights.push_back(new Flight());
      _flightUsages[flight].flight() = &_flights[flight];
    }

    addFlight(geos, "LAX", "US", "DEN", "US");
    addFlight(geos, "DEN", "US", "CHI", "US");
    addFlight(geos, "CHI", "US", "DEN", "US");
    addFlight(geos, "DEN", "US", "PHX", "US");
    addFlight(geos, "PHX", "US", "LAX", "US");
    addFlight(geos, "LAX", "US", "CHI", "US");
    addFlight(geos, "CHI", "US", "DEN", "US");
    addFlight(geos, "DEN", "US", "MIA", "US");
  }

  void setUp()
  {
    _geoPath.reset(new GeoPath);
    _taxName.reset(new TaxName);
    _flightUsages.clear();
    prepareFlights2();
  }

  const std::shared_ptr<PaymentDetail> createPaymentDetail(
      const type::Index id,
      TaxName& taxName,
      type::TicketedPointTag ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly)
  {
    Geo& taxPoint1 = _geoPath->geos()[id];

    Geo taxPoint2;
    std::shared_ptr<PaymentDetail> paymentDetail(
        new PaymentDetail(PaymentRuleData(type::SeqNo(),
                                          ticketedPointTag,
                                          TaxableUnitTagSet::none(),
                                          0,
                                          type::CurrencyCode(UninitializedCode),
                                          type::TaxAppliesToTagInd::Blank),
                          taxPoint1,
                          taxPoint2,
                          taxName));

    paymentDetail->getMutableTaxPointsProperties().resize(_geoPath->geos().size());

    return paymentDetail;
  }

  void setUsStopovers(PaymentDetail& paymentDetail)
  {
    for (unsigned index = 0; index <= 10; index++)
    {
      paymentDetail.getMutableTaxPointsProperties().at(index).isUSLimitStopover = true;
    }
    paymentDetail.getMutableTaxPointsProperties().at(15).isUSLimitStopover = true;
  }

  void testUsRoundTrip()
  {
    std::vector<Trip> journeys;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(0, *_taxName);
    setUsStopovers(*paymentDetail);
    std::vector<FlightUsage> flightUsages;
    for (auto x : _flightUsages)
    {
      flightUsages.push_back(x);
    }
    SpecialTrips::findRoundTrips(*paymentDetail, *_geoPath, flightUsages, journeys);

    CPPUNIT_ASSERT_EQUAL(std::vector<Trip>::size_type(1), journeys.size());
    CPPUNIT_ASSERT_EQUAL(type::Index(0), journeys.front().first);
    CPPUNIT_ASSERT_EQUAL(type::Index(9), journeys.front().second);
  }

  void testIsOverlapped()
  {
    CPPUNIT_ASSERT(IsOverlapped()(std::make_pair(0, 1), std::make_pair(0, 1)));
    CPPUNIT_ASSERT(IsOverlapped()(std::make_pair(0, 1), std::make_pair(0, 3)));
    CPPUNIT_ASSERT(IsOverlapped()(std::make_pair(0, 5), std::make_pair(2, 3)));
    CPPUNIT_ASSERT(IsOverlapped()(std::make_pair(2, 3), std::make_pair(0, 5)));
    CPPUNIT_ASSERT(!IsOverlapped()(std::make_pair(0, 1), std::make_pair(2, 3)));
    CPPUNIT_ASSERT(!IsOverlapped()(std::make_pair(2, 3), std::make_pair(0, 1)));
  }

  void testRemoveOverlappedTrips()
  {
    std::vector<Trip> trips1;
    trips1.push_back(std::make_pair(0, 1));
    trips1.push_back(std::make_pair(2, 3));
    trips1.push_back(std::make_pair(4, 5));
    trips1.push_back(std::make_pair(6, 7));
    trips1.push_back(std::make_pair(8, 9));
    trips1.push_back(std::make_pair(10, 15));

    std::vector<Trip> trips2;
    trips2.push_back(std::make_pair(0, 9));

    SpecialTrips::removeOverlappedTrips(trips1, trips2);

    CPPUNIT_ASSERT_EQUAL(std::vector<Trip>::size_type(1), trips1.size());
    CPPUNIT_ASSERT_EQUAL(type::Index(10), trips1.front().first);
    CPPUNIT_ASSERT_EQUAL(type::Index(15), trips1.front().second);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(SpecialTripsTest);
}
