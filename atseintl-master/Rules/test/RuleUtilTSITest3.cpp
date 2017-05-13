#include "Rules/test/RuleUtilTSITest_Base.h"
#include "DataModel/FareUsage.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

FIXEDFALLBACKVALUE_DECL(APO29538_StopoverMinStay);

class RuleUtilTSITest_TSISetupTurnAround : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_TSISetupTurnAround);
  CPPUNIT_TEST(testBuild_ScopeJour_Furth3);
  CPPUNIT_TEST(testBuild_ScopeJour_Furth3_OrigDestNotMatch);
  CPPUNIT_TEST(testBuild_ScopeJour_Furth2);
  CPPUNIT_TEST(testBuild_ScopeJour_Furth2_OrigDestNotMatch);
  CPPUNIT_TEST(testBuild_ScopeJour_Furth1);
  CPPUNIT_TEST(testBuild_ScopeSubJour_NoTurnAround);
  CPPUNIT_TEST(testBuild_ScopeSubJour_Turn3);
  CPPUNIT_TEST(testBuild_ScopeSubJour_Turn3_OrigDestNotMatch_RoudTrip);
  CPPUNIT_TEST(testBuild_ScopeSubJour_Turn3_OrigDestNotMatch_OpenJaw);
  CPPUNIT_TEST(testBuild_ScopeSubJour_Turn2);
  CPPUNIT_TEST(testBuild_ScopeSubJour_Turn2_OrigDestNotMatch_RoudTrip);
  CPPUNIT_TEST(testBuild_ScopeSubJour_Turn2_OrigDestNotMatch_OpenJaw);
  CPPUNIT_TEST(testBuild_ScopeSubJour_Turn1);
  CPPUNIT_TEST(testBuild_ScopeFare);
  CPPUNIT_TEST(testProcess_A3);
  CPPUNIT_TEST(testProcess_A2);
  CPPUNIT_TEST(testProcess_A1);

  CPPUNIT_TEST_SUITE_END();

public:
  void testBuild_ScopeJour_Furth3()
  {
    // for furthest point =3 turnaroud is _a3 and turnAroundPointAtDest is _a2
    assertBuild(true, &_a3, &_a2);
  }
  void testBuild_ScopeJour_Furth3_OrigDestNotMatch()
  {
    // for furthest point =3 turnaroud is _a3, _a2 destination different then _a3origin
    _a3.origin() = _l5;
    assertBuild(true, &_a3, 0);
  }
  void testBuild_ScopeJour_Furth2()
  {
    // for furthest point =2 turnaroud is _a2 and turnAroundPointAtDest is _a1
    _itin.furthestPointSegmentOrder() = 2;
    assertBuild(true, &_a2, &_a1);
  }
  void testBuild_ScopeJour_Furth2_OrigDestNotMatch()
  {
    // for furthest point =2 turnaroud is _a2, _a1 destination different then _a2 origin
    _itin.furthestPointSegmentOrder() = 2;
    _a2.origin() = _l5;
    assertBuild(true, &_a2, 0);
  }
  void testBuild_ScopeJour_Furth1()
  {
    // for furthest point =1 turnaroud is _a1 and no turnAroundPointAtDest
    _itin.furthestPointSegmentOrder() = 1;
    assertBuild(true, &_a1, 0);
  }
  void testBuild_ScopeSubJour_NoTurnAround()
  {
    // no turnaround in PU, nothing is found
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _pu.turnAroundPoint() = 0;
    assertBuild(true, 0, 0);
  }
  void testBuild_ScopeSubJour_Turn3()
  {
    // for turnaround on _a3turnAroundPointAtDest is _a2
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertBuild(true, &_a3, &_a2);
  }
  void testBuild_ScopeSubJour_Turn3_OrigDestNotMatch_RoudTrip()
  {
    // for turnaround on _a3 _a2 destination different then _a3origin
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _a3.origin() = _l5;
    assertBuild(true, &_a3, 0);
  }
  void testBuild_ScopeSubJour_Turn3_OrigDestNotMatch_OpenJaw()
  {
    // for turnaround on _a3 _a2 destination different then _a3origin, but it's OPENJAW
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _a3.origin() = _l5;
    _pu.puType() = PricingUnit::Type::OPENJAW;
    assertBuild(true, &_a3, &_a2);
  }
  void testBuild_ScopeSubJour_Turn2()
  {
    // for turnaround on _a2 turnAroundPointAtDest is _a1
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _pu.turnAroundPoint() = &_a2;
    assertBuild(true, &_a2, &_a1);
  }
  void testBuild_ScopeSubJour_Turn2_OrigDestNotMatch_RoudTrip()
  {
    // for turnaround on _a2 _a1 destination different then _a2 origin
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _pu.turnAroundPoint() = &_a2;
    _a2.origin() = _l5;
    assertBuild(true, &_a2, 0);
  }
  void testBuild_ScopeSubJour_Turn2_OrigDestNotMatch_OpenJaw()
  {
    // for turnaround on _a2 _a1 destination different then _a1 origin, but it's OPENJAW
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _pu.turnAroundPoint() = &_a2;
    _a2.origin() = _l5;
    _pu.puType() = PricingUnit::Type::OPENJAW;
    assertBuild(true, &_a2, &_a1);
  }
  void testBuild_ScopeSubJour_Turn1()
  {
    // for turnaround on _a1, no turnaround at dest
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _pu.turnAroundPoint() = &_a1;
    assertBuild(true, &_a1, 0);
  }
  void testBuild_ScopeFare()
  {
    // forfare component no turnaround
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertBuild();
  }
  void testProcess_A3()
  {
    // a3 is turnaround on default data
    assertProcess(&_a3, true, false);
  }
  void testProcess_A2()
  {
    // a2 is turnaroundatdest on default data
    assertProcess(&_a2, false, true);
  }
  void testProcess_A1()
  {
    // a1 match nothing
    assertProcess(&_a1, false, false);
  }

  void setUp()
  {
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsi = _memHandle.insert(
        new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, &_itin, &_pu, 0));
    _tsiSetup = _memHandle.create<RuleUtilTSI::TSISetupTurnAround>();
    _loopForward = true;
    _itin.travelSeg() += &_a1, &_a2, &_a3;
    _pu.travelSeg() += &_a1, &_a2, &_a3;
    _itin.furthestPointSegmentOrder() = 3;
    _pu.turnAroundPoint() = &_a3;
    _pu.puType() = PricingUnit::Type::ROUNDTRIP;
    _l1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    _l2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSYD.xml");
    _l3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDUB.xml");
    _l4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocATL.xml");
    _l5 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocOSA.xml");
    _a1.origin() = _l1;
    _a1.destination() = _l2;
    _a2.origin() = _l2;
    _a2.destination() = _l3;
    _a3.origin() = _l3;
    _a3.destination() = _l4;
  }

private:
  void assertBuild(bool ret = true,
                   const TravelSeg* turnaround = 0,
                   const TravelSeg* turnaroundAtDest = 0)
  {
    std::string errMsg;
    CPPUNIT_ASSERT_EQUAL(ret, _tsiSetup->build(_tsiScope, errMsg, *_tsi, _loopForward, *_trx));
    CPPUNIT_ASSERT_EQUAL(turnaround, _tsiSetup->_turnAroundPoint);
    CPPUNIT_ASSERT_EQUAL(turnaroundAtDest, _tsiSetup->_turnAroundPointAtDest);
  }
  void assertProcess(const TravelSeg* seg, bool turnaround, bool turnaroundAtDest)
  {
    RuleUtilTSI::TSITravelSegMarkup tsMarkup;
    std::string errMsg;
    _tsiSetup->build(_tsiScope, errMsg, *_tsi, _loopForward, *_trx);
    _tsiSetup->process(seg, tsMarkup);
    CPPUNIT_ASSERT_EQUAL(turnaround, tsMarkup.isTurnAroundPoint());
    CPPUNIT_ASSERT_EQUAL(turnaroundAtDest, tsMarkup.destIsTurnAroundPoint());
  }
  RuleUtilTSI::TSISetupTurnAround* _tsiSetup;
  RuleUtilTSI::TSIData* _tsi;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
  bool _loopForward;
  Itin _itin;
  PricingUnit _pu;
  AirSeg _a1;
  AirSeg _a2;
  AirSeg _a3;
  Loc* _l1;
  Loc* _l2;
  Loc* _l3;
  Loc* _l4;
  Loc* _l5;
};
class RuleUtilTSITest_TSISetupGateway : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_TSISetupGateway);
  CPPUNIT_TEST(testCollectOutInTvl);

  CPPUNIT_TEST(testBuild_NoGtwy_ScopeJour);
  CPPUNIT_TEST(testBuild_NoGtwy_ScopeSubJour);
  CPPUNIT_TEST(testBuild_NoGtwy_ScopeFare);
  CPPUNIT_TEST(testBuild_GtwOrig_ScopeJour);
  CPPUNIT_TEST(testBuild_GtwOrig_ScopeSubJour);
  CPPUNIT_TEST(testBuild_GtwOrig_ScopeFare);
  CPPUNIT_TEST(testBuild_GtwDest_ScopeJour);
  CPPUNIT_TEST(testBuild_GtwDest_ScopeSubJour);
  CPPUNIT_TEST(testBuild_GtwDest_ScopeFare);
  CPPUNIT_TEST(testBuild_Gtw_ScopeJour);
  CPPUNIT_TEST(testBuild_Gtw_ScopeSubJour);
  CPPUNIT_TEST(testBuild_Gtw_ScopeFare);

  CPPUNIT_TEST(testProcess_GtwOrig_ScopeSubJour);
  CPPUNIT_TEST(testProcess_GtwOrig_ScopeFare);
  CPPUNIT_TEST(testProcess_GtwDest_ScopeSubJour);
  CPPUNIT_TEST(testProcess_GtwDest_ScopeFare);
  CPPUNIT_TEST(testProcess_Gtw_ScopeSubJour);
  CPPUNIT_TEST(testProcess_Gtw_ScopeFare);

  CPPUNIT_TEST_SUITE_END();

public:
  void testCollectOutInTvl()
  {
    std::vector<TravelSeg*> inSegs, outSegs;
    _tsiSetup->collectOutInTvl(_pu, outSegs, inSegs);
    CPPUNIT_ASSERT(outSegs.size() == 2);
    CPPUNIT_ASSERT(inSegs.size() == 1);
    CPPUNIT_ASSERT(outSegs[0] == &_a1);
    CPPUNIT_ASSERT(outSegs[1] == &_a2);
    CPPUNIT_ASSERT(inSegs[0] == &_a3);
  }
  void testBuild_NoGtwy_ScopeJour()
  {
    // no gtwy match criteria - return true, no gtwy found
    assertBuild(true, false);
  }
  void testBuild_NoGtwy_ScopeSubJour()
  {
    // no gtwy match criteria - return true, no gtwy found
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertBuild(true, false);
  }
  void testBuild_NoGtwy_ScopeFare()
  {
    // no gtwy match criteria - return true, no gtwy found
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertBuild(true, false);
  }
  void testBuild_GtwOrig_ScopeJour()
  {
    // no gtwy for journey
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::ORIG_GATEWAY);
    assertBuild(false, false);
  }
  void testBuild_GtwOrig_ScopeSubJour()
  {
    // build succedded
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::ORIG_GATEWAY);
    assertBuild(true, true);
  }
  void testBuild_GtwOrig_ScopeFare()
  {
    // build succedded
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _tsiInfo.matchCriteria().push_back(TSIInfo::ORIG_GATEWAY);
    assertBuild(true, true);
  }
  void testBuild_GtwDest_ScopeJour()
  {
    // no gtwy for journey
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::DEST_GATEWAY);
    assertBuild(false, false);
  }
  void testBuild_GtwDest_ScopeSubJour()
  {
    // build succedded
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::DEST_GATEWAY);
    assertBuild(true, true);
  }
  void testBuild_GtwDest_ScopeFare()
  {
    // build succedded
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _tsiInfo.matchCriteria().push_back(TSIInfo::DEST_GATEWAY);
    assertBuild(true, true);
  }
  void testBuild_Gtw_ScopeJour()
  {
    // no gtwy for journey
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::GATEWAY);
    assertBuild(false, false);
  }
  void testBuild_Gtw_ScopeSubJour()
  {
    // build succedded
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::GATEWAY);
    assertBuild(true, true);
  }
  void testBuild_Gtw_ScopeFare()
  {
    // build succedded
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _tsiInfo.matchCriteria().push_back(TSIInfo::GATEWAY);
    assertBuild(true, true);
  }
  void testProcess_GtwOrig_ScopeSubJour()
  {
    // build succedded
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::ORIG_GATEWAY);
    assertBuild(true, true);
    assertProcess(&_a1, false, true);
    assertProcess(&_a2, true, true);
    assertProcess(&_a3, false, true);
  }
  void testProcess_GtwOrig_ScopeFare()
  {
    // build succedded
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _tsiInfo.matchCriteria().push_back(TSIInfo::ORIG_GATEWAY);
    assertBuild(true, true);
    assertProcess(&_a1, false, true);
    assertProcess(&_a2, true, true);
  }
  void testProcess_GtwDest_ScopeSubJour()
  {
    // build succedded
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::DEST_GATEWAY);
    assertBuild(true, true);
    assertProcess(&_a1, false, false);
    assertProcess(&_a2, true, false);
    assertProcess(&_a3, true, true);
  }
  void testProcess_GtwDest_ScopeFare()
  {
    // build succedded
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _tsiInfo.matchCriteria().push_back(TSIInfo::DEST_GATEWAY);
    assertBuild(true, true);
    assertProcess(&_a1, false, false);
    assertProcess(&_a2, true, true);
  }
  void testProcess_Gtw_ScopeSubJour()
  {
    // build succedded
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::GATEWAY);
    assertBuild(true, true);
    assertProcess(&_a1, false, true);
    assertProcess(&_a2, true, true);
    assertProcess(&_a3, true, true);
  }
  void testProcess_Gtw_ScopeFare()
  {
    // build succedded
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _tsiInfo.matchCriteria().push_back(TSIInfo::GATEWAY);
    assertBuild(true, true);
    assertProcess(&_a1, false, true);
    assertProcess(&_a2, true, true);
  }

  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsi =
        _memHandle.insert(new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, 0, &_pu, &_fm));
    _tsiSetup = _memHandle.create<RuleUtilTSI::TSISetupGateway>();
    _tsiSetup->initialize(*_trx);
    _loopForward = true;
    _pu.travelSeg() += &_a1, &_a2, &_a3;
    _l1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    _l2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocATL.xml");
    _l3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocOSA.xml");
    _pu.fareUsage() += &_fu1, &_fu2;
    _fu1.inbound() = false;
    _fu2.inbound() = true;
    _fu1.travelSeg() += &_a1, &_a2;
    _fu2.travelSeg() += &_a3;
    // FM 1-1, 1-3
    _fm.travelSeg() += &_a1, &_a2;
    // O: 1-1, 1-3 I: 3-1
    _a1.origin() = _l1;
    _a1.destination() = _l2;
    _a2.origin() = _l2;
    _a2.destination() = _l3;
    _a3.origin() = _l3;
    _a3.destination() = _l1;
  }

