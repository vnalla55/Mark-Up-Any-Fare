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


#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/Common/GeoPathProperties.h"
#include "DomainDataObjects/GeoPath.h"
#include "Rules/TaxData.h"
#include "Rules/GeoPathPropertiesCalculator.h"
#include "Rules/TurnaroundCalculator.h"
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
class TurnaroundCalculatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TurnaroundCalculatorTest);

  CPPUNIT_TEST(testCalculate_turnaroundStopoverDomesticUS);
  CPPUNIT_TEST(testCalculate_turnaroundStopoverDomestic);
  CPPUNIT_TEST(testCalculate_turnaroundInternational);
  CPPUNIT_TEST(testCalculate_turnaroundSpecialUS_RTOJLogic);
  CPPUNIT_TEST(testCalculate_turnaroundAlternateDeterminationLogicOff);
  CPPUNIT_TEST(testCalculate_turnaroundAlternateDeterminationLogicOn_StopoverPresent);
  CPPUNIT_TEST(testCalculate_turnaroundAlternateDeterminationLogicOn_NoStopover);
  CPPUNIT_TEST(testCalculate_turnaroundRTW);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _mileageServiceMock.reset(new MileageServiceMock);
  }

  void tearDown() {}

  void testCalculate_turnaroundStopoverDomesticUS()
  {
    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(12, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(17, 00)) // 5h stopover
                   .setArrivalTime(type::Time(18, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(19, 00))
                   .setArrivalTime(type::Time(20, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f4(FlightBuilder()
                   .setDepartureTime(type::Time(21, 00))
                   .setArrivalTime(type::Time(22, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).setConnectionDateShift(0).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).setConnectionDateShift(0).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).setConnectionDateShift(0).build());
    FlightUsage* fu4(FlightUsageBuilder().setFlight(f4).setConnectionDateShift(0).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("US", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .addGeo("US", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .addGeo("US", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .addGeo("US", type::TaxPointTag::Departure)
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
                                         .addTaxPoint("US", type::TaxPointTag::Sale)
                                         .build());

    _mileageServiceMock->pushIndex(5).pushIndex(6).pushIndex(3).pushIndex(4).pushIndex(1).pushIndex(2);

    GeoPathProperties gpp;
    GeoPathPropertiesCalculator geoPropertiesCalculator(*request, *_mileageServiceMock);
    geoPropertiesCalculator.calculate(*itin, gpp);

    TurnaroundCalculator calculator(request->geoPaths()[0],
                                    *_mileageServiceMock,
                                    *gpp.taxPointsProperties,
                                    false,
                                    false);

    const Geo* turnaround = calculator.getTurnaroundPoint(*itin);
    CPPUNIT_ASSERT(turnaround);
    CPPUNIT_ASSERT_EQUAL(static_cast<type::Index>(1), turnaround->id());
  }

  void testCalculate_turnaroundStopoverDomestic()
  {
    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(12, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(17, 00))
                   .setArrivalTime(type::Time(18, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(19, 00))
                   .setArrivalTime(type::Time(20, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f4(FlightBuilder()
                   .setDepartureTime(type::Time(21, 00))
                   .setArrivalTime(type::Time(22, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).setConnectionDateShift(1).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).setConnectionDateShift(1).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).setConnectionDateShift(1).build());
    FlightUsage* fu4(FlightUsageBuilder().setFlight(f4).setConnectionDateShift(1).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("PL", type::TaxPointTag::Arrival)
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("PL", type::TaxPointTag::Arrival)
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("PL", type::TaxPointTag::Arrival)
                         .addGeo("PL", type::TaxPointTag::Departure)
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
                                         .addTaxPoint("US", type::TaxPointTag::Sale)
                                         .build());

    _mileageServiceMock->pushIndex(5).pushIndex(6).pushIndex(3).pushIndex(4).pushIndex(1).pushIndex(2);

    GeoPathProperties gpp;
    GeoPathPropertiesCalculator geoPropertiesCalculator(*request, *_mileageServiceMock);
    geoPropertiesCalculator.calculate(*itin, gpp);

    TurnaroundCalculator calculator(request->geoPaths()[0],
                                    *_mileageServiceMock,
                                    *gpp.taxPointsProperties,
                                    false,
                                    false);

    const Geo* turnaround = calculator.getTurnaroundPoint(*itin);
    CPPUNIT_ASSERT(turnaround);
    CPPUNIT_ASSERT_EQUAL(static_cast<type::Index>(5), turnaround->id());
  }

  void testCalculate_turnaroundInternational()
  {

    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(12, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(17, 00))
                   .setArrivalTime(type::Time(18, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(19, 00))
                   .setArrivalTime(type::Time(20, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f4(FlightBuilder()
                   .setDepartureTime(type::Time(21, 00))
                   .setArrivalTime(type::Time(22, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).setConnectionDateShift(1).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).setConnectionDateShift(1).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).setConnectionDateShift(1).build());
    FlightUsage* fu4(FlightUsageBuilder().setFlight(f4).setConnectionDateShift(1).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("CZ", type::TaxPointTag::Arrival)
                         .addGeo("CZ", type::TaxPointTag::Departure)
                         .addGeo("PL", type::TaxPointTag::Arrival)
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("PL", type::TaxPointTag::Arrival)
                         .addGeo("PL", type::TaxPointTag::Departure)
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
                                         .addTaxPoint("US", type::TaxPointTag::Sale)
                                         .build());

    _mileageServiceMock->pushIndex(5).pushIndex(6).pushIndex(3).pushIndex(4).pushIndex(1).pushIndex(2);

    GeoPathProperties gpp;
    GeoPathPropertiesCalculator geoPropertiesCalculator(*request, *_mileageServiceMock);
    geoPropertiesCalculator.calculate(*itin, gpp);

    TurnaroundCalculator calculator(request->geoPaths()[0],
                                    *_mileageServiceMock,
                                    *gpp.taxPointsProperties,
                                    false,
                                    false);

    const Geo* turnaround = calculator.getTurnaroundPoint(*itin);
    CPPUNIT_ASSERT(turnaround);
    CPPUNIT_ASSERT_EQUAL(static_cast<type::Index>(1), turnaround->id());
  }

  void testCalculate_turnaroundSpecialUS_RTOJLogic()
  {

    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(12, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(17, 00))
                   .setArrivalTime(type::Time(18, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(19, 00))
                   .setArrivalTime(type::Time(20, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f4(FlightBuilder()
                   .setDepartureTime(type::Time(21, 00))
                   .setArrivalTime(type::Time(22, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).setConnectionDateShift(1).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).setConnectionDateShift(1).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).setConnectionDateShift(1).build());
    FlightUsage* fu4(FlightUsageBuilder().setFlight(f4).setConnectionDateShift(1).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("CA", type::TaxPointTag::Arrival)
                         .addGeo("CA", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .addGeo("US", type::TaxPointTag::Departure)
                         .addGeo("US", type::TaxPointTag::Arrival)
                         .addGeo("MX", type::TaxPointTag::Departure)
                         .addGeo("PL", type::TaxPointTag::Arrival)
                         .build());
    setLoc(geoPath->geos()[5], "DFW");
    setLoc(geoPath->geos()[6], "MEX");

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

    _mileageServiceMock->pushIndex(6).pushIndex(5).pushIndex(3).pushIndex(4).pushIndex(1).pushIndex(2);

    GeoPathProperties gpp;
    GeoPathPropertiesCalculator geoPropertiesCalculator(*request, *_mileageServiceMock);
    geoPropertiesCalculator.calculate(*itin, gpp);

    TurnaroundCalculator calculator(request->geoPaths()[0],
                                    *_mileageServiceMock,
                                    *gpp.taxPointsProperties,
                                    true,
                                    false);

    const Geo* turnaround = calculator.getTurnaroundPoint(*itin);
    CPPUNIT_ASSERT(turnaround);
    CPPUNIT_ASSERT_EQUAL(static_cast<type::Index>(5), turnaround->id());
  }

  void testCalculate_turnaroundAlternateDeterminationLogicOff()
  {

    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(12, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(17, 00))
                   .setArrivalTime(type::Time(18, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(19, 00))
                   .setArrivalTime(type::Time(20, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f4(FlightBuilder()
                   .setDepartureTime(type::Time(21, 00))
                   .setArrivalTime(type::Time(22, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).setConnectionDateShift(0).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).setConnectionDateShift(0).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).setConnectionDateShift(10).build());
    FlightUsage* fu4(FlightUsageBuilder().setFlight(f4).setConnectionDateShift(0).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("JP", type::TaxPointTag::Departure)
                         .addGeo("DE", type::TaxPointTag::Arrival)
                         .addGeo("DE", type::TaxPointTag::Departure)
                         .addGeo("PL", type::TaxPointTag::Arrival)
                         .addGeo("DE", type::TaxPointTag::Departure)
                         .addGeo("DE", type::TaxPointTag::Arrival)
                         .addGeo("DE", type::TaxPointTag::Departure)
                         .addGeo("JP", type::TaxPointTag::Arrival)
                         .build());
    setLoc(geoPath->geos()[0], "TYO");
    setLoc(geoPath->geos()[1], "FRA");
    setLoc(geoPath->geos()[2], "FRA");
    setLoc(geoPath->geos()[3], "GDA");
    setLoc(geoPath->geos()[4], "BRE");
    setLoc(geoPath->geos()[5], "FRA");
    setLoc(geoPath->geos()[6], "FRA");
    setLoc(geoPath->geos()[7], "TYO");

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
                                       .addMap(1, 6)
                                       .addMap(1, 7)
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

    _mileageServiceMock->pushIndex(5).pushIndex(6).pushIndex(1).pushIndex(2).pushIndex(4).pushIndex(3);

    GeoPathProperties gpp;
    GeoPathPropertiesCalculator geoPropertiesCalculator(*request, *_mileageServiceMock);
    geoPropertiesCalculator.calculate(*itin, gpp);

    TurnaroundCalculator calculator(request->geoPaths()[0],
                                    *_mileageServiceMock,
                                    *gpp.taxPointsProperties,
                                    false,
                                    false);

    const Geo* turnaround = calculator.getTurnaroundPoint(*itin);
    CPPUNIT_ASSERT(turnaround);
    CPPUNIT_ASSERT_EQUAL(static_cast<type::Index>(5), turnaround->id());
  }

  void testCalculate_turnaroundAlternateDeterminationLogicOn_StopoverPresent()
  {

    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(12, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(17, 00))
                   .setArrivalTime(type::Time(18, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(19, 00))
                   .setArrivalTime(type::Time(20, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f4(FlightBuilder()
                   .setDepartureTime(type::Time(21, 00))
                   .setArrivalTime(type::Time(22, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).setConnectionDateShift(0).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).setConnectionDateShift(0).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).setConnectionDateShift(10).build());
    FlightUsage* fu4(FlightUsageBuilder().setFlight(f4).setConnectionDateShift(0).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("JP", type::TaxPointTag::Departure)
                         .addGeo("DE", type::TaxPointTag::Arrival)
                         .addGeo("DE", type::TaxPointTag::Departure)
                         .addGeo("PL", type::TaxPointTag::Arrival)
                         .addGeo("DE", type::TaxPointTag::Departure)
                         .addGeo("DE", type::TaxPointTag::Arrival)
                         .addGeo("DE", type::TaxPointTag::Departure)
                         .addGeo("JP", type::TaxPointTag::Arrival)
                         .build());
    setLoc(geoPath->geos()[0], "TYO");
    setLoc(geoPath->geos()[1], "FRA");
    setLoc(geoPath->geos()[2], "FRA");
    setLoc(geoPath->geos()[3], "GDA");
    setLoc(geoPath->geos()[4], "BRE");
    setLoc(geoPath->geos()[5], "FRA");
    setLoc(geoPath->geos()[6], "FRA");
    setLoc(geoPath->geos()[7], "TYO");

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
                                       .addMap(1, 6)
                                       .addMap(1, 7)
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

    _mileageServiceMock->pushIndex(5).pushIndex(6).pushIndex(1).pushIndex(2).pushIndex(4).pushIndex(3);

    GeoPathProperties gpp;
    GeoPathPropertiesCalculator geoPropertiesCalculator(*request, *_mileageServiceMock);
    geoPropertiesCalculator.calculate(*itin, gpp);

    TurnaroundCalculator calculator(request->geoPaths()[0],
                                    *_mileageServiceMock,
                                    *gpp.taxPointsProperties,
                                    false,
                                    true);

    const Geo* turnaround = calculator.getTurnaroundPoint(*itin);
    CPPUNIT_ASSERT(turnaround);
    CPPUNIT_ASSERT_EQUAL(static_cast<type::Index>(4), turnaround->id());
  }

  void testCalculate_turnaroundAlternateDeterminationLogicOn_NoStopover()
  {

    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(12, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(17, 00))
                   .setArrivalTime(type::Time(18, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(19, 00))
                   .setArrivalTime(type::Time(20, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f4(FlightBuilder()
                   .setDepartureTime(type::Time(21, 00))
                   .setArrivalTime(type::Time(22, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).setConnectionDateShift(0).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).setConnectionDateShift(0).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).setConnectionDateShift(0).build());
    FlightUsage* fu4(FlightUsageBuilder().setFlight(f4).setConnectionDateShift(0).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("JP", type::TaxPointTag::Departure)
                         .addGeo("DE", type::TaxPointTag::Arrival)
                         .addGeo("DE", type::TaxPointTag::Departure)
                         .addGeo("PL", type::TaxPointTag::Arrival)
                         .addGeo("PL", type::TaxPointTag::Departure)
                         .addGeo("DE", type::TaxPointTag::Arrival)
                         .addGeo("DE", type::TaxPointTag::Departure)
                         .addGeo("JP", type::TaxPointTag::Arrival)
                         .build());
    setLoc(geoPath->geos()[0], "TYO");
    setLoc(geoPath->geos()[1], "FRA");
    setLoc(geoPath->geos()[2], "FRA");
    setLoc(geoPath->geos()[3], "GDA");
    setLoc(geoPath->geos()[4], "GDA");
    setLoc(geoPath->geos()[5], "FRA");
    setLoc(geoPath->geos()[6], "FRA");
    setLoc(geoPath->geos()[7], "TYO");

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
                                       .addMap(1, 6)
                                       .addMap(1, 7)
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

    _mileageServiceMock->pushIndex(5).pushIndex(6).pushIndex(1).pushIndex(2).pushIndex(4).pushIndex(3);

    GeoPathProperties gpp;
    GeoPathPropertiesCalculator geoPropertiesCalculator(*request, *_mileageServiceMock);
    geoPropertiesCalculator.calculate(*itin, gpp);

    TurnaroundCalculator calculator(request->geoPaths()[0],
                                    *_mileageServiceMock,
                                    *gpp.taxPointsProperties,
                                    false,
                                    true);

    const Geo* turnaround = calculator.getTurnaroundPoint(*itin);
    CPPUNIT_ASSERT(turnaround);
    CPPUNIT_ASSERT_EQUAL(static_cast<type::Index>(5), turnaround->id());
  }

  void testCalculate_turnaroundRTW()
  {

    Flight* f1(FlightBuilder()
                   .setDepartureTime(type::Time(10, 00))
                   .setArrivalTime(type::Time(12, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f2(FlightBuilder()
                   .setDepartureTime(type::Time(17, 00))
                   .setArrivalTime(type::Time(18, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f3(FlightBuilder()
                   .setDepartureTime(type::Time(19, 00))
                   .setArrivalTime(type::Time(20, 00))
                   .setArrivalDateShift(0)
                   .build());
    Flight* f4(FlightBuilder()
                   .setDepartureTime(type::Time(21, 00))
                   .setArrivalTime(type::Time(22, 00))
                   .setArrivalDateShift(0)
                   .build());

    FlightUsage* fu1(FlightUsageBuilder().setFlight(f1).setConnectionDateShift(0).build());
    FlightUsage* fu2(FlightUsageBuilder().setFlight(f2).setConnectionDateShift(2).build());
    FlightUsage* fu3(FlightUsageBuilder().setFlight(f3).setConnectionDateShift(0).build());
    FlightUsage* fu4(FlightUsageBuilder().setFlight(f4).setConnectionDateShift(0).build());

    GeoPath* geoPath(GeoPathBuilder()
                         .addGeo("US", type::TaxPointTag::Departure)
                         .addGeo("CA", type::TaxPointTag::Arrival)
                         .addGeo("CA", type::TaxPointTag::Departure)
                         .addGeo("DE", type::TaxPointTag::Arrival)
                         .addGeo("DE", type::TaxPointTag::Departure)
                         .addGeo("JP", type::TaxPointTag::Arrival)
                         .addGeo("JP", type::TaxPointTag::Departure)
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
                                         .addTaxPoint("US", type::TaxPointTag::Sale)
                                         .build());

    _mileageServiceMock->pushIndex(5).pushIndex(6).pushIndex(3).pushIndex(4).pushIndex(1).pushIndex(2);
    _mileageServiceMock->setRtw(true);

    GeoPathProperties gpp;
    GeoPathPropertiesCalculator geoPropertiesCalculator(*request, *_mileageServiceMock);
    geoPropertiesCalculator.calculate(*itin, gpp);

    TurnaroundCalculator calculator(request->geoPaths()[0],
                                    *_mileageServiceMock,
                                    *gpp.taxPointsProperties,
                                    false,
                                    false);

    const Geo* turnaround = calculator.getTurnaroundPoint(*itin);
    CPPUNIT_ASSERT(turnaround);
    CPPUNIT_ASSERT_EQUAL(static_cast<type::Index>(5), turnaround->id());
  }

private:
  void setLoc(Geo& geo, const char* loc)
  {
    codeFromString(loc, geo.loc().code());
    codeFromString(loc, geo.loc().cityCode());
  }

  std::unique_ptr<MileageServiceMock> _mileageServiceMock;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TurnaroundCalculatorTest);
} // namespace tax
