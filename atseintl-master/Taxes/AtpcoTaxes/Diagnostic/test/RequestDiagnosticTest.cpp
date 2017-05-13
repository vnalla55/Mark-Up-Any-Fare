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
#include <boost/lexical_cast.hpp>

#include "test/include/CppUnitHelperMacros.h"

#include "Diagnostic/RequestDiagnostic.h"
#include "DomainDataObjects/Request.h"
#include "DomainDataObjects/PointOfSale.h"

#include <memory>

namespace tax
{

class RequestDiagnosticTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RequestDiagnosticTest);

  CPPUNIT_TEST(testTaxPointsPrinter);
  CPPUNIT_TEST(testFarePathsPrinter);
  CPPUNIT_TEST(testOptionalServicesPrinter);
  CPPUNIT_TEST(testBaggagePrinter);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _request.reset(new Request);
    _parameters.clear();

    _flight = new Flight();

    _requestDiagnostic.reset(new RequestDiagnostic(*_request, _parameters));
  }

  void tearDown() { delete _flight; }

  void testTaxPointsPrinter()
  {
    fillInPointOfSale();
    fillInTicketingDate();
    fillInGeoPaths();
    addItin();
    _request->allItins()[0].id() = 0;
    fillInFlightUsages(_request->allItins().back());

    _parameters.push_back(new Parameter);
    Parameter& parameter = _parameters.back();
    parameter.name() = "TP";

    _requestDiagnostic->createMessages(_messages);

    std::string messages;
    for (uint32_t i = 0; i < _messages.size(); i++)
    {
      messages.append(_messages[i]._content);
    }

    std::string expected[] = { "***************************************************************",
                               "            ----  DIAGNOSTIC 830 - REQUEST INFO ----           ",
                               "***************************************************************",
                               "---------------------- TAX POINT ANALYSIS ---------------------",
                               "            ----       POINT OF SALE INFO       ----           ",
                               "",
                               "    POS  NATION     DATE   ",
                               "    KRK    PL    2013-04-01",
                               "",
                               "            ----         ITINERARY INFO         ----           ",
                               "            ----          ITINERARY: 1          ----           ",
                               "",
                               "    TXPT  TXPT TYPE NATION HIDDEN    DATE     TIME",
                               "  1  LC1  DEPARTURE   N1      X   2013-04-02 12:50",
                               "  2  LC2   ARRIVAL    N2          2013-04-02 20:20",
                               "  3  LC1  DEPARTURE   N1      X      OPEN     OPEN",
                               "  4  LC2   ARRIVAL    N2             OPEN     OPEN",
                               "",
                               "---------------------------------------------------------------",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());

    for (uint32_t i = 0; i < sizeof(expected) / sizeof(std::string); i++)
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + boost::lexical_cast<std::string>(i + 1),
                                   expected[i],
                                   _messages[i]._content);
  }

  void testFarePathsPrinter()
  {
    fillInGeoPaths();
    fillInFares();
    fillInFarePaths();
    fillInGeoPathMappings();
    addItin();
    _request->allItins()[0].id() = 0;
    _request->allItins()[0].geoPathRefId() = 0;
    _request->allItins()[0].farePathRefId() = 0;
    _request->allItins()[0].farePathGeoPathMappingRefId() = 0;
    Itin& itin = _request->allItins()[0];
    itin.geoPathMapping() = &_request->geoPathMappings()[itin.farePathGeoPathMappingRefId().value()];

    _parameters.push_back(new Parameter);
    Parameter& parameter = _parameters.back();
    parameter.name() = "FP";

    _requestDiagnostic->createMessages(_messages);

    std::string messages;
    for (uint32_t i = 0; i < _messages.size(); i++)
    {
      messages.append(_messages[i]._content);
    }

    std::string expected[] = { "***************************************************************",
                               "            ----  DIAGNOSTIC 830 - REQUEST INFO ----           ",
                               "***************************************************************",
                               "---------------------- FARE PATH ANALYSIS ---------------------",
                               "            ----            ITIN: 1             ----           ",
                               "",
                               "VALIDATING CARRIER: LO",
                               "TOTAL AMOUNT: 300",
                               "",
                               "     BASIS  TYPE ONEWAY/RT DIRECTIONALITY AMOUNT",
                               "",
                               "  1  FAREMARKET: LC1-LC2",
                               "      X2RE   EU      R            5         100 ",
                               "",
                               "  2  FAREMARKET: LC1-LC2",
                               "      Y2RE   US      O            5         200 ",
                               "",
                               "---------------------------------------------------------------",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());

    for (uint32_t i = 0; i < sizeof(expected) / sizeof(std::string); i++)
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + boost::lexical_cast<std::string>(i + 1),
                                   expected[i],
                                   _messages[i]._content);
  }

  void testOptionalServicesPrinter()
  {
    fillInGeoPaths();
    fillInOptionalServices();
    fillInOptionalServicePaths();
    fillInGeoPathMappings();
    addItin();
    _request->allItins()[0].id() = 0;
    _request->allItins()[0].geoPathRefId() = 0;
    _request->allItins()[0].optionalServicePathRefId() = 0;
    _request->allItins()[0].optionalServicePathGeoPathMappingRefId() = 0;

    _parameters.push_back(new Parameter);
    Parameter& parameter = _parameters.back();
    parameter.name() = "OC";

    _requestDiagnostic->createMessages(_messages);

    std::string messages;
    for (uint32_t i = 0; i < _messages.size(); i++)
    {
      messages.append(_messages[i]._content);
    }

    std::string expected[] = { "***************************************************************",
                               "            ----  DIAGNOSTIC 830 - REQUEST INFO ----           ",
                               "***************************************************************",
                               "------------------ OPTIONAL SERVICES ANALYSIS -----------------",
                               "            ----            ITIN: 1             ----           ",
                               "",
                               "    TYPE SVCSUBTYPECODE SVCGROUP SVCSUBGROUP CARRIER AMOUNT",
                               "",
                               "  1  MARKET: LC1-LC2",
                               "      F        1GG         LM         ML        LO     100 ",
                               "",
                               "---------------------------------------------------------------",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());

    for (uint32_t i = 0; i < sizeof(expected) / sizeof(std::string); i++)
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + boost::lexical_cast<std::string>(i + 1),
                                   expected[i],
                                   _messages[i]._content);
  }

  void testBaggagePrinter()
  {
    fillInGeoPaths();
    fillInOptionalServices();
    fillInOptionalServicePaths();
    fillInGeoPathMappings();
    addItin();
    _request->allItins()[0].id() = 0;
    _request->allItins()[0].geoPathRefId() = 0;
    _request->allItins()[0].optionalServicePathRefId() = 0;
    _request->allItins()[0].optionalServicePathGeoPathMappingRefId() = 0;

    _parameters.push_back(new Parameter);
    Parameter& parameter = _parameters.back();
    parameter.name() = "BG";

    _requestDiagnostic->createMessages(_messages);

    std::string messages;
    for (uint32_t i = 0; i < _messages.size(); i++)
    {
      messages.append(_messages[i]._content);
    }

    std::string expected[] = { "***************************************************************",
                               "            ----  DIAGNOSTIC 830 - REQUEST INFO ----           ",
                               "***************************************************************",
                               "----------------------- BAGGAGE ANALYSIS ----------------------",
                               "            ----            ITIN: 1             ----           ",
                               "",
                               "    TYPE SVCSUBTYPECODE SVCGROUP SVCSUBGROUP CARRIER AMOUNT",
                               "",
                               "  1  MARKET: LC1-LC2",
                               "      C        1GG         BG         BB        XC     200 ",
                               "",
                               "---------------------------------------------------------------",
                               "" };

    CPPUNIT_ASSERT_EQUAL(sizeof(expected) / sizeof(std::string), _messages.size());

    for (uint32_t i = 0; i < sizeof(expected) / sizeof(std::string); i++)
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Line number " + boost::lexical_cast<std::string>(i + 1),
                                   expected[i],
                                   _messages[i]._content);
  }