private:
  void assertBuild(bool ret, bool foundGtwy)
  {
    std::string errMsg;
    CPPUNIT_ASSERT_EQUAL(ret, _tsiSetup->build(_tsiScope, errMsg, *_tsi, _loopForward, *_trx));
    CPPUNIT_ASSERT_EQUAL(foundGtwy, _tsiSetup->_foundGateways);
  }

  void assertProcess(const TravelSeg* seg, bool gtwyOrig, bool gtwyDest)
  {
    RuleUtilTSI::TSITravelSegMarkup tsMarkup;
    _tsiSetup->process(seg, tsMarkup);
    CPPUNIT_ASSERT_EQUAL(gtwyOrig, tsMarkup.departsOrigGateway());
    CPPUNIT_ASSERT_EQUAL(gtwyDest, tsMarkup.arrivesDestGateway());
  }

  RuleUtilTSI::TSISetupGateway* _tsiSetup;
  RuleUtilTSI::TSIData* _tsi;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
  bool _loopForward;
  PricingUnit _pu;
  AirSeg _a1;
  AirSeg _a2;
  AirSeg _a3;
  Loc* _l1;
  Loc* _l2;
  Loc* _l3;
  Loc* _l4;
  Loc* _l5;
  FareUsage _fu1;
  FareUsage _fu2;
  FareMarket _fm;
};

class RuleUtilTSITest_TSISetupOverWater : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_TSISetupOverWater);
  CPPUNIT_TEST(testBuild_ScopeJour);
  CPPUNIT_TEST(testBuild_ScopeSubJour);
  CPPUNIT_TEST(testBuild_ScopeFare);

  CPPUNIT_TEST(testProcess_ScopeJour_A1);
  CPPUNIT_TEST(testProcess_ScopeJour_A2);
  CPPUNIT_TEST(testProcess_ScopeJour_A3);
  CPPUNIT_TEST(testProcess_ScopeSubJour_A1);
  CPPUNIT_TEST(testProcess_ScopeSubJour_A2);
  CPPUNIT_TEST(testProcess_ScopeSubJour_A3);
  CPPUNIT_TEST(testProcess_ScopeFare_A1);
  CPPUNIT_TEST(testProcess_ScopeFare_A2);
  CPPUNIT_TEST(testProcess_ScopeFare_A3);
  CPPUNIT_TEST_SUITE_END();

public:
  void testBuild_ScopeJour()
  {
    // only 1 segment is over water
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    assertBuild(true, 1);
  }
  void testBuild_ScopeSubJour()
  {
    // only 1 segment is over water
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertBuild(true, 1);
  }
  void testBuild_ScopeFare()
  {
    // only 1 segment is over water
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertBuild(true, 1);
  }
  void testProcess_ScopeJour_A1()
  {
    // only a2 is over water
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    assertProcess(&_a1, false);
  }
  void testProcess_ScopeJour_A2()
  {
    // only a2 is over water
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    assertProcess(&_a2, true);
  }
  void testProcess_ScopeJour_A3()
  {
    // only a2 is over water
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    assertProcess(&_a3, false);
  }
  void testProcess_ScopeSubJour_A1()
  {
    // only a2 is over water
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertProcess(&_a1, false);
  }
  void testProcess_ScopeSubJour_A2()
  {
    // only a2 is over water
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertProcess(&_a2, true);
  }
  void testProcess_ScopeSubJour_A3()
  {
    // only a2 is over water
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertProcess(&_a3, false);
  }
  void testProcess_ScopeFare_A1()
  {
    // only a2 is over water
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertProcess(&_a1, false);
  }
  void testProcess_ScopeFare_A2()
  {
    // only a2 is over water
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertProcess(&_a2, true);
  }
  void testProcess_ScopeFare_A3()
  {
    // only a2 is over water
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertProcess(&_a3, false);
  }

  void setUp()
  {
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsi = _memHandle.insert(
        new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, &_itin, &_pu, &_fm));
    _tsiSetup = _memHandle.create<RuleUtilTSI::TSISetupOverWater>();
    _loopForward = true;
    _pu.travelSeg() += &_a1, &_a2, &_a3;
    _itin.travelSeg() += &_a1, &_a2, &_a3;
    _fm.travelSeg() += &_a1, &_a2, &_a3;
    _l1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    _l2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocATL.xml");
    _l3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocOSA.xml");
    _l4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTYO.xml");
    // 1-1 1-3 3-3
    _a1.origin() = _l1;
    _a1.destination() = _l2;
    _a2.origin() = _l2;
    _a2.destination() = _l3;
    _a3.origin() = _l3;
    _a3.destination() = _l4;
  }

private:
  void assertBuild(bool ret, size_t size)
  {
    std::string errMsg;
    CPPUNIT_ASSERT_EQUAL(ret, _tsiSetup->build(_tsiScope, errMsg, *_tsi, _loopForward, *_trx));
    CPPUNIT_ASSERT_EQUAL(size, _tsiSetup->_overWaterSegments.size());
  }
  void assertProcess(const TravelSeg* seg, bool overWater)
  {
    std::string errMsg;
    _tsiSetup->build(_tsiScope, errMsg, *_tsi, _loopForward, *_trx);
    RuleUtilTSI::TSITravelSegMarkup tsMarkup;
    _tsiSetup->process(seg, tsMarkup);
    CPPUNIT_ASSERT_EQUAL(overWater, tsMarkup.isOverWater());
  }

  RuleUtilTSI::TSISetupOverWater* _tsiSetup;
  RuleUtilTSI::TSIData* _tsi;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
  bool _loopForward;
  PricingUnit _pu;
  AirSeg _a1;
  AirSeg _a2;
  AirSeg _a3;
  Loc* _l1;
  Loc* _l2;
  Loc* _l3;
  Loc* _l4;
  FareMarket _fm;
  Itin _itin;
};

