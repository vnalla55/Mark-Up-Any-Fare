// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Common/SafeEnumToString.h"
#include "Common/Timestamp.h"
#include "DataModel/Common/CodeIO.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FarePath.h"
#include "DomainDataObjects/FareUsage.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Mapping.h"
#include "DomainDataObjects/Request.h"
#include "Processor/RequestAnalyzer.h"
#include "Rules/RequestLogicError.h"
#include "ServiceInterfaces/LocService.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "test/LocServiceMock.h"
#include "test/ServicesMock.h"
#include "test/include/CppUnitHelperMacros.h"

#include <memory>
#include <stdexcept>

namespace tax
{

class RequestAnalyzerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RequestAnalyzerTest);

  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testEmptyRequest_noTicketingDate);
  CPPUNIT_TEST(testEmptyRequest_noPaymentCurrency);
  CPPUNIT_TEST(testRequest);

  CPPUNIT_TEST(testWrongFareUsageIndex);
  CPPUNIT_TEST(testFareUsageValue);

  CPPUNIT_TEST(testNoFares);

  CPPUNIT_TEST(testWrongItinIndex);

  CPPUNIT_TEST(testFlightUsagesGeoPathMismatch1);
  CPPUNIT_TEST(testFlightUsagesGeoPathMismatch2);

  CPPUNIT_TEST(testItin_WrongGeoPathRefId);
  CPPUNIT_TEST(testItin_GeoPathValue);

  CPPUNIT_TEST(testItin_WrongFarePathRefId);
  CPPUNIT_TEST(testItin_FarePathValue);

  CPPUNIT_TEST(testItin_YqYrPathRefIdNotPresent);
  CPPUNIT_TEST(testItin_WrongYqYrPathRefId);
  CPPUNIT_TEST(testItin_YqYrPathValue);

  CPPUNIT_TEST(testItin_WrongFarePathGeoPathMappingRefId);
  CPPUNIT_TEST(testItin_GeoPathMappingIndexValue);

  CPPUNIT_TEST(testItin_WrongMapIndex);

  CPPUNIT_TEST(testItin_MappingsNumNotEqualToFareUsages);

  CPPUNIT_TEST(testItin_FareUsageWrongFlightRefId);
  CPPUNIT_TEST(testItin_FareUsageFlightValue);

  CPPUNIT_TEST(testInitData);
  CPPUNIT_TEST(testGeoPathInitData);
  CPPUNIT_TEST(testInitDates);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _request.reset(new Request);
    _request->ticketingOptions().ticketingDate() = type::Date(2013, 7, 16);
    _request->ticketingOptions().ticketingTime() = type::Time(13, 46);
    _request->ticketingOptions().paymentCurrency() = type::CurrencyCode("USD");

    auto locServiceMock = std::make_unique<LocServiceMock>();
    _locServiceMock = locServiceMock.get();
    _servicesMock.reset(new ServicesMock);
    _servicesMock->setLocService(locServiceMock.release());
    _servicesMock->setFallbackService(new FallbackServiceServer());

    _dummyNation.reset(new type::Nation("__"));
  }

  void tearDown() {}

  void testConstructor() { CPPUNIT_ASSERT_NO_THROW(RequestAnalyzer(*_request, *_servicesMock)); }

  void testEmptyRequest_noTicketingDate()
  {
    createRequest();
    _locServiceMock->setNation(*_dummyNation);
    _request->ticketingOptions().ticketingDate() = type::Date::blank_date();

    analyzeRequestThrow();
  }

  void testEmptyRequest_noPaymentCurrency()
  {
    createRequest();
    _locServiceMock->setNation(*_dummyNation);
    _request->ticketingOptions().paymentCurrency() = type::CurrencyCode();

    analyzeRequestThrow();
  }

  void testRequest()
  {
    createRequest();
    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestNoThrow();
  }

  void testWrongFareUsageIndex()
  {
    createRequest();
    _request->farePaths()[1].fareUsages()[1].index() = 10;
    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestThrow();
  }

  void testFareUsageValue()
  {
    createRequest();
    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestNoThrow();

    FareUsage& fareUsage = _request->farePaths()[1].fareUsages()[1];
    CPPUNIT_ASSERT(fareUsage.fare() == &_request->fares()[fareUsage.index()]);
  }

  void testNoFares()
  {
    createRequest();
    _request->fares().clear();
    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestThrow();
  }

  void testWrongItinIndex()
  {
    createRequest();
    _request->allItins()[0].id() = 1;

    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestThrow();
  }

  void testFlightUsagesGeoPathMismatch1()
  {
    createRequest();
    _request->allItins()[0].flightUsages().pop_back();

    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestThrow();
  }

  void testFlightUsagesGeoPathMismatch2()
  {
    createRequest();
    _request->allItins()[0].flightUsages().push_back(FlightUsage());

    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestThrow();
  }

  void testItin_WrongGeoPathRefId()
  {
    createRequest();
    _request->allItins()[0].geoPathRefId() = 10;

    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestThrow();
  }

  void testItin_GeoPathValue()
  {
    createRequest();
    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestNoThrow();

    Itin& itin = _request->allItins()[0];
    const GeoPath* geoPath = &_request->geoPaths()[itin.geoPathRefId()];
    CPPUNIT_ASSERT_EQUAL(itin.geoPath(), geoPath);
  }

  void testItin_WrongFarePathRefId()
  {
    createRequest();
    _request->allItins()[0].farePathRefId() = 10;

    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestThrow();
  }

  void testItin_FarePathValue()
  {
    createRequest();
    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestNoThrow();

    Itin& itin = _request->allItins()[0];
    const FarePath* farePath = &_request->farePaths()[itin.farePathRefId()];
    CPPUNIT_ASSERT_EQUAL(itin.farePath(), farePath);
  }

  void testItin_YqYrPathRefIdNotPresent()
  {
    createRequest();
    _request->allItins()[0].yqYrPathRefId() = std::numeric_limits<type::Index>::max();

    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestNoThrow();
  }

  void testItin_WrongYqYrPathRefId()
  {
    createRequest();
    _request->allItins()[0].yqYrPathRefId() = 10;

    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestThrow();
  }

  void testItin_YqYrPathValue()
  {
    createRequest();
    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestNoThrow();

    Itin& itin = _request->allItins()[0];
    CPPUNIT_ASSERT(itin.yqYrPathRefId().has_value());
    const YqYrPath* yqYrPath = &_request->yqYrPaths()[itin.yqYrPathRefId().value()];
    CPPUNIT_ASSERT_EQUAL(itin.yqYrPath(), yqYrPath);
  }

  void testItin_WrongFarePathGeoPathMappingRefId()
  {
    createRequest();
    _request->allItins()[0].farePathGeoPathMappingRefId() = 10;

    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestThrow();
  }

  void testItin_GeoPathMappingIndexValue()
  {
    createRequest();
    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestNoThrow();

    Itin& itin = _request->allItins()[0];
    CPPUNIT_ASSERT(itin.farePathGeoPathMappingRefId().has_value());
    const GeoPathMapping* geoPathMapping =
        &_request->geoPathMappings()[itin.farePathGeoPathMappingRefId().value()];
    CPPUNIT_ASSERT_EQUAL(itin.geoPathMapping(), geoPathMapping);
  }

  void testItin_WrongMapIndex()
  {
    createRequest();

    CPPUNIT_ASSERT(_request->allItins()[0].farePathGeoPathMappingRefId().has_value());
    const type::Index index = _request->allItins()[0].farePathGeoPathMappingRefId().value();
    _request->geoPathMappings()[index].mappings()[0].maps()[0].index() = 10;

    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestThrow();
  }

  void testItin_MappingsNumNotEqualToFareUsages()
  {
    createRequest();

    std::vector<FareUsage>& fareUsages =
        _request->farePaths()[_request->allItins()[0].farePathRefId()].fareUsages();
    fareUsages.push_back(FareUsage());
    fareUsages.back().index() = 0;

    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestThrow();
  }

  void testItin_FareUsageWrongFlightRefId()
  {
    createRequest();
    _request->allItins()[1].flightUsages()[1].flightRefId() =
        type::Index(_request->flights().size() + 10);

    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestThrow();
  }

  void testItin_FareUsageFlightValue()
  {
    createRequest();
    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestNoThrow();

    const FlightUsage& flightUsage = _request->allItins()[1].flightUsages()[1];
    const Flight* flight = &_request->flights()[flightUsage.flightRefId()];
    CPPUNIT_ASSERT_EQUAL(flightUsage.flight(), flight);
  }

  void testInitData()
  {
    createRequest();

    _request->pointsOfSale().push_back(PointOfSale());
    _request->pointsOfSale().back().loc() = type::AirportCode("AAA");

    type::Nation nation = "BX";
    _locServiceMock->setNation(nation);

    analyzeRequestNoThrow();

    CPPUNIT_ASSERT_EQUAL(_request->pointsOfSale()[0].loc(),
                         _request->posTaxPoints()[0].loc().code());
    CPPUNIT_ASSERT_EQUAL(nation, _request->posTaxPoints()[0].loc().nation());
    CPPUNIT_ASSERT_EQUAL(type::TaxPointTag(type::TaxPointTag::Sale),
                         _request->posTaxPoints()[0].loc().tag());
    CPPUNIT_ASSERT_EQUAL(type::UnticketedTransfer(type::UnticketedTransfer::No),
                         _request->posTaxPoints()[0].unticketedTransfer());
    CPPUNIT_ASSERT_EQUAL(type::Index(0), _request->posTaxPoints()[0].id());
  }

  void testGeoPathInitData()
  {
    createRequest();

    type::Nation nation = "CX";
    _locServiceMock->setNation(nation);

    type::CityCode cityCode = "DDD";
    _locServiceMock->setCityCode(cityCode);

    analyzeRequestNoThrow();

    CPPUNIT_ASSERT_EQUAL(type::Index(2), _request->geoPaths()[2].id());

    const std::vector<Geo>& geos = _request->geoPaths()[1].geos();

    CPPUNIT_ASSERT_EQUAL(type::Index(2), geos[2].id());
    CPPUNIT_ASSERT_EQUAL(nation, geos[2].loc().nation());
    CPPUNIT_ASSERT_EQUAL(cityCode, geos[2].loc().cityCode());

    const Geo* geo = &geos[0];
    CPPUNIT_ASSERT_EQUAL(geo, geos[1].prev());

    geo = &geos[2];
    CPPUNIT_ASSERT_EQUAL(geo, geos[1].next());

    CPPUNIT_ASSERT(geos[geos.size() - 1].isLast());
  }

  void testInitDates()
  {
    createRequest();
    _locServiceMock->setNation(*_dummyNation);
    analyzeRequestNoThrow();

    CPPUNIT_ASSERT(type::Date::blank_date() !=
                   _request->allItins()[1].flightUsages()[1].departureDate());
  }