private:
  std::unique_ptr<Request> _request;
  boost::ptr_vector<Parameter> _parameters;

  Flight* _flight;

  std::unique_ptr<RequestDiagnostic> _requestDiagnostic;
  boost::ptr_vector<Message> _messages;

  void fillInPointOfSale()
  {
    _request->pointsOfSale().push_back(PointOfSale());
    _request->pointsOfSale().back().loc() = "KRK";

    Geo posTaxPoint;
    posTaxPoint.loc().nation() = "PL";
    _request->posTaxPoints().push_back(posTaxPoint);
  }

  void fillInTicketingDate()
  {
    TicketingOptions& ticketing = _request->ticketingOptions();
    ticketing.ticketingDate() = type::Date(2013, 4, 1);
  }

  void fillInGeoPaths()
  {
    std::vector<GeoPath>& geoPaths = _request->geoPaths();
    geoPaths.push_back(GeoPath());

    fillInGeoPath();
  }

  void fillInGeoPath()
  {
    fillInUnticketedGeo(0);
    fillInTicketedGeo(1);
    fillInUnticketedGeo(2);
    fillInTicketedGeo(3);
  }

  void fillInUnticketedGeo(const type::Index& id)
  {
    GeoPath& geoPath = _request->geoPaths().back();
    geoPath.geos().push_back(Geo());
    Geo& geo1 = geoPath.geos().back();
    geo1.loc().tag() = type::TaxPointTag::Departure;
    geo1.unticketedTransfer() = type::UnticketedTransfer::Yes;
    geo1.id() = id;
    geo1.loc().code() = "LC1";
    geo1.loc().nation() = "N1";
  }

  void fillInTicketedGeo(const type::Index& id)
  {
    GeoPath& geoPath = _request->geoPaths().back();
    geoPath.geos().push_back(Geo());
    Geo& geo2 = geoPath.geos().back();
    geo2.loc().tag() = type::TaxPointTag::Arrival;
    geo2.unticketedTransfer() = type::UnticketedTransfer::No;
    geo2.id() = id;
    geo2.loc().code() = "LC2";
    geo2.loc().nation() = "N2";
  }

  void fillInFlightUsages(Itin& itin)
  {
    itin.flightUsages().push_back(FlightUsage());
    FlightUsage& flightUsage1 = itin.flightUsages().back();
    flightUsage1.flight() = _flight;
    flightUsage1.markDepartureDate(type::Date(2013, 4, 2));
    flightUsage1.openSegmentIndicator() = type::OpenSegmentIndicator::Fixed;
    _flight->departureTime() = type::Time(12, 50);
    _flight->arrivalTime() = type::Time(20, 20);

    itin.flightUsages().push_back(FlightUsage());
    FlightUsage& flightUsage2 = itin.flightUsages().back();
    flightUsage2.flight() = _flight;
    flightUsage2.openSegmentIndicator() =
        type::OpenSegmentIndicator::Open; // TODO: add DateFixed
  }

  void fillInFarePaths()
  {
    _request->farePaths().push_back(FarePath());
    FarePath& farePath = _request->farePaths().back();
    farePath.validatingCarrier() = "LO";
    farePath.totalAmount() = 300;
    farePath.fareUsages().resize(2);
    farePath.fareUsages()[0].index() = 0;
    farePath.fareUsages()[1].index() = 1;
  }

  void addItin()
  {
    std::vector<Itin>& itins = _request->allItins();
    itins.push_back(Itin());
    _request->itins().push_back(&itins.back());
  }

  void fillInGeoPathMappings()
  {
    _request->geoPathMappings().push_back(GeoPathMapping());
    GeoPathMapping& mapping = _request->geoPathMappings().back();

    mapping.mappings().push_back(Mapping());
    mapping.mappings().back().maps().resize(2);
    mapping.mappings().back().maps()[0].index() = 0;
    mapping.mappings().back().maps()[1].index() = 1;

    mapping.mappings().push_back(Mapping());
    mapping.mappings().back().maps().resize(2);
    mapping.mappings().back().maps()[0].index() = 2;
    mapping.mappings().back().maps()[1].index() = 3;
  }

  void fillInFares()
  {
    _request->fares().push_back(Fare());
    _request->fares().back().amount() = 100;
    _request->fares().back().basis() = "X2RE";
    _request->fares().back().directionality() = type::Directionality::Both;
    _request->fares().back().oneWayRoundTrip() = 'R';
    _request->fares().back().type() = "EU";

    _request->fares().push_back(Fare());
    _request->fares().back().amount() = 200;
    _request->fares().back().basis() = "Y2RE";
    _request->fares().back().directionality() = type::Directionality::Both;
    _request->fares().back().oneWayRoundTrip() = 'O';
    _request->fares().back().type() = "US";
  }

  void fillInOptionalServicePaths()
  {
    _request->optionalServicePaths().push_back(OptionalServicePath());
    OptionalServicePath& optionalServicePath = _request->optionalServicePaths().back();
    optionalServicePath.optionalServiceUsages().resize(2);
    optionalServicePath.optionalServiceUsages()[0].index() = 0;
    optionalServicePath.optionalServiceUsages()[1].index() = 1;
  }

  void fillInOptionalServices()
  {
    _request->optionalServices().push_back(OptionalService());
    _request->optionalServices().back().amount() = 100;
    _request->optionalServices().back().subCode() = "1GG";
    _request->optionalServices().back().serviceGroup() = "LM";
    _request->optionalServices().back().serviceSubGroup() = "ML";
    _request->optionalServices().back().ownerCarrier() = "LO";
    _request->optionalServices().back().type() = type::OptionalServiceTag::FlightRelated;

    _request->optionalServices().push_back(OptionalService());
    _request->optionalServices().back().amount() = 200;
    _request->optionalServices().back().subCode() = "1GG";
    _request->optionalServices().back().serviceGroup() = "BG";
    _request->optionalServices().back().serviceSubGroup() = "BB";
    _request->optionalServices().back().ownerCarrier() = "XC";
    _request->optionalServices().back().type() = type::OptionalServiceTag::BaggageCharge;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RequestDiagnosticTest);

} // namespace tse