class RuleUtilTSITest_TSISetupInternational : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_TSISetupInternational);
  CPPUNIT_TEST(testGetInternationalSegmentsForStopoverTSI_USUS);
  CPPUNIT_TEST(testGetInternationalSegmentsForStopoverTSI_USCACAUS);
  CPPUNIT_TEST(testGetInternationalSegmentsForStopoverTSI_USCACAMX);
  CPPUNIT_TEST(testGetInternationalSegmentsForStopoverTSI_PLUSCA);
  CPPUNIT_TEST(testGetInternationalSegmentsForStopoverTSI_PLUKUKPL);
  CPPUNIT_TEST(testGetInternationalSegmentsForStopoverTSI_USUSVEUS);
  CPPUNIT_TEST(testGetInternationalSegmentsForStopoverTSI_AUJPAU);

  CPPUNIT_TEST(testGetInternationalSegmentsForTSI18_USUS);
  CPPUNIT_TEST(testGetInternationalSegmentsForTSI18_USUSCAPL);
  CPPUNIT_TEST(testGetInternationalSegmentsForTSI18_CAPL);
  CPPUNIT_TEST(testGetInternationalSegmentsForTSI18_AUAUPL);

  CPPUNIT_TEST(testBuild_Frwrd_NonStop_ScopeJour);
  CPPUNIT_TEST(testBuild_Frwrd_NonStop_ScopeSubJour);
  CPPUNIT_TEST(testBuild_Frwrd_NonStop_ScopeFare);
  CPPUNIT_TEST(testBuild_Frwrd_Stop_ScopeJour);
  CPPUNIT_TEST(testBuild_Frwrd_Stop_ScopeSubJour);
  CPPUNIT_TEST(testBuild_Frwrd_Stop_ScopeFare);
  CPPUNIT_TEST(testBuild_Back_NonStop_ScopeJour);
  CPPUNIT_TEST(testBuild_Back_NonStop_ScopeSubJour);
  CPPUNIT_TEST(testBuild_Back_NonStop_ScopeFare);
  CPPUNIT_TEST(testBuild_Back_Stop_ScopeJour);
  CPPUNIT_TEST(testBuild_Back_Stop_ScopeSubJour);
  CPPUNIT_TEST(testBuild_Back_Stop_ScopeFare);

  CPPUNIT_TEST(testProcess_Frwrd_NonStop_ScopeJour_A1);
  CPPUNIT_TEST(testProcess_Frwrd_NonStop_ScopeJour_A2);
  CPPUNIT_TEST(testProcess_Frwrd_NonStop_ScopeJour_A3);
  CPPUNIT_TEST(testProcess_Frwrd_NonStop_ScopeSubJour_A1);
  CPPUNIT_TEST(testProcess_Frwrd_NonStop_ScopeSubJour_A2);
  CPPUNIT_TEST(testProcess_Frwrd_NonStop_ScopeSubJour_A3);
  CPPUNIT_TEST(testProcess_Frwrd_NonStop_ScopeFare_A1);
  CPPUNIT_TEST(testProcess_Frwrd_NonStop_ScopeFare_A2);
  CPPUNIT_TEST(testProcess_Frwrd_NonStop_ScopeFare_A3);
  CPPUNIT_TEST(testProcess_Frwrd_Stop_ScopeJour_A1);
  CPPUNIT_TEST(testProcess_Frwrd_Stop_ScopeJour_A2);
  CPPUNIT_TEST(testProcess_Frwrd_Stop_ScopeJour_A3);
  CPPUNIT_TEST(testProcess_Frwrd_Stop_ScopeSubJour_A1);
  CPPUNIT_TEST(testProcess_Frwrd_Stop_ScopeSubJour_A2);
  CPPUNIT_TEST(testProcess_Frwrd_Stop_ScopeSubJour_A3);
  CPPUNIT_TEST(testProcess_Frwrd_Stop_ScopeFare_A1);
  CPPUNIT_TEST(testProcess_Frwrd_Stop_ScopeFare_A2);
  CPPUNIT_TEST(testProcess_Frwrd_Stop_ScopeFare_A3);
  CPPUNIT_TEST(testProcess_Back_NonStop_ScopeJour_A1);
  CPPUNIT_TEST(testProcess_Back_NonStop_ScopeJour_A2);
  CPPUNIT_TEST(testProcess_Back_NonStop_ScopeJour_A3);
  CPPUNIT_TEST(testProcess_Back_NonStop_ScopeSubJour_A1);
  CPPUNIT_TEST(testProcess_Back_NonStop_ScopeSubJour_A2);
  CPPUNIT_TEST(testProcess_Back_NonStop_ScopeSubJour_A3);
  CPPUNIT_TEST(testProcess_Back_NonStop_ScopeFare_A1);
  CPPUNIT_TEST(testProcess_Back_NonStop_ScopeFare_A2);
  CPPUNIT_TEST(testProcess_Back_NonStop_ScopeFare_A3);
  CPPUNIT_TEST(testProcess_Back_Stop_ScopeJour_A1);
  CPPUNIT_TEST(testProcess_Back_Stop_ScopeJour_A2);
  CPPUNIT_TEST(testProcess_Back_Stop_ScopeJour_A3);
  CPPUNIT_TEST(testProcess_Back_Stop_ScopeSubJour_A1);
  CPPUNIT_TEST(testProcess_Back_Stop_ScopeSubJour_A2);
  CPPUNIT_TEST(testProcess_Back_Stop_ScopeSubJour_A3);
  CPPUNIT_TEST(testProcess_Back_Stop_ScopeFare_A1);
  CPPUNIT_TEST(testProcess_Back_Stop_ScopeFare_A2);
  CPPUNIT_TEST(testProcess_Back_Stop_ScopeFare_A3);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGetInternationalSegmentsForStopoverTSI_USUS()
  {
    // travel domestic, none segment is international
    std::vector<TravelSeg*> outSeg, *in = createTravelSegmentVector("US", "US", 0);
    _tsiSetup->getInternationalSegmentsForStopoverTSI(*_locSFO, *in, outSeg);
    checkTravelSegmentVector(outSeg, 0);
  }
  void testGetInternationalSegmentsForStopoverTSI_USCACAUS()
  {
    // travel wholy within USCA, none segment is international
    std::vector<TravelSeg*> outSeg, *in = createTravelSegmentVector("US", "CA", "CA", "US", 0);
    _tsiSetup->getInternationalSegmentsForStopoverTSI(*_locSFO, *in, outSeg);
    checkTravelSegmentVector(outSeg, 0);
  }
  void testGetInternationalSegmentsForStopoverTSI_USCACAMX()
  {
    // travel outside USCA
    std::vector<TravelSeg*> outSeg, *in = createTravelSegmentVector("US", "CA", "CA", "MX", 0);
    _tsiSetup->getInternationalSegmentsForStopoverTSI(*_locSFO, *in, outSeg);
    checkTravelSegmentVector(outSeg, "US", "CA", "CA", "MX", 0);
  }
  void testGetInternationalSegmentsForStopoverTSI_PLUSCA()
  {
    // travel outside USCA, for referencing location "US", skip PL-US
    std::vector<TravelSeg*> outSeg, *in = createTravelSegmentVector("PL", "US", "CA", 0);
    _tsiSetup->getInternationalSegmentsForStopoverTSI(*_locSFO, *in, outSeg);
    checkTravelSegmentVector(outSeg, "US", "CA", 0);
  }
  void testGetInternationalSegmentsForStopoverTSI_PLUKUKPL()
  {
    // travel outside USCA
    std::vector<TravelSeg*> outSeg, *in = createTravelSegmentVector("PL", "UK", "UK", "PL", 0);
    _tsiSetup->getInternationalSegmentsForStopoverTSI(*_locSFO, *in, outSeg);
    checkTravelSegmentVector(outSeg, "PL", "UK", "UK", "PL", 0);
  }
  void testGetInternationalSegmentsForStopoverTSI_USUSVEUS()
  {
    // travel outside USCA, for referencing location "AU", skip nothing
    std::vector<TravelSeg*> outSeg, *in = createTravelSegmentVector("US", "US", "VE", "US", 0);
    _tsiSetup->getInternationalSegmentsForStopoverTSI(*_locSYD, *in, outSeg);
    checkTravelSegmentVector(outSeg, "US", "US", "VE", "US", 0);
  }
  void testGetInternationalSegmentsForStopoverTSI_AUJPAU()
  {
    // travel outside USCA, for referencing location "AU", skip JPAU
    std::vector<TravelSeg*> outSeg, *in = createTravelSegmentVector("AU", "JP", "AU", 0);
    _tsiSetup->getInternationalSegmentsForStopoverTSI(*_locSYD, *in, outSeg);
    checkTravelSegmentVector(outSeg, "AU", "JP", 0);
  }

  void testGetInternationalSegmentsForTSI18_USUS()
  {
    // travel within US, no intenational segmenst
    std::vector<TravelSeg*> outSeg, *in = createTravelSegmentVector("US", "US", 0);
    _tsiSetup->getInternationalSegmentsForTSI18(*in, outSeg);
    checkTravelSegmentVector(outSeg, 0);
  }
  void testGetInternationalSegmentsForTSI18_USUSCAPL()
  {
    // travel within US-CA for tsi 18 and cat2 is threaded as internationa
    std::vector<TravelSeg*> outSeg, *in = createTravelSegmentVector("US", "US", "CA", "PL", 0);
    _tsiSetup->getInternationalSegmentsForTSI18(*in, outSeg);
    checkTravelSegmentVector(outSeg, "US", "CA", "PL", 0);
  }
  void testGetInternationalSegmentsForTSI18_CAPL()
  {
    // travel within US-CA for tsi 18 and cat2 is threaded as internationa
    std::vector<TravelSeg*> outSeg, *in = createTravelSegmentVector("CA", "PL", 0);
    _tsiSetup->getInternationalSegmentsForTSI18(*in, outSeg);
    checkTravelSegmentVector(outSeg, "CA", "PL", 0);
  }
  void testGetInternationalSegmentsForTSI18_AUAUPL()
  {
    // travel within US-CA for tsi 18 and cat2 is threaded as internationa
    std::vector<TravelSeg*> outSeg, *in = createTravelSegmentVector("AU", "AU", "PL", 0);
    _tsiSetup->getInternationalSegmentsForTSI18(*in, outSeg);
    checkTravelSegmentVector(outSeg, "AU", "PL", 0);
  }

  void testBuild_Frwrd_NonStop_ScopeJour()
  {
    // nonstop - one international segment
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    assertBuild(true, 1);
  }
  void testBuild_Frwrd_NonStop_ScopeSubJour()
  {
    // nonstop - one international segment
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertBuild(true, 1);
  }
  void testBuild_Frwrd_NonStop_ScopeFare()
  {
    // nonstop - one international segment
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertBuild(true, 1);
  }
  void testBuild_Frwrd_Stop_ScopeJour()
  {
    // with stop - referencing loaction SFO, 2 internationals
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertBuild(true, 2);
  }
  void testBuild_Frwrd_Stop_ScopeSubJour()
  {
    // with stop - referencing loaction SFO, 2 internationals
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertBuild(true, 2);
  }
  void testBuild_Frwrd_Stop_ScopeFare()
  {
    // with stop - referencing loaction SFO, 2 internationals
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertBuild(true, 2);
  }
  void testBuild_Back_NonStop_ScopeJour()
  {
    // nonstop - one international segment
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _loopForward = false;
    assertBuild(true, 1);
  }
  void testBuild_Back_NonStop_ScopeSubJour()
  {
    // nonstop - one international segment
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _loopForward = false;
    assertBuild(true, 1);
  }
  void testBuild_Back_NonStop_ScopeFare()
  {
    // nonstop - one international segment
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _loopForward = false;
    assertBuild(true, 1);
  }
  void testBuild_Back_Stop_ScopeJour()
  {
    // with stop - referencing loaction TYO, 1 international
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _loopForward = false;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertBuild(true, 1);
  }
  void testBuild_Back_Stop_ScopeSubJour()
  {
    // with stop - referencing loaction TYO, 1 international
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _loopForward = false;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertBuild(true, 1);
  }
  void testBuild_Back_Stop_ScopeFare()
  {
    // with stop - referencing loaction TYO, 1 international
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _loopForward = false;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertBuild(true, 1);
  }
  void testProcess_Frwrd_NonStop_ScopeJour_A1()
  {
    // A1 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    assertProcess(&_a1, false);
  }
  void testProcess_Frwrd_NonStop_ScopeJour_A2()
  {
    // A2 is international
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    assertProcess(&_a2, true);
  }
  void testProcess_Frwrd_NonStop_ScopeJour_A3()
  {
    // A3 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    assertProcess(&_a3, false);
  }
  void testProcess_Frwrd_NonStop_ScopeSubJour_A1()
  {
    // A1 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertProcess(&_a1, false);
  }
  void testProcess_Frwrd_NonStop_ScopeSubJour_A2()
  {
    // A2 is international
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertProcess(&_a2, true);
  }
  void testProcess_Frwrd_NonStop_ScopeSubJour_A3()
  {
    // A3 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertProcess(&_a3, false);
  }
  void testProcess_Frwrd_NonStop_ScopeFare_A1()
  {
    // A1 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertProcess(&_a1, false);
  }
  void testProcess_Frwrd_NonStop_ScopeFare_A2()
  {
    // A2 is international
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertProcess(&_a2, true);
  }
  void testProcess_Frwrd_NonStop_ScopeFare_A3()
  {
    // A3 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertProcess(&_a3, false);
  }
  void testProcess_Frwrd_Stop_ScopeJour_A1()
  {
    // A1 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a1, false);
  }
  void testProcess_Frwrd_Stop_ScopeJour_A2()
  {
    // A2 is international
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a2, true);
  }
  void testProcess_Frwrd_Stop_ScopeJour_A3()
  {
    // A3 is international (referencing to SFO)
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a3, true);
  }
  void testProcess_Frwrd_Stop_ScopeSubJour_A1()
  {
    // A1 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a1, false);
  }
  void testProcess_Frwrd_Stop_ScopeSubJour_A2()
  {
    // A2 is international
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a2, true);
  }
  void testProcess_Frwrd_Stop_ScopeSubJour_A3()
  {
    // A3 is international
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a3, true);
  }
  void testProcess_Frwrd_Stop_ScopeFare_A1()
  {
    // A1 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a1, false);
  }
  void testProcess_Frwrd_Stop_ScopeFare_A2()
  {
    // A2 is international
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a2, true);
  }
  void testProcess_Frwrd_Stop_ScopeFare_A3()
  {
    // A3 is i nterantional
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a3, true);
  }
  void testProcess_Back_NonStop_ScopeJour_A1()
  {
    // A1 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _loopForward = false;
    assertProcess(&_a1, false);
  }
  void testProcess_Back_NonStop_ScopeJour_A2()
  {
    // A2 is international
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _loopForward = false;
    assertProcess(&_a2, true);
  }
  void testProcess_Back_NonStop_ScopeJour_A3()
  {
    // A3 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _loopForward = false;
    assertProcess(&_a3, false);
  }
  void testProcess_Back_NonStop_ScopeSubJour_A1()
  {
    // A1 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _loopForward = false;
    assertProcess(&_a1, false);
  }
  void testProcess_Back_NonStop_ScopeSubJour_A2()
  {
    // A2 is international
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _loopForward = false;
    assertProcess(&_a2, true);
  }
  void testProcess_Back_NonStop_ScopeSubJour_A3()
  {
    // A3 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _loopForward = false;
    assertProcess(&_a3, false);
  }
  void testProcess_Back_NonStop_ScopeFare_A1()
  {
    // a1 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _loopForward = false;
    assertProcess(&_a1, false);
  }
  void testProcess_Back_NonStop_ScopeFare_A2()
  {
    // a2 is inetrantional
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _loopForward = false;
    assertProcess(&_a2, true);
  }
  void testProcess_Back_NonStop_ScopeFare_A3()
  {
    // a3 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _loopForward = false;
    assertProcess(&_a3, false);
  }
  void testProcess_Back_Stop_ScopeJour_A1()
  {
    // a1 is international (referencing to TYO)
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _loopForward = false;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a1, true);
  }
  void testProcess_Back_Stop_ScopeJour_A2()
  {
    // a2 is domestic (referencing to TYO)
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _loopForward = false;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a2, false);
  }
  void testProcess_Back_Stop_ScopeJour_A3()
  {
    // a3 is domestic
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _loopForward = false;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a3, false);
  }
  void testProcess_Back_Stop_ScopeSubJour_A1()
  {
    // A1 is international (referencing to TYO)
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _loopForward = false;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a1, true);
  }
  void testProcess_Back_Stop_ScopeSubJour_A2()
  {
    // A2 is doemestic (referencing to TYO)
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _loopForward = false;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a2, false);
  }
  void testProcess_Back_Stop_ScopeSubJour_A3()
  {
    // A3 is doemstic
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _loopForward = false;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a3, false);
  }
  void testProcess_Back_Stop_ScopeFare_A1()
  {
    // A1 is international (referencing to TYO)
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _loopForward = false;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a1, true);
  }
  void testProcess_Back_Stop_ScopeFare_A2()
  {
    // A2 is domestic (referencing to TYO)
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _loopForward = false;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a2, false);
  }
  void testProcess_Back_Stop_ScopeFare_A3()
  {
    // A3 is doemstic
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _loopForward = false;
    _tsiInfo.matchCriteria().push_back(TSIInfo::STOP_OVER);
    assertProcess(&_a3, false);
  }

  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _tsi = _memHandle.insert(
        new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, &_itin, &_pu, &_fm));
    _tsiSetup = _memHandle.create<RuleUtilTSI::TSISetupInternational>();
    _loopForward = true;
    _pu.travelSeg() += &_a1, &_a2, &_a3;
    _itin.travelSeg() += &_a1, &_a2, &_a3;
    _fm.travelSeg() += &_a1, &_a2, &_a3;
    _l1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    _l2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocATL.xml");
    _l3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocOSA.xml");
    _l4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTYO.xml");
    // 1-1 1-3 3-3
    _a1.origin() = _l1;
    _a1.destination() = _l2;
    _a2.origin() = _l2;
    _a2.destination() = _l3;
    _a3.origin() = _l3;
    _a3.destination() = _l4;
  }

