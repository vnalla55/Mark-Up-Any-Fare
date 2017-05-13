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

#include "Taxes/TestServer/Xform/XmlParser.h"
#include "Taxes/TestServer/Xform/SelectTagsList.h"
#include "Taxes/TestServer/Xform/XmlTagsFactory.h"
#include "Taxes/TestServer/Xform/NaturalXmlTagsList.h"
#include "Taxes/TestServer/Xform/XmlTagsFactory.h"
#include "Taxes/AtpcoTaxes/Common/Consts.h"
#include "Taxes/AtpcoTaxes/Common/SafeEnumToString.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/CodeIO.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/Request.h"
#include "Taxes/AtpcoTaxes/DomainDataObjects/XmlCache.h"
#include "Taxes/AtpcoTaxes/Rules/XmlParsingError.h"

#include <stdexcept>

namespace tax
{

class XmlParserTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XmlParserTest);

  CPPUNIT_TEST(testParseUnknownXMLElement);
  CPPUNIT_TEST(testParseUnknownXMLAttribute);

  CPPUNIT_TEST(testTicketingDate);
  CPPUNIT_TEST(testTicketingTime);

  CPPUNIT_TEST(testTravelDate);

  CPPUNIT_TEST(testPointOfSaleLoc);
  CPPUNIT_TEST(testPointOfSaleAgentAirlineDept_AgentOfficeDesignator);

  CPPUNIT_TEST(testGeoPathsNumber);
  CPPUNIT_TEST(testGeoPathGeosNumber);
  CPPUNIT_TEST(testGeoPathValues);

  CPPUNIT_TEST(testYqYrPaths);

  CPPUNIT_TEST(testFaresNumber);
  CPPUNIT_TEST(testFareValues);

  CPPUNIT_TEST(testFarePathsNumber);
  CPPUNIT_TEST(testFarePathValues);
  CPPUNIT_TEST(testFarePathFareUsageNumber);
  CPPUNIT_TEST(testFarePathFareUsageValues);

  CPPUNIT_TEST(testOptionalServicesNumber);
  CPPUNIT_TEST(testOptionalServiceValues);

  CPPUNIT_TEST(testOptionalServicePathsNumber);
  CPPUNIT_TEST(testOptionalServicePathValues);
  CPPUNIT_TEST(testOptionalServicePathOptionalServiceUsageNumber);
  CPPUNIT_TEST(testOptionalServicePathOptionalServiceUsageValues);

  CPPUNIT_TEST(testGeoPathMappingNumber);
  CPPUNIT_TEST(testGeoPathMappingValue);

  CPPUNIT_TEST(testFlightsNumber);
  CPPUNIT_TEST(testFlightValues);

  CPPUNIT_TEST(testItinsNumber);
  CPPUNIT_TEST(testItinValues);

  CPPUNIT_TEST(testDiagnostic);

  // Services
  CPPUNIT_TEST(testCacheRulesRecordNumber);
  CPPUNIT_TEST(testCacheTaxName);

  CPPUNIT_TEST(testCacheJourney);
  CPPUNIT_TEST(testCacheJourneyLoc1);
  CPPUNIT_TEST(testCacheJourneyZoneTblNo);

  CPPUNIT_TEST(testCacheTravelWhollyWithin);
  CPPUNIT_TEST(testCacheTravelWhollyWithinLoc);
  CPPUNIT_TEST(testCacheTravelWhollyWithinZoneTblNo);

  CPPUNIT_TEST(testCacheTxptType);
  CPPUNIT_TEST(testCacheTxptTypeLoc);
  CPPUNIT_TEST(testCacheTxptTypeZoneTblNo);

  CPPUNIT_TEST(testCacheSeqNo);
  CPPUNIT_TEST(testCacheVendor);
  CPPUNIT_TEST(testCacheCurrencyOfSale);
  CPPUNIT_TEST(testCacheTaxAmt);
  CPPUNIT_TEST(testCacheTaxCurrency);
  CPPUNIT_TEST(testCacheTaxCurDecimals);
  CPPUNIT_TEST(testCacheTaxPercent);

  CPPUNIT_TEST(testCacheRtnToOrig);
  CPPUNIT_TEST(testCacheExemptTag);

  CPPUNIT_TEST(testCacheDates);
  CPPUNIT_TEST(testCacheDatesFirstLast);

  CPPUNIT_TEST(testCacheTravelDateTag);
  CPPUNIT_TEST(testCacheTicketedPointTag);
  CPPUNIT_TEST(testCacheAlternateRuleRefTag);
  CPPUNIT_TEST(testCacheTaxMatchingApplTag);

  CPPUNIT_TEST(testHistoricSaleDates);
  CPPUNIT_TEST(testHistoricTrvlDates);

  CPPUNIT_TEST(testCacheLocNumber);
  CPPUNIT_TEST(testCacheLocValues);

  CPPUNIT_TEST(testCacheTaxPointLoc1TransferType);

  CPPUNIT_TEST(testCacheTaxPointLoc1);
  CPPUNIT_TEST(testCacheTaxPointLoc1Type);
  CPPUNIT_TEST(testCacheTaxPointLoc1Zone);
  CPPUNIT_TEST(testCacheTaxPointLoc2);
  CPPUNIT_TEST(testCacheTaxPointLoc2Type);
  CPPUNIT_TEST(testCacheTaxPointLoc2Zone);
  CPPUNIT_TEST(testCacheTaxPointIntlDomLoc2Ind);
  CPPUNIT_TEST(testCacheTaxPointLoc2Compare);

  CPPUNIT_TEST(testCacheMileageDistanceNumber);
  CPPUNIT_TEST(testCacheMileageDistanceValue);

  CPPUNIT_TEST(testCacheStopoverTime);

  CPPUNIT_TEST(testCacheCarrierFlightSegments);
  CPPUNIT_TEST(testCacheCarrierApplications);
  CPPUNIT_TEST(testCacheServiceBaggage);

  CPPUNIT_TEST(testCachePassengerTypeCodeItems);

  CPPUNIT_TEST(testCurrencyConversion);
  CPPUNIT_TEST(testCacheAKHIFactor);

  CPPUNIT_TEST(testCacheConnectionsTagsEmpty);
  CPPUNIT_TEST(testCacheConnectionsTagsValue);
  CPPUNIT_TEST(testCacheConnectionsTagsBlank);
  CPPUNIT_TEST(testCacheConnectionsTagsCount);

  CPPUNIT_TEST(testServiceFeeSecurityItems);
  CPPUNIT_TEST(testCacheReportingRecord);
  CPPUNIT_TEST(testCacheRepricingEntry);

  CPPUNIT_TEST(testTicketingFee);
  CPPUNIT_TEST(testTicketingFeePath);

  CPPUNIT_TEST(testCacheCustomer);

  CPPUNIT_TEST(testRulesRecord_SvcFeesSecurityItemNo);

  CPPUNIT_TEST_SUITE_END();

