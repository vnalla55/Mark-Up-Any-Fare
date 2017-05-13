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
#include "test/include/CppUnitHelperMacros.h"

#include "DomainDataObjects/Request.h"
#include "Processor/ItinGroupingUtil.h"

#include "test/FlightBuilder.h"
#include "test/FlightUsageBuilder.h"
#include "test/GeoPathBuilder.h"
#include "test/GeoPathMappingBuilder.h"
#include "test/ItinBuilder.h"
#include "test/RequestBuilder.h"

#include <cassert>
#include <stdexcept>
#include <memory>

namespace tax
{

class ItinGroupingUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ItinGroupingUtilTest);

  CPPUNIT_TEST(testDateSegmenter_US);
  CPPUNIT_TEST(testDateSegmenter_PL);
  CPPUNIT_TEST(testFlightSegmenter_DE_noLimits);
  CPPUNIT_TEST(testFlightSegmenter_DE_departureLimits);
  CPPUNIT_TEST(testFlightSegmenter_US_arrivalLimits);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    buildRequest();
    _ticketingDate.reset(new type::Timestamp(type::Date(2015, 06, 30), type::Time(14, 00)));
  }

  void tearDown()
  {
    _request.reset(0);
  }

  void testDateSegmenter_US()
  {
    type::Nation nation("US");

    ItinGroupingUtil::DateSegmenter ds(nation, *_ticketingDate);
    ds._journeyDateLimits.push_back(type::Date(2015, 7, 1));
    ds._journeyDateLimits.push_back(type::Date(2015, 8, 1));
    ds._journeyDateLimits.push_back(type::Date(2015, 9, 1));
    ds._departureDateLimits.push_back(type::Date(2015, 7, 1));
    ds._departureDateLimits.push_back(type::Date(2015, 8, 1));
    ds._departureDateLimits.push_back(type::Date(2015, 8, 9));
    ds._departureDateLimits.push_back(type::Date(2015, 8, 20));
    ds._departureDateLimits.push_back(type::Date(2015, 9, 1));

    std::string key;
    const Itin& itin = _request->allItins()[0];
    ds.buildDateSegmentKey(itin, key);

    CPPUNIT_ASSERT_EQUAL(std::string("|US-JRN2|2|3"), key);
  }

  void testDateSegmenter_PL()
  {
    type::Nation nation("PL");

    ItinGroupingUtil::DateSegmenter ds(nation, *_ticketingDate);
    ds._departureDateLimits.push_back(type::Date(2015, 8, 10));

    std::string key;
    const Itin& itin = _request->allItins()[0];
    ds.buildDateSegmentKey(itin, key);

    CPPUNIT_ASSERT_EQUAL(std::string("|PL|0|1"), key);
  }

  void testFlightSegmenter_DE_noLimits()
  {
    type::Nation nation("DE");

    ItinGroupingUtil::FlightSegmenter fs(nation, *_ticketingDate);

    std::string key;
    const Itin& itin = _request->allItins()[0];
    fs.buildFltSegmentKey(itin, key);

    CPPUNIT_ASSERT_EQUAL(std::string("|FLTRNG-DELHLHLHLH_aLHLHLHLH"), key);
  }

  void testFlightSegmenter_DE_departureLimits()
  {
    type::Nation nation("DE");

    ItinGroupingUtil::FlightSegmenter fs(nation, *_ticketingDate);
    ItinGroupingUtil::FlightSegmenter::FlightRangeKey fltRangeKey =
        std::make_pair(type::CarrierCode("LH"), type::TaxPointTag::Departure);
    fs._flightRangesBefore[fltRangeKey].push_back(std::make_pair(2501, 3500));
    fs._flightRangesBefore[fltRangeKey].push_back(std::make_pair(3501, 4500));
    fs._flightRangesAfter[fltRangeKey].push_back(std::make_pair(2001, 3000));
    fs._flightRangesAfter[fltRangeKey].push_back(std::make_pair(3001, 3500));
    fs._flightRangesAfter[fltRangeKey].push_back(std::make_pair(3501, 3999));
    fs._flightRangesAfter[fltRangeKey].push_back(std::make_pair(4000, 5000));

    std::string key;
    const Itin& itin = _request->allItins()[0];
    fs.buildFltSegmentKey(itin, key);

    CPPUNIT_ASSERT_EQUAL(std::string("|FLTRNG-DELH_0LH_3LH_1LH_2_aLHLHLHLH"), key);
  }

  void testFlightSegmenter_US_arrivalLimits()
  {
    type::Nation nation("US");

    ItinGroupingUtil::FlightSegmenter fs(nation, *_ticketingDate);
    ItinGroupingUtil::FlightSegmenter::FlightRangeKey fltRangeKey =
        std::make_pair(type::CarrierCode("LH"), type::TaxPointTag::Arrival);
    fs._flightRangesBefore[fltRangeKey].push_back(std::make_pair(4001, 4005));
    fs._flightRangesBefore[fltRangeKey].push_back(std::make_pair(4006, 4010));
    fs._flightRangesAfter[fltRangeKey].push_back(std::make_pair(2501, 3000));
    fs._flightRangesAfter[fltRangeKey].push_back(std::make_pair(3001, 3999));
    fs._flightRangesAfter[fltRangeKey].push_back(std::make_pair(4000, 4999));

    std::string key;
    const Itin& itin = _request->allItins()[0];
    fs.buildFltSegmentKey(itin, key);

    CPPUNIT_ASSERT_EQUAL(std::string("|FLTRNG-USLHLH_aLH_1LH_2"), key);
  }