private:
  void assertBuild(bool ret, size_t size)
  {
    std::string errMsg;
    CPPUNIT_ASSERT_EQUAL(ret, _tsiSetup->build(_tsiScope, errMsg, *_tsi, _loopForward, *_trx));
    CPPUNIT_ASSERT_EQUAL(size, _tsiSetup->_intlSegments.size());
  }
  void assertProcess(const TravelSeg* seg, bool international)
  {
    std::string errMsg;
    _tsiSetup->build(_tsiScope, errMsg, *_tsi, _loopForward, *_trx);
    RuleUtilTSI::TSITravelSegMarkup tsMarkup;
    _tsiSetup->process(seg, tsMarkup);
    CPPUNIT_ASSERT_EQUAL(international, tsMarkup.isInternational());
  }

  std::vector<TravelSeg*>* createTravelSegmentVector(const char* nation, ...)
  {
    va_list args;
    va_start(args, nation);
    const char* n1, *n2 = nation;
    std::vector<TravelSeg*>* vec = _memHandle.create<std::vector<TravelSeg*> >();
    while (n2)
    {
      n1 = n2;
      n2 = va_arg(args, const char*);
      if (n2 == 0)
        break;
      AirSeg* as = _memHandle.create<AirSeg>();
      Loc* loc = _memHandle.create<Loc>();
      loc->nation() = n1;
      as->origin() = loc;
      loc = _memHandle.create<Loc>();
      loc->nation() = n2;
      as->destination() = loc;
      vec->push_back(as);
    }
    va_end(args);
    return vec;
  }
  void checkTravelSegmentVector(std::vector<TravelSeg*>& outSeg, const char* nation, ...)
  {
    va_list args;
    va_start(args, nation);
    const char* n1, *n2 = nation;
    // no segments in vector
    if (!n2)
      CPPUNIT_ASSERT(outSeg.empty());
    std::vector<TravelSeg*>::iterator it = outSeg.begin();
    for (; it != outSeg.end(); it++)
    {
      n1 = n2;
      n2 = va_arg(args, const char*);
      if (n2 == 0)
        CPPUNIT_ASSERT_MESSAGE("not enought elements in vector", false);
      CPPUNIT_ASSERT_EQUAL(NationCode(n1), (*it)->origin()->nation());
      CPPUNIT_ASSERT_EQUAL(NationCode(n2), (*it)->destination()->nation());
    }
    va_end(args);
  }

  RuleUtilTSI::TSISetupInternational* _tsiSetup;
  RuleUtilTSI::TSIData* _tsi;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
  bool _loopForward;
  PricingUnit _pu;
  AirSeg _a1;
  AirSeg _a2;
  AirSeg _a3;
  Loc* _l1;
  Loc* _l2;
  Loc* _l3;
  Loc* _l4;
  FareMarket _fm;
  Itin _itin;
};

class RuleUtilTSITest_TSISetupChain : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_TSISetupChain);
  CPPUNIT_TEST(testBuildTSIChain_NoMatchCriteria);
  CPPUNIT_TEST(testBuildTSIChain_Gateway);
  CPPUNIT_TEST(testBuildTSIChain_GatewayOrig);
  CPPUNIT_TEST(testBuildTSIChain_GatewayDest);
  CPPUNIT_TEST(testBuildTSIChain_OverWater);
  CPPUNIT_TEST(testBuildTSIChain_International);
  CPPUNIT_TEST(testBuildTSIChain_IntlDomtransfer);

  CPPUNIT_TEST(testPassCheckFareBreakOnly_NoCheck_CheckOrig_CheckDest_NoFB);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_NoCheck_CheckOrig_CheckDest_FBOrig);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_NoCheck_CheckOrig_CheckDest_FBDest);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_NoCheck_CheckOrig_NoCheckDest_NoFB);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_NoCheck_CheckOrig_NoCheckDest_FBOrig);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_NoCheck_CheckOrig_NoCheckDest_FBDest);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_NoCheck_NoCheckOrig_CheckDest_NoFB);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_NoCheck_NoCheckOrig_CheckDest_FBOrig);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_NoCheck_NoCheckOrig_CheckDest_FBDest);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_NoCheck_NoCheckOrig_NoCheckDest_NoFB);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_NoCheck_NoCheckOrig_NoCheckDest_FBOrig);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_NoCheck_NoCheckOrig_NOCheckDest_FBDest);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_Check_CheckOrig_CheckDest_NoFB);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_Check_CheckOrig_CheckDest_FBOrig);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_Check_CheckOrig_CheckDest_FBDest);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_Check_CheckOrig_NoCheckDest_NoFB);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_Check_CheckOrig_NoCheckDest_FBOrig);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_Check_CheckOrig_NoCheckDest_FBDest);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_Check_NoCheckOrig_CheckDest_NoFB);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_Check_NoCheckOrig_CheckDest_FBOrig);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_Check_NoCheckOrig_CheckDest_FBDest);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_Check_NoCheckOrig_NoCheckDest_NoFB);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_Check_NoCheckOrig_NoCheckDest_FBOrig);
  CPPUNIT_TEST(testPassCheckFareBreakOnly_Check_NoCheckOrig_NOCheckDest_FBDest);
  CPPUNIT_TEST_SUITE_END();

public:
  void testBuildTSIChain_NoMatchCriteria()
  {
    // turnaround is added always
    assertBuildTSIChain(RuleUtilTSI::TSISetup::TSI_TURNAROUND);
  }
  void testBuildTSIChain_Gateway()
  {
    // turnaround is added always, check for gateway
    _tsiInfo.matchCriteria().push_back(TSIInfo::GATEWAY);
    assertBuildTSIChain(RuleUtilTSI::TSISetup::TSI_TURNAROUND, RuleUtilTSI::TSISetup::TSI_GATEWAY);
  }
  void testBuildTSIChain_GatewayOrig()
  {
    // turnaround is added always, check for gateway
    _tsiInfo.matchCriteria().push_back(TSIInfo::ORIG_GATEWAY);
    assertBuildTSIChain(RuleUtilTSI::TSISetup::TSI_TURNAROUND, RuleUtilTSI::TSISetup::TSI_GATEWAY);
  }
  void testBuildTSIChain_GatewayDest()
  {
    // turnaround is added always, check for gateway
    _tsiInfo.matchCriteria().push_back(TSIInfo::DEST_GATEWAY);
    assertBuildTSIChain(RuleUtilTSI::TSISetup::TSI_TURNAROUND, RuleUtilTSI::TSISetup::TSI_GATEWAY);
  }
  void testBuildTSIChain_OverWater()
  {
    // turnaround is added always, check for gateway
    _tsiInfo.matchCriteria().push_back(TSIInfo::OVER_WATER);
    assertBuildTSIChain(RuleUtilTSI::TSISetup::TSI_TURNAROUND,
                        RuleUtilTSI::TSISetup::TSI_OVERWATER);
  }
  void testBuildTSIChain_International()
  {
    // turnaround is added always, check for gateway
    _tsiInfo.matchCriteria().push_back(TSIInfo::INTERNATIONAL);
    assertBuildTSIChain(RuleUtilTSI::TSISetup::TSI_TURNAROUND,
                        RuleUtilTSI::TSISetup::TSI_INTERNATIONAL);
  }
  void testBuildTSIChain_IntlDomtransfer()
  {
    // turnaround is added always, check for gateway
    _tsiInfo.matchCriteria().push_back(TSIInfo::INTL_DOM_TRANSFER);
    assertBuildTSIChain(RuleUtilTSI::TSISetup::TSI_TURNAROUND,
                        RuleUtilTSI::TSISetup::TSI_INTERNATIONAL);
  }
  void testPassCheckFareBreakOnly_NoCheck_CheckOrig_CheckDest_NoFB()
  {
    assertPassCheckFareBreakOnly(true, false, true, true, false, false);
  }
  void testPassCheckFareBreakOnly_NoCheck_CheckOrig_CheckDest_FBOrig()
  {
    assertPassCheckFareBreakOnly(true, false, true, true, true, false);
  }
  void testPassCheckFareBreakOnly_NoCheck_CheckOrig_CheckDest_FBDest()
  {
    assertPassCheckFareBreakOnly(true, false, true, true, false, true);
  }
  void testPassCheckFareBreakOnly_NoCheck_CheckOrig_NoCheckDest_NoFB()
  {
    assertPassCheckFareBreakOnly(true, false, true, false, false, false);
  }
  void testPassCheckFareBreakOnly_NoCheck_CheckOrig_NoCheckDest_FBOrig()
  {
    assertPassCheckFareBreakOnly(true, false, true, false, true, false);
  }
  void testPassCheckFareBreakOnly_NoCheck_CheckOrig_NoCheckDest_FBDest()
  {
    assertPassCheckFareBreakOnly(true, false, true, false, false, true);
  }
  void testPassCheckFareBreakOnly_NoCheck_NoCheckOrig_CheckDest_NoFB()
  {
    assertPassCheckFareBreakOnly(true, false, false, true, false, false);
  }
  void testPassCheckFareBreakOnly_NoCheck_NoCheckOrig_CheckDest_FBOrig()
  {
    assertPassCheckFareBreakOnly(true, false, false, true, true, false);
  }
  void testPassCheckFareBreakOnly_NoCheck_NoCheckOrig_CheckDest_FBDest()
  {
    assertPassCheckFareBreakOnly(true, false, false, true, false, true);
  }
  void testPassCheckFareBreakOnly_NoCheck_NoCheckOrig_NoCheckDest_NoFB()
  {
    assertPassCheckFareBreakOnly(true, false, false, false, false, false);
  }
  void testPassCheckFareBreakOnly_NoCheck_NoCheckOrig_NoCheckDest_FBOrig()
  {
    assertPassCheckFareBreakOnly(true, false, false, false, true, false);
  }
  void testPassCheckFareBreakOnly_NoCheck_NoCheckOrig_NOCheckDest_FBDest()
  {
    assertPassCheckFareBreakOnly(true, false, false, false, false, true);
  }
  void testPassCheckFareBreakOnly_Check_CheckOrig_CheckDest_NoFB()
  {
    // check fare breaks, but no breaks
    assertPassCheckFareBreakOnly(false, true, true, true, false, false);
  }
  void testPassCheckFareBreakOnly_Check_CheckOrig_CheckDest_FBOrig()
  {
    assertPassCheckFareBreakOnly(true, true, true, true, true, false);
  }
  void testPassCheckFareBreakOnly_Check_CheckOrig_CheckDest_FBDest()
  {
    assertPassCheckFareBreakOnly(true, true, true, true, false, true);
  }
  void testPassCheckFareBreakOnly_Check_CheckOrig_NoCheckDest_NoFB()
  {
    // check fare break at orign, no afre breaks
    assertPassCheckFareBreakOnly(false, true, true, false, false, false);
  }
  void testPassCheckFareBreakOnly_Check_CheckOrig_NoCheckDest_FBOrig()
  {
    assertPassCheckFareBreakOnly(true, true, true, false, true, false);
  }
  void testPassCheckFareBreakOnly_Check_CheckOrig_NoCheckDest_FBDest()
  {
    // check fare break at orign, no fare break at dest
    assertPassCheckFareBreakOnly(false, true, true, false, false, true);
  }
  void testPassCheckFareBreakOnly_Check_NoCheckOrig_CheckDest_NoFB()
  {
    // check fare break at dest, no fare breaks
    assertPassCheckFareBreakOnly(false, true, false, true, false, false);
  }
  void testPassCheckFareBreakOnly_Check_NoCheckOrig_CheckDest_FBOrig()
  {
    // check fare break at dest, fare break in orig
    assertPassCheckFareBreakOnly(false, true, false, true, true, false);
  }
  void testPassCheckFareBreakOnly_Check_NoCheckOrig_CheckDest_FBDest()
  {
    assertPassCheckFareBreakOnly(true, true, false, true, false, true);
  }
  void testPassCheckFareBreakOnly_Check_NoCheckOrig_NoCheckDest_NoFB()
  {
    // not checking orig or dest - always fail
    assertPassCheckFareBreakOnly(false, true, false, false, false, false);
  }
  void testPassCheckFareBreakOnly_Check_NoCheckOrig_NoCheckDest_FBOrig()
  {
    // not checking orig or dest - always fail
    assertPassCheckFareBreakOnly(false, true, false, false, true, false);
  }
  void testPassCheckFareBreakOnly_Check_NoCheckOrig_NOCheckDest_FBDest()
  {
    // not checking orig or dest - always fail
    assertPassCheckFareBreakOnly(false, true, false, false, false, true);
  }

  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    _tsi = _memHandle.insert(new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, 0, 0, &_fm));
    _tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG;
    _fm.travelSeg().push_back(_segSFOSYD);
  }