public:
  XmlParserTest()
  : _xmlTagsFactory(0)
  {
    _xmlRequestGeoPath = "<TaxRq>\
        <GeoPath>\
          <Geo Loc=\"AA1\" Type=\"C\" UnticketedTransfer=\"Y\"/>\
          <Geo Loc=\"AA2\" Type=\"D\" />\
        </GeoPath>\
        <GeoPath>\
          <Geo Loc=\"BB1\" Type=\"E\" />\
          <Geo Loc=\"BB2\" Type=\"F\" />\
          <Geo Loc=\"BB3\" Type=\"G\" UnticketedTransfer=\"N\"/>\
        </GeoPath>\
      </TaxRq>";

    _xmlRequestYqYrPath = "<TaxRq>\
        <YqYr Amount=\"13\" Code=\"YR\" Type=\"I\" />\
        <YqYr Amount=\"15\" Code=\"YQ\" Type=\"F\" />\
        <YqYr Amount=\"17\" Code=\"YR\" Type=\"I\" TaxIncluded=\"0\" />\
        <YqYr Amount=\"19\" Code=\"YQ\" Type=\"F\" TaxIncluded=\"1\" />\
        <YqYrPath>\
          <YqYrUsage YqYrRefId=\"0\" />\
          <YqYrUsage YqYrRefId=\"1\" />\
        </YqYrPath>\
        <YqYrPath>\
          <YqYrUsage YqYrRefId=\"2\" />\
          <YqYrUsage YqYrRefId=\"3\" />\
        </YqYrPath>\
      </TaxRq>";

    _xmlRequestFares = "<TaxRq>\
        <Fare Amount=\"50\" BasisCode=\"YX2RT\" Directionality=\"0\"\
          OneWayRoundTrip=\"O\" TypeCode=\"ER\" />\
        <Fare Amount=\"100\" SellAmount=\"90\" IsNetRemitAvailable=\"0\"\
          BasisCode=\"YX2OW\" Directionality=\"3\" OneWayRoundTrip=\"O\" TypeCode=\"EU\"/>\
        <Fare Amount=\"10\" SellAmount=\"9\" IsNetRemitAvailable=\"1\"\
          BasisCode=\"YXOW\" Directionality=\"3\" OneWayRoundTrip=\"X\" TypeCode=\"BR\" />\
      </TaxRq>";

    _xmlRequestFarePaths = "<TaxRq>\
        <FarePath TotalAmount=\"150\" TotalAmountBeforeDiscount=\"200\" ValidatingCarrier=\"LH\">\
          <FareUsage FareRefId=\"0\" />\
          <FareUsage FareRefId=\"1\" />\
          <FareUsage FareRefId=\"2\" />\
        </FarePath>\
        <FarePath TotalAmount=\"40\" TotalAmountBeforeDiscount=\"60\" ValidatingCarrier=\"BA\" OutputPassengerCode=\"CNN\">\
          <FareUsage FareRefId=\"3\" />\
          <FareUsage FareRefId=\"4\" />\
        </FarePath>\
      </TaxRq>";

    _xmlRequestOptionalServices = "<TaxRq>\
        <OptionalService Id=\"0\" Amount=\"50\" ServiceSubTypeCode=\"BGG\" Type=\"F\" OwnerCarrier=\"LO\" \
         PointOfDeliveryLoc=\"DAL\" />\
        <OptionalService Id=\"1\" Amount=\"30\" ServiceSubTypeCode=\"ACC\" Type=\"M\" OwnerCarrier=\"US\" \
         PointOfDeliveryLoc=\"KRK\" />\
      </TaxRq>";

    _xmlRequestOptionalServicePaths = "<TaxRq>\
        <OptionalServicePath Id=\"0\">\
          <OptionalServiceUsage OptionalServiceRefId=\"0\" />\
          <OptionalServiceUsage OptionalServiceRefId=\"1\" />\
          <OptionalServiceUsage OptionalServiceRefId=\"2\" />\
        </OptionalServicePath>\
      </TaxRq>";

    _xmlRequestGeoPathMapping = "<TaxRq><GeoPathMapping>\
        <Mapping>\
          <Map GeoRefId=\"0\" />\
          <Map GeoRefId=\"1\" />\
        </Mapping>\
        <Mapping>\
          <Map GeoRefId=\"2\" />\
          <Map GeoRefId=\"3\" />\
        </Mapping>\
      </GeoPathMapping></TaxRq>";

    _xmlRequestFlights = "<TaxRq>\
        <Flight MarketingCarrier=\"AA\" OperatingCarrier=\"DD\"\
          Equipment=\"ICE\" MarketingCarrierFlightNumber=\"99\"\
          ArrivalDateShift=\"1\" ArrivalTime=\"01:15\" DepartureTime=\"17:34\" />\
        <Flight MarketingCarrier=\"BB\" OperatingCarrier=\"EE\"\
          Equipment=\"TEE\" MarketingCarrierFlightNumber=\"100\"\
          ArrivalDateShift=\"0\" ArrivalTime=\"19:23\" DepartureTime=\"12:55\" />\
        <Flight MarketingCarrier=\"CC\" OperatingCarrier=\"FF\"\
          Equipment=\"TIC\" MarketingCarrierFlightNumber=\"101\"\
          ArrivalDateShift=\"1\" ArrivalTime=\"02:18\" DepartureTime=\"21:31\" />\
      </TaxRq>";

    _xmlRequestItins = "<TaxRq>\
        <FlightPath>\
          <FlightUsage ForcedConnection=\"X\" ConnectionDateShift=\"0\" FlightRefId=\"0\" />\
          <FlightUsage ConnectionDateShift=\"1\" FlightRefId=\"1\" />\
        </FlightPath>\
        <FlightPath>\
          <FlightUsage ForcedConnection=\"O\" ConnectionDateShift=\"0\" FlightRefId=\"3\" />\
          <FlightUsage ConnectionDateShift=\"4\" FlightRefId=\"4\" />\
          <FlightUsage ConnectionDateShift=\"6\" FlightRefId=\"6\" />\
        </FlightPath>\
        <Itin FarePathGeoPathMappingRefId=\"0\" FarePathRefId=\"0\" GeoPathRefId=\"0\" Id=\"0\"\
          TravelOriginDate=\"2013-03-12\" PassengerRefId=\"1\" FlightPathRefId=\"0\">\
        </Itin>\
        <Itin FarePathGeoPathMappingRefId=\"1\" FarePathRefId=\"2\" GeoPathRefId=\"1\" Id=\"1\"\
          TravelOriginDate=\"2013-04-12\" PassengerRefId=\"2\" FlightPathRefId=\"1\">\
        </Itin>\
      </TaxRq>";

    _xmlRequestCache = "<TaxRq><TestData>\
        <Loc Id=\"3\" LocCode=\"KRK\" NationCode=\"PL\" CityCode=\"ABC\" />\
        <Loc Id=\"5\" LocCode=\"MUC\" NationCode=\"DE\" CityCode=\"DEF\"\
          AlaskaZone=\"B\" State=\"USTX\"/>\
        <Loc Id=\"7\" LocCode=\"FRA\" NationCode=\"DE\" CityCode=\"GHI\" />\
      </TestData></TaxRq>";

    _xmlRequestCacheMileage = "<TaxRq><TestData>\
         <Mileage GeoPathRefId=\"0\">\
           <Distance FromGeoRefId =\"0\" ToGeoRefId=\"1\" Miles=\"100\" />\
           <Distance FromGeoRefId =\"0\" ToGeoRefId=\"2\" Miles=\"100\" />\
           <Distance FromGeoRefId =\"0\" ToGeoRefId=\"3\" Miles=\"50\" />\
         </Mileage></TestData></TaxRq>";

    _xmlRequestRulesRecord = "<TaxRq><TestData>\
         <RulesRecord NATION=\"PL\" TAXCODE=\"001\" TAXTYPE=\"F\" TAXPOINTTAG=\"A\"\
           PERCENTFLATTAG=\"F\" SEQNO=\"1\" VENDOR=\"ATP\" TAXAMT=\"11\" TAXCURRENCY=\"PLN\"\
           TAXCURDECIMALS=\"2\" TAXPERCENT=\"22\" RTNTOORIG=\"Y\" EXEMPTTAG=\"N\"\
           EFFDATE=\"2013-01-01\" DISCDATE=\"2099-12-31\"\
           EXPIREDATE=\"2099-12-31\" TVLFIRSTYEAR=\"13\"\
           TVLLASTYEAR=\"99\" TVLFIRSTMONTH=\"2\" TVLLASTMONTH=\"99\"\
           TVLFIRSTDAY=\"1\" TVLLASTDAY=\"99\" TRAVELDATEAPPTAG=\"J\" TICKETEDPOINTTAG=\"X\"\
         />\
         </TestData></TaxRq>";
  }

  void setUp()
  {
    _xmlTagsFactory = new XmlTagsFactory();
    _xmlTagsFactory->registerList(new NaturalXmlTagsList);
  }

  void tearDown()
  {
    delete _xmlTagsFactory;
  }

  void testParseUnknownXMLElement()
  {
    std::string xmlRequest =
      "<TaxRq> <UnknownElement TicketingDate=\"2013-05-06\" /> </TaxRq>";
    rapidxml::xml_document<> parsedRequest;
    parsedRequest.parse<0>(&xmlRequest[0]);
    const XmlTagsList& tags = selectTagsList(parsedRequest, *_xmlTagsFactory);

    CPPUNIT_ASSERT_THROW(XmlParser().parse(_inputRequest, _xmlCache, parsedRequest, tags), XmlParsingError);
  }

  void testParseUnknownXMLAttribute()
  {
    std::string xmlRequest =
      "<TaxRq> <ProcessingOptions UnknownAttribute=\"2013-05-06\" /> </TaxRq>";

    rapidxml::xml_document<> parsedRequest;
    parsedRequest.parse<0>(&xmlRequest[0]);
    const XmlTagsList& tags = selectTagsList(parsedRequest, *_xmlTagsFactory);

    CPPUNIT_ASSERT_THROW(XmlParser().parse(_inputRequest, _xmlCache, parsedRequest, tags), XmlParsingError);
  }

  ////////////////////////////////////////////////////////////////

  void testTicketingDate()
  {
    const std::string xmlRequest =
      "<TaxRq> <TicketingOptions TicketingDate=\"2013-05-06\" /> </TaxRq>";
    parseInputXml(xmlRequest);

    CPPUNIT_ASSERT_EQUAL(boost::gregorian::date(2013, 5, 6),
                         _inputRequest.ticketingOptions().ticketingDate());
  }

  void testTravelDate()
  {
    const std::string xmlRequest = "<TaxRq>\
        <FlightPath>\
          <FlightUsage ConnectionDateShift=\"0\" FlightRefId=\"0\" />\
          <FlightUsage ConnectionDateShift=\"2\" FlightRefId=\"1\" />\
        </FlightPath>\
        <Itin FarePathGeoPathMappingRefId=\"0\" FarePathRefId=\"0\" GeoPathRefId=\"0\" Id=\"0\"\
        TravelOriginDate=\"2013-05-06\" FlightPathRefId=\"0\">\
        </Itin>\
      </TaxRq>";

    parseInputXml(xmlRequest);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Date not parsed", boost::gregorian::date(2013, 5, 6),
                                 _inputRequest.itins()[0]._travelOriginDate);
  }

  void testTicketingTime()
  {
    const std::string xmlRequest = "<TaxRq> <TicketingOptions TicketingTime=\"02:18\" /> </TaxRq>";
    parseInputXml(xmlRequest);

    CPPUNIT_ASSERT_EQUAL(boost::posix_time::time_duration(2, 18, 0, 0),
                         _inputRequest.ticketingOptions().ticketingTime());
  }

  void testPointOfSaleLoc()
  {
    const std::string xmlRequest = "<TaxRq> <PointOfSale Id =\"0\" PointOfSaleLoc=\"KRK\" /> "
      "<PointOfSale Id =\"1\" PointOfSaleLoc=\"WAW\" /> </TaxRq>";
    parseInputXml(xmlRequest);

    type::AirportCode locCode = "KRK";
    type::AirportCode xmlLocCode = _inputRequest.pointsOfSale()[0].loc();
    CPPUNIT_ASSERT_EQUAL(locCode, xmlLocCode);

    locCode = "WAW";
    xmlLocCode = _inputRequest.pointsOfSale()[1].loc();
    CPPUNIT_ASSERT_EQUAL(locCode, xmlLocCode);
  }

  void testPointOfSaleAgentAirlineDept_AgentOfficeDesignator()
  {
    const std::string xmlRequest =
        "<TaxRq>"
        "<PointOfSale Id=\"0\" AgentAirlineDept=\"KRK\" AgentOfficeDesignator=\"ARN\" />"
        "<PointOfSale Id=\"1\" AgentAirlineDept=\"KTW\" AgentOfficeDesignator=\"NYO\" />"
        "</TaxRq>";

    parseInputXml(xmlRequest);

    CPPUNIT_ASSERT_EQUAL(_inputRequest.pointsOfSale()[0].agentAirlineDept(), std::string("KRK"));
    CPPUNIT_ASSERT_EQUAL(_inputRequest.pointsOfSale()[0].agentOfficeDesignator(), std::string("ARN"));
    CPPUNIT_ASSERT_EQUAL(_inputRequest.pointsOfSale()[1].agentAirlineDept(), std::string("KTW"));
    CPPUNIT_ASSERT_EQUAL(_inputRequest.pointsOfSale()[1].agentOfficeDesignator(), std::string("NYO"));
  }

  void testGeoPathsNumber()
  {
    parseInputXml(_xmlRequestGeoPath);
    CPPUNIT_ASSERT_MESSAGE("Wrong GeoPaths number", _inputRequest.geoPaths().size() == 2);
  }

  void testGeoPathGeosNumber()
  {
    parseInputXml(_xmlRequestGeoPath);

    CPPUNIT_ASSERT_MESSAGE("Wrong Geos number in GeoPath",
                           _inputRequest.geoPaths()[0]._geos.size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Wrong Geos number in GeoPath",
                           _inputRequest.geoPaths()[1]._geos.size() == 3);
  }

  void testGeoPathValues()
  {
    parseInputXml(_xmlRequestGeoPath);

    type::LocCode locCode = _inputRequest.geoPaths()[1]._geos[2]._loc.code().asString();
    CPPUNIT_ASSERT_MESSAGE("Wrong Loc value " + locCode, locCode == "BB3");

    CPPUNIT_ASSERT_MESSAGE("Wrong Type value", _inputRequest.geoPaths()[1]._geos[2]._loc.tag() ==
                                                 static_cast<tax::type::TaxPointTag>('G'));

    type::UnticketedTransfer unticketedTransfer =
      _inputRequest.geoPaths()[1]._geos[2]._unticketedTransfer;
    CPPUNIT_ASSERT_MESSAGE("Wrong UnticketedTransfer value ",
                           unticketedTransfer == type::UnticketedTransfer::No);
  }

  void testYqYrPaths()
  {
    parseInputXml(_xmlRequestYqYrPath);

    CPPUNIT_ASSERT_MESSAGE("Wrong YqYrPaths number", _inputRequest.yqYrPaths().size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr number", _inputRequest.yqYrs().size() == 4);
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr number in YqYrPath[0]",
        _inputRequest.yqYrPaths()[0]._yqYrUsages.size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr number in YqYrPath[1]",
        _inputRequest.yqYrPaths()[1]._yqYrUsages.size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr code", _inputRequest.yqYrs()[0]._code == "YR");
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr type", _inputRequest.yqYrs()[0]._type == 'I');
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr amount",
        _inputRequest.yqYrs()[0]._amount == type::MoneyAmount(13));
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr taxIncluded", _inputRequest.yqYrs()[0]._taxIncluded == false);

    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr code", _inputRequest.yqYrs()[1]._code == "YQ");
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr type", _inputRequest.yqYrs()[1]._type == 'F');
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr amount",
        _inputRequest.yqYrs()[1]._amount == type::MoneyAmount(15));
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr taxIncluded", _inputRequest.yqYrs()[1]._taxIncluded == false);

    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr code", _inputRequest.yqYrs()[2]._code == "YR");
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr type", _inputRequest.yqYrs()[2]._type == 'I');
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr amount",
        _inputRequest.yqYrs()[2]._amount == type::MoneyAmount(17));
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr taxIncluded", _inputRequest.yqYrs()[2]._taxIncluded == false);

    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr code", _inputRequest.yqYrs()[3]._code == "YQ");
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr type", _inputRequest.yqYrs()[3]._type == 'F');
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr amount",
        _inputRequest.yqYrs()[3]._amount == type::MoneyAmount(19));
    CPPUNIT_ASSERT_MESSAGE("Wrong YqYr taxIncluded", _inputRequest.yqYrs()[3]._taxIncluded == true);
  }

  void testFaresNumber()
  {
    parseInputXml(_xmlRequestFares);
    CPPUNIT_ASSERT_MESSAGE("Wrong Fares number", _inputRequest.fares().size() == 3);
  }

  void testFareValues()
  {
    parseInputXml(_xmlRequestFares);

    type::MoneyAmount fareAmount = _inputRequest.fares()[2]._amount;
    CPPUNIT_ASSERT_MESSAGE("Wrong Amount value " + boost::lexical_cast<std::string>(fareAmount),
                           fareAmount == type::MoneyAmount(10));

    type::MoneyAmount fareSellAmount = _inputRequest.fares()[2]._sellAmount;
    CPPUNIT_ASSERT_MESSAGE("Wrong SellAmount value " +
                             boost::lexical_cast<std::string>(fareSellAmount),
                           fareSellAmount == type::MoneyAmount(9));

    bool fareIsNetRemitAvailable = _inputRequest.fares()[2]._isNetRemitAvailable;
    CPPUNIT_ASSERT_MESSAGE("Wrong IsNetRemitAvailable value" + fareIsNetRemitAvailable,
                           fareIsNetRemitAvailable == true);

    type::FareBasisCode basisCode = _inputRequest.fares()[2]._basis;
    CPPUNIT_ASSERT_MESSAGE("Wrong BasisCode value" + basisCode, basisCode == "YXOW");

    type::Directionality directionality = _inputRequest.fares()[2]._directionality;
    CPPUNIT_ASSERT_MESSAGE("Wrong Directionality value " +
                             boost::lexical_cast<std::string>(directionality),
                           directionality == type::Directionality::Between);

    type::OneWayRoundTrip oneWayRoundTrip = _inputRequest.fares()[2]._oneWayRoundTrip;
    CPPUNIT_ASSERT_MESSAGE("Wrong OneWayRoundTrip value" + oneWayRoundTrip, oneWayRoundTrip == 'X');

    type::FareTypeCode typeCode = _inputRequest.fares()[2]._type;
    CPPUNIT_ASSERT_MESSAGE("Wrong TypeCode value" + typeCode, typeCode == "BR");
  }

  void testFarePathsNumber()
  {
    parseInputXml(_xmlRequestFarePaths);
    CPPUNIT_ASSERT_MESSAGE("Wrong FarePaths number", _inputRequest.farePaths().size() == 2);
  }

  void testFarePathValues()
  {
    parseInputXml(_xmlRequestFarePaths);

    type::MoneyAmount totalAmount = _inputRequest.farePaths()[1]._totalAmount;
    CPPUNIT_ASSERT_MESSAGE("Wrong TotalAmount value " +
                             boost::lexical_cast<std::string>(totalAmount),
                           totalAmount == type::MoneyAmount(40));

    type::MoneyAmount totalAmountBeforeDiscount = _inputRequest.farePaths()[1]._totalAmountBeforeDiscount;
    CPPUNIT_ASSERT_MESSAGE("Wrong TotalAmountBeforeDiscount value " +
                             boost::lexical_cast<std::string>(totalAmountBeforeDiscount),
                             totalAmountBeforeDiscount == type::MoneyAmount(60));

    type::CarrierCode validatingCarrier = _inputRequest.farePaths()[0]._validatingCarrier;
    CPPUNIT_ASSERT_MESSAGE("Wrong ValidatingCarrier value: " + validatingCarrier.asString(),
                           validatingCarrier == "LH");

    type::PassengerCode outputPtc(_inputRequest.farePaths()[0]._outputPtc);
    CPPUNIT_ASSERT_MESSAGE("Wrong Output PTC value: " + outputPtc.asString(), outputPtc.empty());

    outputPtc = _inputRequest.farePaths()[1]._outputPtc;
    CPPUNIT_ASSERT_MESSAGE("Wrong Output PTC value: " + outputPtc.asString(), outputPtc == "CNN");
  }

  void testFarePathFareUsageNumber()
  {
    parseInputXml(_xmlRequestFarePaths);

    CPPUNIT_ASSERT_MESSAGE("Wrong FareUsage number",
                           _inputRequest.farePaths()[0]._fareUsages.size() == 3);
    CPPUNIT_ASSERT_MESSAGE("Wrong FareUsage number",
                           _inputRequest.farePaths()[1]._fareUsages.size() == 2);
  }

  void testFarePathFareUsageValues()
  {
    parseInputXml(_xmlRequestFarePaths);

    type::Index index = _inputRequest.farePaths()[1]._fareUsages[1]._fareRefId;
    CPPUNIT_ASSERT_MESSAGE("Wrong Index value " + boost::lexical_cast<std::string>(index),
                           index == type::Index(4));
  }

  void testOptionalServicesNumber()
  {
    parseInputXml(_xmlRequestOptionalServices);
    CPPUNIT_ASSERT_MESSAGE("Wrong OptionalServices number", _inputRequest.optionalServices().size() == 2);
  }

  void testOptionalServiceValues()
  {
    parseInputXml(_xmlRequestOptionalServices);

    type::Index id = _inputRequest.optionalServices()[1]._id;
    CPPUNIT_ASSERT_MESSAGE("Wrong Id value " + boost::lexical_cast<std::string>(id),
                           id == type::Index(1));

    type::MoneyAmount amount = _inputRequest.optionalServices()[1]._amount;
    CPPUNIT_ASSERT_MESSAGE("Wrong Amount value " + boost::lexical_cast<std::string>(amount),
                           amount == type::MoneyAmount(30));

    type::OcSubCode subCode = _inputRequest.optionalServices()[1]._subCode;
    CPPUNIT_ASSERT_MESSAGE("Wrong SubCode value " + subCode.asString(), subCode == "ACC");

    type::OptionalServiceTag type = _inputRequest.optionalServices()[1]._type;
    CPPUNIT_ASSERT_MESSAGE("Wrong TaxType value " + boost::lexical_cast<std::string>(type), type == type::OptionalServiceTag::Merchandise);

    type::CarrierCode ownerCarrier = _inputRequest.optionalServices()[1]._ownerCarrier;
    CPPUNIT_ASSERT_MESSAGE("Wrong OwnerCarrier value" + ownerCarrier.asString(), ownerCarrier == "US");

    type::LocCode pointOfDeliveryLoc = _inputRequest.optionalServices()[1]._pointOfDeliveryLoc.asString();
    CPPUNIT_ASSERT_MESSAGE("Wrong PointOfDeliveryLoc value" + pointOfDeliveryLoc, pointOfDeliveryLoc == "KRK");
  }

  void testOptionalServicePathsNumber()
  {
    parseInputXml(_xmlRequestOptionalServicePaths);
    CPPUNIT_ASSERT_MESSAGE("Wrong OptionalServicePaths number", _inputRequest.optionalServicePaths().size() == 1);
  }

  void testOptionalServicePathValues()
  {
    parseInputXml(_xmlRequestOptionalServicePaths);

    type::Index id = _inputRequest.optionalServicePaths()[0]._id;
    CPPUNIT_ASSERT_MESSAGE("Wrong Id value " + boost::lexical_cast<std::string>(id),
                           id == type::Index(0));
  }

  void testOptionalServicePathOptionalServiceUsageNumber()
  {
    parseInputXml(_xmlRequestOptionalServicePaths);

    CPPUNIT_ASSERT_MESSAGE("Wrong OptionalServiceUsage number",
                           _inputRequest.optionalServicePaths()[0]._optionalServiceUsages.size() == 3);
  }

  void testOptionalServicePathOptionalServiceUsageValues()
  {
    parseInputXml(_xmlRequestOptionalServicePaths);

    type::Index optionalServiceRefId =
        _inputRequest.optionalServicePaths()[0]._optionalServiceUsages[2]._optionalServiceRefId;
    CPPUNIT_ASSERT_MESSAGE("Wrong OptionalServiceRefId value " +
                           boost::lexical_cast<std::string>(optionalServiceRefId),
                           optionalServiceRefId == type::Index(2));
  }

  void testGeoPathMappingNumber()
  {
    parseInputXml(_xmlRequestGeoPathMapping);

    CPPUNIT_ASSERT_MESSAGE("Wrong Mapping number",
                           _inputRequest.geoPathMappings()[0]._mappings.size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Wrong Map number",
                           _inputRequest.geoPathMappings()[0]._mappings[0].maps().size() == 2);
  }

  void testGeoPathMappingValue()
  {
    parseInputXml(_xmlRequestGeoPathMapping);

    type::Index index = _inputRequest.geoPathMappings()[0]._mappings[1].maps()[1]._geoRefId;
    CPPUNIT_ASSERT_MESSAGE("Wrong Mapping number", index == 3);
  }

  void testFlightsNumber()
  {
    parseInputXml(_xmlRequestFlights);
    CPPUNIT_ASSERT_MESSAGE("Wrong Flights number", _inputRequest.flights().size() == 3);
  }

  void testFlightValues()
  {
    parseInputXml(_xmlRequestFlights);

    CPPUNIT_ASSERT_MESSAGE("Wrong ArrivalDateShift value",
                           _inputRequest.flights()[2]._arrivalDateShift == 1);

    CPPUNIT_ASSERT_MESSAGE("Wrong ArrivalTime value",
                           _inputRequest.flights()[2]._arrivalTime ==
                             boost::posix_time::time_duration(2, 18, 0, 0));

    CPPUNIT_ASSERT_MESSAGE("Wrong DepartureTime value",
                           _inputRequest.flights()[2]._departureTime ==
                             boost::posix_time::time_duration(21, 31, 0, 0));

    CPPUNIT_ASSERT_MESSAGE("Wrong MarketingCarrierFlightNumber",
                           _inputRequest.flights()[2]._marketingCarrierFlightNumber ==
                             type::FlightNumber(101));

    CPPUNIT_ASSERT_MESSAGE("Wrong Equipment value",
                           _inputRequest.flights()[2]._equipment == type::EquipmentCode("TIC"));

    CPPUNIT_ASSERT_MESSAGE("Wrong MarketingCarrier value",
                           _inputRequest.flights()[2]._marketingCarrier ==
                             type::CarrierCode("CC"));

    CPPUNIT_ASSERT_MESSAGE("Wrong OperatingCarrier value",
                           _inputRequest.flights()[2]._operatingCarrier ==
                             type::CarrierCode("FF"));
  }

  void testItinsNumber()
  {
    parseInputXml(_xmlRequestItins);

    CPPUNIT_ASSERT_MESSAGE("Wrong Itin number", _inputRequest.itins().size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Wrong FlightUsage number",
                           _inputRequest.flightPaths()[0]._flightUsages.size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Wrong FlightUsage number",
                           _inputRequest.flightPaths()[1]._flightUsages.size() == 3);
  }

  void testItinValues()
  {
    parseInputXml(_xmlRequestItins);

    CPPUNIT_ASSERT(_inputRequest.itins()[1]._farePathGeoPathMappingRefId.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong FarePathGeoPathMappingRefId value", type::Index(1),
                                 _inputRequest.itins()[1]._farePathGeoPathMappingRefId.value());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong FarePathRefId value", type::Index(2),
                                 _inputRequest.itins()[1]._farePathRefId);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong GeoPathRefId value", type::Index(1),
                                 _inputRequest.itins()[1]._geoPathRefId);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Id value", type::Index(1), _inputRequest.itins()[1]._id);

    CPPUNIT_ASSERT_MESSAGE("Wrong ConnectionDateShift value",
                           _inputRequest.flightPaths()[1]._flightUsages[2]._connectionDateShift == 6);

    CPPUNIT_ASSERT_MESSAGE("Wrong FlightRefId value",
                           _inputRequest.flightPaths()[1]._flightUsages[2]._flightRefId == 6);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong ForcedConnection value",
                                 type::ForcedConnection(type::ForcedConnection::Connection),
                                 _inputRequest.flightPaths()[0]._flightUsages[0]._forcedConnection);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong ForcedConnection value",
                                 type::ForcedConnection(type::ForcedConnection::Stopover),
                                 _inputRequest.flightPaths()[1]._flightUsages[0]._forcedConnection);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong ForcedConnection value",
                                 type::ForcedConnection(type::ForcedConnection::Blank),
                                 _inputRequest.flightPaths()[1]._flightUsages[1]._forcedConnection);

    CPPUNIT_ASSERT_MESSAGE("Wrong passenger index", _inputRequest.itins()[0]._passengerRefId == 1);

    CPPUNIT_ASSERT_MESSAGE("Wrong passenger index", _inputRequest.itins()[1]._passengerRefId == 2);
  }

  void testDiagnostic()
  {
    const std::string xmlRequest = "<TaxRq><Diagnostic Id=\"801\">\
        <Parameter Name=\"TX\" Value=\"PLUS001\" />\
        <Parameter Name=\"SQ\" Value=\"200\" />\
      </Diagnostic></TaxRq>";

    parseInputXml(xmlRequest);

    CPPUNIT_ASSERT_MESSAGE("Wrong Diagnostic number", _inputRequest.diagnostic().number() == 801);

    CPPUNIT_ASSERT_MESSAGE("Wrong Diagnostic parameters count",
                           _inputRequest.diagnostic().parameters().size() == 2);

    CPPUNIT_ASSERT_MESSAGE("Wrong Diagnostic parameters",
                           _inputRequest.diagnostic().parameters()[1].name() == "SQ");

    CPPUNIT_ASSERT_MESSAGE("Wrong Diagnostic parameters",
                           _inputRequest.diagnostic().parameters()[1].value() == "200");
  }

  ////////////////////////////////////////////////////////////////
  void testCacheRulesRecordNumber()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord NATION=\"PL\" />\
        <RulesRecord NATION=\"PL\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong Services/RulesRecord number", _xmlCache.rulesRecords().size() == 2);
  }

  void testRulesRecord_SvcFeesSecurityItemNo()
  {
    std::string xmlRequest =
        "<TaxRq>"
          "<TestData>"
            "<RulesRecord SVCFEESSECURITYITEMNO=\"10\" />"
          "</TestData>"
        "</TaxRq>";

    parseInputXml(xmlRequest);

    CPPUNIT_ASSERT_EQUAL(_xmlCache.rulesRecords().at(0).svcFeesSecurityItemNo, type::Index(10));
  }

  void testCacheTaxName()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord NATION=\"PL\" TAXCODE=\"US\"\
        TAXTYPE=\"001\" TAXPOINTTAG=\"D\" PERCENTFLATTAG=\"P\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong NATION value", type::Nation("PL"),
                                 _xmlCache.rulesRecords().at(0).taxName.nation());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong TAXCODE value", type::TaxCode("US"),
                                 _xmlCache.rulesRecords().at(0).taxName.taxCode());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong TAXTYPE value", type::TaxType("001"),
                                 _xmlCache.rulesRecords().at(0).taxName.taxType());

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTTAG value",
                           _xmlCache.rulesRecords().at(0).taxName.taxPointTag() ==
                             static_cast<tax::type::TaxPointTag>('D'));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong PERCENTFLATTAG value",
                                 type::PercentFlatTag(type::PercentFlatTag::Percent),
                                 _xmlCache.rulesRecords().at(0).taxName.percentFlatTag());
  }

  void testCacheJourney()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord JRNYIND=\"A\" JRNYLOC1TYPE=\"C\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong JRNYIND value",
                           _xmlCache.rulesRecords().at(0).jrnyInd == static_cast<type::JrnyInd>('A'));

    CPPUNIT_ASSERT_MESSAGE("Wrong JRNYIND value",
                           _xmlCache.rulesRecords().at(0).jrnyLocZone1.type() ==
                             static_cast<type::LocType>('C'));
  }

  void testCacheJourneyLoc1()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord JRNYLOC1=\"SOME\"/>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong JRNYLOC1 value",
                           _xmlCache.rulesRecords().at(0).jrnyLocZone1.code() ==
                             type::LocZoneText("SOME"));
  }

  void testCacheJourneyZoneTblNo()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord JRNYLOC1ZONETBLNO=\"ANOTHER\"/>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong JRNYLOC1ZONETBLNO value",
                           _xmlCache.rulesRecords().at(0).jrnyLocZone1.code() ==
                             type::LocZoneText("ANOTHER"));
  }

  void testCacheTravelWhollyWithin()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TRVLWHOLLYWITHINLOCTYPE=\"C\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TRVLWHOLLYWITHINLOCTYPE value",
                           _xmlCache.rulesRecords().at(0).trvlWhollyWithin.type() ==
                             static_cast<type::LocType>('C'));
  }

  void testCacheTravelWhollyWithinLoc()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TRVLWHOLLYWITHINLOC=\"SOME\"/>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TRVLWHOLLYWITHINLOC value",
                           _xmlCache.rulesRecords().at(0).trvlWhollyWithin.code() ==
                             type::LocZoneText("SOME"));
  }

  void testCacheTravelWhollyWithinZoneTblNo()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TRVLWHOLLYWITHINLOCZONETBLNO=\"ANOTHER\"/>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TRVLWHOLLYWITHINLOCZONETBLNO value",
                           _xmlCache.rulesRecords().at(0).trvlWhollyWithin.code() ==
                             type::LocZoneText("ANOTHER"));
  }

  void testCacheTxptType()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TAXPOINTLOC1TYPE=\"C\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTLOC1TYPE value",
                           _xmlCache.rulesRecords().at(0).taxPointLocZone1.type() ==
                             static_cast<type::LocType>('C'));
  }

  void testCacheTxptTypeLoc()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TAXPOINTLOC1=\"SOME\"/>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTLOC1 value",
                           _xmlCache.rulesRecords().at(0).taxPointLocZone1.code() ==
                             type::LocZoneText("SOME"));
  }

  void testCacheTxptTypeZoneTblNo()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TAXPOINTLOC1ZONETBLNO=\"ANOTHER\"/>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTLOC1ZONETBLNO value",
                           _xmlCache.rulesRecords().at(0).taxPointLocZone1.code() ==
                             type::LocZoneText("ANOTHER"));
  }

  void testCacheSeqNo()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord SEQNO=\"123\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong SEQNO value", type::SeqNo(123),
                                 _xmlCache.rulesRecords().at(0).seqNo);
  }

  void testCacheVendor()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord VENDOR=\"VNDR\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong VENDOR value", type::Vendor("VNDR"),
                                 _xmlCache.rulesRecords().at(0).vendor);
  }

  void testCacheCurrencyOfSale()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord CURRENCYOFSALE=\"LMN\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong CURRENCYOFSALE value", type::CurrencyCode("LMN"),
                                 _xmlCache.rulesRecords().at(0).currencyOfSale);
  }

  void testCacheTaxAmt()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TAXAMT=\"45\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong TAXAMT value", type::MoneyAmount(45),
                                 _xmlCache.rulesRecords().at(0).taxAmt);
  }

  void testCacheTaxCurrency()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TAXCURRENCY=\"PLN\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong TAXCURRENCY value", type::CurrencyCode("PLN"),
                                 _xmlCache.rulesRecords().at(0).taxCurrency);
  }

  void testCacheTaxCurDecimals()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TAXCURDECIMALS=\"5\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong TAXCURDECIMALS value", type::CurDecimals(5),
                                 _xmlCache.rulesRecords().at(0).taxCurDecimals);
  }

  void testCacheTaxPercent()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TAXPERCENT=\"5000000\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong TAXPERCENT value", type::Percent(5000000),
                                 _xmlCache.rulesRecords().at(0).taxPercent);
  }

  void testCacheRtnToOrig()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord RTNTOORIG=\"C\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong RTNTOORIG value", _xmlCache.rulesRecords().at(0).rtnToOrig ==
                                                      static_cast<type::RtnToOrig>('C'));
  }

  void testCacheExemptTag()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord EXEMPTTAG=\"X\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong EXEMPTTAG value", _xmlCache.rulesRecords().at(0).exemptTag ==
                                                      static_cast<type::ExemptTag>('X'));
  }

  void testCacheDates()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord\
          EFFDATE=\"2013-03-12\" \
          DISCDATE=\"2013-03-14\" \
          EXPIREDATE=\"2013-03-16 14:44\"/>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL(type::Date(2013, 3, 12), _xmlCache.rulesRecords().at(0).effDate);
    CPPUNIT_ASSERT_EQUAL(type::Date(2013, 3, 14), _xmlCache.rulesRecords().at(0).discDate);
    CPPUNIT_ASSERT_EQUAL(type::Date(2013, 3, 16),
                         _xmlCache.rulesRecords().at(0).expiredDate.date());
    CPPUNIT_ASSERT_EQUAL(type::Time(14, 44), _xmlCache.rulesRecords().at(0).expiredDate.time());
  }

  void testHistoricSaleDates()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord\
          HISTORICSALEEFFDATE=\"2013-03-12\" \
          HISTORICSALEDISCDATE=\"2013-03-14\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL(type::Date(2013, 3, 12), _xmlCache.rulesRecords().at(0).histSaleEffDate);
    CPPUNIT_ASSERT_EQUAL(type::Date(2013, 3, 14), _xmlCache.rulesRecords().at(0).histSaleDiscDate);
  }

  void testHistoricTrvlDates()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord\
          HISTORICTRVLEFFDATE=\"2014-06-22\" \
          HISTORICTRVLDISCDATE=\"2014-06-24\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL(type::Date(2014, 6, 22), _xmlCache.rulesRecords().at(0).histTrvlEffDate);
    CPPUNIT_ASSERT_EQUAL(type::Date(2014, 6, 24), _xmlCache.rulesRecords().at(0).histTrvlDiscDate);
  }

  void testCacheDatesFirstLast()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord \
          TVLFIRSTYEAR=\"2013\" \
          TVLLASTYEAR=\"2014\" \
          TVLFIRSTMONTH=\"4\" \
          TVLLASTMONTH=\"5\" \
          TVLFIRSTDAY=\"6\" \
          TVLLASTDAY=\"7\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL(int16_t(2013), _xmlCache.rulesRecords().at(0).firstTravelYear);
    CPPUNIT_ASSERT_EQUAL(int16_t(2014), _xmlCache.rulesRecords().at(0).lastTravelYear);
    CPPUNIT_ASSERT_EQUAL(int16_t(4), _xmlCache.rulesRecords().at(0).firstTravelMonth);
    CPPUNIT_ASSERT_EQUAL(int16_t(5), _xmlCache.rulesRecords().at(0).lastTravelMonth);
    CPPUNIT_ASSERT_EQUAL(int16_t(6), _xmlCache.rulesRecords().at(0).firstTravelDay);
    CPPUNIT_ASSERT_EQUAL(int16_t(7), _xmlCache.rulesRecords().at(0).lastTravelDay);
  }

  void testCacheTravelDateTag()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TRAVELDATEAPPTAG=\"J\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    type::TravelDateAppTag travelDateAppTag = _xmlCache.rulesRecords().at(0).travelDateTag;

    CPPUNIT_ASSERT_MESSAGE("Wrong TRAVELDATEAPPTAG value",
                           travelDateAppTag == type::TravelDateAppTag::Journey);
  }

  void testCacheTicketedPointTag()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TICKETEDPOINTTAG=\"U\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TICKETEDPOINTTAG value",
                           _xmlCache.rulesRecords().at(0).ticketedPointTag ==
                             static_cast<type::TicketedPointTag>('U'));
  }

  void testCacheAlternateRuleRefTag()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord ALTERNATERULEREFTAG=\" \" />\
        <RulesRecord ALTERNATERULEREFTAG=\"1\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong ALTERNATERULEREFTAG value",
                           _xmlCache.rulesRecords().at(0).alternateRuleRefTag ==
                             ALTERNATERULEREFTAGBLANK);

    CPPUNIT_ASSERT_MESSAGE("Wrong ALTERNATERULEREFTAG value",
                           _xmlCache.rulesRecords().at(1).alternateRuleRefTag == 1);
  }

  void testCacheTaxMatchingApplTag()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TAXMATCHINGAPPLTAG=\"  \" />\
        <RulesRecord TAXMATCHINGAPPLTAG=\"01\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXMATCHINGAPPLTAG value",
                           _xmlCache.rulesRecords().at(0).taxMatchingApplTag == "  ");

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXMATCHINGAPPLTAG value",
                           _xmlCache.rulesRecords().at(1).taxMatchingApplTag == "01");
  }

  void testCacheLocNumber()
  {
    parseInputXml(_xmlRequestCache);
    CPPUNIT_ASSERT_MESSAGE("Wrong Services/Loc number", _xmlCache.nations().size() == 3);
  }

  void testCacheLocValues()
  {
    parseInputXml(_xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong Loc/Index number",
                           _xmlCache.nations().at(1).id() == type::Index(5));

    CPPUNIT_ASSERT_MESSAGE("Wrong Loc/NationCode number",
                           _xmlCache.nations().at(1).nationCode() == type::Nation("DE"));

    CPPUNIT_ASSERT_MESSAGE("Wrong Loc/LocCode number",
                           _xmlCache.nations().at(1).locCode() == type::AirportCode("MUC"));

    CPPUNIT_ASSERT_MESSAGE("Wrong Loc/CityCode number",
                           _xmlCache.nations().at(1).cityCode() == type::CityCode("DEF"));

    CPPUNIT_ASSERT_MESSAGE("Wrong Loc/AlaskaZone",
                           _xmlCache.nations().at(1).alaskaZone() == type::AlaskaZone::B);

    CPPUNIT_ASSERT_MESSAGE("Wrong Loc/State",
                           _xmlCache.nations().at(1).state() == type::StateProvinceCode("USTX"));
  }

  void testCacheTaxPointLoc1TransferType()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord TAXPOINTLOC1TRNSFRTYPE=\"D\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTLOC1TRNSFRTYPE value",
                           _xmlCache.rulesRecords().at(0).taxPointLoc1TransferType ==
                             static_cast<tax::type::TransferTypeTag>('D'));
  }

  void testCacheTaxPointLoc1()
  {
    std::string xmlRequestCache =
      "<TaxRq><TestData><RulesRecord TAXPOINTLOC1=\"KRK\" /></TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTLOC1 value",
                           _xmlCache.rulesRecords().at(0).taxPointLocZone1.code() ==
                             type::LocZoneText("KRK"));
  }

  void testCacheTaxPointLoc1Zone()
  {
    std::string xmlRequestCache =
      "<TaxRq><TestData><RulesRecord TAXPOINTLOC1ZONETBLNO=\"1234\" /></TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTLOC1ZONETBLNO value",
                           _xmlCache.rulesRecords().at(0).taxPointLocZone1.code() ==
                             type::LocZoneText("1234"));
  }

  void testCacheTaxPointLoc1Type()
  {
    std::string xmlRequestCache =
      "<TaxRq><TestData><RulesRecord TAXPOINTLOC1TYPE=\"NA\" /></TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTLOC1 value",
                           _xmlCache.rulesRecords().at(0).taxPointLocZone1.type() ==
                             static_cast<type::LocType>('N'));
  }

  void testCacheTaxPointLoc2()
  {
    std::string xmlRequestCache =
      "<TaxRq><TestData><RulesRecord TAXPOINTLOC2=\"KRK\" /></TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTLOC2 value",
                           _xmlCache.rulesRecords().at(0).taxPointLocZone2.code() ==
                             type::LocZoneText("KRK"));
  }

  void testCacheTaxPointLoc2Zone()
  {
    std::string xmlRequestCache =
      "<TaxRq><TestData><RulesRecord TAXPOINTLOC2ZONETBLNO=\"1234\" /></TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTLOC2ZONETBLNO value",
                           _xmlCache.rulesRecords().at(0).taxPointLocZone2.code() ==
                             type::LocZoneText("1234"));
  }

  void testCacheTaxPointLoc2Type()
  {
    std::string xmlRequestCache =
      "<TaxRq><TestData><RulesRecord TAXPOINTLOC2TYPE=\"NA\" /></TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTLOC2 value",
                           _xmlCache.rulesRecords().at(0).taxPointLocZone2.type() ==
                             static_cast<type::LocType>('N'));
  }

  void testCacheTaxPointIntlDomLoc2Ind()
  {
    std::string xmlRequestCache =
      "<TaxRq><TestData><RulesRecord TAXPOINTLOC2INTLDOMIND=\"I\" /></TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTLOC2INTLDOMIND value",
                           _xmlCache.rulesRecords().at(0).taxPointLoc2IntlDomInd ==
                             type::IntlDomInd::International);
  }

  void testCacheTaxPointLoc2Compare()
  {
    std::string xmlRequestCache =
      "<TaxRq><TestData><RulesRecord TAXPOINTLOC2COMPARE=\"X\" /></TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong TAXPOINTLOC2COMPARE value",
                           _xmlCache.rulesRecords().at(0).taxPointLoc2Compare ==
                             type::TaxPointLoc2Compare::Point);
  }

  void testCacheMileageDistanceNumber()
  {
    parseInputXml(_xmlRequestCacheMileage);

    CPPUNIT_ASSERT_MESSAGE("Wrong Mileage number", _xmlCache.mileages().size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong Distance number",
                           _xmlCache.mileages()[0].distances().size() == 3);
  }

  void testCacheStopoverTime()
  {
    std::string xmlRequestCache = "<TaxRq><TestData><RulesRecord STOPOVERTIMETAG=\"613\"\
      STOPOVERTIMEUNIT=\"N\"/></TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong STOPOVERTIMETAG value",
                           _xmlCache.rulesRecords().at(0).stopoverTimeTag == "613");
    CPPUNIT_ASSERT_MESSAGE("Wrong STOPOVERTIMEUNIT value",
                           _xmlCache.rulesRecords().at(0).stopoverTimeUnit ==
                             type::StopoverTimeUnit::Minutes);
  }

  void testCacheMileageDistanceValue()
  {
    parseInputXml(_xmlRequestCacheMileage);

    CPPUNIT_ASSERT_MESSAGE("Wrong Distance/FromGeoRefId number",
                           _xmlCache.mileages()[0].distances()[0].fromGeoRefId() == type::Index(0));
    CPPUNIT_ASSERT_MESSAGE("Wrong Distance/ToGeoRefId number",
                           _xmlCache.mileages()[0].distances()[0].toGeoRefId() == type::Index(1));
    CPPUNIT_ASSERT_MESSAGE("Wrong Distance/Miles number",
                           _xmlCache.mileages()[0].distances()[0].miles() == type::Miles(100));
  }

  void testTicketingOptions()
  {
    const std::string xmlRequest = "<TaxRq>"
                                   "<TicketingOptions PaymentCurrency=\"IJK\" />"
                                   "<TicketingOptions FormOfPayment=\"M\" />"
                                   "</TaxRq>";
    parseInputXml(xmlRequest);

    CPPUNIT_ASSERT_EQUAL(type::CurrencyCode("IJK"), _request.ticketingOptions().paymentCurrency());
    CPPUNIT_ASSERT_EQUAL(type::FormOfPayment(type::FormOfPayment::Miles),
                         _request.ticketingOptions().formOfPayment());
  }

  void testCacheCarrierFlightSegments()
  {
    const std::string xmlRequest = "<TaxRq><TestData>\
        <CarrierFlt VENDOR=\"ATP\" ITEMNO=\"4396\">\
          <CarrierFltSeg FLT1=\"0051\" FLT2=\"0056\"\
          MARKETINGCARRIER=\"LH\" OPERATINGCARRIER=\"AA\" />\
        </CarrierFlt>\
        <CarrierFlt VENDOR=\"ATP\" ITEMNO=\"4397\">\
          <CarrierFltSeg FLT1=\"0067\" FLT2=\"0070\"\
          MARKETINGCARRIER=\"LH\" OPERATINGCARRIER=\"AA\" />\
        </CarrierFlt>\
        <CarrierFlt VENDOR=\"ATP\" ITEMNO=\"4398\">\
          <CarrierFltSeg FLT1=\"0011\" FLT2=\"0028\"\
          MARKETINGCARRIER=\"BA\" OPERATINGCARRIER=\"AA\" />\
        </CarrierFlt>\
        <CarrierFlt VENDOR=\"ATP\" ITEMNO=\"4399\">\
          <CarrierFltSeg FLT1=\"0100\" FLT2=\"0299\"\
          MARKETINGCARRIER=\"BA\" OPERATINGCARRIER=\"AA\" />\
        </CarrierFlt>\
        <CarrierFlt VENDOR=\"ATP\" ITEMNO=\"4400\">\
          <CarrierFltSeg FLT1=\"0300\" FLT2=\"0499\"\
          MARKETINGCARRIER=\"BA\" OPERATINGCARRIER=\"AA\" />\
        </CarrierFlt>\
        <CarrierFlt VENDOR=\"ATP\" ITEMNO=\"4401\">\
          <CarrierFltSeg FLT1=\"0500\" FLT2=\"0699\"\
          MARKETINGCARRIER=\"BA\" OPERATINGCARRIER=\"AA\" />\
        </CarrierFlt>\
        <CarrierFlt VENDOR=\"ATP\" ITEMNO=\"4402\">\
          <CarrierFltSeg FLT1=\"0700\" FLT2=\"0899\"\
          MARKETINGCARRIER=\"AA\" OPERATINGCARRIER=\"AA\" />\
        </CarrierFlt>\
      </TestData></TaxRq>";
    parseInputXml(xmlRequest);

    CPPUNIT_ASSERT_EQUAL(std::size_t(7), _xmlCache.carrierFlights().size());
    CarrierFlight& cf = _xmlCache.carrierFlights()[5];
    CPPUNIT_ASSERT_EQUAL(tax::type::Vendor("ATP"), cf.vendor);
    CPPUNIT_ASSERT_EQUAL(type::Index(4401), cf.itemNo);
    CPPUNIT_ASSERT_EQUAL(tax::type::CarrierCode("BA"), cf.segments[0].marketingCarrier);
    CPPUNIT_ASSERT_EQUAL(tax::type::CarrierCode("AA"), cf.segments[0].operatingCarrier);
    CPPUNIT_ASSERT_EQUAL(tax::type::FlightNumber(500), cf.segments[0].flightFrom);
    CPPUNIT_ASSERT_EQUAL(tax::type::FlightNumber(699), cf.segments[0].flightTo);
  }

  void testCacheCarrierApplications()
  {
    const std::string xmlRequest = "<TaxRq><TestData>\
         <CarrierApplication ITEMNO=\"1000\" VENDOR=\"ATP\">\
           <CarrierApplEntry APPLIND=\" \" CARRIER=\"BA\" />\
         </CarrierApplication>\
         <CarrierApplication ITEMNO=\"1001\" VENDOR=\"ATP\">\
           <CarrierApplEntry APPLIND=\"X\" CARRIER=\"BA\" />\
           <CarrierApplEntry APPLIND=\" \" CARRIER=\"$$\" />\
         </CarrierApplication>\
         <CarrierApplication ITEMNO=\"1002\" VENDOR=\"ATP\">\
           <CarrierApplEntry APPLIND=\" \" CARRIER=\"XX\" />\
         </CarrierApplication>\
       </TestData></TaxRq>";
    parseInputXml(xmlRequest);

    CPPUNIT_ASSERT_EQUAL(std::size_t(3), _xmlCache.carrierApplications().size());
    CarrierApplication& ca = _xmlCache.carrierApplications()[1];
    CPPUNIT_ASSERT_EQUAL(tax::type::Vendor("ATP"), ca.vendor);
    CPPUNIT_ASSERT_EQUAL(type::Index(1001), ca.itemNo);
    CPPUNIT_ASSERT_EQUAL(tax::type::CarrierCode("BA"), ca.entries[0].carrier);
    CPPUNIT_ASSERT_EQUAL(tax::type::CarrierCode("$$"), ca.entries[1].carrier);
    CPPUNIT_ASSERT(ca.entries[0].applind == type::CarrierApplicationIndicator::Negative);
    CPPUNIT_ASSERT(ca.entries[1].applind == type::CarrierApplicationIndicator::Positive);
  }

  void testCurrencyConversion()
  {
    const std::string xmlRequestCache =
      "<TaxRq><TestData>"
      "<CurrencyConversion FromCurrency=\"EUR\" ToCurrency=\"USD\" BSR=\"1.11\" />"
      "<CurrencyConversion FromCurrency=\"USD\" ToCurrency=\"EUR\" BSR=\"2.22\" />"
      "<CurrencyConversion FromCurrency=\"EUR\" ToCurrency=\"PLN\" BSR=\"3.33\" />"
      "<CurrencyConversion FromCurrency=\"PLN\" ToCurrency=\"EUR\" BSR=\"4.44\" />"
      "</TestData></TaxRq>";

    parseInputXml(xmlRequestCache);
    CPPUNIT_ASSERT_MESSAGE("Wrong Services/CurrencyConversion number",
                           _xmlCache.currencyConversions().size() == 4);

    CurrencyConversion& currencyConversion = _xmlCache.currencyConversions().at(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong FromCurrency value", type::CurrencyCode("USD"),
                                 currencyConversion.fromCurrency);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong ToCurrency value", type::CurrencyCode("EUR"),
                                 currencyConversion.toCurrency);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong BSR value", type::BSRValue(222,100), currencyConversion.bsr);
  }

  void testCacheServiceBaggage()
  {
    const std::string xmlRequest = "<TaxRq><TestData>\
        <ServiceBaggage ITEMNO=\"207\" VENDOR=\"ATP\">\
        <ServiceBaggageItem APPLTAG=\"X\" TAXTYPESUBCODE=\"001\" TAXCODE=\"QM\"/>\
       </ServiceBaggage>\
       <ServiceBaggage ITEMNO=\"123\" VENDOR=\"ATP\">\
        <ServiceBaggageItem APPLTAG=\"Y\" TAXTYPESUBCODE=\" \" TAXCODE=\" \"/>\
        <ServiceBaggageItem APPLTAG=\"Y\" TAXTYPESUBCODE=\"003\" TAXCODE=\"YQ\"/>\
       </ServiceBaggage>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequest);

    CPPUNIT_ASSERT_EQUAL(2UL, _xmlCache.serviceBaggage().size());
    ServiceBaggage& ca = _xmlCache.serviceBaggage()[1];
    CPPUNIT_ASSERT_EQUAL(tax::type::Vendor("ATP"), ca.vendor);
    CPPUNIT_ASSERT_EQUAL(type::Index(123), ca.itemNo);
    CPPUNIT_ASSERT(ca.entries[0].applTag == type::ServiceBaggageAppl::Positive);
    CPPUNIT_ASSERT_EQUAL(tax::type::TaxTypeOrSubCode("003"), ca.entries[1].taxTypeSubcode);
    CPPUNIT_ASSERT_EQUAL(tax::type::TaxCode("YQ"), ca.entries[1].taxCode);
  }

  void testCacheAKHIFactor()
  {
    const std::string xmlRequest = "<TaxRq><TestData>\
        <AKHIFactor LocCode=\"ABC\" HawaiiPercent=\"38\" ZoneAPercent=\"100\"\
        ZoneBPercent=\"200\" ZoneCPercent=\"250\" ZoneDPercent=\"300\" />\
        <AKHIFactor LocCode=\"XYZ\" HawaiiPercent=\"78\" ZoneAPercent=\"500\"\
        ZoneBPercent=\"550\" ZoneCPercent=\"600\" ZoneDPercent=\"650\" />\
      </TestData></TaxRq>";

    parseInputXml(xmlRequest);

    CPPUNIT_ASSERT_EQUAL(2UL, _xmlCache.aKHIFactor().size());
    AKHIFactor& ak = _xmlCache.aKHIFactor()[1];
    CPPUNIT_ASSERT_EQUAL(tax::type::AirportCode("XYZ"), ak.locCode);
    CPPUNIT_ASSERT_EQUAL(tax::type::Percent(78), ak.hawaiiPercent);
    CPPUNIT_ASSERT_EQUAL(tax::type::Percent(500), ak.zoneAPercent);
    CPPUNIT_ASSERT_EQUAL(tax::type::Percent(550), ak.zoneBPercent);
    CPPUNIT_ASSERT_EQUAL(tax::type::Percent(600), ak.zoneCPercent);
    CPPUNIT_ASSERT_EQUAL(tax::type::Percent(650), ak.zoneDPercent);
  }

  void testCachePassengerTypeCodeItems()
  {
    const std::string xmlRequest = "<TaxRq><TestData>\
        <PassengerTypeCode ITEMNO=\"207\" VENDOR=\"ATP\">\
        <PassengerTypeCodeItem APPLTAG=\"X\" PSGRTYPE=\"ADT\" PSGRMINAGE=\"15\" PSGRMAXAGE=\"200\"\
                               PSGRSTATUS=\"N\" LOCTYPE=\"N\" LOC=\"PL\" PTCMATCHIND=\"I\"/>\
       </PassengerTypeCode>\
       <PassengerTypeCode ITEMNO=\"123\" VENDOR=\"SBR\">\
        <PassengerTypeCodeItem APPLTAG=\"Y\" PSGRTYPE=\"CNN\" PSGRMINAGE=\"2\" PSGRMAXAGE=\"15\"\
                               PSGRSTATUS=\"E\" LOCTYPE=\"S\" LOC=\"DE\" PTCMATCHIND=\"I\"/>\
        <PassengerTypeCodeItem APPLTAG=\" \" PSGRTYPE=\"MIL\" PSGRMINAGE=\"17\" PSGRMAXAGE=\"60\"\
                               PSGRSTATUS=\"R\" LOCTYPE=\"N\" LOC=\"US\" PTCMATCHIND=\"O\"/>\
       </PassengerTypeCode>\
      </TestData></TaxRq>";
    parseInputXml(xmlRequest);
    CPPUNIT_ASSERT_EQUAL(type::Index(2), _xmlCache.passengerTypeCodes().size());
    CPPUNIT_ASSERT_EQUAL(type::Index(1), _xmlCache.passengerTypeCodes()[0].entries.size());
    CPPUNIT_ASSERT_EQUAL(type::Index(207), _xmlCache.passengerTypeCodes()[0].itemNo);
    CPPUNIT_ASSERT_EQUAL(type::Vendor("ATP"), _xmlCache.passengerTypeCodes()[0].vendor);

    PassengerTypeCodeItem* item = &_xmlCache.passengerTypeCodes()[0].entries[0];
    CPPUNIT_ASSERT_EQUAL(type::PtcApplTag(type::PtcApplTag::NotPermitted), item->applTag);
    CPPUNIT_ASSERT_EQUAL(type::PassengerCode("ADT"), item->passengerType);
    CPPUNIT_ASSERT_EQUAL(int16_t(15), item->minimumAge);
    CPPUNIT_ASSERT_EQUAL(int16_t(200), item->maximumAge);
    CPPUNIT_ASSERT_EQUAL(type::PassengerStatusTag(type::PassengerStatusTag::National),
                         item->statusTag);
    CPPUNIT_ASSERT_EQUAL(type::LocType(type::LocType::Nation), item->location.type());
    CPPUNIT_ASSERT_EQUAL(tax::type::LocZoneText("PL"), item->location.code());
    CPPUNIT_ASSERT_EQUAL(type::PtcMatchIndicator(type::PtcMatchIndicator::Input),
                         item->matchIndicator);

    CPPUNIT_ASSERT_EQUAL(type::Index(2), _xmlCache.passengerTypeCodes()[1].entries.size());
    CPPUNIT_ASSERT_EQUAL(type::Index(123), _xmlCache.passengerTypeCodes()[1].itemNo);
    CPPUNIT_ASSERT_EQUAL(type::Vendor("SBR"), _xmlCache.passengerTypeCodes()[1].vendor);

    item = &_xmlCache.passengerTypeCodes()[1].entries[0];
    CPPUNIT_ASSERT_EQUAL(type::PtcApplTag(type::PtcApplTag::Permitted), item->applTag);
    CPPUNIT_ASSERT_EQUAL(type::PassengerCode("CNN"), item->passengerType);
    CPPUNIT_ASSERT_EQUAL(int16_t(2), item->minimumAge);
    CPPUNIT_ASSERT_EQUAL(int16_t(15), item->maximumAge);
    CPPUNIT_ASSERT_EQUAL(type::PassengerStatusTag(type::PassengerStatusTag::Employee),
                         item->statusTag);
    CPPUNIT_ASSERT_EQUAL(type::LocType(type::LocType::StateProvince), item->location.type());
    CPPUNIT_ASSERT_EQUAL(type::LocZoneText("DE"), item->location.code());
    CPPUNIT_ASSERT_EQUAL(type::PtcMatchIndicator(type::PtcMatchIndicator::Input),
                         item->matchIndicator);

    item = &_xmlCache.passengerTypeCodes()[1].entries[1];
    CPPUNIT_ASSERT_EQUAL(type::PtcApplTag(type::PtcApplTag::Blank), item->applTag);
    CPPUNIT_ASSERT_EQUAL(type::PassengerCode("MIL"), item->passengerType);
    CPPUNIT_ASSERT_EQUAL(int16_t(17), item->minimumAge);
    CPPUNIT_ASSERT_EQUAL(int16_t(60), item->maximumAge);
    CPPUNIT_ASSERT_EQUAL(type::PassengerStatusTag(type::PassengerStatusTag::Resident),
                         item->statusTag);
    CPPUNIT_ASSERT_EQUAL(type::LocType(type::LocType::Nation), item->location.type());
    CPPUNIT_ASSERT_EQUAL(type::LocZoneText("US"), item->location.code());
    CPPUNIT_ASSERT_EQUAL(type::PtcMatchIndicator(type::PtcMatchIndicator::Output),
                         item->matchIndicator);
  }

  void testCacheConnectionsTagsEmpty()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord/>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _xmlCache.rulesRecords().at(0).connectionsTags.size());
  }

  void testCacheConnectionsTagsValue()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord CONNECTIONSTAG1=\"F\"/>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Wrong CONNECTIONSTAG1 value",
      type::ConnectionsTag(type::ConnectionsTag::DifferentMarketingCarrier),
      *_xmlCache.rulesRecords().at(0).connectionsTags.begin());
  }

  void testCacheConnectionsTagsBlank()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord CONNECTIONSTAG2=\" \"/>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL(size_t(0), _xmlCache.rulesRecords().at(0).connectionsTags.size());
  }

  void testCacheConnectionsTagsCount()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord CONNECTIONSTAG3=\"J\" CONNECTIONSTAG7=\"E\" CONNECTIONSTAG1=\" \"/>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _xmlCache.rulesRecords().at(0).connectionsTags.size());
  }

  void testServiceFeeSecurityItems()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
        <RulesRecord POSLOCZONETBLNO=\"78342845\"/>\
        <ServiceFeeSecurity VENDOR=\"ATP\" ITEMNO=\"78342843\"/>\
        <ServiceFeeSecurity VENDOR=\"ATP\" ITEMNO=\"78342844\">\
        <ServiceFeeSecurityItem TRAVELAGENCYIND=\"X\" CXRGDSCODE=\"1S\" \
            DUTYFUNCTIONCODE=\"1F\" LOCTYPE=\"C\" LOC=\"KRK\" \
            CODETYPE=\"I\" CODE=\"12412\"/>\
        <ServiceFeeSecurityItem TRAVELAGENCYIND=\" \" CXRGDSCODE=\"LH\" \
            DUTYFUNCTIONCODE=\"1A\" LOCTYPE=\"A\" LOC=\"FRA\" \
            CODETYPE=\"T\" CODE=\"W3H0\"/>\
        </ServiceFeeSecurity>\
      </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);
    CPPUNIT_ASSERT_EQUAL(type::LocZoneText("78342845"), _xmlCache.rulesRecords().at(0).pointOfSale.code());
    CPPUNIT_ASSERT_EQUAL(type::Index(2), _xmlCache.serviceFeeSecurity().size());

    CPPUNIT_ASSERT_EQUAL(type::Vendor("ATP"), _xmlCache.serviceFeeSecurity()[0].vendor);
    CPPUNIT_ASSERT_EQUAL(type::Index(78342843), _xmlCache.serviceFeeSecurity()[0].itemNo);
    CPPUNIT_ASSERT_EQUAL(type::Index(0), _xmlCache.serviceFeeSecurity()[0].entries.size());

    CPPUNIT_ASSERT_EQUAL(type::Vendor("ATP"), _xmlCache.serviceFeeSecurity()[1].vendor);
    CPPUNIT_ASSERT_EQUAL(type::Index(78342844), _xmlCache.serviceFeeSecurity()[1].itemNo);
    CPPUNIT_ASSERT_EQUAL(type::Index(2), _xmlCache.serviceFeeSecurity()[1].entries.size());

    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[0].travelAgencyIndicator,
                         type::TravelAgencyIndicator(type::TravelAgencyIndicator::Agency));
    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[0].carrierGdsCode,
                         type::CarrierGdsCode("1S"));
    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[0].dutyFunctionCode,
                         type::DutyFunctionCode("1F"));
    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[0].location.type(),
                         type::LocType(type::LocType::City));
    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[0].location.code(),
                         type::LocZoneText("KRK"));
    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[0].codeType,
                         type::CodeType(type::CodeType::AgencyNumber));
    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[0].code,
                         std::string("12412"));

    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[1].travelAgencyIndicator,
                         type::TravelAgencyIndicator(type::TravelAgencyIndicator::Blank));
    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[1].carrierGdsCode,
                         type::CarrierGdsCode("LH"));
    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[1].dutyFunctionCode,
                         type::DutyFunctionCode("1A"));
    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[1].location.type(),
                         type::LocType(type::LocType::Area));
    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[1].location.code(),
                         type::LocZoneText("FRA"));
    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[1].codeType,
                         type::CodeType(type::CodeType::AgencyPCC));
    CPPUNIT_ASSERT_EQUAL(_xmlCache.serviceFeeSecurity()[1].entries[1].code,
                         std::string("W3H0"));
  }

  void testCacheReportingRecord()
  {
    const std::string xmlRequest = "<TaxRq><TestData>\
<ReportingRecord VENDOR=\"ATP\" NATION=\"IT\" TAXCARRIER=\"YY\" TAXCODE=\"EX\" TAXTYPE=\"001\">\
<ReportingRecordEntry TAXNAME=\"UPA\"/>\
</ReportingRecord>\
<ReportingRecord VENDOR=\"ATP\" NATION=\"IT\" TAXCARRIER=\"YY\" TAXCODE=\"EX\" TAXTYPE=\"002\">\
<ReportingRecordEntry TAXNAME=\"UPA1\"/>\
<ReportingRecordEntry TAXNAME=\"UPA2\"/>\
</ReportingRecord>\
</TestData></TaxRq>";

    parseInputXml(xmlRequest);
    boost::ptr_vector<ReportingRecord>& reportingRecords = _xmlCache.reportingRecords();
    CPPUNIT_ASSERT_EQUAL(size_t(2), reportingRecords.size());
    {
      ReportingRecord& ca = reportingRecords[0];
      CPPUNIT_ASSERT_EQUAL(tax::type::Vendor("ATP"), ca.vendor);
      CPPUNIT_ASSERT_EQUAL(tax::type::Nation("IT"), ca.nation);
      CPPUNIT_ASSERT_EQUAL(tax::type::CarrierCode("YY"), ca.taxCarrier);
      CPPUNIT_ASSERT_EQUAL(tax::type::TaxCode("EX"), ca.taxCode);
      CPPUNIT_ASSERT_EQUAL(tax::type::TaxType("001"), ca.taxType);
      CPPUNIT_ASSERT_EQUAL(size_t(1), ca.entries.size());
      CPPUNIT_ASSERT(ca.entries[0].taxLabel == "UPA");
    }
    {
      ReportingRecord& ca = reportingRecords[1];
      CPPUNIT_ASSERT_EQUAL(tax::type::TaxType("002"), ca.taxType);
      CPPUNIT_ASSERT_EQUAL(size_t(2), ca.entries.size());
      CPPUNIT_ASSERT(ca.entries[0].taxLabel == "UPA1");
      CPPUNIT_ASSERT(ca.entries[1].taxLabel == "UPA2");
    }
  }

  void testCacheRepricingEntry()
  {
    const std::string xmlRequest = "<TaxRq><TestData>\
<RepricingEntry ItinId=\"0\" TaxPointBegin=\"2\" TaxPointEnd=\"3\" RepricedAmount=\"100\" />\
<RepricingEntry ItinId=\"1\" TaxPointBegin=\"4\" TaxPointEnd=\"5\" RepricedAmount=\"200\" />\
</TestData></TaxRq>";

    parseInputXml(xmlRequest);
    boost::ptr_vector<RepricingEntry>& repricingEntries = _xmlCache.repricingEntries();
    CPPUNIT_ASSERT_EQUAL(size_t(2), repricingEntries.size());
    {
      RepricingEntry& re = repricingEntries[0];
      CPPUNIT_ASSERT_EQUAL(tax::type::Index(0), re.itinId);
      CPPUNIT_ASSERT_EQUAL(tax::type::Index(2), re.taxPointBegin);
      CPPUNIT_ASSERT_EQUAL(tax::type::Index(3), re.taxPointEnd);
      CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(100), re.repricedAmount);
    }
    {
      RepricingEntry& re = repricingEntries[1];
      CPPUNIT_ASSERT_EQUAL(tax::type::Index(1), re.itinId);
      CPPUNIT_ASSERT_EQUAL(tax::type::Index(4), re.taxPointBegin);
      CPPUNIT_ASSERT_EQUAL(tax::type::Index(5), re.taxPointEnd);
      CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(200), re.repricedAmount);
    }
  }

  void testTicketingFee()
  {
    const std::string xmlRequest = "<TaxRq>"
        "<TicketingFee Id=\"0\" Amount=\"100\" TaxAmount=\"10\" ServiceSubTypeCode=\"BG\" />"
        "<TicketingFee Id=\"1\" Amount=\"150\" TaxAmount=\"15\" ServiceSubTypeCode=\"SA\" /></TaxRq>";

    parseInputXml(xmlRequest);
    CPPUNIT_ASSERT_MESSAGE("Wrong TicketingFees number", _inputRequest.ticketingFees().size() == 2);

    CPPUNIT_ASSERT_EQUAL(tax::type::Index(0), _inputRequest.ticketingFees()[0]._id);
    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(100), _inputRequest.ticketingFees()[0]._amount);
    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(10), _inputRequest.ticketingFees()[0]._taxAmount);
    CPPUNIT_ASSERT_EQUAL(tax::type::TktFeeSubCode("BG"), _inputRequest.ticketingFees()[0]._subCode);

    CPPUNIT_ASSERT_EQUAL(tax::type::Index(1), _inputRequest.ticketingFees()[1]._id);
    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(150), _inputRequest.ticketingFees()[1]._amount);
    CPPUNIT_ASSERT_EQUAL(tax::type::MoneyAmount(15), _inputRequest.ticketingFees()[1]._taxAmount);
    CPPUNIT_ASSERT_EQUAL(tax::type::TktFeeSubCode("SA"), _inputRequest.ticketingFees()[1]._subCode);
  }

  void testTicketingFeePath()
  {
    const std::string xmlRequest = "<TaxRq>"
        "<TicketingFeePath Id=\"0\">"
          "<TicketingFeeUsage TicketingFeeRefId=\"0\" />"
          "<TicketingFeeUsage TicketingFeeRefId=\"1\" />"
        "</TicketingFeePath>"
        "<TicketingFeePath Id=\"1\">"
          "<TicketingFeeUsage TicketingFeeRefId=\"0\"/>"
        "</TicketingFeePath>"
        "</TaxRq>";

    parseInputXml(xmlRequest);
    CPPUNIT_ASSERT_MESSAGE("Wrong TicketingFeePaths number", _inputRequest.ticketingFeePaths().size() == 2);
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(0), _inputRequest.ticketingFeePaths()[0]._id);

    InputTicketingFeePath feePath1 = _inputRequest.ticketingFeePaths()[0];
    InputTicketingFeePath feePath2 = _inputRequest.ticketingFeePaths()[1];

    CPPUNIT_ASSERT_EQUAL(tax::type::Index(0), feePath1._id);
    CPPUNIT_ASSERT_MESSAGE("Wrong TicketingFeeUsages number",
        feePath1._ticketingFeeUsages.size() == 2);
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(0), feePath1._ticketingFeeUsages[0]._ticketingFeeRefId);
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(1), feePath1._ticketingFeeUsages[1]._ticketingFeeRefId);

    CPPUNIT_ASSERT_EQUAL(tax::type::Index(1), feePath2._id);
    CPPUNIT_ASSERT_MESSAGE("Wrong TicketingFeeUsages number",
        feePath2._ticketingFeeUsages.size() == 1);
    CPPUNIT_ASSERT_EQUAL(tax::type::Index(0), feePath2._ticketingFeeUsages[0]._ticketingFeeRefId);
  }

  void testCacheCustomer()
  {
    std::string xmlRequestCache = "<TaxRq><TestData>\
          <Customer AgentPCC=\"MSK8\" EXEMPTDUFORJJ=\"1\" EXEMPTDUFORT4=\"0\" EXEMPTDUFORG3=\"1\" />\
          <Customer AgentPCC=\"MSK9\" EXEMPTDUFORJJ=\"0\" EXEMPTDUFORT4=\"1\" EXEMPTDUFORG3=\"0\" />\
        </TestData></TaxRq>";

    parseInputXml(xmlRequestCache);

    CPPUNIT_ASSERT_MESSAGE("Wrong AgentPCC value",
        _xmlCache.customers().at(0)._pcc == type::PseudoCityCode("MSK8"));
    CPPUNIT_ASSERT_MESSAGE("Wrong EXEMPTDUFORJJ value", _xmlCache.customers().at(0)._exemptDuJJ);
    CPPUNIT_ASSERT_MESSAGE("Wrong EXEMPTDUFORT4 value", !_xmlCache.customers().at(0)._exemptDuT4);
    CPPUNIT_ASSERT_MESSAGE("Wrong EXEMPTDUFORG3 value", _xmlCache.customers().at(0)._exemptDuG3);

    CPPUNIT_ASSERT_MESSAGE("Wrong EXEMPTDUFORJJ value", !_xmlCache.customers().at(1)._exemptDuJJ);
    CPPUNIT_ASSERT_MESSAGE("Wrong EXEMPTDUFORT4 value", _xmlCache.customers().at(1)._exemptDuT4);
    CPPUNIT_ASSERT_MESSAGE("Wrong EXEMPTDUFORG3 value", !_xmlCache.customers().at(1)._exemptDuG3);
    }
private:
  Request _request;
  InputRequest _inputRequest;
  XmlCache _xmlCache;

  std::string _xmlRequestGeoPath;
  std::string _xmlRequestYqYrPath;
  std::string _xmlRequestFares;
  std::string _xmlRequestFarePaths;
  std::string _xmlRequestOptionalServices;
  std::string _xmlRequestOptionalServicePaths;
  std::string _xmlRequestGeoPathMapping;
  std::string _xmlRequestFlights;
  std::string _xmlRequestItins;
  std::string _xmlRequestCache;
  std::string _xmlRequestCacheMileage;
  std::string _xmlRequestRulesRecord;

  XmlTagsFactory* _xmlTagsFactory;

  void parseInputXml(std::string xml)
  {
    rapidxml::xml_document<> parsedRequest;
    parsedRequest.parse<0>(&xml[0]);
    const XmlTagsList& tags = selectTagsList(parsedRequest, *_xmlTagsFactory);
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("XML not parsed", XmlParser().parse(_inputRequest, _xmlCache, parsedRequest, tags));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(XmlParserTest);

} // namespace tax
