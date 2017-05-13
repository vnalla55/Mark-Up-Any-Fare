//-------------------------------------------------------------------
//
//  File:        S8BrandingServiceCallerTest.cpp
//  Created:     April 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "BrandingService/BrandingServiceCaller.h"
#include "Common/BrandingUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diag891Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{
class S8MockBrandingServiceCaller : public BrandingServiceCaller<S8>
{
public:
  S8MockBrandingServiceCaller(PricingTrx& trx) : BrandingServiceCaller<S8>(trx) {}
};

class S8BrandingServiceCallerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(S8BrandingServiceCallerTest);
  CPPUNIT_TEST(testCreateDiag_False);
  CPPUNIT_TEST(testCreateDiag891_True);
  CPPUNIT_TEST(testCreateDiag990_False);
  CPPUNIT_TEST(testDisplayResponse_FareDisplay_True);
  CPPUNIT_TEST(testDisplayResponse_Pricing_Diag_NotActive_True);
  CPPUNIT_TEST(testDisplayResponse_Pricing_Diag_Active_False);
  CPPUNIT_TEST(testNoBrandingUrl);
  CPPUNIT_TEST(testInvalidBrandingUrl);
  CPPUNIT_TEST(testBrandingServiceAvailable);
  CPPUNIT_TEST(testBrandingServiceUnavailable);
  CPPUNIT_TEST(testPrintDiagnosticFdRequest);
  CPPUNIT_TEST(testPrintDiagnosticFdResponse);
  CPPUNIT_TEST(testPrintDiagnosticPricingRequest);
  CPPUNIT_TEST(testPrintDiagnosticPricingResponse);
  CPPUNIT_TEST(testDisplayResponse_Diag_INVALID_TRX_True);
  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _pricingTrx = _memHandle.create<PricingTrx>();
    _fareDisplayTrx = _memHandle.create<FareDisplayTrx>();
    _pricingOptions = _memHandle.create<PricingOptions>();
    _fareDisplayReq = _memHandle.create<FareDisplayRequest>();
    _pricingTrx->setRequest(_fareDisplayReq);
    _memHandle.insert(_pricingServiceCallerMock = new S8MockBrandingServiceCaller(*_pricingTrx));
    _memHandle.insert(_fareDisplayServiceCallerMock =
                          new S8MockBrandingServiceCaller(*_fareDisplayTrx));
    _fareDisplayTrx->setRequest(_fareDisplayReq);
    _pricingServiceCallerMock->_flushDiagMsg = false;
  }

  void tearDown() { _memHandle.clear(); }

  void getResponse(std::string& xml)
  {
    xml = ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
           "<GetAirlineBrandsRS xmlns=\"http://services.sabre.com/Branding/V1\" "
           "xmlns:ns2=\"http://opentravel.org/common/message/v02\" "
           "xmlns:ns3=\"http://opentravel.org/common/v02\" "
           "xmlns:ns4=\"http://services.sabre.com/STL_Payload/v02_00\" "
           "xmlns:ns5=\"http://services.sabre.com/STL/v02\">"
           "<BrandingResponse>"
           "<ResponseSource pseudoCityCode=\"80K2\" iATANumber=\"9999999\" "
           "clientID=\"FQ\" requestType=\"\" requestingCarrierGDS=\"\" "
           "GEOLocation=\"MIA\" departmentCode=\"80K2\" officeDesignator=\"\"/>"
           "<BrandingResults>"
           "<CarrierBrandsData/>"
           "</BrandingResults>"
           "</BrandingResponse>"
           "</GetAirlineBrandsRS>");
  }

  void getRequest(std::string& xml)
  {
    xml = ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
           "<GetAirlineBrandsRQ xmlns=\"http://stl.sabre.com/Merchandising/v1\" "
           "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
           "xsi:schemaLocation=\"http://services.sabre.com/Branding/V1 "
           "../../AirlineBranding_1_0_0.xsd\">"
           "<BrandingRequest transactID=\"9889915546558883622\" version=\"01\">"
           "<RequestSource GEOLocation=\"MIA\" clientID=\"FQ\" departmentCode=\"80K2\" "
           "iATANumber=\"9999999\" officeDesignator=\"\" pseudoCityCode=\"80K2\" "
           "requestType=\"\" requestingCarrierGDS=\"1S\"/>"
           "<XRA MID=\"mid\" CID=\"cid\"/>"
           "<BrandingCriteria>"
           "<MarketRequest marketID=\"1\">"
           "<MarketCriteria direction=\"\" globalIndicator=\"\">"
           "<DepartureDate>2013-08-20</DepartureDate>"
           "<DepartureAirportCode>LAX</DepartureAirportCode>"
           "<ArrivalAirportCode>SYD</ArrivalAirportCode>"
           "<PassengerTypes>"
           "<Type>ADT</Type>"
           "<Type>NEG</Type>"
           "</PassengerTypes>"
           "</MarketCriteria>"
           "<CarrierList>"
           "<Carrier>DL</Carrier>"
           "</CarrierList>"
           "<ExclCarrierList/>"
           "</MarketRequest>"
           "<AccountCodeList/>"
           "<SalesDate>2013-08-08 13:36:00</SalesDate>"
           "</BrandingCriteria>"
           "</BrandingRequest>"
           "</GetAirlineBrandsRQ>");
  }

  void createDiag(DiagnosticTypes diagType = Diagnostic891)
  {
    _pricingTrx->diagnostic().diagnosticType() = diagType;
    if (diagType != DiagnosticNone)
    {
      _pricingTrx->diagnostic().activate();
    }
  }

  void createFdDiag(DiagnosticTypes diagType = Diagnostic195)
  {
    _fareDisplayTrx->diagnostic().diagnosticType() = diagType;
    _fareDisplayTrx->getRequest()->diagnosticNumber() = diagType;
    if (diagType != DiagnosticNone)
    {
      _fareDisplayTrx->diagnostic().activate();
    }
  }

  void getRequestExpectedString(std::string& expected)
  {
    expected = ("***************************************************************\n"
                " S8BRAND XML REQUEST - START\n"
                "***************************************************************\n"
                " \n"
                " -GETAIRLINEBRANDSRQ-                                          \n"
                "  SCHEMALOCATION-:HTTP://SERVICES.SABRE.COM/BRANDING/V1 ../../A\n"
                "IRLINEBRANDING_1_0_0.XSD:                                      \n"
                "  -BRANDINGREQUEST-                                            \n"
                "   TRANSACTID-:9889915546558883622:                            \n"
                "   VERSION-:01:                                                \n"
                "   -REQUESTSOURCE-                                             \n"
                "    GEOLOCATION-:MIA:                                          \n"
                "    CLIENTID-:FQ:                                              \n"
                "    DEPARTMENTCODE-:80K2:                                      \n"
                "    IATANUMBER-:9999999:                                       \n"
                "    OFFICEDESIGNATOR-::                                        \n"
                "    PSEUDOCITYCODE-:80K2:                                      \n"
                "    REQUESTTYPE-::                                             \n"
                "    REQUESTINGCARRIERGDS-:1S:                                  \n"
                "   -/REQUESTSOURCE                                             \n"
                "   -XRA-                                                       \n"
                "    MID-:MID:                                                  \n"
                "    CID-:CID:                                                  \n"
                "   -/XRA                                                       \n"
                "   -BRANDINGCRITERIA-                                          \n"
                "    -MARKETREQUEST-                                            \n"
                "     MARKETID-:1:                                              \n"
                "     -MARKETCRITERIA-                                          \n"
                "      DIRECTION-::                                             \n"
                "      GLOBALINDICATOR-::                                       \n"
                "      -DEPARTUREDATE-2013-08-20-/DEPARTUREDATE                 \n"
                "      -DEPARTUREAIRPORTCODE-LAX-/DEPARTUREAIRPORTCODE          \n"
                "      -ARRIVALAIRPORTCODE-SYD-/ARRIVALAIRPORTCODE              \n"
                "      -PASSENGERTYPES-                                         \n"
                "       -TYPE-ADT-/TYPE                                         \n"
                "       -TYPE-NEG-/TYPE                                         \n"
                "      -/PASSENGERTYPES                                         \n"
                "     -/MARKETCRITERIA                                          \n"
                "     -CARRIERLIST-                                             \n"
                "      -CARRIER-DL-/CARRIER                                     \n"
                "     -/CARRIERLIST                                             \n"
                "     -EXCLCARRIERLIST-                                         \n"
                "     -/EXCLCARRIERLIST                                         \n"
                "    -/MARKETREQUEST                                            \n"
                "    -ACCOUNTCODELIST-                                          \n"
                "    -/ACCOUNTCODELIST                                          \n"
                "    -SALESDATE-2013-08-08 13:36:00-/SALESDATE                  \n"
                "   -/BRANDINGCRITERIA                                          \n"
                "  -/BRANDINGREQUEST                                            \n"
                " -/GETAIRLINEBRANDSRQ                                          \n"
                "\n \n"
                "***************************************************************\n"
                " S8BRAND XML REQUEST - END\n"
                "***************************************************************\n \n");
  }

  void getRequestExpectedStringFareDisplay(std::string& expected)
  {
    expected = ("***************************************************************\n"
                " S8BRAND XML REQUEST - START\n"
                "***************************************************************\n"
                " \n"
                "- XML\n"
                " VERSION-:1.0:\n"
                " ENCODING-:UTF-8: \n"
                "-GETAIRLINEBRANDSRQ\n"
                " XMLNS-:HTTP://STL.SABRE.COM/MERCHANDISING/V1:\n"
                " XMLNS:XSI-:HTTP://WWW.W3.ORG/2001/XMLSCHEMA-INSTANCE:\n"
                " XSI:SCHEMALOCATION-:HTTP://SERVICES.SABRE.COM/BRANDING/V1 ../.\n"
                "./AIRLINEBRANDING_1_0_0.XSD:\n"
                " -BRANDINGREQUEST\n"
                "  TRANSACTID-:9889915546558883622:\n"
                "  VERSION-:01:\n"
                "  -REQUESTSOURCE\n"
                "   GEOLOCATION-:MIA:\n"
                "   CLIENTID-:FQ:\n"
                "   DEPARTMENTCODE-:80K2:\n"
                "   IATANUMBER-:9999999:\n"
                "   OFFICEDESIGNATOR-::\n"
                "   PSEUDOCITYCODE-:80K2:\n"
                "   REQUESTTYPE-::\n"
                "   REQUESTINGCARRIERGDS-:1S: \n"
                "  -XRA\n"
                "   MID-:MID:\n"
                "   CID-:CID: \n"
                "  -BRANDINGCRITERIA\n"
                "   -MARKETREQUEST\n"
                "    MARKETID-:1:\n"
                "    -MARKETCRITERIA\n"
                "     DIRECTION-::\n"
                "     GLOBALINDICATOR-::\n"
                "     -DEPARTUREDATE-2013-08-20-/DEPARTUREDATE\n"
                "     -DEPARTUREAIRPORTCODE-LAX-/DEPARTUREAIRPORTCODE\n"
                "     -ARRIVALAIRPORTCODE-SYD-/ARRIVALAIRPORTCODE\n"
                "     -PASSENGERTYPES\n"
                "      -TYPE-ADT-/TYPE\n"
                "      -TYPE-NEG-/TYPE\n"
                "     -/PASSENGERTYPES\n"
                "    -/MARKETCRITERIA\n"
                "    -CARRIERLIST\n"
                "     -CARRIER-DL-/CARRIER\n"
                "    -/CARRIERLIST\n"
                "    -EXCLCARRIERLIST \n"
                "   -/MARKETREQUEST\n"
                "   -ACCOUNTCODELIST \n"
                "   -SALESDATE-2013-08-0813:36:00-/SALESDATE\n"
                "  -/BRANDINGCRITERIA\n"
                " -/BRANDINGREQUEST\n"
                "-/GETAIRLINEBRANDSRQ\n\n \n"
                "***************************************************************\n"
                " S8BRAND XML REQUEST - END\n"
                "***************************************************************\n \n");
  }

  void getResponseExpectedString(std::string& expected)
  {
    expected += ("***************************************************************\n"
                 " S8BRAND XML RESPONSE - START\n"
                 "***************************************************************\n"
                 " \n"
                 " -GETAIRLINEBRANDSRS-                                          \n"
                 "  -BRANDINGRESPONSE-                                           \n"
                 "   -RESPONSESOURCE-                                            \n"
                 "    PSEUDOCITYCODE-:80K2:                                      \n"
                 "    IATANUMBER-:9999999:                                       \n"
                 "    CLIENTID-:FQ:                                              \n"
                 "    REQUESTTYPE-::                                             \n"
                 "    REQUESTINGCARRIERGDS-::                                    \n"
                 "    GEOLOCATION-:MIA:                                          \n"
                 "    DEPARTMENTCODE-:80K2:                                      \n"
                 "    OFFICEDESIGNATOR-::                                        \n"
                 "   -/RESPONSESOURCE                                            \n"
                 "   -BRANDINGRESULTS-                                           \n"
                 "    -CARRIERBRANDSDATA-                                        \n"
                 "    -/CARRIERBRANDSDATA                                        \n"
                 "   -/BRANDINGRESULTS                                           \n"
                 "  -/BRANDINGRESPONSE                                           \n"
                 " -/GETAIRLINEBRANDSRS                                          \n"
                 "\n \n"
                 "***************************************************************\n"
                 " S8BRAND XML RESPONSE - END\n"
                 "***************************************************************\n \n");
  }

  void getResponseExpectedStringFareDisplay(std::string& expected)
  {
    expected += ("***************************************************************\n"
                 " S8BRAND XML RESPONSE - START\n"
                 "***************************************************************\n"
                 " \n"
                 "- XML\n"
                 " VERSION-:1.0:\n"
                 " ENCODING-:UTF-8:\n"
                 " STANDALONE-:YES: \n"
                 "-GETAIRLINEBRANDSRS\n"
                 " XMLNS-:HTTP://SERVICES.SABRE.COM/BRANDING/V1:\n"
                 " XMLNS:NS2-:HTTP://OPENTRAVEL.ORG/COMMON/MESSAGE/V02:\n"
                 " XMLNS:NS3-:HTTP://OPENTRAVEL.ORG/COMMON/V02:\n"
                 " XMLNS:NS4-:HTTP://SERVICES.SABRE.COM/STL_PAYLOAD/V02_00:\n"
                 " XMLNS:NS5-:HTTP://SERVICES.SABRE.COM/STL/V02:\n"
                 " -BRANDINGRESPONSE\n"
                 "  -RESPONSESOURCE\n"
                 "   PSEUDOCITYCODE-:80K2:\n"
                 "   IATANUMBER-:9999999:\n"
                 "   CLIENTID-:FQ:\n"
                 "   REQUESTTYPE-::\n"
                 "   REQUESTINGCARRIERGDS-::\n"
                 "   GEOLOCATION-:MIA:\n"
                 "   DEPARTMENTCODE-:80K2:\n"
                 "   OFFICEDESIGNATOR-:: \n"
                 "  -BRANDINGRESULTS\n"
                 "   -CARRIERBRANDSDATA \n"
                 "  -/BRANDINGRESULTS\n"
                 " -/BRANDINGRESPONSE\n"
                 "-/GETAIRLINEBRANDSRS\n\n \n"
                 "***************************************************************\n"
                 " S8BRAND XML RESPONSE - END\n"
                 "***************************************************************\n \n");
  }

  void testCreateDiag_False()
  {
    _pricingTrx->setTrxType(PricingTrx::PRICING_TRX);
    CPPUNIT_ASSERT(!_pricingServiceCallerMock->createDiag());
  }

  void testCreateDiag891_True()
  {
    createDiag();
    _pricingTrx->setTrxType(PricingTrx::PRICING_TRX);
    bool status = _pricingServiceCallerMock->createDiag();
    CPPUNIT_ASSERT(status);
    CPPUNIT_ASSERT(_pricingServiceCallerMock->_diag891);
  }

  void testCreateDiag990_False()
  {
    DiagnosticTypes diagType = Diagnostic990;
    createDiag(diagType);
    _pricingTrx->setTrxType(PricingTrx::PRICING_TRX);
    CPPUNIT_ASSERT(!_pricingServiceCallerMock->createDiag());
  }

  void testDisplayResponse_FareDisplay_True()
  {
    std::string response;
    getResponse(response);
    CPPUNIT_ASSERT(!_pricingServiceCallerMock->displayXML(response, "BRAND XML RESPONSE"));
  }

  void testDisplayResponse_Pricing_Diag_NotActive_True()
  {
    std::string response;
    getResponse(response);
    _pricingTrx->setTrxType(PricingTrx::PRICING_TRX);
    CPPUNIT_ASSERT(!_pricingServiceCallerMock->displayXML(response, "BRAND XML RESPONSE"));
  }

  bool isContains(const std::string& msg, const std::string& phrase)
  {
    return msg.find(phrase) != std::string::npos;
  }

  void testDisplayResponse_Pricing_Diag_Active_False()
  {
    createDiag();
    _pricingTrx->setTrxType(PricingTrx::PRICING_TRX);
    std::string response;
    getResponse(response);
    CPPUNIT_ASSERT(!_pricingServiceCallerMock->displayXML(response, "BRAND XML RESPONSE"));
    CPPUNIT_ASSERT(isContains(_pricingServiceCallerMock->_diag891->str(), "BRAND XML RESPONSE"));
  }

  void testNoBrandingUrl()
  {
    std::string request, response;
    bool rc = _pricingServiceCallerMock->callBranding(request, response);
    CPPUNIT_ASSERT(rc == false);
  }

  void testInvalidBrandingUrl()
  {
    unsetenv("http_proxy");
    SpecificTestConfigInitializer::setValue("URL", "TestURL", "S8_BRAND_SVC");
    std::string request, response;
    bool rc = _pricingServiceCallerMock->callBranding(request, response);
    CPPUNIT_ASSERT(rc == false);
  }

  void testBrandingServiceAvailable()
  {
    createDiag();
    _pricingTrx->setTrxType(PricingTrx::PRICING_TRX);
    std::string response;
    getResponse(response);
    CPPUNIT_ASSERT(!_pricingServiceCallerMock->displayXML(response, "BRAND XML RESPONSE"));
    CPPUNIT_ASSERT(_pricingServiceCallerMock->_diag891->str().find(
                       "BRANDING SERVICE IS UNAVAILABLE") == std::string::npos);
  }

  void testBrandingServiceUnavailable()
  {
    createDiag();
    _pricingTrx->setTrxType(PricingTrx::PRICING_TRX);
    std::string response;
    getResponse(response);
    CPPUNIT_ASSERT(!_pricingServiceCallerMock->displayXML(
        response, "BRAND XML RESPONSE", StatusBrandingService::BS_UNAVAILABLE));
    CPPUNIT_ASSERT(_pricingServiceCallerMock->_diag891->str().find(
                       "BRANDING SERVICE IS UNAVAILABLE") != std::string::npos);
  }

  void testPrintDiagnosticFdRequest()
  {
    std::string request, response;
    getRequest(request);
    createFdDiag(Diagnostic195);
    _fareDisplayTrx->diagnostic().diagParamMap().insert(
        std::make_pair(Diagnostic::DISPLAY_DETAIL, "S8BRANDREQ"));
    _fareDisplayTrx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
    _fareDisplayServiceCallerMock->printDiagnostic(
        request, response, StatusBrandingService::NO_BS_ERROR);
    std::string expected;
    getRequestExpectedStringFareDisplay(expected);
    CPPUNIT_ASSERT_EQUAL(expected, _fareDisplayTrx->response().str());
  }

  void testPrintDiagnosticFdResponse()
  {
    std::string request, response;
    getResponse(response);
    createFdDiag(Diagnostic195);
    _fareDisplayTrx->diagnostic().diagParamMap().insert(
        std::make_pair(Diagnostic::DISPLAY_DETAIL, "S8BRANDRES"));
    _fareDisplayTrx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
    _fareDisplayServiceCallerMock->printDiagnostic(
        request, response, StatusBrandingService::NO_BS_ERROR);
    std::string expected;
    getResponseExpectedStringFareDisplay(expected);
    CPPUNIT_ASSERT_EQUAL(expected, _fareDisplayTrx->response().str());
  }

  void testPrintDiagnosticPricingRequest()
  {
    std::string request, response;
    getRequest(request);
    createDiag(Diagnostic890);
    _pricingTrx->setTrxType(PricingTrx::PRICING_TRX);
    _pricingServiceCallerMock->printPricingShoppingDiagnostic(
        request, response, StatusBrandingService::NO_BS_ERROR);
    std::string expected;
    getRequestExpectedString(expected);
    CPPUNIT_ASSERT_EQUAL(expected, _pricingServiceCallerMock->_diag890->str());
  }

  void testPrintDiagnosticPricingResponse()
  {
    std::string request, response;
    getResponse(response);
    createDiag(Diagnostic891);
    _pricingTrx->setTrxType(PricingTrx::PRICING_TRX);
    _pricingServiceCallerMock->printPricingShoppingDiagnostic(
        request, response, StatusBrandingService::NO_BS_ERROR);
    std::string expected;
    getResponseExpectedString(expected);
    CPPUNIT_ASSERT_EQUAL(expected, _pricingServiceCallerMock->_diag891->str());
  }

  void testDisplayResponse_Diag_INVALID_TRX_True()
  {
    std::string response;
    getResponse(response);
    _pricingTrx->setTrxType(PricingTrx::REPRICING_TRX);
    CPPUNIT_ASSERT(_pricingServiceCallerMock->displayXML(response, "BRAND XML RESPONSE"));
  }

protected:
  PricingOptions* _pricingOptions;
  FareDisplayRequest* _fareDisplayReq;
  PricingTrx* _pricingTrx;
  FareDisplayTrx* _fareDisplayTrx;
  S8MockBrandingServiceCaller* _pricingServiceCallerMock;
  S8MockBrandingServiceCaller* _fareDisplayServiceCallerMock;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(S8BrandingServiceCallerTest);

} // tse
