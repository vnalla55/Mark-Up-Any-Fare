#include "test/include/CppUnitHelperMacros.h"
#include <iostream>
#include <time.h>
#include <vector>
#include <set>

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DiskCache.h"
#include "Rules/RuleUtil.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/GeoRuleItem.h"
#include "Diagnostic/DCFactory.h"

#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include <boost/assign/std/vector.hpp>
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

using namespace boost::assign;
using namespace tse;

class RuleUtilTest2 : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RuleUtilTest2);
  CPPUNIT_TEST(testCheckTravelSegLoc_matchLoc1);
  CPPUNIT_TEST(testCheckTravelSegLoc_matchLoc2);
  CPPUNIT_TEST(testCheckTravelSegLoc_NoMatchDFW);
  CPPUNIT_TEST(testCheckTravelSegLoc_NoMatchLoc1);
  CPPUNIT_TEST(testCheckTravelSegLoc_NoMatchLoc2);

  CPPUNIT_TEST(testCheckTravelSeg_checkOrig_checkDest_NoFltStp);
  CPPUNIT_TEST(testCheckTravelSeg_checkOrig_checkDest_FltStp_NoHS);
  CPPUNIT_TEST(testCheckTravelSeg_NoCheckOrig_NoCheckDest_FltStp_MatchHS1);
  CPPUNIT_TEST(testCheckTravelSeg_NoCheckOrig_NoCheckDest_FltStp_MatchHS2);
  CPPUNIT_TEST(testCheckTravelSeg_NoCheckOrig_NoCheckDest_FltStp_NoMatchHS);

  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_ChDest_ChOrig_ChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_ChDest_ChOrig_NoChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_ChDest_NoChOrig_ChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_ChDest_NoChOrig_NoChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_NoChDest_ChOrig_ChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_NoChDest_ChOrig_NoChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_NoChDest_NoChOrig_ChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_NoChDest_NoChOrig_NoChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_ChDest_ChOrig_ChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_ChDest_ChOrig_NoChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_ChDest_NoChOrig_ChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_ChDest_NoChOrig_NoChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_NoChDest_ChOrig_ChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_NoChDest_ChOrig_NoChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_NoChDest_NoChOrig_ChDest);
  CPPUNIT_TEST(testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_NoChDest_NoChOrig_NoChDest);

  CPPUNIT_TEST(testValidateGeoRuleItemCreateDiag_Diag);
  CPPUNIT_TEST(testValidateGeoRuleItemCreateDiag_NoDDGEO);
  CPPUNIT_TEST(testValidateGeoRuleItemCreateDiag_NoCallerDiag);
  CPPUNIT_TEST(testValidateGeoRuleItemCreateDiag_DiffCallerDiag);

  CPPUNIT_TEST(testValidateGeoRuleItemWriteGeoRule_Diag);
  CPPUNIT_TEST(testValidateGeoRuleItemWriteGeoRule_Loc1None);
  CPPUNIT_TEST(testValidateGeoRuleItemWriteGeoRule_Loc2None);
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
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _options = _memHandle.create<PricingOptions>();
    _trx->setRequest(_request);
    _trx->setOptions(_options);
    _locSFO = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    _locSYD = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    _segSFOSYD = _memHandle.create<AirSeg>();
    _segSFOSYD->origin() = _locSFO;
    _segSFOSYD->destination() = _locSYD;
    _callerDiag = Diagnostic315;
    _trx->diagnostic().activate();
    _trx->diagnostic().diagnosticType() = Diagnostic315;
    _trx->diagnostic().diagParamMap().insert(std::pair<std::string, std::string>(
        Diagnostic::DISPLAY_DETAIL, RuleConst::DIAGNOSTIC_INCLUDE_GEO));
  }
  void tearDown() { _memHandle.clear(); }

  void testCheckTravelSegLoc_matchLoc1() { assertCheckTravelSegLoc(true, _locSFO); }
  void testCheckTravelSegLoc_matchLoc2() { assertCheckTravelSegLoc(true, _locSYD); }
  void testCheckTravelSegLoc_NoMatchDFW()
  {
    assertCheckTravelSegLoc(false,
                            TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml"));
  }
  void testCheckTravelSegLoc_NoMatchLoc1() { assertCheckTravelSegLoc(true, _locSFO, "KRK", "SFO"); }
  void testCheckTravelSegLoc_NoMatchLoc2() { assertCheckTravelSegLoc(true, _locSYD, "SFO", "SYD"); }
  void testCheckTravelSeg_checkOrig_checkDest_NoFltStp()
  {
    assertCheckTravelSeg(true, true, true, false, RuleUtil::LOGIC_AND);
  }
  void testCheckTravelSeg_checkOrig_checkDest_FltStp_NoHS()
  {
    assertCheckTravelSeg(true, true, true, true, RuleUtil::LOGIC_OR);
  }
  void testCheckTravelSeg_NoCheckOrig_NoCheckDest_FltStp_MatchHS1()
  {
    _segSFOSYD->hiddenStops().push_back(_locSFO);
    assertCheckTravelSeg(true, false, false, true, RuleUtil::LOGIC_OR);
  }
  void testCheckTravelSeg_NoCheckOrig_NoCheckDest_FltStp_MatchHS2()
  {
    _segSFOSYD->hiddenStops().push_back(_locSYD);
    assertCheckTravelSeg(true, false, false, true, RuleUtil::LOGIC_OR);
  }
  void testCheckTravelSeg_NoCheckOrig_NoCheckDest_FltStp_NoMatchHS()
  {
    _segSFOSYD->hiddenStops().push_back(
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml"));
    assertCheckTravelSeg(false, false, false, true, RuleUtil::LOGIC_OR);
  }

  void testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_ChDest_ChOrig_ChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(false, true, true, true, true);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_ChDest_ChOrig_NoChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(false, true, true, true, false);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_ChDest_NoChOrig_ChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(false, true, true, false, true);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_ChDest_NoChOrig_NoChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(false, true, true, false, false);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_NoChDest_ChOrig_ChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(false, true, false, true, true);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_NoChDest_ChOrig_NoChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(true, true, false, true, false);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_NoChDest_NoChOrig_ChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(false, true, false, false, true);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_ChOrig_NoChDest_NoChOrig_NoChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(true, true, false, false, false);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_ChDest_ChOrig_ChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(false, false, true, true, true);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_ChDest_ChOrig_NoChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(false, false, true, true, false);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_ChDest_NoChOrig_ChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(true, false, true, false, true);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_ChDest_NoChOrig_NoChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(true, false, true, false, false);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_NoChDest_ChOrig_ChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(false, false, false, true, true);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_NoChDest_ChOrig_NoChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(false, false, false, true, false);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_NoChDest_NoChOrig_ChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(true, false, false, false, true);
  }
  void testGetTvlSegBtwAndGeoDirectConflicted_NoChOrig_NoChDest_NoChOrig_NoChDest()
  {
    assertGetTvlSegBtwAndGeoDirectConflicted(true, false, false, false, false);
  }
  void testValidateGeoRuleItemCreateDiag_Diag()
  {
    std::string msg = " \n"
                      "---- TABLE995 INPUT DATA ----\n"
                      "ITEM NO: 99  VENDOR: ATP  SCOPE: SUB JOURNEY\n"
                      "FARE MARKET-   ORIG: SFO  DEST: SYD  CARRIER: LO\n";
    assertValidateGeoRuleItemCreateDiag(true, msg);
  }
  void testValidateGeoRuleItemCreateDiag_NoDDGEO()
  {
    _trx->diagnostic().diagParamMap().clear();
    assertValidateGeoRuleItemCreateDiag(false, "");
  }
  void testValidateGeoRuleItemCreateDiag_NoCallerDiag()
  {
    _callerDiag = DiagnosticNone;
    assertValidateGeoRuleItemCreateDiag(false, "");
  }
  void testValidateGeoRuleItemCreateDiag_DiffCallerDiag()
  {
    _callerDiag = Diagnostic301;
    assertValidateGeoRuleItemCreateDiag(false, "");
  }
  void testValidateGeoRuleItemWriteGeoRule_Diag()
  {
    std::string msg = "\n---- TABLE995 DB DATA ----\n"
                      "LOC1- P-SYD\n"
                      "LOC2- P-SFO\n"
                      "TSI-     NO: 1\n \n\n";
    assertvalidateGeoRuleItemWriteGeoRule(getGeoRuleItem("SYD", "SFO"), msg);
  }
  void testValidateGeoRuleItemWriteGeoRule_Loc1None()
  {
    std::string msg = "\n---- TABLE995 DB DATA ----\n"
                      "LOC1-    NULL\n"
                      "LOC2- P-SFO\n"
                      "TSI-     NO: 1\n \n\n";
    assertvalidateGeoRuleItemWriteGeoRule(getGeoRuleItem("", "SFO"), msg);
  }
  void testValidateGeoRuleItemWriteGeoRule_Loc2None()
  {
    std::string msg = "\n---- TABLE995 DB DATA ----\n"
                      "LOC1- P-SYD\n"
                      "LOC2-    NULL\n"
                      "TSI-     NO: 1\n \n\n";
    assertvalidateGeoRuleItemWriteGeoRule(getGeoRuleItem("SYD", ""), msg);
  }

  void assertvalidateGeoRuleItemWriteGeoRule(GeoRuleItem* geo, std::string msg)
  {
    DCFactory* factory = DCFactory::instance();
    DiagCollector* diag = factory->create(*_trx);
    diag->enable(_callerDiag);
    RuleUtil::validateGeoRuleItemWriteGeoRule(geo, _callerDiag, factory, *diag);
    CPPUNIT_ASSERT_EQUAL(msg, _trx->diagnostic().toString());
  }
  void assertValidateGeoRuleItemCreateDiag(bool ret, std::string msg)
  {
    uint32_t itemNo(99);
    VendorCode vendorCode("ATP");
    RuleConst::TSIScopeParamType defaultScope(RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY);
    FareMarket fm;
    fm.origin() = _locSFO;
    fm.destination() = _locSYD;
    fm.governingCarrier() = "LO";
    DCFactory* factory = 0;
    DiagCollector* diag = 0;
    CPPUNIT_ASSERT(ret ==
                   RuleUtil::validateGeoRuleItemCreateDiag(
                       *_trx, _callerDiag, factory, diag, itemNo, vendorCode, defaultScope, &fm));
    if (ret)
    {
      CPPUNIT_ASSERT(factory != 0);
      CPPUNIT_ASSERT(diag != 0);
      CPPUNIT_ASSERT_EQUAL(msg, diag->str());
    }
    else
    {
      CPPUNIT_ASSERT(factory == 0);
      CPPUNIT_ASSERT(diag == 0);
    }
  }

  void
  assertGetTvlSegBtwAndGeoDirectConflicted(bool ret, bool orig1, bool dest1, bool orig2, bool dest2)
  {
    bool o1(orig1), d1(dest1), o2(orig2), d2(dest2);
    CPPUNIT_ASSERT(ret == RuleUtil::getTvlSegBtwAndGeoDirectConflicted(o1, d1, o2, d2));
  }
  void assertCheckTravelSeg(
      bool ret, bool origCheck, bool destCheck, bool fltStopCheck, const Indicator chkLogic)
  {
    bool orig(origCheck), dest(destCheck), fltStop(fltStopCheck);
    GeoRuleItem* geo = getGeoRuleItem(std::string("SYD"), std::string("SFO"));
    CPPUNIT_ASSERT(
        ret == RuleUtil::checkTravelSeg(
                   *_trx, geo, _segSFOSYD, orig, dest, fltStop, chkLogic, "ATP", GeoTravelType::International));
  }
  void
  assertCheckTravelSegLoc(bool ret, Loc* loc, std::string loc1 = "SYD", std::string loc2 = "SFO")
  {
    GeoRuleItem* geo = getGeoRuleItem(loc1, loc2);
    CPPUNIT_ASSERT(ret == RuleUtil::checkTravelSegLoc(
                              *_trx, geo, *loc, "ATP", GeoTravelType::International, LocUtil::FLIGHT_RULE));
  }
  GeoRuleItem* getGeoRuleItem(std::string loc1, std::string loc2)
  {
    GeoRuleItem* geo = _memHandle.create<GeoRuleItem>();
    geo->tsi() = 1;
    geo->loc1().loc() = loc1;
    geo->loc1().locType() = loc1.size() ? 'P' : ' ';
    geo->loc2().loc() = loc2;
    geo->loc2().locType() = loc2.size() ? 'P' : ' ';
    return geo;
  }

protected:
  PricingTrx* _trx;
  PricingRequest* _request;
  PricingOptions* _options;
  Loc* _locSFO;
  Loc* _locSYD;
  AirSeg* _segSFOSYD;
  TestMemHandle _memHandle;
  DiagnosticTypes _callerDiag;
};
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTest2);
