// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Rules/GeoPathPropertiesCalculator.h"

#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/Common/GeoPathProperties.h"
#include "DomainDataObjects/GeoPath.h"
#include "Rules/TaxData.h"
#include "test/FlightBuilder.h"
#include "test/FlightUsageBuilder.h"
#include "test/GeoPathBuilder.h"
#include "test/GeoPathMappingBuilder.h"
#include "test/ItinBuilder.h"
#include "test/RequestBuilder.h"
#include "test/MileageServiceMock.h"

#include <memory>

namespace tax
{
class GeoPathPropertiesCalculatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(GeoPathPropertiesCalculatorTest);

  CPPUNIT_TEST(testCalculate_fareBreaks_oneFare);
  CPPUNIT_TEST(testCalculate_fareBrakes_twoFares);
  CPPUNIT_TEST(testCalculate_firstLast);
  CPPUNIT_TEST(testCalculate_surface);
  CPPUNIT_TEST(testCalculate_stopovers);
  CPPUNIT_TEST(testCalculate_open);
  CPPUNIT_TEST(testCalculate_open_forcedStopover);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _mileageServiceMock.reset(new MileageServiceMock);
  }

  void tearDown() {}

  void testCalculate_fareBreaks_oneFare()
  {
    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(11, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(12, 00))
                   .setArrivalTime(type::Time(13, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(14, 00))
                   .setArrivalTime(type::Time(15, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).build());


    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("ES", type::TaxPointTag::Arrival)
                         .addGeo("ES", type::TaxPointTag::Departure)
                         .addGeo("SV", type::TaxPointTag::Arrival)
                         .addGeo("SV", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .build());

    Itin* itin(ItinBuilder()
                   .setId(0)
                   .setGeoPath(geoPath)
                   .setGeoPathRefId(geoPath->id())
                   .setFarePathGeoPathMappingRefId(0)
                   .addFlightUsage(fu1)
                   .addFlightUsage(fu2)
                   .addFlightUsage(fu3)
                   .setTravelOriginDate(type::Date(2014, 8, 1))
                   .computeTimeline()
                   .build());

    GeoPathMapping* geoPathMapping(GeoPathMappingBuilder()
                                       .addMap(0, 0)
                                       .addMap(0, 1)
                                       .addMap(0, 2)
                                       .addMap(0, 3)
                                       .addMap(0, 4)
                                       .addMap(0, 5)
                                       .build());

    std::shared_ptr<Request> request(RequestBuilder()
                                         .addItin(itin)
                                         .addFlight(f1)
                                         .addFlight(f2)
                                         .addFlight(f3)
                                         .addGeoPaths(geoPath)
                                         .addGeoPathMappings(geoPathMapping)
                                         .addTaxPoint("PL", type::TaxPointTag::Sale)
                                         .build());

    GeoPathProperties properties;
    properties.taxPointsProperties.reset(new TaxPointsProperties);
    GeoPathPropertiesCalculator calculator(*request, *_mileageServiceMock);
    calculator.calculate(*itin, properties);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), properties.taxPointsProperties->size());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[0].isFareBreak);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[1].isFareBreak);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[2].isFareBreak);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[3].isFareBreak);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[4].isFareBreak);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[5].isFareBreak);
  }

  void testCalculate_fareBrakes_twoFares()
  {
    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(11, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(12, 00))
                   .setArrivalTime(type::Time(13, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(14, 00))
                   .setArrivalTime(type::Time(15, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).build());


    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("ES", type::TaxPointTag::Arrival)
                         .addGeo("ES", type::TaxPointTag::Departure)
                         .addGeo("SV", type::TaxPointTag::Arrival)
                         .addGeo("SV", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .build());

    Itin* itin(ItinBuilder()
                   .setId(0)
                   .setGeoPath(geoPath)
                   .setGeoPathRefId(0)
                   .setFarePathGeoPathMappingRefId(0)
                   .addFlightUsage(fu1)
                   .addFlightUsage(fu2)
                   .addFlightUsage(fu3)
                   .setTravelOriginDate(type::Date(2014, 8, 1))
                   .computeTimeline()
                   .build());

    GeoPathMapping* geoPathMapping(GeoPathMappingBuilder()
                                       .addMap(0, 0)
                                       .addMap(0, 1)
                                       .addMap(0, 2)
                                       .addMap(0, 3)
                                       .addMap(1, 4)
                                       .addMap(1, 5)
                                       .build());

    std::shared_ptr<Request> request(RequestBuilder()
                                         .addItin(itin)
                                         .addFlight(f1)
                                         .addFlight(f2)
                                         .addFlight(f3)
                                         .addGeoPaths(geoPath)
                                         .addGeoPathMappings(geoPathMapping)
                                         .addTaxPoint("PL", type::TaxPointTag::Sale)
                                         .build());

    _mileageServiceMock->pushIndex(1).pushIndex(2).pushIndex(3).pushIndex(4).pushIndex(5);

    GeoPathProperties properties;
    properties.taxPointsProperties.reset(new TaxPointsProperties);
    GeoPathPropertiesCalculator calculator(*request, *_mileageServiceMock);
    calculator.calculate(*itin, properties);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), properties.taxPointsProperties->size());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[0].isFareBreak);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[1].isFareBreak);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[2].isFareBreak);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[3].isFareBreak);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[4].isFareBreak);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[5].isFareBreak);
  }

  void testCalculate_firstLast()
  {
    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(11, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(12, 00))
                   .setArrivalTime(type::Time(13, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("ES", type::TaxPointTag::Arrival)
                         .addGeo("ES", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .build());

    Itin* itin(ItinBuilder()
                   .setId(0)
                   .setGeoPath(geoPath)
                   .setGeoPathRefId(0)
                   .setFarePathGeoPathMappingRefId(0)
                   .addFlightUsage(fu1)
                   .addFlightUsage(fu2)
                   .setTravelOriginDate(type::Date(2014, 8, 1))
                   .computeTimeline()
                   .build());

    GeoPathMapping* geoPathMapping(
        GeoPathMappingBuilder().addMap(0, 0).addMap(0, 1).addMap(0, 2).addMap(0, 3).build());

    std::shared_ptr<Request> request(RequestBuilder()
                                         .addItin(itin)
                                         .addFlight(f1)
                                         .addFlight(f2)
                                         .addGeoPaths(geoPath)
                                         .addGeoPathMappings(geoPathMapping)
                                         .addTaxPoint("PL", type::TaxPointTag::Sale)
                                         .build());

    GeoPathProperties properties;
    properties.taxPointsProperties.reset(new TaxPointsProperties);
    GeoPathPropertiesCalculator calculator(*request, *_mileageServiceMock);
    calculator.calculate(*itin, properties);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), properties.taxPointsProperties->size());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[0].isFirst);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[1].isFirst);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[2].isFirst);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[3].isFirst);

    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[0].isLast);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[1].isLast);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[2].isLast);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[3].isLast);
  }

  void testCalculate_surface()
  {
    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(11, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(12, 00))
                   .setArrivalTime(type::Time(13, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(14, 00))
                   .setArrivalTime(type::Time(15, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("ES", type::TaxPointTag::Arrival)
                         .addGeo("ES", type::TaxPointTag::Departure)
                         .addGeo("SV", type::TaxPointTag::Arrival)
                         .addGeo("SV", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .build());

    Itin* itin(ItinBuilder()
                   .setId(0)
                   .setGeoPath(geoPath)
                   .setGeoPathRefId(0)
                   .setFarePathGeoPathMappingRefId(0)
                   .addFlightUsage(fu1)
                   .addFlightUsage(fu2)
                   .addFlightUsage(fu3)
                   .setTravelOriginDate(type::Date(2014, 8, 1))
                   .computeTimeline()
                   .build());

    geoPath->geos()[1].loc().cityCode() = "MAD";
    geoPath->geos()[2].loc().cityCode() = "BCN";

    GeoPathMapping* geoPathMapping(GeoPathMappingBuilder()
                                       .addMap(0, 0)
                                       .addMap(0, 1)
                                       .addMap(0, 2)
                                       .addMap(0, 3)
                                       .addMap(0, 4)
                                       .addMap(0, 5)
                                       .build());

    std::shared_ptr<Request> request(RequestBuilder()
                                         .addItin(itin)
                                         .addFlight(f1)
                                         .addFlight(f2)
                                         .addFlight(f3)
                                         .addGeoPaths(geoPath)
                                         .addGeoPathMappings(geoPathMapping)
                                         .addTaxPoint("PL", type::TaxPointTag::Sale)
                                         .build());

    _mileageServiceMock->pushIndex(1).pushIndex(2).pushIndex(3).pushIndex(4).pushIndex(5);

    GeoPathProperties properties;
    properties.taxPointsProperties.reset(new TaxPointsProperties);
    GeoPathPropertiesCalculator calculator(*request, *_mileageServiceMock);
    calculator.calculate(*itin, properties);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), properties.taxPointsProperties->size());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[0].isSurface());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[1].isSurface());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[2].isSurface());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[3].isSurface());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[4].isSurface());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[5].isSurface());
  }

  void testCalculate_stopovers()
  {
    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(12, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(14, 00))
                   .setArrivalTime(type::Time(18, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(8, 00))
                   .setArrivalTime(type::Time(10, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f4(FlightBuilder()
                   .setDepartureTime(type::Time(11, 00))
                   .setArrivalTime(type::Time(13, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).setConnectionDateShift(0).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).setConnectionDateShift(0).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).setConnectionDateShift(1).build());
    FlightUsage* fu4(FlightUsageBuilder().setFlight(f4).setConnectionDateShift(1).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("ES", type::TaxPointTag::Arrival)
                         .addGeo("ES", type::TaxPointTag::Departure)
                         .addGeo("SV", type::TaxPointTag::Arrival)
                         .addGeo("SV", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .addGeo("US", type::TaxPointTag::Departure)
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
                   .setTravelOriginDate(type::Date(2014, 8, 1))
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

    std::shared_ptr<Request> request(RequestBuilder()
                                         .addItin(itin)
                                         .addFlight(f1)
                                         .addFlight(f2)
                                         .addFlight(f3)
                                         .addFlight(f4)
                                         .addGeoPaths(geoPath)
                                         .addGeoPathMappings(geoPathMapping)
                                         .addTaxPoint("PL", type::TaxPointTag::Sale)
                                         .build());

    _mileageServiceMock->pushIndex(5).pushIndex(6).pushIndex(3).pushIndex(4).pushIndex(1).pushIndex(2);

    GeoPathProperties properties;
    properties.taxPointsProperties.reset(new TaxPointsProperties);
    GeoPathPropertiesCalculator calculator(*request, *_mileageServiceMock);
    calculator.calculate(*itin, properties);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), properties.taxPointsProperties->size());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[0].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[0].isUSTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[0].isOpen);

    CPPUNIT_ASSERT((*properties.taxPointsProperties)[1].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[1].isTimeStopover.get());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[1].isUSTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[1].isUSTimeStopover.get());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[1].isOpen);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[2].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[2].isTimeStopover.get());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[2].isUSTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[2].isUSTimeStopover.get());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[2].isOpen);

    CPPUNIT_ASSERT((*properties.taxPointsProperties)[3].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[3].isTimeStopover.get());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[3].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[3].isUSTimeStopover.get());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[3].isOpen);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[4].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[4].isTimeStopover.get());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[4].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[4].isUSTimeStopover.get());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[4].isOpen);

    CPPUNIT_ASSERT((*properties.taxPointsProperties)[5].isTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[5].isTimeStopover.get());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[5].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[5].isUSTimeStopover.get());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[5].isOpen);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[6].isTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[6].isTimeStopover.get());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[6].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[6].isUSTimeStopover.get());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[6].isOpen);

    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[7].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[7].isUSTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[7].isOpen);
  }

  void testCalculate_open()
  {
    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(12, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setArrivalDateShift(0)
                   .build());
    Flight* f4(FlightBuilder()
                   .setDepartureTime(type::Time(11, 00))
                   .setArrivalTime(type::Time(13, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).setConnectionDateShift(0).build());
    FlightUsage* fu2(FlightUsageBuilder()
                         .setFlight(f2)
                         .setConnectionDateShift(1)
                         .setOpen(type::OpenSegmentIndicator::DateFixed)
                         .build());
    FlightUsage* fu3(FlightUsageBuilder()
                         .setFlight(f3)
                         .setConnectionDateShift(2)
                         .setOpen(type::OpenSegmentIndicator::DateFixed)
                         .build());
    FlightUsage* fu4(FlightUsageBuilder().setFlight(f4).setConnectionDateShift(3).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("ES", type::TaxPointTag::Arrival)
                         .addGeo("ES", type::TaxPointTag::Departure)
                         .addGeo("SV", type::TaxPointTag::Arrival)
                         .addGeo("SV", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .addGeo("US", type::TaxPointTag::Departure)
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
                   .setTravelOriginDate(type::Date(2014, 8, 1))
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

    std::shared_ptr<Request> request(RequestBuilder()
                                         .addItin(itin)
                                         .addFlight(f1)
                                         .addFlight(f2)
                                         .addFlight(f3)
                                         .addFlight(f4)
                                         .addGeoPaths(geoPath)
                                         .addGeoPathMappings(geoPathMapping)
                                         .addTaxPoint("PL", type::TaxPointTag::Sale)
                                         .build());

    _mileageServiceMock->pushIndex(5).pushIndex(6).pushIndex(3).pushIndex(4).pushIndex(1).pushIndex(2);

    GeoPathProperties properties;
    properties.taxPointsProperties.reset(new TaxPointsProperties);
    GeoPathPropertiesCalculator calculator(*request, *_mileageServiceMock);
    calculator.calculate(*itin, properties);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), properties.taxPointsProperties->size());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[0].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[0].isUSTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[0].isOpen);

    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[1].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[1].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[1].isOpen);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[2].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[2].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[2].isOpen);

    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[3].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[3].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[3].isOpen);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[4].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[4].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[4].isOpen);

    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[5].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[5].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[5].isOpen);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[6].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[6].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[6].isOpen);

    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[7].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[7].isUSTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[7].isOpen);
  }

  void testCalculate_open_forcedStopover()
  {
    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(12, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setArrivalDateShift(0)
                   .build());
    Flight* f4(FlightBuilder()
                   .setDepartureTime(type::Time(11, 00))
                   .setArrivalTime(type::Time(13, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder()
                         .setFlight(f1)
                         .setConnectionDateShift(0)
                         .setForcedConnection(type::ForcedConnection::Connection)
                         .build());
    FlightUsage* fu2(FlightUsageBuilder()
                         .setFlight(f2)
                         .setConnectionDateShift(1)
                         .setOpen(type::OpenSegmentIndicator::DateFixed)
                         .build());
    FlightUsage* fu3(FlightUsageBuilder()
                         .setFlight(f3)
                         .setConnectionDateShift(2)
                         .setOpen(type::OpenSegmentIndicator::DateFixed)
                         .setForcedConnection(type::ForcedConnection::Stopover)
                         .build());
    FlightUsage* fu4(FlightUsageBuilder().setFlight(f4).setConnectionDateShift(3).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("ES", type::TaxPointTag::Arrival)
                         .addGeo("ES", type::TaxPointTag::Departure)
                         .addGeo("SV", type::TaxPointTag::Arrival)
                         .addGeo("SV", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .addGeo("US", type::TaxPointTag::Departure)
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
                   .setTravelOriginDate(type::Date(2014, 8, 1))
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

    std::shared_ptr<Request> request(RequestBuilder()
                                         .addItin(itin)
                                         .addFlight(f1)
                                         .addFlight(f2)
                                         .addFlight(f3)
                                         .addFlight(f4)
                                         .addGeoPaths(geoPath)
                                         .addGeoPathMappings(geoPathMapping)
                                         .addTaxPoint("PL", type::TaxPointTag::Sale)
                                         .build());

    GeoPathProperties properties;
    properties.taxPointsProperties.reset(new TaxPointsProperties);
    GeoPathPropertiesCalculator calculator(*request, *_mileageServiceMock);
    calculator.calculate(*itin, properties);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(8), properties.taxPointsProperties->size());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[0].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[0].isUSTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[0].isOpen);

    CPPUNIT_ASSERT((*properties.taxPointsProperties)[1].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[1].isTimeStopover.get());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[1].isUSTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[1].isUSTimeStopover.get());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[1].isOpen);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[2].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[2].isTimeStopover.get());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[2].isUSTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[2].isUSTimeStopover.get());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[2].isOpen);

    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[3].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[3].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[3].isOpen);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[4].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[4].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[4].isOpen);

    CPPUNIT_ASSERT((*properties.taxPointsProperties)[5].isTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[5].isTimeStopover.get());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[5].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[5].isUSTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[5].isOpen);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[6].isTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[6].isTimeStopover.get());
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[6].isUSTimeStopover);
    CPPUNIT_ASSERT((*properties.taxPointsProperties)[6].isUSTimeStopover.get());
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[6].isOpen);

    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[7].isTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[7].isUSTimeStopover);
    CPPUNIT_ASSERT(!(*properties.taxPointsProperties)[7].isOpen);
  }

private:
  std::unique_ptr<MileageServiceMock> _mileageServiceMock;
};

CPPUNIT_TEST_SUITE_REGISTRATION(GeoPathPropertiesCalculatorTest);
} // namespace tax
