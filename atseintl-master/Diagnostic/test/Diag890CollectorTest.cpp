#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "Diagnostic/Diag890Collector.h"
#include "DataModel/PricingTrx.h"

using namespace std;

namespace tse
{

class Diag890CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag890CollectorTest);
  CPPUNIT_TEST(testDisplayXML_old);
  CPPUNIT_TEST(testDisplayXML_old_noFormat);
  CPPUNIT_TEST(testDisplayXML);
  CPPUNIT_TEST(testDisplayXML_noFormat);
  CPPUNIT_TEST(testDisplayXML_CData);


  CPPUNIT_TEST_SUITE_END();

private:
  Diag890Collector* _diag;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _req;
  ConfigMan* _configMan;
public:
  //---------------------------------------------------------------------
  // testConstructor()
  //---------------------------------------------------------------------
  void testConstructor()
  {
    try
    {
      Diag890Collector diag;
      CPPUNIT_ASSERT_EQUAL(string(""), diag.str());
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _configMan = _memHandle.create<ConfigMan>();
    try
    {
      _diagroot = _memHandle.insert(new Diagnostic(Diagnostic890));
      _diagroot->activate();
      _diag = _memHandle.insert(new Diag890Collector(*_diagroot));
      _diag->enable(Diagnostic890);
      _trx = _memHandle.create<PricingTrx>();
      _diag->trx() = _trx;
      _req = _memHandle.create<PricingRequest>();
      _trx->setRequest(_req);
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void tearDown() { _memHandle.clear(); }

  void testDisplayXML_old()
  {
    std::string xmlRequest;
    DateTime travelDate = DateTime(2013, 6, 22);
    xmlRequest =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?><GetAirlineBrandsRQ "
        "xmlns=\"http://services.sabre.com/Branding/V1\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xsi:schemaLocation=\"http://services.sabre.com/Branding/V1 "
        "../../AirlineBranding_1_0_0.xsd\"><BrandingRequest transactID=\"0\" "
        "version=\"01\"><RequestSource GEOLocation=\"DFW\" clientID=\"FQ\" departmentCode=\"XYZ\" "
        "iATANumber=\"XYZ\" officeDesignator=\"\" pseudoCityCode=\"HDQ\" requestType=\"\" "
        "requestingCarrierGDS=\"1B\"/><BrandingCriteria><MarketRequest "
        "marketID=\"1\"><MarketCriteria direction=\"\" globalIndicator=\"\"><DepartureDate>";
    xmlRequest += travelDate.dateToString(YYYYMMDD, "-");
    xmlRequest += "</DepartureDate><DepartureAirportCode>SYD</"
                  "DepartureAirportCode><ArrivalAirportCode>MEL</"
                  "ArrivalAirportCode><PassengerTypes><Type>ADT</Type></PassengerTypes></"
                  "MarketCriteria><CarrierList><Carrier></Carrier></CarrierList><ExclCarrierList/"
                  "></MarketRequest><AccountCodeList/><SalesDate>";
    xmlRequest += travelDate.dateToString(YYYYMMDD, "-");
    xmlRequest += "</SalesDate></BrandingCriteria></BrandingRequest></GetAirlineBrandsRQ>";

    _diag->displayXML_old(xmlRequest, "BRAND XML REQUEST", true);

    CPPUNIT_ASSERT_EQUAL(string("***************************************************************\n"
                                " BRAND XML REQUEST - START\n"
                                "***************************************************************\n"
                                " \n"
                                " -GETAIRLINEBRANDSRQ-                                          \n"
                                "  SCHEMALOCATION-:HTTP://SERVICES.SABRE.COM/BRANDING/V1 ../../A\n"
                                "IRLINEBRANDING_1_0_0.XSD:                                      \n"
                                "  -BRANDINGREQUEST-                                            \n"
                                "   TRANSACTID-:0:                                              \n"
                                "   VERSION-:01:                                                \n"
                                "   -REQUESTSOURCE-                                             \n"
                                "    GEOLOCATION-:DFW:                                          \n"
                                "    CLIENTID-:FQ:                                              \n"
                                "    DEPARTMENTCODE-:XYZ:                                       \n"
                                "    IATANUMBER-:XYZ:                                           \n"
                                "    OFFICEDESIGNATOR-::                                        \n"
                                "    PSEUDOCITYCODE-:HDQ:                                       \n"
                                "    REQUESTTYPE-::                                             \n"
                                "    REQUESTINGCARRIERGDS-:1B:                                  \n"
                                "   -/REQUESTSOURCE                                             \n"
                                "   -BRANDINGCRITERIA-                                          \n"
                                "    -MARKETREQUEST-                                            \n"
                                "     MARKETID-:1:                                              \n"
                                "     -MARKETCRITERIA-                                          \n"
                                "      DIRECTION-::                                             \n"
                                "      GLOBALINDICATOR-::                                       \n"
                                "      -DEPARTUREDATE-2013-06-22-/DEPARTUREDATE                 \n"
                                "      -DEPARTUREAIRPORTCODE-SYD-/DEPARTUREAIRPORTCODE          \n"
                                "      -ARRIVALAIRPORTCODE-MEL-/ARRIVALAIRPORTCODE              \n"
                                "      -PASSENGERTYPES-                                         \n"
                                "       -TYPE-ADT-/TYPE                                         \n"
                                "      -/PASSENGERTYPES                                         \n"
                                "     -/MARKETCRITERIA                                          \n"
                                "     -CARRIERLIST-                                             \n"
                                "      -CARRIER-                                                \n"
                                "      -/CARRIER                                                \n"
                                "     -/CARRIERLIST                                             \n"
                                "     -EXCLCARRIERLIST-                                         \n"
                                "     -/EXCLCARRIERLIST                                         \n"
                                "    -/MARKETREQUEST                                            \n"
                                "    -ACCOUNTCODELIST-                                          \n"
                                "    -/ACCOUNTCODELIST                                          \n"
                                "    -SALESDATE-2013-06-22-/SALESDATE                           \n"
                                "   -/BRANDINGCRITERIA                                          \n"
                                "  -/BRANDINGREQUEST                                            \n"
                                " -/GETAIRLINEBRANDSRQ                                          \n"
                                "\n"
                                " \n"
                                "***************************************************************\n"
                                " BRAND XML REQUEST - END\n"
                                "***************************************************************\n"
                                " \n"),
                         _diag->str());
  }

  void testDisplayXML_old_noFormat()
  {
    std::string xmlRequest;
    DateTime travelDate = DateTime(2013, 6, 22);
    xmlRequest =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?><GetAirlineBrandsRQ "
        "xmlns=\"http://services.sabre.com/Branding/V1\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xsi:schemaLocation=\"http://services.sabre.com/Branding/V1 "
        "../../AirlineBranding_1_0_0.xsd\"><BrandingRequest transactID=\"0\" "
        "version=\"01\"><RequestSource GEOLocation=\"DFW\" clientID=\"FQ\" departmentCode=\"XYZ\" "
        "iATANumber=\"XYZ\" officeDesignator=\"\" pseudoCityCode=\"HDQ\" requestType=\"\" "
        "requestingCarrierGDS=\"1B\"/><BrandingCriteria><MarketRequest "
        "marketID=\"1\"><MarketCriteria direction=\"\" globalIndicator=\"\"><DepartureDate>";
    xmlRequest += travelDate.dateToString(YYYYMMDD, "-");
    xmlRequest += "</DepartureDate><DepartureAirportCode>SYD</"
                  "DepartureAirportCode><ArrivalAirportCode>MEL</"
                  "ArrivalAirportCode><PassengerTypes><Type>ADT</Type></PassengerTypes></"
                  "MarketCriteria><CarrierList><Carrier></Carrier></CarrierList><ExclCarrierList/"
                  "></MarketRequest><AccountCodeList/><SalesDate>";
    xmlRequest += travelDate.dateToString(YYYYMMDD, "-");
    xmlRequest += "</SalesDate></BrandingCriteria></BrandingRequest></GetAirlineBrandsRQ>";

    _diag->displayXML_old(xmlRequest, "BRAND XML REQUEST", false);

    CPPUNIT_ASSERT_EQUAL(
        string("***************************************************************\n"
               " BRAND XML REQUEST - START\n"
               "***************************************************************\n"
               " \n"
               "<?xml version=\"1.0\" encoding=\"UTF-8\"?><GetAirlineBrandsRQ "
               "xmlns=\"http://services.sabre.com/Branding/V1\" "
               "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
               "xsi:schemaLocation=\"http://services.sabre.com/Branding/V1 "
               "../../AirlineBranding_1_0_0.xsd\"><BrandingRequest transactID=\"0\" "
               "version=\"01\"><RequestSource GEOLocation=\"DFW\" clientID=\"FQ\" "
               "departmentCode=\"XYZ\" iATANumber=\"XYZ\" officeDesignator=\"\" "
               "pseudoCityCode=\"HDQ\" requestType=\"\" "
               "requestingCarrierGDS=\"1B\"/><BrandingCriteria><MarketRequest "
               "marketID=\"1\"><MarketCriteria direction=\"\" "
               "globalIndicator=\"\"><DepartureDate>2013-06-22</"
               "DepartureDate><DepartureAirportCode>SYD</"
               "DepartureAirportCode><ArrivalAirportCode>MEL</"
               "ArrivalAirportCode><PassengerTypes><Type>ADT</Type></PassengerTypes></"
               "MarketCriteria><CarrierList><Carrier></Carrier></CarrierList><ExclCarrierList/></"
               "MarketRequest><AccountCodeList/><SalesDate>2013-06-22</SalesDate></"
               "BrandingCriteria></BrandingRequest></GetAirlineBrandsRQ>\n"
               "\n"
               "***************************************************************\n"
               " BRAND XML REQUEST - END\n"
               "***************************************************************\n"
               " \n"),
        _diag->str());
  }

  void testDisplayXML()
    {
      std::string xmlRequest;
      DateTime travelDate = DateTime(2013, 6, 22);
      xmlRequest =
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?><GetAirlineBrandsRQ "
          "xmlns=\"http://services.sabre.com/Branding/V1\" "
          "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
          "xsi:schemaLocation=\"http://services.sabre.com/Branding/V1 "
          "../../AirlineBranding_1_0_0.xsd\"><BrandingRequest transactID=\"0\" "
          "version=\"01\"><RequestSource GEOLocation=\"DFW\" clientID=\"FQ\" departmentCode=\"XYZ\" "
          "iATANumber=\"XYZ\" officeDesignator=\"\" pseudoCityCode=\"HDQ\" requestType=\"\" "
          "requestingCarrierGDS=\"1B\"/><BrandingCriteria><MarketRequest "
          "marketID=\"1\"><MarketCriteria direction=\"\" globalIndicator=\"\"><DepartureDate>";
      xmlRequest += travelDate.dateToString(YYYYMMDD, "-");
      xmlRequest += "</DepartureDate><DepartureAirportCode>SYD</"
                    "DepartureAirportCode><ArrivalAirportCode>MEL</"
                    "ArrivalAirportCode><PassengerTypes><Type>ADT</Type></PassengerTypes></"
                    "MarketCriteria><CarrierList><Carrier></Carrier></CarrierList><ExclCarrierList/"
                    "></MarketRequest><AccountCodeList/><SalesDate>";
      xmlRequest += travelDate.dateToString(YYYYMMDD, "-");
      xmlRequest += "</SalesDate></BrandingCriteria></BrandingRequest></GetAirlineBrandsRQ>";

      _diag->displayXML(xmlRequest, "BRAND XML REQUEST", BS_GREEN_SCREEN_FORMAT);

      CPPUNIT_ASSERT_EQUAL(string("***************************************************************\n"
                                  " BRAND XML REQUEST - START\n"
                                  "***************************************************************\n"
                                  " \n"
                                  " -GETAIRLINEBRANDSRQ-                                          \n"
                                  "  SCHEMALOCATION-:HTTP://SERVICES.SABRE.COM/BRANDING/V1 ../../A\n"
                                  "IRLINEBRANDING_1_0_0.XSD:                                      \n"
                                  "  -BRANDINGREQUEST-                                            \n"
                                  "   TRANSACTID-:0:                                              \n"
                                  "   VERSION-:01:                                                \n"
                                  "   -REQUESTSOURCE-                                             \n"
                                  "    GEOLOCATION-:DFW:                                          \n"
                                  "    CLIENTID-:FQ:                                              \n"
                                  "    DEPARTMENTCODE-:XYZ:                                       \n"
                                  "    IATANUMBER-:XYZ:                                           \n"
                                  "    OFFICEDESIGNATOR-::                                        \n"
                                  "    PSEUDOCITYCODE-:HDQ:                                       \n"
                                  "    REQUESTTYPE-::                                             \n"
                                  "    REQUESTINGCARRIERGDS-:1B:                                  \n"
                                  "   -/REQUESTSOURCE                                             \n"
                                  "   -BRANDINGCRITERIA-                                          \n"
                                  "    -MARKETREQUEST-                                            \n"
                                  "     MARKETID-:1:                                              \n"
                                  "     -MARKETCRITERIA-                                          \n"
                                  "      DIRECTION-::                                             \n"
                                  "      GLOBALINDICATOR-::                                       \n"
                                  "      -DEPARTUREDATE-2013-06-22-/DEPARTUREDATE                 \n"
                                  "      -DEPARTUREAIRPORTCODE-SYD-/DEPARTUREAIRPORTCODE          \n"
                                  "      -ARRIVALAIRPORTCODE-MEL-/ARRIVALAIRPORTCODE              \n"
                                  "      -PASSENGERTYPES-                                         \n"
                                  "       -TYPE-ADT-/TYPE                                         \n"
                                  "      -/PASSENGERTYPES                                         \n"
                                  "     -/MARKETCRITERIA                                          \n"
                                  "     -CARRIERLIST-                                             \n"
                                  "      -CARRIER-                                                \n"
                                  "      -/CARRIER                                                \n"
                                  "     -/CARRIERLIST                                             \n"
                                  "     -EXCLCARRIERLIST-                                         \n"
                                  "     -/EXCLCARRIERLIST                                         \n"
                                  "    -/MARKETREQUEST                                            \n"
                                  "    -ACCOUNTCODELIST-                                          \n"
                                  "    -/ACCOUNTCODELIST                                          \n"
                                  "    -SALESDATE-2013-06-22-/SALESDATE                           \n"
                                  "   -/BRANDINGCRITERIA                                          \n"
                                  "  -/BRANDINGREQUEST                                            \n"
                                  " -/GETAIRLINEBRANDSRQ                                          \n"
                                  "\n"
                                  " \n"
                                  "***************************************************************\n"
                                  " BRAND XML REQUEST - END\n"
                                  "***************************************************************\n"
                                  " \n"),
                           _diag->str());
      CPPUNIT_ASSERT(_diagroot->getAdditionalData().empty());
    }

    void testDisplayXML_noFormat()
    {
      std::string xmlRequest;
      DateTime travelDate = DateTime(2013, 6, 22);
      xmlRequest =
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?><GetAirlineBrandsRQ "
          "xmlns=\"http://services.sabre.com/Branding/V1\" "
          "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
          "xsi:schemaLocation=\"http://services.sabre.com/Branding/V1 "
          "../../AirlineBranding_1_0_0.xsd\"><BrandingRequest transactID=\"0\" "
          "version=\"01\"><RequestSource GEOLocation=\"DFW\" clientID=\"FQ\" departmentCode=\"XYZ\" "
          "iATANumber=\"XYZ\" officeDesignator=\"\" pseudoCityCode=\"HDQ\" requestType=\"\" "
          "requestingCarrierGDS=\"1B\"/><BrandingCriteria><MarketRequest "
          "marketID=\"1\"><MarketCriteria direction=\"\" globalIndicator=\"\"><DepartureDate>";
      xmlRequest += travelDate.dateToString(YYYYMMDD, "-");
      xmlRequest += "</DepartureDate><DepartureAirportCode>SYD</"
                    "DepartureAirportCode><ArrivalAirportCode>MEL</"
                    "ArrivalAirportCode><PassengerTypes><Type>ADT</Type></PassengerTypes></"
                    "MarketCriteria><CarrierList><Carrier></Carrier></CarrierList><ExclCarrierList/"
                    "></MarketRequest><AccountCodeList/><SalesDate>";
      xmlRequest += travelDate.dateToString(YYYYMMDD, "-");
      xmlRequest += "</SalesDate></BrandingCriteria></BrandingRequest></GetAirlineBrandsRQ>";

      _diag->displayXML(xmlRequest, "BRAND XML REQUEST", BS_NO_FORMAT);

      CPPUNIT_ASSERT_EQUAL(
          string("***************************************************************\n"
                 " BRAND XML REQUEST - START\n"
                 "***************************************************************\n"
                 " \n"
                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?><GetAirlineBrandsRQ "
                 "xmlns=\"http://services.sabre.com/Branding/V1\" "
                 "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                 "xsi:schemaLocation=\"http://services.sabre.com/Branding/V1 "
                 "../../AirlineBranding_1_0_0.xsd\"><BrandingRequest transactID=\"0\" "
                 "version=\"01\"><RequestSource GEOLocation=\"DFW\" clientID=\"FQ\" "
                 "departmentCode=\"XYZ\" iATANumber=\"XYZ\" officeDesignator=\"\" "
                 "pseudoCityCode=\"HDQ\" requestType=\"\" "
                 "requestingCarrierGDS=\"1B\"/><BrandingCriteria><MarketRequest "
                 "marketID=\"1\"><MarketCriteria direction=\"\" "
                 "globalIndicator=\"\"><DepartureDate>2013-06-22</"
                 "DepartureDate><DepartureAirportCode>SYD</"
                 "DepartureAirportCode><ArrivalAirportCode>MEL</"
                 "ArrivalAirportCode><PassengerTypes><Type>ADT</Type></PassengerTypes></"
                 "MarketCriteria><CarrierList><Carrier></Carrier></CarrierList><ExclCarrierList/></"
                 "MarketRequest><AccountCodeList/><SalesDate>2013-06-22</SalesDate></"
                 "BrandingCriteria></BrandingRequest></GetAirlineBrandsRQ>\n"
                 "\n"
                 "***************************************************************\n"
                 " BRAND XML REQUEST - END\n"
                 "***************************************************************\n"
                 " \n"),
          _diag->str());

      CPPUNIT_ASSERT(_diagroot->getAdditionalData().empty());
    }

    void testDisplayXML_CData()
    {
      std::string xmlRequest;
      DateTime travelDate = DateTime(2013, 6, 22);
      xmlRequest =
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?><GetAirlineBrandsRQ "
          "xmlns=\"http://services.sabre.com/Branding/V1\" "
          "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
          "xsi:schemaLocation=\"http://services.sabre.com/Branding/V1 "
          "../../AirlineBranding_1_0_0.xsd\"><BrandingRequest transactID=\"0\" "
          "version=\"01\"><RequestSource GEOLocation=\"DFW\" clientID=\"FQ\" departmentCode=\"XYZ\" "
          "iATANumber=\"XYZ\" officeDesignator=\"\" pseudoCityCode=\"HDQ\" requestType=\"\" "
          "requestingCarrierGDS=\"1B\"/><BrandingCriteria><MarketRequest "
          "marketID=\"1\"><MarketCriteria direction=\"\" globalIndicator=\"\"><DepartureDate>";
      xmlRequest += travelDate.dateToString(YYYYMMDD, "-");
      xmlRequest += "</DepartureDate><DepartureAirportCode>SYD</"
          "DepartureAirportCode><ArrivalAirportCode>MEL</"
          "ArrivalAirportCode><PassengerTypes><Type>ADT</Type></PassengerTypes></"
          "MarketCriteria><CarrierList><Carrier></Carrier></CarrierList><ExclCarrierList/"
          "></MarketRequest><AccountCodeList/><SalesDate>";
      xmlRequest += travelDate.dateToString(YYYYMMDD, "-");
      xmlRequest += "</SalesDate></BrandingCriteria></BrandingRequest></GetAirlineBrandsRQ>";

      _diag->displayXML(xmlRequest, "BRAND XML REQUEST", BS_CDATA_SECTION);

      CPPUNIT_ASSERT_EQUAL(
          string("***************************************************************\n"
              " BRAND XML REQUEST - START\n"
              "***************************************************************\n"
              " \n"
              " BRAND SERVICE MESSAGE AVAILABLE IN CDATA SECTION\n"
              "***************************************************************\n"
              " BRAND XML REQUEST - END\n"
              "***************************************************************\n"
              " \n"),
              _diag->str());
      std::string expectedOutput = "<![CDATA[" + xmlRequest + "]]>\n";
      CPPUNIT_ASSERT_EQUAL(expectedOutput, _diagroot->getAdditionalData());
    }

};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag890CollectorTest);
} // tse