private:
  void assertPassCheckFareBreakOnly(
      bool ret, bool checkFareBreak, bool checkOrig, bool checkDest, bool fbAtOrig, bool fbAtDest)
  {
    RuleUtilTSI::TSITravelSegMarkup tsm;
    _tsiChain._checkFareBreaksOnly = checkFareBreak;
    _tsiChain._checkOrig = checkOrig;
    _tsiChain._checkDest = checkDest;
    tsm.fareBreakAtOrigin() = fbAtOrig;
    tsm.fareBreakAtDestination() = fbAtDest;
    CPPUNIT_ASSERT_EQUAL(ret, _tsiChain.passCheckFareBreakOnly(tsm));
    if (!ret)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("NOT FARE-BREAK"), tsm.noMatchReason());
    }
  }
  void assertBuildTSIChain(RuleUtilTSI::TSISetup::TSISETUPTYPE type)
  {
    _tsiChain.buildTSIChain(*_trx, *_tsi, true, true);
    CPPUNIT_ASSERT_EQUAL((size_t)1, _tsiChain._setupChain.size());
    testChainItem(type);
  }
  void assertBuildTSIChain(RuleUtilTSI::TSISetup::TSISETUPTYPE type1,
                           RuleUtilTSI::TSISetup::TSISETUPTYPE type2)
  {
    _tsiChain.buildTSIChain(*_trx, *_tsi, true, true);
    CPPUNIT_ASSERT_EQUAL((size_t)2, _tsiChain._setupChain.size());
    testChainItem(type1);
    testChainItem(type2);
  }
  void testChainItem(RuleUtilTSI::TSISetup::TSISETUPTYPE type)
  {
    bool foundItem = false;
    std::vector<RuleUtilTSI::TSISetup*>::iterator it = _tsiChain._setupChain.begin();
    std::vector<RuleUtilTSI::TSISetup*>::iterator ie = _tsiChain._setupChain.end();
    for (; it != ie; it++)
    {
      if ((*it)->tsiType() == type)
        foundItem = true;
    }
    CPPUNIT_ASSERT(foundItem);
  }

  RuleUtilTSI::TSISetupChain _tsiChain;
  RuleUtilTSI::TSIData* _tsi;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
  FareMarket _fm;
};

class RuleUtilTSITest_SetupTravelSegMarkup : public RuleUtilTSITest_Base
{
  CPPUNIT_TEST_SUITE(RuleUtilTSITest_SetupTravelSegMarkup);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeJour_NoMatchCriteria);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeJour_NoStop_FBADNo);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeJour_NoStop_FBAD1);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeJour_NoStop_FBAD2);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeJour_NoStop_FBAD3);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeJour_Stop_StopNo);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeJour_Stop_Stop1);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeJour_Stop_Stop2);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeJour_Stop_Stop3);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeSubJour_NoMatchCriteria);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeSubJour_NoStop_FBADNo);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeSubJour_NoStop_FBAD1);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeSubJour_NoStop_FBAD2);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeSubJour_NoStop_FBAD3);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeSubJour_Stop_StopNo);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeSubJour_Stop_Stop1);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeSubJour_Stop_Stop2);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeSubJour_Stop_Stop3);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetFurthest_ScopeFare);

  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeJour_NoMatchCriteria);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeJour_NoFareBreak);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD1_Stop);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD1_NoStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD1_NoStop_DiffCXR);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD1_NoStop_DiffFlight);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD2_Stop);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD2_NoStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD2_NoStop_DiffCXR);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD2_NoStop_DiffFlight);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD3_Stop);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD3_NoStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeSubJour_NoMatchCriteria);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeSubJour_NoFareBreak);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD1_Stop);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD1_NoStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD1_NoStop_DiffCXR);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD1_NoStop_DiffFlight);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD2_Stop);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD2_NoStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD2_NoStop_DiffCXR);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD2_NoStop_DiffFlight);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD3_Stop);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD3_NoStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetStopover_ScopeFareJour);

  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_NoFM_LoopFwrd_EmptyCont_NoSegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_NoFM_LoopFwrd_EmptyCont_SegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_NoFM_LoopFwrd_EmptyCont_SegForcedStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_NoFM_LoopFwrd_Cont_NoSegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_NoFM_LoopFwrd_Cont_SegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_NoFM_LoopFwrd_Cont_SegForcedStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_NoFM_LoopBack_EmptyCont_NoSegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_NoFM_LoopBack_EmptyCont_SegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_NoFM_LoopBack_EmptyCont_SegForcedStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_NoFM_LoopBack_Cont_NoSegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_NoFM_LoopBack_Cont_SegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_NoFM_LoopBack_Cont_SegForcedStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_EmptyCont_NoSegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_EmptyCont_SegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_EmptyCont_SegForcedStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_EmptyCont_FMA1);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_EmptyCont_FMA2);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_Cont_NoSegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_Cont_SegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_Cont_SegForcedStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_Cont_FMA1);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_Cont_FMA2);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_EmptyCont_NoSegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_EmptyCont_SegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_EmptyCont_SegForcedStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_EmptyCont_FMA1);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_EmptyCont_FMA2);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_Cont_NoSegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_Cont_SegStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_Cont_SegForcedStop);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_Cont_FMA1);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_Cont_FMA2);

  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegPTF_NoPTF);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegPTF_PTF_GdPA);
  CPPUNIT_TEST(testSetupTravelSegMarkupCreateSegPTF_PTF_GdAT);

  CPPUNIT_TEST(testSetupTravelSegMarkupGetStart_PricingUnit_Fwdr);
  CPPUNIT_TEST(testSetupTravelSegMarkupGetStart_PricingUnit_Back);
  CPPUNIT_TEST(testSetupTravelSegMarkupGetStart_FareUsage_Fwdr);
  CPPUNIT_TEST(testSetupTravelSegMarkupGetStart_FareUsage_Back);
  CPPUNIT_TEST(testSetupTravelSegMarkupGetStart_TravelSeg_Fwdr);
  CPPUNIT_TEST(testSetupTravelSegMarkupGetStart_TravelSeg_Back);

  CPPUNIT_TEST(testSetupTravelSegMarkupGetEnd_PricingUnit_Fwdr);
  CPPUNIT_TEST(testSetupTravelSegMarkupGetEnd_PricingUnit_Back);
  CPPUNIT_TEST(testSetupTravelSegMarkupGetEnd_FareUsage_Fwdr);
  CPPUNIT_TEST(testSetupTravelSegMarkupGetEnd_FareUsage_Back);
  CPPUNIT_TEST(testSetupTravelSegMarkupGetEnd_TravelSeg_Fwdr);
  CPPUNIT_TEST(testSetupTravelSegMarkupGetEnd_TravelSeg_Back);

  CPPUNIT_TEST(testSetupTravelSegMarkupSetDirection_Inbound);
  CPPUNIT_TEST(testSetupTravelSegMarkupSetDirection_Outbound);

  CPPUNIT_TEST(setupTravelSegMarkupJour_FP_LoopFwrd_NoFareBreak);
  CPPUNIT_TEST(setupTravelSegMarkupJour_FP_LoopFwrd_FareBreak);
  CPPUNIT_TEST(setupTravelSegMarkupJour_FP_LoopBack_NoFareBreak);
  CPPUNIT_TEST(setupTravelSegMarkupJour_FP_LoopBack_FareBreak);
  CPPUNIT_TEST(setupTravelSegMarkupJour_Itin_LoopFwrd_NoFareBreak);
  CPPUNIT_TEST(setupTravelSegMarkupJour_Itin_LoopFwrd_FareBreak);
  CPPUNIT_TEST(setupTravelSegMarkupJour_Itin_LoopBack_NoFareBreak);
  CPPUNIT_TEST(setupTravelSegMarkupJour_Itin_LoopBack_FareBreak);

  CPPUNIT_TEST(setupTravelSegMarkupSubJour_LoopFwrd_NoFareBreak);
  CPPUNIT_TEST(setupTravelSegMarkupSubJour_LoopFwrd_FareBreak);
  CPPUNIT_TEST(setupTravelSegMarkupSubJour_LoopBack_NoFareBreak);
  CPPUNIT_TEST(setupTravelSegMarkupSubJour_LoopBack_FareBreak);

  CPPUNIT_TEST(setupTravelSegMarkupFare_NoFareBreak);
  CPPUNIT_TEST(setupTravelSegMarkupFare_FareBreak);
  CPPUNIT_TEST(setupTravelSegMarkupFare_FMDirUnknown);

  CPPUNIT_TEST(testSetupTravelSegMarkupRtw);

  CPPUNIT_TEST_SUITE_END();