private:
  std::unique_ptr<Request> _request;
  std::unique_ptr<type::Timestamp> _ticketingDate;

  void buildRequest()
  {
    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(16, 00))
                   .setArrivalTime(type::Time(19, 00))
                   .setArrivalDateShift(0)
                   .setMarketingCarrier("LH")
                   .setMarketingCarrierFlightNumber(3000)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(9, 00))
                   .setArrivalTime(type::Time(17, 00))
                   .setArrivalDateShift(0)
                   .setMarketingCarrier("LH")
                   .setMarketingCarrierFlightNumber(4000)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(8, 00))
                   .setArrivalTime(type::Time(23, 00))
                   .setArrivalDateShift(0)
                   .setMarketingCarrier("LH")
                   .setMarketingCarrierFlightNumber(4010)
                   .build());
    Flight* f4(FlightBuilder()
                   .setDepartureTime(type::Time(8, 00))
                   .setArrivalTime(type::Time(11, 00))
                   .setArrivalDateShift(0)
                   .setMarketingCarrier("LH")
                   .setMarketingCarrierFlightNumber(3510)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).setConnectionDateShift(0).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).setConnectionDateShift(1).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).setConnectionDateShift(7).build());
    FlightUsage* fu4(FlightUsageBuilder().setFlight(f4).setConnectionDateShift(1).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("DE", type::TaxPointTag::Arrival)
                         .addGeo("DE", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .addGeo("US", type::TaxPointTag::Departure)
                         .addGeo("DE", type::TaxPointTag::Arrival)
                         .addGeo("DE", type::TaxPointTag::Departure)
                         .addGeo("PL", type::TaxPointTag::Arrival)
                         .build());

    Itin* itin(ItinBuilder()
                   .setId(0)
                   .setGeoPath(geoPath)
                   .setGeoPathRefId(0)
                   .setFarePathGeoPathMappingRefId(0)
                   .addFlightUsage(fu1)
                   .addFlightUsage(fu2)
                   .addFlightUsage(fu3)
                   .addFlightUsage(fu4)
                   .setTravelOriginDate(type::Date(2015, 8, 2))
                   .computeTimeline()
                   .build());

    GeoPathMapping* geoPathMapping(GeoPathMappingBuilder()
                                       .addMap(0, 0)
                                       .addMap(0, 1)
                                       .addMap(0, 2)
                                       .addMap(0, 3)
                                       .addMap(0, 4)
                                       .addMap(0, 5)
                                       .addMap(0, 6)
                                       .addMap(0, 7)
                                       .build());

    _request.reset(RequestBuilder()
                     .addItin(itin)
                     .addFlight(f1)
                     .addFlight(f2)
                     .addFlight(f3)
                     .addFlight(f4)
                     .addGeoPaths(geoPath)
                     .addGeoPathMappings(geoPathMapping)
                     .addTaxPoint("US", type::TaxPointTag::Sale)
                     .build());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ItinGroupingUtilTest);
} // namespace tax
