#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include <ctime>
#include <iostream>
#include <unistd.h>
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/FarePath.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/Diag255Collector.h"
#include "Rules/RuleConst.h"
#include "DataModel/Trx.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/Config/ConfigMan.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;
namespace tse
{
class DiagnosticUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DiagnosticUtilTest);
  CPPUNIT_TEST(testIsvalidForCarrierDiagReq);
  CPPUNIT_TEST(testIsvalidForCarrierDiagReqSecurityFilter);
  CPPUNIT_TEST(testShowFareAmount);
  CPPUNIT_TEST(testIsDisplayKeywordPresent);
  CPPUNIT_TEST(testIsJointVenture);
  CPPUNIT_TEST(testIsAirlineAgent);
  CPPUNIT_TEST(testIsFCRequested);
  CPPUNIT_TEST(testIsFareOwnedByAirline);
  CPPUNIT_TEST(testPtfFilter);
  CPPUNIT_TEST(testPuFilter);
  CPPUNIT_TEST(testFpFilter);
  CPPUNIT_TEST(testContainerToString);
  CPPUNIT_TEST_SUITE_END();

protected:
  class MockFareMarket : public FareMarket
  {
  public:
    MockFareMarket() : FareMarket()
    {
      _loc1.loc() = "DEN";
      _loc2.loc() = "LON";

      origin() = &_loc1;
      destination() = &_loc2;
    }

  protected:
    Loc _loc1;
    Loc _loc2;
  };

  class MockPaxTypeFare : public PaxTypeFare
  {
  public:
    MockPaxTypeFare(TariffCategory tariffCat) : PaxTypeFare()
    {
      _tariffCrossRefInfo._tariffCat = tariffCat;

      _fareInfo.vendor() = "ATP";
      _fareInfo.carrier() = "BA";
      _fareInfo.ruleNumber() = "JP01";
      _fareInfo.fareClass() = "Y";

      _fare.initialize(Fare::FS_ForeignDomestic, &_fareInfo, _fareMarket, &_tariffCrossRefInfo);

      setFare(&_fare);
      fareMarket() = &_fareMarket;
    }

    FareInfo _fareInfo;
    Fare _fare;
    TariffCrossRefInfo _tariffCrossRefInfo;
    MockFareMarket _fareMarket;
  };

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testIsvalidForCarrierDiagReq()
  {
    Diagnostic diagroot(AllFareDiagnostic);
    diagroot.activate();

    Diag255Collector diag(diagroot);
    diag.enable(AllFareDiagnostic);

    CPPUNIT_ASSERT(diag.isActive());

    MockPaxTypeFare ptFare1(0);
    PricingTrx trx;
    CPPUNIT_ASSERT(DiagnosticUtil::isvalidForCarrierDiagReq(trx, ptFare1));

    trx.diagnostic().diagnosticType() = Diagnostic255;
    trx.diagnostic().activate();

    CPPUNIT_ASSERT(DiagnosticUtil::isvalidForCarrierDiagReq(trx, ptFare1));

    // Force a "false" return to make sure showFareAmount() is called:

    MockPaxTypeFare ptFare2(RuleConst::PRIVATE_TARIFF);
    trx.setRequest(new PricingRequest());
    trx.getRequest()->ticketingAgent() = new Agent();
    trx.setOptions(new PricingOptions());
    trx.getOptions()->eprKeywords().insert("PRIMEH");
    trx.billing() = new Billing();
    CPPUNIT_ASSERT(!DiagnosticUtil::isvalidForCarrierDiagReq(trx, ptFare2));
  }

  void testIsvalidForCarrierDiagReqSecurityFilter()
  {
    Diagnostic diagroot(AllFareDiagnostic);
    diagroot.activate();

    Diag255Collector diag(diagroot);
    diag.enable(AllFareDiagnostic);

    CPPUNIT_ASSERT(diag.isActive());

    MockPaxTypeFare ptFare1(0);
    PricingTrx trx;
    CPPUNIT_ASSERT(DiagnosticUtil::isvalidForCarrierDiagReq(trx, ptFare1));

    trx.diagnostic().diagnosticType() = Diagnostic255;
    trx.diagnostic().activate();

    CPPUNIT_ASSERT(DiagnosticUtil::isvalidForCarrierDiagReq(trx, ptFare1));

    // Force a "false" return to make sure showFareAmount() is called:
    MockPaxTypeFare ptFare2(RuleConst::PRIVATE_TARIFF);
    trx.setRequest(new PricingRequest());
    trx.getRequest()->ticketingAgent() = new Agent();
    trx.setOptions(new PricingOptions());
    trx.getOptions()->eprKeywords().insert("PRIMEH");
    trx.billing() = new Billing();
    trx.diagnostic().diagParamMap().insert(std::pair<std::string, std::string>(
        std::string(Diagnostic::FARE_CLASS_CODE), std::string("TESE")));
    CPPUNIT_ASSERT(!DiagnosticUtil::isvalidForCarrierDiagReq(trx, ptFare2));
  }

  void testShowFareAmount()
  {
    PricingTrx trx;
    CPPUNIT_ASSERT(DiagnosticUtil::showFareAmount(true, trx, "", ""));
    CPPUNIT_ASSERT(DiagnosticUtil::showFareAmount(false, trx, "YY", ""));

    // Test airline agent
    trx.setRequest(new PricingRequest());
    trx.getRequest()->ticketingAgent() = new Agent();
    trx.setOptions(new PricingOptions());

    CPPUNIT_ASSERT(DiagnosticUtil::showFareAmount(false, trx, "AA", "AA"));
    CPPUNIT_ASSERT(!DiagnosticUtil::showFareAmount(false, trx, "AA", "BB"));

    // Test joint venture agent
    trx.getOptions()->eprKeywords().insert("PRIMEH");
    CPPUNIT_ASSERT(!DiagnosticUtil::showFareAmount(false, trx, "AA", "AA"));

    Diagnostic diagroot(AllFareDiagnostic);
    diagroot.activate();

    Diag255Collector diag(diagroot);
    diag.enable(AllFareDiagnostic);

    trx.diagnostic().diagnosticType() = Diagnostic255;
    trx.diagnostic().activate();

    std::map<std::string, std::string>& diagParamMap = trx.diagnostic().diagParamMap();
    diagParamMap[Diagnostic::FARE_CLASS_CODE] = "0";

    CPPUNIT_ASSERT(DiagnosticUtil::showFareAmount(false, trx, "AA", "AA"));
  }

  void testIsDisplayKeywordPresent()
  {
    PricingOptions options;
    CPPUNIT_ASSERT(!DiagnosticUtil::isDisplayKeywordPresent(options));

    options.eprKeywords().insert("ABCDEF");
    CPPUNIT_ASSERT(!DiagnosticUtil::isDisplayKeywordPresent(options));

    options.eprKeywords().insert("CRSAGT");
    CPPUNIT_ASSERT(DiagnosticUtil::isDisplayKeywordPresent(options));

    options.eprKeywords().clear();
    options.eprKeywords().insert("TMPCRS");
    CPPUNIT_ASSERT(DiagnosticUtil::isDisplayKeywordPresent(options));

    // test both present
    options.eprKeywords().insert("CRSAGT");
    CPPUNIT_ASSERT(DiagnosticUtil::isDisplayKeywordPresent(options));
  }

  void testIsJointVenture()
  {
    PricingTrx trx;
    trx.setOptions(new PricingOptions());
    trx.getOptions()->eprKeywords().insert("PRIMEH");
    CPPUNIT_ASSERT(DiagnosticUtil::isJointVenture(trx));

    trx.getOptions()->eprKeywords().clear();
    trx.getOptions()->eprKeywords().insert("PRIMEH");
    CPPUNIT_ASSERT(DiagnosticUtil::isJointVenture(trx));

    // test both present
    trx.getOptions()->eprKeywords().insert("PRIMEH");
    CPPUNIT_ASSERT(DiagnosticUtil::isJointVenture(trx));

    trx.getOptions()->eprKeywords().clear();

    trx.setRequest(new PricingRequest());

    // test NULL ticketingAgent()
    CPPUNIT_ASSERT(!DiagnosticUtil::isJointVenture(trx));

    trx.getRequest()->ticketingAgent() = new Agent();
    CPPUNIT_ASSERT(!DiagnosticUtil::isJointVenture(trx));

    trx.getRequest()->ticketingAgent()->cxrCode() = "XX";
    CPPUNIT_ASSERT(!DiagnosticUtil::isJointVenture(trx));

    trx.getRequest()->ticketingAgent()->cxrCode() = "1B";
    CPPUNIT_ASSERT(DiagnosticUtil::isJointVenture(trx));

    trx.getRequest()->ticketingAgent()->cxrCode() = "1J";
    CPPUNIT_ASSERT(DiagnosticUtil::isJointVenture(trx));
  }

  void testIsAirlineAgent()
  {
    PricingTrx trx;
    trx.setRequest(new PricingRequest());

    // test NULL ticketingAgent()
    CPPUNIT_ASSERT(!DiagnosticUtil::isAirlineAgent(trx));

    trx.getRequest()->ticketingAgent() = new Agent();
    trx.setOptions(new PricingOptions());
    CPPUNIT_ASSERT(DiagnosticUtil::isAirlineAgent(trx));

    trx.getRequest()->ticketingAgent()->tvlAgencyPCC() = "ABCD";
    CPPUNIT_ASSERT(!DiagnosticUtil::isAirlineAgent(trx));

    trx.getOptions()->eprKeywords().insert("PRIMEH");
    trx.getRequest()->ticketingAgent()->tvlAgencyPCC() = "";
    CPPUNIT_ASSERT(!DiagnosticUtil::isAirlineAgent(trx));
  }

  void testIsFCRequested()
  {
    PricingTrx trx;
    Diagnostic diagroot(AllFareDiagnostic);
    diagroot.activate();

    Diag255Collector diag(diagroot);
    diag.enable(AllFareDiagnostic);

    CPPUNIT_ASSERT(diag.isActive());

    trx.diagnostic().diagnosticType() = Diagnostic255;
    trx.diagnostic().activate();

    CPPUNIT_ASSERT(!DiagnosticUtil::isFCRequested(trx));

    std::map<std::string, std::string>& diagParamMap = trx.diagnostic().diagParamMap();
    diagParamMap["DUMMY"] = "0";
    CPPUNIT_ASSERT(!DiagnosticUtil::isFCRequested(trx));

    diagParamMap[Diagnostic::FARE_CLASS_CODE] = "0";
    CPPUNIT_ASSERT(DiagnosticUtil::isFCRequested(trx));

    diagParamMap.clear();
    diagParamMap[Diagnostic::ADDON_FARE_CLASS_CODE] = "0";
    CPPUNIT_ASSERT(DiagnosticUtil::isFCRequested(trx));

    // test both
    diagParamMap[Diagnostic::FARE_CLASS_CODE] = "0";
    CPPUNIT_ASSERT(DiagnosticUtil::isFCRequested(trx));
  }

  void testIsFareOwnedByAirline()
  {
    CPPUNIT_ASSERT(DiagnosticUtil::isFareOwnedByAirline("LA", "LA"));
    CPPUNIT_ASSERT(!DiagnosticUtil::isFareOwnedByAirline("LA", "LP"));

    tse::ConfigMan config;
    config.setValue("MCP_PREFERRED_CARRIERS", "LA-LA/LP/4M/XL", "XFORMS_MCP");
    MCPCarrierUtil::initialize(config);

    CPPUNIT_ASSERT(DiagnosticUtil::isFareOwnedByAirline("LA", "LP"));
    CPPUNIT_ASSERT(DiagnosticUtil::isFareOwnedByAirline("LA", "XL"));
    CPPUNIT_ASSERT(!DiagnosticUtil::isFareOwnedByAirline("LP", "XL"));
  }

  void setUpForFilters(std::string itinIndex)
  {
    _memHandle.get(_trx);
    _memHandle.get(_ptf);
    _memHandle.get(_pu);
    _memHandle.get(_fp);

    // PaxFareType initialize arguments
    Fare* fare = 0;
    _memHandle.get(fare);

    FareMarket* fm = 0;
    _memHandle.get(fm);

    std::vector<TravelSeg*> key;
    std::vector<uint16_t> val;
    val.push_back(3);
    val.push_back(5);
    val.push_back(7);
    val.push_back(9);

    _ptf->initialize(fare, 0, fm, *_trx);

    _trx->diagnostic().diagParamMap().insert(std::make_pair("ITIN_INDEX", itinIndex));
  }

  void testPtfFilter()
  {
    setUpForFilters("7");
    CPPUNIT_ASSERT(!DiagnosticUtil::filter(*_trx, *_ptf));
  }

  void testPuFilter()
  {
    setUpForFilters("7");
    CPPUNIT_ASSERT(!DiagnosticUtil::filter(*_trx, *_pu));
  }

  void testFpFilter()
  {
    setUpForFilters("7");
    CPPUNIT_ASSERT(!DiagnosticUtil::filter(*_trx, *_fp));
  }

  void testContainerToString()
  {
    std::vector<CarrierCode> v1;
    v1.push_back("AA");
    v1.push_back("AF");
    v1.push_back("BA");
    v1.push_back("UA");
    v1.push_back("LH");

    CPPUNIT_ASSERT_EQUAL(std::string("AA,AF,BA,UA,LH"), DiagnosticUtil::containerToString(v1));
    CPPUNIT_ASSERT_EQUAL(std::string("AA|AF|BA|UA|LH"), DiagnosticUtil::containerToString(v1, true));

    std::set<CarrierCode> s1(v1.begin(), v1.end());

    CPPUNIT_ASSERT_EQUAL(std::string("AA,AF,BA,LH,UA"), DiagnosticUtil::containerToString(s1));
    CPPUNIT_ASSERT_EQUAL(std::string("AA|AF|BA|LH|UA"), DiagnosticUtil::containerToString(s1, true));



  }

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PaxTypeFare* _ptf;
  PricingUnit* _pu;
  FarePath* _fp;
};
CPPUNIT_TEST_SUITE_REGISTRATION(DiagnosticUtilTest);
}
