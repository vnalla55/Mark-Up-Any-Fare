#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/SeasonalAppl.h"
#include "DBAccess/TSIInfo.h"
#include "Rules/SeasonalApplication.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class SeasonalApplicationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SeasonalApplicationTest);

  CPPUNIT_TEST(testValidateFU_WithNoFarePath_OnePU_Pass_RestrictOnJourneyOrigin);
  CPPUNIT_TEST(testValidateFU_WithNoFarePath_OnePU_Fail_RestrictOnJourneyOrigin);
  CPPUNIT_TEST(testValidateFU_WithNoFarePath_TwoPU_Pass_RestrictOnJourneyOrigin);
  CPPUNIT_TEST(testValidateFU_WithNoFarePath_TwoPU_Fail_RestrictOnJourneyOrigin);

  CPPUNIT_TEST(testValidateFU_WithFarePath_OnePU_Pass_RestrictOnJourneyOrigin);
  CPPUNIT_TEST(testValidateFU_WithFarePath_OnePU_Fail_RestrictOnJourneyOrigin);
  CPPUNIT_TEST(testValidateFU_WithFarePath_TwoPU_Pass_RestrictOnJourneyOrigin);
  CPPUNIT_TEST(testValidateFU_WithFarePath_TwoPU_Fail_RestrictOnJourneyOrigin);

  CPPUNIT_TEST(testValidateFU_WithNoFarePath_NoTSI_OnePU_PassPUOrigin);
  CPPUNIT_TEST(testValidateFU_WithNoFarePath_NoTSI_OnePU_FailPUOrigin);
  CPPUNIT_TEST(testValidateFU_WithNoFarePath_NoTSI_TwoPU_FirstPassSecondFail);
  CPPUNIT_TEST(testValidateFU_WithNoFarePath_NoTSI_TwoPU_FirstFailSecondPass);

  CPPUNIT_TEST(testValidateFU_WithFarePath_NoTSI_OnePU_PassPUOrigin);
  CPPUNIT_TEST(testValidateFU_WithFarePath_NoTSI_OnePU_FailPUOrigin);
  CPPUNIT_TEST(testValidateFU_WithFarePath_NoTSI_TwoPU_FirstPassSecondFail);
  CPPUNIT_TEST(testValidateFU_WithFarePath_NoTSI_TwoPU_FirstFailSecondPass);

  CPPUNIT_TEST(testValidatePTF_Softpass_GeoTblItemNo);
  CPPUNIT_TEST(testValidatePTF_Pass_NoGeoTblItemNo);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _dataHandle = &(_trx.dataHandle());
    _request.ticketingAgent() = &_tktAgent;
    _trx.setRequest(&_request);
    _trx.setOptions(&_options);

    buildTravelPoints();
    buildTravelSegs();
    buildAllTestFareMarkets();
    buildAllPaxTypeFaresAndFareUsages();

    _itin.travelSeg().push_back(_tvlSeg12);
    _itin.travelSeg().push_back(_tvlSeg23);
    _geoUseTSI59 = 3456;

    setSeasonMarToApr(_seasonRule_MarToApr);
  }

  void tearDown() { _memHandle.clear(); }

  void buildTravelPoints()
  {
    _loc1 = "ABC";
    _loc2 = "BCD";
    _loc3 = "CDE";
  }

  void buildTravelSegs()
  {
    _tvlSeg12 = _memHandle.create<AirSeg>();
    _tvlSeg12->pnrSegment() = 1;
    _tvlSeg12->origAirport() = _loc1;
    _tvlSeg12->destAirport() = _loc2;

    _tvlSeg23 = _memHandle.create<AirSeg>();
    _tvlSeg23->pnrSegment() = 2;
    _tvlSeg23->origAirport() = _loc2;
    _tvlSeg23->destAirport() = _loc3;
  }

  void buildAllTestFareMarkets()
  {
    _dataHandle->get(_fm12);
    _fm12->travelSeg().push_back(_tvlSeg12);

    _dataHandle->get(_fm23);
    _fm23->travelSeg().push_back(_tvlSeg23);
  }

  void buildAllPaxTypeFaresAndFareUsages()
  {
    _dataHandle->get(_ptf12);

    // need to set PaxTypeFare vendor for retrieving GEO record
    FareInfo* fareInfo12;
    Fare* fare12;
    _dataHandle->get(fareInfo12);
    _dataHandle->get(fare12);
    fareInfo12->vendor() = "ATP";
    fare12->setFareInfo(fareInfo12);
    _ptf12->setFare(fare12);

    _ptf12->fareMarket() = _fm12;
    _dataHandle->get(_fu12);
    _fu12->paxTypeFare() = _ptf12;
    _fu12->travelSeg().push_back(_tvlSeg12);

    _dataHandle->get(_ptf23);
    FareInfo* fareInfo23;
    Fare* fare23;
    _dataHandle->get(fareInfo23);
    _dataHandle->get(fare23);
    fareInfo23->vendor() = "ATP";
    fare23->setFareInfo(fareInfo23);
    _ptf23->setFare(fare23);

    _ptf23->fareMarket() = _fm23;
    _dataHandle->get(_fu23);
    _fu23->paxTypeFare() = _ptf23;
    _fu23->travelSeg().push_back(_tvlSeg23);

    _dataHandle->get(_pu12o_23i);
    _pu12o_23i->fareUsage().push_back(_fu12);
    _pu12o_23i->fareUsage().push_back(_fu23);
    _pu12o_23i->travelSeg().push_back(_tvlSeg12);
    _pu12o_23i->travelSeg().push_back(_tvlSeg23);

    _dataHandle->get(_pu12);
    _pu12->fareUsage().push_back(_fu12);
    _pu12->travelSeg().push_back(_tvlSeg12);

    _dataHandle->get(_pu23);
    _pu23->fareUsage().push_back(_fu23);
    _pu23->travelSeg().push_back(_tvlSeg23);
  }

  void setSeasonMarToApr(SeasonalAppl& seasonRule)
  {
    seasonRule.tvlstartyear() = 0;
    seasonRule.tvlstartmonth() = 3;
    seasonRule.tvlstartDay() = 0;
    seasonRule.tvlStopyear() = 0;
    seasonRule.tvlStopmonth() = 4;
    seasonRule.tvlStopDay() = 0;
  }

  void testValidateFU_WithNoFarePath_OnePU_Pass_RestrictOnJourneyOrigin()
  {
    // TSI 59 is Journey origin
    checkGeoUseTSI59(_geoUseTSI59);
    _seasonRule_MarToApr.geoTblItemNo() = _geoUseTSI59;

    _itin.travelSeg().front()->departureDT() = DateTime(2009, 3, 4, 10, 25, 0);

    CPPUNIT_ASSERT_EQUAL(
        PASS, _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu12o_23i, *_fu12));
    CPPUNIT_ASSERT_EQUAL(
        PASS, _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu12o_23i, *_fu23));
  }

  void testValidateFU_WithNoFarePath_OnePU_Fail_RestrictOnJourneyOrigin()
  {
    // TSI 59 is Journey origin
    checkGeoUseTSI59(_geoUseTSI59);
    _seasonRule_MarToApr.geoTblItemNo() = _geoUseTSI59;

    _itin.travelSeg().front()->departureDT() = DateTime(2009, 5, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(
        FAIL, _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu12o_23i, *_fu12));
    CPPUNIT_ASSERT_EQUAL(
        FAIL, _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu12o_23i, *_fu23));
  }

  void testValidateFU_WithNoFarePath_TwoPU_Pass_RestrictOnJourneyOrigin()
  {
    // TSI 59 is Journey origin
    checkGeoUseTSI59(_geoUseTSI59);
    _seasonRule_MarToApr.geoTblItemNo() = _geoUseTSI59;

    _pu12->travelSeg().front()->departureDT() = DateTime(2009, 3, 4, 10, 25, 0);
    _pu23->travelSeg().front()->departureDT() = DateTime(2009, 5, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(PASS,
                         _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu12, *_fu12));
    CPPUNIT_ASSERT_EQUAL(PASS,
                         _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu23, *_fu23));
  }

  void testValidateFU_WithNoFarePath_TwoPU_Fail_RestrictOnJourneyOrigin()
  {
    // TSI 59 is Journey origin
    checkGeoUseTSI59(_geoUseTSI59);
    _seasonRule_MarToApr.geoTblItemNo() = _geoUseTSI59;

    _pu12->travelSeg().front()->departureDT() = DateTime(2009, 2, 4, 10, 25, 0);
    _pu23->travelSeg().front()->departureDT() = DateTime(2009, 4, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(FAIL,
                         _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu12, *_fu12));
    CPPUNIT_ASSERT_EQUAL(FAIL,
                         _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu23, *_fu23));
  }

  void testValidateFU_WithFarePath_OnePU_Pass_RestrictOnJourneyOrigin()
  {
    // TSI 59 is Journey origin
    checkGeoUseTSI59(_geoUseTSI59);
    _seasonRule_MarToApr.geoTblItemNo() = _geoUseTSI59;

    FarePath fp;
    fp.itin() = &_itin;
    fp.pricingUnit().push_back(_pu12o_23i);

    _itin.travelSeg().front()->departureDT() = DateTime(2009, 3, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(PASS,
                         _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu12o_23i, *_fu12));
    CPPUNIT_ASSERT_EQUAL(PASS,
                         _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu12o_23i, *_fu23));
  }

  void testValidateFU_WithFarePath_OnePU_Fail_RestrictOnJourneyOrigin()
  {
    // TSI 59 is Journey origin
    checkGeoUseTSI59(_geoUseTSI59);
    _seasonRule_MarToApr.geoTblItemNo() = _geoUseTSI59;

    FarePath fp;
    fp.itin() = &_itin;
    fp.pricingUnit().push_back(_pu12o_23i);

    _itin.travelSeg().front()->departureDT() = DateTime(2009, 5, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(FAIL,
                         _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu12o_23i, *_fu12));
    CPPUNIT_ASSERT_EQUAL(FAIL,
                         _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu12o_23i, *_fu23));
  }

  void testValidateFU_WithFarePath_TwoPU_Pass_RestrictOnJourneyOrigin()
  {
    // TSI 59 is Journey origin
    checkGeoUseTSI59(_geoUseTSI59);
    _seasonRule_MarToApr.geoTblItemNo() = _geoUseTSI59;

    FarePath fp;
    fp.itin() = &_itin;
    fp.pricingUnit().push_back(_pu12);
    fp.pricingUnit().push_back(_pu23);

    _pu12->travelSeg().front()->departureDT() = DateTime(2009, 3, 4, 10, 25, 0);
    _pu23->travelSeg().front()->departureDT() = DateTime(2009, 5, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(PASS, _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu12, *_fu12));
    CPPUNIT_ASSERT_EQUAL(PASS, _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu23, *_fu23));
  }

  void testValidateFU_WithFarePath_TwoPU_Fail_RestrictOnJourneyOrigin()
  {
    // TSI 59 is Journey origin
    checkGeoUseTSI59(_geoUseTSI59);
    _seasonRule_MarToApr.geoTblItemNo() = _geoUseTSI59;

    FarePath fp;
    fp.itin() = &_itin;
    fp.pricingUnit().push_back(_pu12);
    fp.pricingUnit().push_back(_pu23);

    _pu12->travelSeg().front()->departureDT() = DateTime(2009, 2, 4, 10, 25, 0);
    _pu23->travelSeg().front()->departureDT() = DateTime(2009, 4, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(FAIL, _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu12, *_fu12));
    CPPUNIT_ASSERT_EQUAL(FAIL, _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu23, *_fu23));
  }

  void testValidateFU_WithNoFarePath_NoTSI_OnePU_PassPUOrigin()
  {
    _seasonRule_MarToApr.geoTblItemNo() = 0;

    _pu12o_23i->travelSeg().front()->departureDT() = DateTime(2009, 3, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(
        PASS, _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu12o_23i, *_fu12));
    CPPUNIT_ASSERT_EQUAL(
        PASS, _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu12o_23i, *_fu23));
  }

  void testValidateFU_WithNoFarePath_NoTSI_OnePU_FailPUOrigin()
  {
    _seasonRule_MarToApr.geoTblItemNo() = 0;

    _pu12o_23i->travelSeg().front()->departureDT() = DateTime(2009, 5, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(
        FAIL, _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu12o_23i, *_fu12));
    CPPUNIT_ASSERT_EQUAL(
        FAIL, _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu12o_23i, *_fu23));
  }

  void testValidateFU_WithNoFarePath_NoTSI_TwoPU_FirstPassSecondFail()
  {
    _seasonRule_MarToApr.geoTblItemNo() = 0;

    _pu12->travelSeg().front()->departureDT() = DateTime(2009, 3, 4, 10, 25, 0);
    _pu23->travelSeg().front()->departureDT() = DateTime(2009, 5, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(PASS,
                         _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu12, *_fu12));
    CPPUNIT_ASSERT_EQUAL(FAIL,
                         _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu23, *_fu23));
  }

  void testValidateFU_WithNoFarePath_NoTSI_TwoPU_FirstFailSecondPass()
  {
    _seasonRule_MarToApr.geoTblItemNo() = 0;

    _pu12->travelSeg().front()->departureDT() = DateTime(2009, 2, 4, 10, 25, 0);
    _pu23->travelSeg().front()->departureDT() = DateTime(2009, 4, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(FAIL,
                         _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu12, *_fu12));
    CPPUNIT_ASSERT_EQUAL(PASS,
                         _season.validate(_trx, &_seasonRule_MarToApr, &_itin, *_pu23, *_fu23));
  }

  void testValidateFU_WithFarePath_NoTSI_OnePU_PassPUOrigin()
  {
    _seasonRule_MarToApr.geoTblItemNo() = 0;

    FarePath fp;
    fp.itin() = &_itin;
    fp.pricingUnit().push_back(_pu12o_23i);

    _pu12o_23i->travelSeg().front()->departureDT() = DateTime(2009, 3, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(PASS,
                         _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu12o_23i, *_fu12));
    CPPUNIT_ASSERT_EQUAL(PASS,
                         _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu12o_23i, *_fu23));
  }

  void testValidateFU_WithFarePath_NoTSI_OnePU_FailPUOrigin()
  {
    _seasonRule_MarToApr.geoTblItemNo() = 0;

    FarePath fp;
    fp.itin() = &_itin;
    fp.pricingUnit().push_back(_pu12o_23i);

    _pu12o_23i->travelSeg().front()->departureDT() = DateTime(2009, 5, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(FAIL,
                         _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu12o_23i, *_fu12));
    CPPUNIT_ASSERT_EQUAL(FAIL,
                         _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu12o_23i, *_fu23));
  }

  void testValidateFU_WithFarePath_NoTSI_TwoPU_FirstPassSecondFail()
  {
    FarePath fp;
    fp.itin() = &_itin;
    fp.pricingUnit().push_back(_pu12);
    fp.pricingUnit().push_back(_pu23);

    _pu12->travelSeg().front()->departureDT() = DateTime(2009, 3, 4, 10, 25, 0);
    _pu23->travelSeg().front()->departureDT() = DateTime(2009, 5, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(PASS, _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu12, *_fu12));
    CPPUNIT_ASSERT_EQUAL(FAIL, _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu23, *_fu23));
  }

  void testValidateFU_WithFarePath_NoTSI_TwoPU_FirstFailSecondPass()
  {
    FarePath fp;
    fp.itin() = &_itin;
    fp.pricingUnit().push_back(_pu12);
    fp.pricingUnit().push_back(_pu23);

    _pu12->travelSeg().front()->departureDT() = DateTime(2009, 2, 4, 10, 25, 0);
    _pu23->travelSeg().front()->departureDT() = DateTime(2009, 4, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(FAIL, _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu12, *_fu12));
    CPPUNIT_ASSERT_EQUAL(PASS, _season.validate(_trx, &_seasonRule_MarToApr, fp, *_pu23, *_fu23));
  }

  void testValidatePTF_Softpass_GeoTblItemNo()
  {
    checkGeoUseTSI59(_geoUseTSI59);
    _seasonRule_MarToApr.geoTblItemNo() = _geoUseTSI59;
    _tvlSeg12->departureDT() = DateTime(2009, 3, 4, 10, 25, 0);
    _tvlSeg23->departureDT() = DateTime(2009, 3, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(SOFTPASS,
                         _season.validate(_trx, _itin, *_ptf12, &_seasonRule_MarToApr, *_fm12));
  }

  void testValidatePTF_Pass_NoGeoTblItemNo()
  {
    _seasonRule_MarToApr.geoTblItemNo() = 0;
    _tvlSeg12->departureDT() = DateTime(2009, 3, 4, 10, 25, 0);
    _tvlSeg23->departureDT() = DateTime(2009, 3, 4, 10, 25, 0);
    CPPUNIT_ASSERT_EQUAL(PASS,
                         _season.validate(_trx, _itin, *_ptf12, &_seasonRule_MarToApr, *_fm12));
  }

private:
  void checkGeoUseTSI59(int geoUseTSI59)
  {
    // if fail this test, need to querry database to get a good Geo
    const std::vector<tse::GeoRuleItem*>& geoRuleItems =
        _dataHandle->getGeoRuleItem("ATP", geoUseTSI59);

    CPPUNIT_ASSERT_EQUAL((size_t)(1), geoRuleItems.size());
    GeoRuleItem& info = *geoRuleItems.front();
    CPPUNIT_ASSERT_EQUAL(VendorCode("ATP"), info.vendor());
    CPPUNIT_ASSERT_EQUAL(59, (int)(info.tsi()));
    CPPUNIT_ASSERT_EQUAL(LOCTYPE_NONE, info.loc1().locType());
    CPPUNIT_ASSERT_EQUAL(LOCTYPE_NONE, info.loc2().locType());
  };

  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;

    GeoRuleItem* getGeo(int tsi, LocTypeCode lt, LocCode lc)
    {
      GeoRuleItem* ret = _memHandle.create<GeoRuleItem>();
      ret->tsi() = tsi;
      ret->loc1().locType() = lt;
      ret->loc1().loc() = lc;
      ret->vendor() = "ATP";
      return ret;
    }

    const std::vector<GeoRuleItem*>& getGeoRuleItem(const VendorCode& vendor, int itemno)
    {
      std::vector<GeoRuleItem*>* ret = _memHandle.create<std::vector<GeoRuleItem*> >();
      if (vendor == "ATP")
      {
        if (itemno == 3456)
        {
          ret->push_back(getGeo(59, ' ', ""));
          return *ret;
        }
      }
      return DataHandleMock::getGeoRuleItem(vendor, itemno);
    }

    const TSIInfo* getTSI(int key)
    {
      if (key == 59)
      {
        TSIInfo* ret = _memHandle.create<TSIInfo>();
        ret->tsi() = key;
        ret->geoRequired() = ' ';
        ret->geoNotType() = ' ';
        ret->geoOut() = ' ';
        ret->geoItinPart() = ' ';
        ret->geoCheck() = 'O';
        ret->loopDirection() = 'F';
        ret->loopOffset() = 0;
        ret->loopToSet() = 0;
        ret->loopMatch() = 'O';
        ret->scope() = 'J';
        ret->type() = 'O';
        return ret;
      }
      return DataHandleMock::getTSI(key);
    }
  };

  PricingTrx _trx;
  Itin _itin;
  Agent _tktAgent;

  PricingRequest _request;
  PricingOptions _options;
  DataHandle* _dataHandle;
  LocCode _loc1, _loc2, _loc3;
  TravelSeg* _tvlSeg12, *_tvlSeg23;

  FareMarket* _fm12, *_fm23;
  PaxTypeFare* _ptf12, *_ptf23;
  FareUsage* _fu12, *_fu23;
  PricingUnit* _pu12o_23i, *_pu12, *_pu23;

  SeasonalAppl _seasonRule_MarToApr;
  SeasonalApplication _season;
  int _geoUseTSI59;
  TestMemHandle _memHandle;
  RootLoggerGetOff _logger;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SeasonalApplicationTest);

} // tse
