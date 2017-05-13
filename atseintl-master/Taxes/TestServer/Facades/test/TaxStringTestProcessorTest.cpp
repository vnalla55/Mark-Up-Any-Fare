#include "test/include/CppUnitHelperMacros.h"
#include "TestServer/Facades/TaxStringTestProcessor.h"

namespace tax
{

class TaxStringTestProcessorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxStringTestProcessorTest);
  CPPUNIT_TEST(nonXmlInput);
  CPPUNIT_TEST(unknownRootElement);
  CPPUNIT_TEST(badXmlContent);
  CPPUNIT_TEST_SUITE_END();

public:

  void nonXmlInput()
  {
    std::string input = "**WTF**";
    TaxStringTestProcessor proc;
    CPPUNIT_ASSERT(proc.processString(input).getResponseMessage().empty());
  }

  void unknownRootElement()
  {
    std::string input = "<BadRoot><Content/></BadRoot>";
    TaxStringTestProcessor proc;
    CPPUNIT_ASSERT(proc.processString(input).getResponseMessage().empty());
  }

  void badXmlContent()
  {
    std::string input = "<TaxRq><BadContent/></TaxRq>";
    TaxStringTestProcessor proc;
    std::string ans = proc.processString(input).getResponseMessage();
    CPPUNIT_ASSERT(ans.find("<Error>") != ans.npos);
    CPPUNIT_ASSERT(ans.find("<Itins>") == ans.npos);
  }

  void validXmlContent()
  {
    std::string input =
      "<TaxRq>"
      "  <PointOfSale PointOfSaleLoc=\"TYO\" />"
      "  <ProcessingOptions />"
      "  <TicketingOptions FormOfPayment=\" \" PaymentCurrency=\"USD\" TicketingDate=\"2013-04-30\" TicketingTime=\"11:15\" />"
      "  <GeoPath>"
      "    <Geo Type=\"D\" Loc=\"BJS\" />"
      "    <Geo Type=\"A\" Loc=\"TYO\" />"
      "  </GeoPath>"
      "  <Passenger PassengerCode=\"ADT\" />"
      "  <Fare BasisCode=\"WFM0FR\" TypeCode=\"PSD\" OneWayRoundTrip=\"O\" Directionality=\" \" Amount=\"12\" />"
      "  <FarePath TotalAmount=\"34\" >"
      "    <FareUsage FareRefId=\"0\" />"
      "  </FarePath>"
      "  <GeoPathMapping>"
      "    <Mapping>"
      "      <Map GeoRefId=\"0\" />"
      "      <Map GeoRefId=\"1\" />"
      "    </Mapping>"
      "  </GeoPathMapping>"
      "  <Itin PassengerRefId=\"0\" Id=\"0\" GeoPathRefId=\"0\" FarePathRefId=\"0\" FarePathGeoPathMappingRefId=\"0\" />"
      "</TaxRq>";

    TaxStringTestProcessor proc;
    std::string ans = proc.processString(input).getResponseMessage();
    CPPUNIT_ASSERT(ans.find("<Error>") == ans.npos);
    CPPUNIT_ASSERT(ans.find("<Itins>") != ans.npos);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxStringTestProcessorTest);

} // namespace Tax
