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
#include <stdexcept>
#include "test/include/CppUnitHelperMacros.h"

#include "Taxes/AtpcoTaxes/ServiceInterfaces/DefaultServices.h"
#include "TestServer/Xform/StringToRequestBuilder.h"
#include "Taxes/AtpcoTaxes/Rules/XmlParsingError.h"
#include "TestServer/Xform/XmlTagsFactory.h"
#include "TestServer/Xform/NaturalXmlTagsList.h"
#include "TestServer/Xform/TestCacheBuilder.h"

namespace tax
{

class StringToRequestBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StringToRequestBuilderTest);

  CPPUNIT_TEST(testBuildRequest_empty);
  CPPUNIT_TEST(testBuildRequest);
  CPPUNIT_TEST(testBuildRequest_buildXmlCache);

  CPPUNIT_TEST(testSetServices);

  CPPUNIT_TEST(testProcessTaxes_diagnostic);

  CPPUNIT_TEST(testConvertResponse);

  CPPUNIT_TEST_SUITE_END();

public:
  StringToRequestBuilderTest()
  : _xmlTagsFactory(0)
  {
  }

  void setUp()
  {
    _receivedXml =
        "<TaxRq>"
        "<TicketingOptions TicketingDate=\"2013-05-06\" />"
        "<TicketingOptions TicketingTime=\"02:18\" />"
        "<TicketingOptions PaymentCurrency=\"IJK\" />"
        "<TestData>"
        "<Customer AgentPCC=\"MSK8\" EXEMPTDUFORJJ=\"1\" EXEMPTDUFORT4=\"0\" EXEMPTDUFORG3=\"1\" />"
        "<Loc Id=\"3\" LocCode=\"KRK\" NationCode=\"PL\" CityCode=\"ABC\" />"
        "<Mileage GeoPathRefId=\"0\">"
        "<Distance FromGeoRefId =\"0\" ToGeoRefId=\"1\" Miles=\"100\" />"
        "</Mileage>"
        "<CarrierFlt VENDOR=\"ATP\" ITEMNO=\"4396\">"
        "<CarrierFltSeg FLT1=\"0051\" FLT2=\"0056\"MARKETINGCARRIER=\"LH\" OPERATINGCARRIER=\"AA\" "
        "/>"
        "</CarrierFlt>"
        "<CarrierApplication ITEMNO=\"1000\" VENDOR=\"ATP\">"
        "<CarrierApplEntry APPLIND=\" \" CARRIER=\"BA\" />"
        "</CarrierApplication>"
        "<CurrencyConversion FromCurrency=\"EUR\" ToCurrency=\"USD\" BSR=\"1.11\" />"
        "<AKHIFactor LocCode=\"ABC\" HawaiiPercent=\"38\" ZoneAPercent=\"100\" ZoneBPercent=\"200\""
        "ZoneCPercent=\"250\" ZoneDPercent=\"300\" />"
        "<ServiceBaggage ITEMNO=\"207\" VENDOR=\"ATP\">"
        "<ServiceBaggageItem APPLTAG=\"X\" TAXTYPESUBCODE=\"001\" TAXCODE=\"QM\"/>"
        "</ServiceBaggage>"
        "<PassengerTypeCode ITEMNO=\"207\" VENDOR=\"ATP\">"
        "<PassengerTypeCodeItem APPLTAG=\"X\" PSGRTYPE=\"ADT\" PSGRMINAGE=\"15\""
        "PSGRMAXAGE=\"200\" PSGRSTATUS=\"N\" LOCTYPE=\"N\" LOC=\"PL\" PTCMATCHIND=\"I\"/>"
        "</PassengerTypeCode>"
        "<ReportingRecord VENDOR=\"ATP\" NATION=\"IT\" TAXCARRIER=\"YY\" TAXCODE=\"EX\" "
        "TAXTYPE=\"001\">"
        "<ReportingRecordEntry TAXNAME=\"UPA\"/>"
        "</ReportingRecord>"
        "<ServiceFeeSecurity VENDOR=\"ATP\" ITEMNO=\"178342844\">"
        "<ServiceFeeSecurityItem TRAVELAGENCYIND=\"X\" CXRGDSCODE=\"unused\" \
          DUTYFUNCTIONCODE=\"unused\" LOCTYPE=\" \" LOC=\"unused\" \
          CODETYPE=\"unused\" CODE=\"unused\"/>"
        "</ServiceFeeSecurity>"
        "</TestData><Diagnostic Id=\"802\" /></TaxRq>";

    _xmlTagsFactory = new XmlTagsFactory();
    _xmlTagsFactory->registerList(new NaturalXmlTagsList);
  }

  void tearDown() { delete _xmlTagsFactory; }

  void testBuildRequest_empty()
  {
    std::string receivedXml;
    StringToRequestBuilder builder(*_xmlTagsFactory);

    CPPUNIT_ASSERT_THROW(builder.buildRequest(receivedXml), std::domain_error);
  }

  void testBuildRequest()
  {
    const std::string receivedXml = "<TaxRq>\
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

    StringToRequestBuilder builder(*_xmlTagsFactory);
    CPPUNIT_ASSERT_EQUAL(size_t(0), builder.getInputRequest().geoPaths().size());

    builder.buildRequest(receivedXml);
    CPPUNIT_ASSERT_EQUAL(size_t(2), builder.getInputRequest().geoPaths().size());
  }

  void testBuildRequest_buildXmlCache()
  {
    StringToRequestBuilder builder(*_xmlTagsFactory);
    builder.buildRequest(_receivedXml);

    CPPUNIT_ASSERT_EQUAL(false, builder._xmlCache.nations().empty());
    CPPUNIT_ASSERT_EQUAL(false, builder._xmlCache.mileages().empty());
    CPPUNIT_ASSERT_EQUAL(false, builder._xmlCache.carrierFlights().empty());
    CPPUNIT_ASSERT_EQUAL(false, builder._xmlCache.carrierApplications().empty());
    CPPUNIT_ASSERT_EQUAL(false, builder._xmlCache.currencyConversions().empty());
    CPPUNIT_ASSERT_EQUAL(false, builder._xmlCache.aKHIFactor().empty());
    CPPUNIT_ASSERT_EQUAL(false, builder._xmlCache.serviceBaggage().empty());
    CPPUNIT_ASSERT_EQUAL(false, builder._xmlCache.passengerTypeCodes().empty());
    CPPUNIT_ASSERT_EQUAL(false, builder._xmlCache.reportingRecords().empty());
    CPPUNIT_ASSERT_EQUAL(false, builder._xmlCache.serviceFeeSecurity().empty());
  }

  void testSetServices()
  {
    StringToRequestBuilder builder(*_xmlTagsFactory);
    builder.buildRequest(_receivedXml);

    DefaultServices services;
    TestCacheBuilder().buildCache(services, builder.getXmlCache());

    CPPUNIT_ASSERT_NO_THROW(services.locService());
  }

  void testProcessTaxes_diagnostic()
  {
    StringToRequestBuilder builder(*_xmlTagsFactory);
    builder.buildRequest(_receivedXml);
    //   builder.processTaxes();

    //   CPPUNIT_ASSERT(!builder.getResponse()._diagnosticResponse->_messages.empty());
  }

  void testConvertResponse()
  {
    StringToRequestBuilder builder(*_xmlTagsFactory);
    builder.buildRequest(_receivedXml);
    //   builder.processTaxes();
    //   builder.convertResponse();

    //   CPPUNIT_ASSERT(!builder.getResponseMessage().empty());
  }

private:
  std::string _receivedXml;
  XmlTagsFactory* _xmlTagsFactory;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StringToRequestBuilderTest);

} // namespace tax