public:
  void testSetupTravelSegMarkupSetFurthest_ScopeJour_NoMatchCriteria()
  {
    // no FURTHEST match crieria, nothing is set
    _tsiInfo.matchCriteria().clear();
    assertSetupTravelSegMarkupSetFurthest(true, -1);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeJour_NoStop_FBADNo()
  {
    // no fare breaks
    assertSetupTravelSegMarkupSetFurthest(true, -1);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeJour_NoStop_FBAD1()
  {
    // fare break on first segment, match it as furthest
    _tsms.find(&_aSeg[0])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetFurthest(true, 0);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeJour_NoStop_FBAD2()
  {
    // fare break on second segment, match it as furthest
    _tsms.find(&_aSeg[1])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetFurthest(true, 1);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeJour_NoStop_FBAD3()
  {
    // fare break on third segment, match it as furthest
    _tsms.find(&_aSeg[2])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetFurthest(true, 2);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeJour_Stop_StopNo()
  {
    // no stopovers
    _stopoverExists = true;
    assertSetupTravelSegMarkupSetFurthest(true, -1);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeJour_Stop_Stop1()
  {
    // stopover on first
    _stopoverExists = true;
    _tsms.find(&_aSeg[0])->isStopover() = true;
    assertSetupTravelSegMarkupSetFurthest(true, 0);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeJour_Stop_Stop2()
  {
    // stopover on second
    _stopoverExists = true;
    _tsms.find(&_aSeg[1])->isStopover() = true;
    assertSetupTravelSegMarkupSetFurthest(true, 1);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeJour_Stop_Stop3()
  {
    // stopover on third
    _stopoverExists = true;
    _tsms.find(&_aSeg[2])->isStopover() = true;
    assertSetupTravelSegMarkupSetFurthest(true, 2);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeSubJour_NoMatchCriteria()
  {
    // no FURTHEST match crieria, nothing is set
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsiInfo.matchCriteria().clear();
    assertSetupTravelSegMarkupSetFurthest(true, -1);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeSubJour_NoStop_FBADNo()
  {
    // no fare breaks
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertSetupTravelSegMarkupSetFurthest(true, -1);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeSubJour_NoStop_FBAD1()
  {
    // fare break on first segment, match it as furthest
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[0])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetFurthest(true, 0);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeSubJour_NoStop_FBAD2()
  {
    // fare break on second segment, match it as furthest
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[1])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetFurthest(true, 1);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeSubJour_NoStop_FBAD3()
  {
    // fare break on third segment, match it as furthest
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[2])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetFurthest(true, 2);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeFare()
  {
    // no support for fare component
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertSetupTravelSegMarkupSetFurthest(false, 0);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeSubJour_Stop_StopNo()
  {
    // no stopover
    _stopoverExists = true;
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertSetupTravelSegMarkupSetFurthest(true, -1);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeSubJour_Stop_Stop1()
  {
    // stopover on first
    _stopoverExists = true;
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[0])->isStopover() = true;
    assertSetupTravelSegMarkupSetFurthest(true, 0);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeSubJour_Stop_Stop2()
  {
    // stopover on second
    _stopoverExists = true;
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[1])->isStopover() = true;
    assertSetupTravelSegMarkupSetFurthest(true, 1);
  }
  void testSetupTravelSegMarkupSetFurthest_ScopeSubJour_Stop_Stop3()
  {
    // stopover on third
    _stopoverExists = true;
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[2])->isStopover() = true;
    assertSetupTravelSegMarkupSetFurthest(true, 2);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeJour_NoMatchCriteria()
  {
    // no match criteria - do nothing
    _tsiInfo.matchCriteria().clear();
    assertSetupTravelSegMarkupSetStopover(true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeJour_NoFareBreak()
  {
    // no fare breaks - nothing happend
    assertSetupTravelSegMarkupSetStopover(true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD1_Stop()
  {
    //  there are stropovers - do nothing
    _stopoverExists = true;
    _tsms.find(&_aSeg[0])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetStopover(true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD1_NoStop()
  {
    // no stopovers found, carrier equal, nothing happend
    _tsms.find(&_aSeg[0])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetStopover(true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD1_NoStop_DiffCXR()
  {
    // no stopovers found, carrier differs, mark stopover
    _tsms.find(&_aSeg[0])->fareBreakAtDestination() = true;
    _aSeg[0].carrier() = "YY";
    assertSetupTravelSegMarkupSetStopover(true, true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD1_NoStop_DiffFlight()
  {
    // no stopovers found, flight numbers differs, mark stopover
    _tsms.find(&_aSeg[0])->fareBreakAtDestination() = true;
    _aSeg[0].flightNumber() = 999;
    assertSetupTravelSegMarkupSetStopover(true, true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD2_Stop()
  {
    //  there are stropovers - do nothing
    _stopoverExists = true;
    _tsms.find(&_aSeg[1])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetStopover(true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD2_NoStop()
  {
    // no stopovers found, carrier equal, nothing happend
    _tsms.find(&_aSeg[1])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetStopover(true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD2_NoStop_DiffCXR()
  {
    // no stopovers found, carrier differs, mark stopover
    _tsms.find(&_aSeg[1])->fareBreakAtDestination() = true;
    _aSeg[2].carrier() = "YY";
    assertSetupTravelSegMarkupSetStopover(true, false, true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD2_NoStop_DiffFlight()
  {
    // no stopovers found, flight numbers differs, mark stopover
    _tsms.find(&_aSeg[1])->fareBreakAtDestination() = true;
    _aSeg[2].flightNumber() = 999;
    assertSetupTravelSegMarkupSetStopover(true, false, true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD3_Stop()
  {
    //  there are stropovers - do nothing
    _stopoverExists = true;
    _tsms.find(&_aSeg[2])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetStopover(true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeJour_FBAD3_NoStop()
  {
    // no stopovers found, next segment is arunk - mark stopover
    _tsms.find(&_aSeg[2])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetStopover(true, false, false, true);
  }

  void testSetupTravelSegMarkupSetStopover_ScopeSubJour_NoMatchCriteria()
  {
    // no match criteria - do nothing
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsiInfo.matchCriteria().clear();
    assertSetupTravelSegMarkupSetStopover(true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeSubJour_NoFareBreak()
  {
    // no fare breaks - nothing happend
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    assertSetupTravelSegMarkupSetStopover(true, false, false, true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD1_Stop()
  {
    //  there are stropovers - do nothing
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _stopoverExists = true;
    _tsms.find(&_aSeg[0])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetStopover(true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD1_NoStop()
  {
    // no stopovers found, carrier equal, nothing happend
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[0])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetStopover(true, false, false, true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD1_NoStop_DiffCXR()
  {
    // no stopovers found, carrier differs, mark stopover
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[0])->fareBreakAtDestination() = true;
    _aSeg[0].carrier() = "YY";
    assertSetupTravelSegMarkupSetStopover(true, true, false, true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD1_NoStop_DiffFlight()
  {
    // no stopovers found, flight numbers differs, mark stopover
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[0])->fareBreakAtDestination() = true;
    _aSeg[0].flightNumber() = 999;
    assertSetupTravelSegMarkupSetStopover(true, true, false, true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD2_Stop()
  {
    //  there are stropovers - do nothing
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _stopoverExists = true;
    _tsms.find(&_aSeg[1])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetStopover(true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD2_NoStop()
  {
    // no stopovers found, carrier equal, nothing happend
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[1])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetStopover(true, false, false, true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD2_NoStop_DiffCXR()
  {
    // no stopovers found, carrier differs, mark stopover
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[1])->fareBreakAtDestination() = true;
    _aSeg[2].carrier() = "YY";
    assertSetupTravelSegMarkupSetStopover(true, false, true, true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD2_NoStop_DiffFlight()
  {
    // no stopovers found, flight numbers differs, mark stopover
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[1])->fareBreakAtDestination() = true;
    _aSeg[2].flightNumber() = 999;
    assertSetupTravelSegMarkupSetStopover(true, false, true, true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD3_Stop()
  {
    //  there are stropovers - do nothing
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _stopoverExists = true;
    _tsms.find(&_aSeg[2])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetStopover(true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeSubJour_FBAD3_NoStop()
  {
    // no stopovers found, next segment is arunk - mark stopover
    _tsiScope = RuleConst::TSI_SCOPE_SUB_JOURNEY;
    _tsms.find(&_aSeg[2])->fareBreakAtDestination() = true;
    assertSetupTravelSegMarkupSetStopover(true, false, false, true);
  }
  void testSetupTravelSegMarkupSetStopover_ScopeFareJour()
  {
    _tsiScope = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    assertSetupTravelSegMarkupSetStopover(false);
  }
  void testSetupTravelSegMarkupCreateSegFM_NoFM_LoopFwrd_EmptyCont_NoSegStop()
  {
    // no fare market, no segment added, no stopover, no GD change
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, false);
  }
  void testSetupTravelSegMarkupCreateSegFM_NoFM_LoopFwrd_EmptyCont_SegStop()
  {
    // no fare market, no segment added, stopover, no GD change
    _aSeg[0].stopOver() = true;
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, true);
  }
  void testSetupTravelSegMarkupCreateSegFM_NoFM_LoopFwrd_EmptyCont_SegForcedStop()
  {
    // no fare market, no segment added, stopover, no GD change
    _aSeg[0].forcedStopOver() = 'Y';
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, true);
  }
  void testSetupTravelSegMarkupCreateSegFM_NoFM_LoopFwrd_Cont_NoSegStop()
  {
    // no fare market, first segment added, no stopover, no GD change
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, false, &_aSeg[1]);
  }
  void testSetupTravelSegMarkupCreateSegFM_NoFM_LoopFwrd_Cont_SegStop()
  {
    // no fare market, first segment added, stopover, no GD change
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    _aSeg[0].stopOver() = true;
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, true, &_aSeg[1]);
  }
  void testSetupTravelSegMarkupCreateSegFM_NoFM_LoopFwrd_Cont_SegForcedStop()
  {
    // no fare market, first segment added, stopover, no GD change
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    _aSeg[0].forcedStopOver() = 'Y';
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, true, &_aSeg[1]);
  }
  void testSetupTravelSegMarkupCreateSegFM_NoFM_LoopBack_EmptyCont_NoSegStop()
  {
    // no fare market, no segment added, no stopover, no GD change
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, false);
  }
  void testSetupTravelSegMarkupCreateSegFM_NoFM_LoopBack_EmptyCont_SegStop()
  {
    // no fare market, no segment added, stopover, no GD change
    _aSeg[0].stopOver() = true;
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, true);
  }
  void testSetupTravelSegMarkupCreateSegFM_NoFM_LoopBack_EmptyCont_SegForcedStop()
  {
    // no fare market, no segment added, stopover, no GD change
    _aSeg[0].forcedStopOver() = 'Y';
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, true);
  }
  void testSetupTravelSegMarkupCreateSegFM_NoFM_LoopBack_Cont_NoSegStop()
  {
    // no fare market, first segment added, no stopover, no GD change
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, false, &_aSeg[1]);
  }
  void testSetupTravelSegMarkupCreateSegFM_NoFM_LoopBack_Cont_SegStop()
  {
    // no fare market, first segment added, stopover, no GD change
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    _aSeg[0].stopOver() = true;
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, true, &_aSeg[1]);
  }
  void testSetupTravelSegMarkupCreateSegFM_NoFM_LoopBack_Cont_SegForcedStop()
  {
    // no fare market, first segment added, stopover, no GD change
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    _aSeg[0].forcedStopOver() = 'Y';
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, true, &_aSeg[1]);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_EmptyCont_NoSegStop()
  {
    // fare market, no segment added, no stopover, GD set to PA
    setFM();
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, false, 0, GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_EmptyCont_SegStop()
  {
    // fare market, no segment added, stopover, GD set to PA
    setFM();
    _aSeg[0].stopOver() = true;
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, true, 0, GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_EmptyCont_SegForcedStop()
  {
    // fare market, no segment added, stopover, GD set to PA
    setFM();
    _aSeg[0].forcedStopOver() = 'Y';
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, true, 0, GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_EmptyCont_FMA1()
  {
    fallback::fixed::value::APO29538_StopoverMinStay.set(true);

    // fare market, no segment added, stopover on A1, GD set to PA
    setFM(&_aSeg[0]);
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, true, 0, GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_EmptyCont_FMA2()
  {
    fallback::fixed::value::APO29538_StopoverMinStay.set(true);

    // fare market, no segment added, stopover on A2, GD set to PA
    setFM(&_aSeg[1]);
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, false, 0, GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_Cont_NoSegStop()
  {
    // fare market, first segment added, no stopover, GD set to PA
    setFM();
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, false, &_aSeg[1], GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_Cont_SegStop()
  {
    // fare market, first segment added, stopover, GD set to PA
    setFM();
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    _aSeg[0].stopOver() = true;
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, true, &_aSeg[1], GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_Cont_SegForcedStop()
  {
    // fare market, first segment added, stopover, GD set to PA
    setFM();
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    _aSeg[0].forcedStopOver() = 'Y';
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, true, &_aSeg[1], GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_Cont_FMA1()
  {
    fallback::fixed::value::APO29538_StopoverMinStay.set(true);

    // fare market, first segment added, stopover, GD set to PA
    setFM(&_aSeg[0]);
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, true, &_aSeg[1], GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopFwrd_Cont_FMA2()
  {
    fallback::fixed::value::APO29538_StopoverMinStay.set(true);

    // fare market, first segment added, stopover, GD set to PA
    setFM(&_aSeg[1]);
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], true, false, &_aSeg[1], GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_EmptyCont_NoSegStop()
  {
    // fare market, no segment added, no stopover, GD set to PA
    setFM();
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, false, 0, GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_EmptyCont_SegStop()
  {
    // fare market, no segment added, stopover, GD set to PA
    setFM();
    _aSeg[0].stopOver() = true;
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, true, 0, GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_EmptyCont_SegForcedStop()
  {
    // fare market, no segment added, stopover, GD set to PA
    setFM();
    _aSeg[0].forcedStopOver() = 'Y';
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, true, 0, GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_EmptyCont_FMA1()
  {
    fallback::fixed::value::APO29538_StopoverMinStay.set(true);

    // fare market, first segment added, stopover on A1, GD set to PA
    setFM(&_aSeg[0]);
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, true, 0, GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_EmptyCont_FMA2()
  {
    fallback::fixed::value::APO29538_StopoverMinStay.set(true);

    // fare market, first segment added, stopover on A2, GD set to PA
    setFM(&_aSeg[1]);
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, false, 0, GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_Cont_NoSegStop()
  {
    // fare market, first segment added, no stopover, GD set to PA
    setFM();
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, false, &_aSeg[1], GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_Cont_SegStop()
  {
    // fare market, first segment added, stopover, GD set to PA
    setFM();
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    _aSeg[0].stopOver() = true;
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, true, &_aSeg[1], GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_Cont_SegForcedStop()
  {
    // fare market, first segment added, stopover, GD set to PA
    setFM();
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    _aSeg[0].forcedStopOver() = 'Y';
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, true, &_aSeg[1], GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_Cont_FMA1()
  {
    fallback::fixed::value::APO29538_StopoverMinStay.set(true);

    // fare market, first segment added, stopover, GD set to PA
    setFM(&_aSeg[0]);
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, true, &_aSeg[1], GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegFM_FM_LoopBack_Cont_FMA2()
  {
    fallback::fixed::value::APO29538_StopoverMinStay.set(true);

    // fare market, first segment added, stopover, GD set to PA
    setFM(&_aSeg[1]);
    RuleUtilTSI::setupTravelSegMarkupCreateSegFM(_tsms2, &_aSeg[1], 0, GeoTravelType(), true, 0, 0);
    assertSetupTravelSegMarkupCreateSegFM(&_aSeg[0], false, false, &_aSeg[1], GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegPTF_NoPTF()
  {
    // without PaxTypeFare  this function works exactly like setupTravelSegMarkupCreateSegFM without
    // FM
    assertSetupTravelSegMarkupCreateSegPTF(&_aSeg[0], true, false);
  }
  void testSetupTravelSegMarkupCreateSegPTF_PTF_GdPA()
  {
    // check if match on global direction
    setPTF(GlobalDirection::PA);
    assertSetupTravelSegMarkupCreateSegPTF(&_aSeg[0], false, false, 0, GlobalDirection::PA);
  }
  void testSetupTravelSegMarkupCreateSegPTF_PTF_GdAT()
  {
    // check if match on global direction
    setPTF(GlobalDirection::AT);
    assertSetupTravelSegMarkupCreateSegPTF(&_aSeg[0], false, false, 0, GlobalDirection::AT);
  }
  void testSetupTravelSegMarkupGetStart_PricingUnit_Fwdr()
  {
    std::vector<PricingUnit*>& cont = getContainer<PricingUnit>();
    CPPUNIT_ASSERT_EQUAL(0, RuleUtilTSI::setupTravelSegMarkupGetStart(true, cont));
  }
  void testSetupTravelSegMarkupGetStart_PricingUnit_Back()
  {
    std::vector<PricingUnit*>& cont = getContainer<PricingUnit>();
    CPPUNIT_ASSERT_EQUAL(2, RuleUtilTSI::setupTravelSegMarkupGetStart(false, cont));
  }
  void testSetupTravelSegMarkupGetStart_FareUsage_Fwdr()
  {
    std::vector<FareUsage*>& cont = getContainer<FareUsage>();
    CPPUNIT_ASSERT_EQUAL(0, RuleUtilTSI::setupTravelSegMarkupGetStart(true, cont));
  }
  void testSetupTravelSegMarkupGetStart_FareUsage_Back()
  {
    std::vector<FareUsage*>& cont = getContainer<FareUsage>();
    CPPUNIT_ASSERT_EQUAL(2, RuleUtilTSI::setupTravelSegMarkupGetStart(false, cont));
  }
  void testSetupTravelSegMarkupGetStart_TravelSeg_Fwdr()
  {
    std::vector<TravelSeg*>& cont = getContainer<AirSeg, TravelSeg>();
    CPPUNIT_ASSERT_EQUAL(0, RuleUtilTSI::setupTravelSegMarkupGetStart(true, cont));
  }
  void testSetupTravelSegMarkupGetStart_TravelSeg_Back()
  {
    std::vector<TravelSeg*>& cont = getContainer<AirSeg, TravelSeg>();
    CPPUNIT_ASSERT_EQUAL(2, RuleUtilTSI::setupTravelSegMarkupGetStart(false, cont));
  }
  void testSetupTravelSegMarkupGetEnd_PricingUnit_Fwdr()
  {
    std::vector<PricingUnit*>& cont = getContainer<PricingUnit>();
    CPPUNIT_ASSERT_EQUAL(3, RuleUtilTSI::setupTravelSegMarkupGetEnd(true, cont));
  }
  void testSetupTravelSegMarkupGetEnd_PricingUnit_Back()
  {
    std::vector<PricingUnit*>& cont = getContainer<PricingUnit>();
    CPPUNIT_ASSERT_EQUAL(-1, RuleUtilTSI::setupTravelSegMarkupGetEnd(false, cont));
  }
  void testSetupTravelSegMarkupGetEnd_FareUsage_Fwdr()
  {
    std::vector<FareUsage*>& cont = getContainer<FareUsage>();
    CPPUNIT_ASSERT_EQUAL(3, RuleUtilTSI::setupTravelSegMarkupGetEnd(true, cont));
  }
  void testSetupTravelSegMarkupGetEnd_FareUsage_Back()
  {
    std::vector<FareUsage*>& cont = getContainer<FareUsage>();
    CPPUNIT_ASSERT_EQUAL(-1, RuleUtilTSI::setupTravelSegMarkupGetEnd(false, cont));
  }
  void testSetupTravelSegMarkupGetEnd_TravelSeg_Fwdr()
  {
    std::vector<TravelSeg*>& cont = getContainer<AirSeg, TravelSeg>();
    CPPUNIT_ASSERT_EQUAL(3, RuleUtilTSI::setupTravelSegMarkupGetEnd(true, cont));
  }
  void testSetupTravelSegMarkupGetEnd_TravelSeg_Back()
  {
    std::vector<TravelSeg*>& cont = getContainer<AirSeg, TravelSeg>();
    CPPUNIT_ASSERT_EQUAL(-1, RuleUtilTSI::setupTravelSegMarkupGetEnd(false, cont));
  }
  void testSetupTravelSegMarkupSetDirection_Inbound()
  {
    RuleUtilTSI::TSITravelSegMarkup t;
    RuleUtilTSI::setupTravelSegMarkupSetDirection(t, true);
    CPPUNIT_ASSERT_EQUAL(RuleConst::INBOUND, t.direction());
  }
  void testSetupTravelSegMarkupSetDirection_Outbound()
  {
    RuleUtilTSI::TSITravelSegMarkup t;
    RuleUtilTSI::setupTravelSegMarkupSetDirection(t, false);
    CPPUNIT_ASSERT_EQUAL(RuleConst::OUTBOUND, t.direction());
  }
  void setupTravelSegMarkupJour_FP_LoopFwrd_NoFareBreak()
  {
    // set Fare path, check if all validation pass
    setFPPU();
    setupTravelSegMarkupJour(true, true, 3);
    testTSM(&_aSeg[0], true, false, false);
    testTSM(&_aSeg[1], false, true, false);
    testTSM(&_aSeg[2], true, true, true);
  }
  void setupTravelSegMarkupJour_FP_LoopFwrd_FareBreak()
  {
    // set Fare path, check if all validation pass
    setFPPU();
    setupTravelSegMarkupJour(true, true, 3, true);
    testTSM(&_aSeg[0], true, false, false);
    testTSM(&_aSeg[1], false, true, false, "NOT FARE-BREAK");
    testTSM(&_aSeg[2], true, true, true);
  }
  void setupTravelSegMarkupJour_FP_LoopBack_NoFareBreak()
  {
    // set Fare path, check if all validation pass
    setFPPU();
    setupTravelSegMarkupJour(true, false, 3);
    testTSM(&_aSeg[0], true, false, false);
    testTSM(&_aSeg[1], false, true, false);
    testTSM(&_aSeg[2], true, true, true);
  }
  void setupTravelSegMarkupJour_FP_LoopBack_FareBreak()
  {
    // set Fare path, check if all validation pass
    setFPPU();
    setupTravelSegMarkupJour(true, false, 3, true);
    testTSM(&_aSeg[0], true, false, false);
    testTSM(&_aSeg[1], false, true, false, "NOT FARE-BREAK");
    testTSM(&_aSeg[2], true, true, true);
  }
  void setupTravelSegMarkupJour_Itin_LoopFwrd_NoFareBreak()
  {
    // add 2 more sectors, validate that all process took place
    _itin.travelSeg().push_back(&_aSeg[1]);
    _itin.travelSeg().push_back(&_aSeg[2]);
    setupTravelSegMarkupJour(true, true, 3);
    testTSM(&_aSeg[0], true, false, false);
    testTSM(&_aSeg[1], false, false, true);
    testTSM(&_aSeg[2], false, true, true);
  }
  void setupTravelSegMarkupJour_Itin_LoopFwrd_FareBreak()
  {
    // add 2 more sectors, validate that all process took place (dir not set)
    _itin.travelSeg().push_back(&_aSeg[1]);
    _itin.travelSeg().push_back(&_aSeg[2]);
    setupTravelSegMarkupJour(true, true, 3, true);
    testTSM(&_aSeg[0], true, false, false);
    testTSM(&_aSeg[1], false, false, false, "NOT FARE-BREAK");
    testTSM(&_aSeg[2], false, true, false, "NOT FARE-BREAK");
  }
  void setupTravelSegMarkupJour_Itin_LoopBack_NoFareBreak()
  {
    // add 2 more sectors, validate that all process took place
    _itin.travelSeg().push_back(&_aSeg[1]);
    _itin.travelSeg().push_back(&_aSeg[2]);
    setupTravelSegMarkupJour(true, false, 3);
    testTSM(&_aSeg[0], true, false, false);
    testTSM(&_aSeg[1], false, false, true);
    testTSM(&_aSeg[2], false, true, true);
  }
  void setupTravelSegMarkupJour_Itin_LoopBack_FareBreak()
  {
    // add 2 more sectors, validate that all process took place (dir not set)
    _itin.travelSeg().push_back(&_aSeg[1]);
    _itin.travelSeg().push_back(&_aSeg[2]);
    setupTravelSegMarkupJour(true, false, 3, true);
    testTSM(&_aSeg[0], true, false, false);
    testTSM(&_aSeg[1], false, false, false, "NOT FARE-BREAK");
    testTSM(&_aSeg[2], false, true, false, "NOT FARE-BREAK");
  }
  void setupTravelSegMarkupSubJour_LoopFwrd_NoFareBreak()
  {
    // set Pricing unit, check if all validation pass
    setFPPU();
    setupTravelSegMarkupSubJour(true, true, 3);
    testTSM(&_aSeg[0], true, false, false);
    testTSM(&_aSeg[1], false, true, false);
    testTSM(&_aSeg[2], true, true, true);
  }
  void setupTravelSegMarkupSubJour_LoopFwrd_FareBreak()
  {
    // set Pricing unit, check if all validation pass
    setFPPU();
    setupTravelSegMarkupSubJour(true, true, 3, true);
    testTSM(&_aSeg[0], true, false, false);
    testTSM(&_aSeg[1], false, true, false, "NOT FARE-BREAK");
    testTSM(&_aSeg[2], true, true, true);
  }
  void setupTravelSegMarkupSubJour_LoopBack_NoFareBreak()
  {
    // set Pricing unit, check if all validation pass
    setFPPU();
    setupTravelSegMarkupSubJour(true, false, 3);
    testTSM(&_aSeg[0], true, false, false);
    testTSM(&_aSeg[1], false, true, false);
    testTSM(&_aSeg[2], true, true, true);
  }
  void setupTravelSegMarkupSubJour_LoopBack_FareBreak()
  {
    // set Pricing unit, check if all validation pass
    setFPPU();
    setupTravelSegMarkupSubJour(true, false, 3, true);
    testTSM(&_aSeg[0], true, false, false);
    testTSM(&_aSeg[1], false, true, false, "NOT FARE-BREAK");
    testTSM(&_aSeg[2], true, true, true);
  }
  void setupTravelSegMarkupFare_NoFareBreak()
  {
    // no fare breaks for fare component
    setFM2();
    setupTravelSegMarkupFare(true, true, 3);
    testTSM(&_aSeg[0], false, false, false);
    testTSM(&_aSeg[1], false, false, false);
    testTSM(&_aSeg[2], false, false, false);
  }
  void setupTravelSegMarkupFare_FareBreak()
  {
    // no fare breaks for fare component
    setFM2();
    setupTravelSegMarkupFare(true, true, 3, true);
    testTSM(&_aSeg[0], false, false, false);
    testTSM(&_aSeg[1], false, false, false);
    testTSM(&_aSeg[2], false, false, false);
  }
  void setupTravelSegMarkupFare_FMDirUnknown()
  {
    setFMWithPU();
    setupTravelSegMarkupFare(true, true, 3, true);
    testTSM(&_aSeg[0], false, false, true);
    testTSM(&_aSeg[1], false, false, true);
    testTSM(&_aSeg[2], false, false, true);
  }
  void testSetupTravelSegMarkupRtw()
  {
    _options->setRtw(true);
    setupFpRtw();
    setupTravelSegMarkupSubJour(true, true, 3);
    CPPUNIT_ASSERT_EQUAL(RuleConst::OUTBOUND, _tsms2.find(&_aSeg[0])->direction());
    CPPUNIT_ASSERT_EQUAL(RuleConst::OUTBOUND, _tsms2.find(&_aSeg[1])->direction());
    CPPUNIT_ASSERT_EQUAL(RuleConst::OUTBOUND, _tsms2.find(&_aSeg[2])->direction());
    RuleUtilTSI::setupTravelSegMarkupRtwPostprocess(*_trx, *_tsi, _tsms2);
    CPPUNIT_ASSERT_EQUAL(RuleConst::OUTBOUND, _tsms2.find(&_aSeg[0])->direction());
    CPPUNIT_ASSERT_EQUAL(RuleConst::INBOUND, _tsms2.find(&_aSeg[1])->direction());
    CPPUNIT_ASSERT_EQUAL(RuleConst::INBOUND, _tsms2.find(&_aSeg[2])->direction());
  }

  void setUp()
  {
    RuleUtilTSITest_Base::setUp();
    _vendor = "ATP";
    _tsiScope = RuleConst::TSI_SCOPE_JOURNEY;
    _fm = 0;
    _ptf = 0;
    _fp = 0;
    _tsi = new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, &_itin, &_pu, 0);
    _memHandle.insert(_tsi);
    _tsiSetup = _memHandle.create<RuleUtilTSI::TSISetupInternational>();
    _stopoverExists = false;
    _pu.travelSeg().push_back(&_aSeg[0]);
    _itin.travelSeg().push_back(&_aSeg[0]);
    _itin.furthestPointSegmentOrder() = 1;
    _l1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    _l2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocATL.xml");
    _l3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocOSA.xml");
    _l4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTYO.xml");
    // 1-1 1-3 3-3
    _aSeg[0].origin() = _l1;
    _aSeg[0].destination() = _l2;
    _aSeg[1].origin() = _l2;
    _aSeg[1].destination() = _l3;
    _aSeg[2].origin() = _l3;
    _aSeg[2].destination() = _l4;
    _aSeg[0].segmentOrder() = 1;
    _aSeg[1].segmentOrder() = 2;
    _aSeg[2].segmentOrder() = 3;
    // 3air segs and 1 arunk
    RuleUtilTSI::TSITravelSegMarkup tsm;
    tsm.travelSeg() = &_aSeg[0];
    tsm.nextTravelSeg() = &_aSeg[1];
    _tsms.push_back(tsm);
    tsm.travelSeg() = &_aSeg[1];
    tsm.nextTravelSeg() = &_aSeg[2];
    _tsms.push_back(tsm);
    tsm.travelSeg() = &_aSeg[2];
    tsm.nextTravelSeg() = &_arunk;
    _tsms.push_back(tsm);
    tsm.travelSeg() = &_arunk;
    tsm.nextTravelSeg() = 0;
    _tsms.push_back(tsm);
    // add match criteria
    _tsiInfo.matchCriteria() += TSIInfo::FURTHEST, TSIInfo::STOP_OVER;
    for (int i = 0; i < 3; i++)
    {
      _aSeg[i].carrier() = "XX";
      _aSeg[i].flightNumber() = 123;
    }
    _arunk.segmentType() = Arunk;
    _tsiInfo.geoCheck() = RuleConst::TSI_APP_CHECK_ORIG;
  }

private:
  void setFPPU()
  {
    _fp = _memHandle.create<FarePath>();
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    PaxTypeFare* ptf1 = _memHandle.create<PaxTypeFare>();
    PaxTypeFare* ptf2 = _memHandle.create<PaxTypeFare>();
    Fare* f1 = _memHandle.create<Fare>();
    Fare* f2 = _memHandle.create<Fare>();
    FareInfo* fi1 = _memHandle.create<FareInfo>();
    FareInfo* fi2 = _memHandle.create<FareInfo>();
    ptf1->setFare(f1);
    ptf2->setFare(f2);
    f1->setFareInfo(fi1);
    f2->setFareInfo(fi2);
    fu1->paxTypeFare() = ptf1;
    fu2->paxTypeFare() = ptf2;
    _fp->pricingUnit().push_back(pu);
    pu->fareUsage() += fu1;
    pu->fareUsage().push_back(fu2);
    fu1->inbound() = false;
    fu2->inbound() = true;
    fu1->travelSeg() += &_aSeg[0], &_aSeg[1];
    fu2->travelSeg().push_back(&_aSeg[2]);
    _tsi = _memHandle.insert(
        new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, _fp, &_itin, pu, _fm));
  }
  void setupFpRtw()
  {
    _itin.travelSeg().clear();
    _itin.travelSeg() += &_aSeg[0], &_aSeg[1], &_aSeg[2];
    _fp = _memHandle.create<FarePath>();
    PricingUnit*  pu    = _memHandle.create<PricingUnit>();
    FareUsage*    fu    = _memHandle.create<FareUsage>();
    PaxTypeFare*  ptf   = _memHandle.create<PaxTypeFare>();
    Fare*         fare  = _memHandle.create<Fare>();
    FareInfo*     fi    = _memHandle.create<FareInfo>();
    fare->setFareInfo(fi);
    ptf->setFare(fare);
    fu->paxTypeFare() = ptf;
    fu->inbound() = false;
    fu->travelSeg() = _itin.travelSeg();
    pu->fareUsage() += fu;
    pu->travelSeg() = _itin.travelSeg();
    _fp->pricingUnit().push_back(pu);
    _tsi = _memHandle.insert(new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, _fp, &_itin, pu, _fm));
  }
  void setFM2()
  {
    _fm = _memHandle.create<FareMarket>();
    _tsi = _memHandle.insert(
        new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, &_itin, 0, _fm));
    _fm->travelSeg() += &_aSeg[0], &_aSeg[1], &_aSeg[2];
  }
  void setFMWithPU()
  {
    _fm = _memHandle.create<FareMarket>();
    _tsi = _memHandle.insert(
        new RuleUtilTSI::TSIData(_tsiInfo, _tsiScope, _vendor, 0, &_itin, &_pu, _fm));
    _fm->travelSeg() += &_aSeg[0], &_aSeg[1], &_aSeg[2];
    _fm->direction() = FMDirection::UNKNOWN;
    FareUsage* fu1 = _memHandle.create<FareUsage>();
    fu1->paxTypeFare() = _memHandle.create<PaxTypeFare>();
    fu1->paxTypeFare()->fareMarket() = _memHandle.create<FareMarket>();
    fu1->paxTypeFare()->fareMarket()->travelSeg() += &_aSeg[0];
    FareUsage* fu2 = _memHandle.create<FareUsage>();
    fu2->paxTypeFare() = _memHandle.create<PaxTypeFare>();
    fu2->paxTypeFare()->fareMarket() = _memHandle.create<FareMarket>();
    fu2->paxTypeFare()->fareMarket()->travelSeg() += &_aSeg[0], &_aSeg[1], &_aSeg[2];
    fu2->inbound() = true;
    _pu.fareUsage() += fu1, fu2;
  }
  void setupTravelSegMarkupJour(
      bool ret, bool loop, size_t segs, bool farebreak = false, bool stop = false)
  {
    bool stopover = false;
    _tsiChain.buildTSIChain(*_trx, *_tsi, loop, farebreak);
    CPPUNIT_ASSERT_EQUAL(
        ret,
        RuleUtilTSI::setupTravelSegMarkupJour(*_trx, 0, *_tsi, _tsms2, loop, _tsiChain, stopover));
    CPPUNIT_ASSERT_EQUAL(segs, _tsms2.size());
    CPPUNIT_ASSERT_EQUAL(stop, stopover);
  }
  void setupTravelSegMarkupSubJour(
      bool ret, bool loop, size_t segs, bool farebreak = false, bool stop = false)
  {
    bool stopover = false;
    _tsiChain.buildTSIChain(*_trx, *_tsi, loop, farebreak);
    CPPUNIT_ASSERT_EQUAL(ret,
                         RuleUtilTSI::setupTravelSegMarkupSubJour(
                             *_trx, 0, *_tsi, _tsms2, loop, _tsiChain, stopover));
    CPPUNIT_ASSERT_EQUAL(segs, _tsms2.size());
    CPPUNIT_ASSERT_EQUAL(stop, stopover);
  }
  void setupTravelSegMarkupFare(
      bool ret, bool loop, size_t segs, bool farebreak = false, bool stop = false)
  {
    bool stopover = false;
    _tsiChain.buildTSIChain(*_trx, *_tsi, loop, farebreak);
    CPPUNIT_ASSERT_EQUAL(
        ret,
        RuleUtilTSI::setupTravelSegMarkupFare(*_trx, 0, *_tsi, _tsms2, loop, _tsiChain, stopover));
    CPPUNIT_ASSERT_EQUAL(segs, _tsms2.size());
    CPPUNIT_ASSERT_EQUAL(stop, stopover);
  }
  void testTSM(const TravelSeg* seg,
               bool fbAtOrig,
               bool fbAtDest,
               bool inbound,
               std::string nomatch = "NOT CHECKED")
  {
    RuleUtilTSI::TSITravelSegMarkup& t = *_tsms2.find(seg);
    CPPUNIT_ASSERT_EQUAL(fbAtOrig, t.fareBreakAtOrigin());
    CPPUNIT_ASSERT_EQUAL(fbAtDest, t.fareBreakAtDestination());
    CPPUNIT_ASSERT_EQUAL(inbound, t.direction() == RuleConst::INBOUND);
    CPPUNIT_ASSERT_EQUAL(nomatch, t.noMatchReason());
  }

  template <class T>
  std::vector<T*>& getContainer(int len = 3)
  {
    return getContainer<T, T>(len);
  }
  template <class T, class RetT>
  std::vector<RetT*>& getContainer(int len = 3)
  {
    std::vector<RetT*>* puVec = _memHandle.create<std::vector<RetT*> >();
    for (int i = 0; i < len; i++)
    {
      T* pu = _memHandle.create<T>();
      puVec->push_back(pu);
    }
    return *puVec;
  }
  void assertSetupTravelSegMarkupCreateSegPTF(TravelSeg* ts,
                                              bool loop,
                                              bool stp,
                                              TravelSeg* nts = 0,
                                              GlobalDirection gd = GlobalDirection::ZZ,
                                              std::string reason = "NOT CHECKED")
  {
    RuleUtilTSI::TSITravelSegMarkup& t = RuleUtilTSI::setupTravelSegMarkupCreateSegPTF(
        _tsms2, ts, nts, GeoTravelType(), loop, _ptf, 0);

    CPPUNIT_ASSERT_EQUAL(ts, t.travelSeg());
    CPPUNIT_ASSERT_EQUAL(stp, t.isStopover());
    CPPUNIT_ASSERT_EQUAL(reason, t.noMatchReason());
    CPPUNIT_ASSERT_EQUAL(gd, t.globalDirection());
    if (loop)
    {
      // for loop forward, then previous TSM shoul have ts set as next travel segment
      if (nts)
        CPPUNIT_ASSERT_EQUAL(ts, _tsms2.find(nts)->nextTravelSeg());
    }
    else
    {
      // if loop backawrd, then curretn TSM should have next travel segment as previous
      if (nts)
        CPPUNIT_ASSERT_EQUAL(nts, t.nextTravelSeg());
    }
  }
  void setPTF(GlobalDirection gd = GlobalDirection::PA)
  {
    setFM();
    _ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fi = _memHandle.create<FareInfo>();
    _ptf->setFare(fare);
    fare->setFareInfo(fi);
    fi->globalDirection() = gd;
    _ptf->fareMarket() = _fm;
  }

  void setFM(TravelSeg* stopover = 0, GlobalDirection gd = GlobalDirection::PA)
  {
    _fm = _memHandle.create<FareMarket>();
    _fm->setGlobalDirection(gd);
    if (stopover)
      _fm->stopOverTravelSeg().insert(stopover);
  }
  void assertSetupTravelSegMarkupCreateSegFM(TravelSeg* ts,
                                             bool loop,
                                             bool stp,
                                             TravelSeg* nts = 0,
                                             GlobalDirection gd = GlobalDirection::ZZ,
                                             std::string reason = "NOT CHECKED")
  {
    RuleUtilTSI::TSITravelSegMarkup& t = RuleUtilTSI::setupTravelSegMarkupCreateSegFM(
        _tsms2, ts, nts, GeoTravelType(), loop, _fm, 0);
    CPPUNIT_ASSERT_EQUAL(ts, t.travelSeg());
    CPPUNIT_ASSERT_EQUAL(stp, t.isStopover());
    CPPUNIT_ASSERT_EQUAL(reason, t.noMatchReason());
    CPPUNIT_ASSERT_EQUAL(gd, t.globalDirection());
    if (loop)
    {
      // for loop forward, then previous TSM shoul have ts set as next travel segment
      if (nts)
        CPPUNIT_ASSERT_EQUAL(ts, _tsms2.find(nts)->nextTravelSeg());
    }
    else
    {
      // if loop backawrd, then curretn TSM should have next travel segment as previous
      if (nts)
        CPPUNIT_ASSERT_EQUAL(nts, t.nextTravelSeg());
    }
  }
  void assertSetupTravelSegMarkupSetFurthest(bool ret, int furthest)
  {
    CPPUNIT_ASSERT_EQUAL(
        ret,
        RuleUtilTSI::setupTravelSegMarkupSetFurthest(_tsiScope, *_tsi, _stopoverExists, _tsms));
    if (ret)
    {
      // check if furthest is set correctly
      for (int i = 0; i < 3; i++)
        CPPUNIT_ASSERT_EQUAL(i == furthest, _tsms.find(&_aSeg[i])->isFurthest());
    }
  }
  void assertSetupTravelSegMarkupSetStopover(
      bool ret, bool s1 = false, bool s2 = false, bool s3 = false, bool s4 = false)
  {
    CPPUNIT_ASSERT_EQUAL(ret,
                         RuleUtilTSI::setupTravelSegMarkupSetStopover(
                             _tsiScope, *_tsi, &_fareUsage, _stopoverExists, _tsms,*_trx));
    if (ret)
    {
      /*
      CPPUNIT_ASSERT_EQUAL(s1, _tsms.find(&_aSeg[0])->isStopover());
      CPPUNIT_ASSERT_EQUAL(s2, _tsms.find(&_aSeg[1])->isStopover());
      CPPUNIT_ASSERT_EQUAL(s3, _tsms.find(&_aSeg[2])->isStopover());
      CPPUNIT_ASSERT_EQUAL(s4, _tsms.find(&_arunk)->isStopover());
      */
    }
  }

  RuleUtilTSI::TSISetupChain _tsiChain;
  RuleUtilTSI::TSITravelSegMarkupContainer _tsms;
  RuleUtilTSI::TSITravelSegMarkupContainer _tsms2;
  bool _stopoverExists;
  RuleUtilTSI::TSISetupInternational* _tsiSetup;
  RuleUtilTSI::TSIData* _tsi;
  TSIInfo _tsiInfo;
  VendorCode _vendor;
  RuleConst::TSIScopeType _tsiScope;
  PricingUnit _pu;
  Itin _itin;
  AirSeg _aSeg[3];
  ArunkSeg _arunk;
  Loc* _l1;
  Loc* _l2;
  Loc* _l3;
  Loc* _l4;
  FareMarket* _fm;
  PaxTypeFare* _ptf;
  FarePath* _fp;
  FareUsage _fareUsage;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_TSISetupTurnAround);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_TSISetupGateway);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_TSISetupOverWater);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_TSISetupInternational);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_TSISetupChain);
CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTSITest_SetupTravelSegMarkup);
}