private:
  std::unique_ptr<Request> _request;
  std::unique_ptr<ServicesMock> _servicesMock;
  LocServiceMock* _locServiceMock;
  std::unique_ptr<type::Nation> _dummyNation;

  void incrementIndex(const type::Index& maxValue, type::Index& index)
  {
    index = (index < maxValue - 1) ? (index + 1) : 0;
  }

  void createRequest()
  {
    // create fares
    const uint32_t faresNum = 3;
    _request->fares().resize(faresNum);

    // create farePaths, fareUsages
    type::Index fareIndex = 0;
    const uint32_t farePathsNum = 3;
    const uint32_t fareUsagesNum = 3;

    _request->pointsOfSale().push_back(PointOfSale());

    _request->farePaths().resize(farePathsNum);
    for (uint32_t i = 0; i < farePathsNum; i++)
    {
      FarePath& farePath = _request->farePaths()[i];

      farePath.fareUsages().resize(fareUsagesNum);
      for (uint32_t j = 0; j < fareUsagesNum; j++)
      {
        FareUsage& fareUsage = farePath.fareUsages()[j];

        incrementIndex(faresNum, fareIndex);
        fareUsage.index() = fareIndex;
      }
    }

    // create passenger
    const uint32_t passengersNum = 3;
    for (uint32_t i = 0; i < passengersNum; i++)
    {
      _request->passengers().push_back(Passenger());
    }

    // YqYrPaths
    const uint32_t yqYrPathsNum = 3;
    _request->yqYrPaths().resize(yqYrPathsNum);

    // create geoPaths, geos
    const uint32_t geoPathsNum = 3;
    const uint32_t geosNum = 4;
    _request->geoPaths().resize(geoPathsNum);
    for (uint32_t i = 0; i < geoPathsNum; i++)
    {
      for (uint32_t j = 0; j < geosNum; j++)
      {
        _request->geoPaths()[i].geos().push_back(Geo());
      }
    }

    // create geoPathMappings, mappings, maps
    type::Index mapIndex = 0;
    const uint32_t geoPathMappingsNum = 3;
    _request->geoPathMappings().resize(geoPathMappingsNum);
    for (uint32_t i = 0; i < geoPathMappingsNum; i++)
    {
      const uint32_t mappingsNum = fareUsagesNum;
      for (uint32_t j = 0; j < mappingsNum; j++)
      {
        _request->geoPathMappings()[i].mappings().push_back(Mapping());

        const uint32_t mapsNum = 4;
        for (uint32_t k = 0; k < mapsNum; k++)
        {
          _request->geoPathMappings()[i].mappings()[j].maps().push_back(Map());

          incrementIndex(geosNum, mapIndex);
          _request->geoPathMappings()[i].mappings()[j].maps()[k].index() = mapIndex;
        }
      }
    }

    // create flights
    const uint32_t flightsNum = 5;
    _request->flights().resize(flightsNum);

    // create itins
    type::Index geoPathRefId = 0;
    type::Index farePathRefId = 0;
    type::Index yqYrPathRefId = 0;
    type::Index farePathGeoPathMappingRefId = 0;
    type::Index flightRefId = 0;
    type::Index passengerRefId = 0;
    const uint32_t itinsNum = 2;

    for (uint32_t i = 0; i < itinsNum; i++)
    {
      _request->allItins().push_back(Itin());
      _request->itins().push_back(&_request->allItins().back());
      Itin& itin = _request->allItins()[i];

      itin.id() = i;

      incrementIndex(geoPathsNum, geoPathRefId);
      itin.geoPathRefId() = geoPathRefId;

      incrementIndex(farePathsNum, farePathRefId);
      itin.farePathRefId() = farePathRefId;

      incrementIndex(yqYrPathsNum, yqYrPathRefId);
      itin.yqYrPathRefId() = yqYrPathRefId;

      incrementIndex(geoPathMappingsNum, farePathGeoPathMappingRefId);
      itin.farePathGeoPathMappingRefId() = farePathGeoPathMappingRefId;

      incrementIndex(passengersNum, passengerRefId);
      itin.passengerRefId() = passengerRefId;

      // create flightUsages
      itin.flightUsages().clear();
      itin.travelOriginDate() = type::Date(2013, 7, 16);
      const uint32_t flightUsageNum = 2;
      for (uint32_t i = 0; i < flightUsageNum; i++)
      {
        itin.flightUsages().push_back(FlightUsage());
        FlightUsage& flightUsage = itin.flightUsages()[i];

        incrementIndex(flightsNum, flightRefId);
        flightUsage.flightRefId() = flightRefId;
      }
    }
  }

  void analyzeRequestNoThrow()
  {
    RequestAnalyzer requestAnalyzer(*_request, *_servicesMock);
    CPPUNIT_ASSERT_NO_THROW(requestAnalyzer.analyze());
  }

  void analyzeRequestThrow()
  {
    RequestAnalyzer requestAnalyzer(*_request, *_servicesMock);
    CPPUNIT_ASSERT_THROW(requestAnalyzer.analyze(), RequestLogicError);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RequestAnalyzerTest);
} // namespace tax
