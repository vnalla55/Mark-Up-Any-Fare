#include "test/include/CppUnitHelperMacros.h"

#include "Common/DateTime.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/DayTimeAppInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "Rules/DayTimeApplication.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestPaxTypeFactory.h"

namespace tse
{

FALLBACKVALUE_DECL(apo45023ApplyCat2DefaultsInOOJPU);
using boost::gregorian::date;
using boost::posix_time::ptime;
using boost::posix_time::hours;
using boost::posix_time::minutes;

class DayTimeApplicationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DayTimeApplicationTest);
  CPPUNIT_TEST(testDOWoccurrence_softpassFCValidation);
  CPPUNIT_TEST(testDOWoccurrence_passPUValidWithGoodOccurredDOW);
  CPPUNIT_TEST(testDOWoccurrence_failPUValidWithBadOccurredDOW);
  CPPUNIT_TEST(testDOWoccurrence_failPUValidWithGoodOccurredDOWBadTODOnFirstFU_RuleOnFirstFU);
  CPPUNIT_TEST(testDOWoccurrence_passPUValidWithGoodOccurredDOWBadTODOnFirstFU_RuleOnSecondFU);
  CPPUNIT_TEST(testDOWoccurrence_failPUValidWithGoodOccurredDOWBadTODOnSecondFU_RuleOnSecondFU);
  CPPUNIT_TEST(testDOWoccurrence_passPUValidWithGoodOccurredDOWBadTODOnSecondFU_RuleOnFirstFU);
  CPPUNIT_TEST(testDOWoccurrence_passPUValidWithGoodOccurredDOWGoodTODOnAllFU);
  CPPUNIT_TEST(testDOWoccurrence_passPUValidWhenFirstFUWithOpenDate);
  CPPUNIT_TEST(testDOWoccurrence_passPUValidWhenFirstFUWithOpenTime);
  CPPUNIT_TEST(testDOWoccurrence_passPUValidWhenSecondFUWithOpenDate);
  CPPUNIT_TEST(testDOWoccurrence_passPUValidWhenSecondFUWithOpenTimePassDOW);
  CPPUNIT_TEST(testDOWoccurrence_failPUValidWhenSecondFUWithOpenTimeFailDOW);

  CPPUNIT_TEST(startStopSimple);
  CPPUNIT_TEST(startStopRange);
  CPPUNIT_TEST(startStopDaily);
  CPPUNIT_TEST(startStopSameDOW);
  CPPUNIT_TEST(startStopSameDOWtimes);
  CPPUNIT_TEST(startStopOpenSeg);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _loc0.loc() = "CHI";
    _loc1.loc() = "LON";

    _loc2.loc() = "LON";
    _loc3.loc() = "FAR";

    _airSeg1 = _memHandle.create<AirSeg>();
    _airSeg1->origin() = &_loc0;
    _airSeg1->destination() = &_loc1;

    _airSeg2 = _memHandle.create<AirSeg>();
    _airSeg2->origin() = &_loc2;
    _airSeg2->destination() = &_loc3;

    _fm1 = _memHandle.create<FareMarket>();
    _fm1->travelSeg().push_back(_airSeg1);
    _fm2 = _memHandle.create<FareMarket>();
    _fm2->travelSeg().push_back(_airSeg2);

    _fareInfo1 = _memHandle.create<FareInfo>();
    _fareInfo1->vendor() = "ATP";
    _fare1 = _memHandle.create<Fare>();
    _fare1->setFareInfo(_fareInfo1);

    _fareInfo2 = _memHandle.create<FareInfo>();
    _fareInfo2->vendor() = "ATP";
    _fare2 = _memHandle.create<Fare>();
    _fare2->setFareInfo(_fareInfo2);

    _ptf1 = _memHandle.create<PaxTypeFare>();
    _ptf1->setFare(_fare1);
    _ptf1->fareMarket() = _fm1;
    _ptf2 = _memHandle.create<PaxTypeFare>();
    _ptf2->setFare(_fare2);
    _ptf2->fareMarket() = _fm2;

    _fu1 = _memHandle.create<FareUsage>();
    _fu2 = _memHandle.create<FareUsage>();
    _fu1->paxTypeFare() = _ptf1;
    _fu1->travelSeg().push_back(_airSeg1);
    _fu2->paxTypeFare() = _ptf2;
    _fu2->travelSeg().push_back(_airSeg2);

    _puWithFU1FU2 = _memHandle.create<PricingUnit>();
    _puWithFU1FU2->fareUsage().push_back(_fu1);
    _puWithFU1FU2->fareUsage().push_back(_fu2);
    _puWithFU1FU2->travelSeg().push_back(_airSeg1);
    _puWithFU1FU2->travelSeg().push_back(_airSeg2);

    _itin.travelSeg().push_back(_airSeg1);
    _itin.travelSeg().push_back(_airSeg2);

    clearDayTimeAppInfo(_record3);
    _validator.initialize(&_record3);

    _dt1_Monday_0615 = DateTime(2004, 8, 9, 6 /*hour*/, 15, 0);
    _dt1_Monday_1015 = DateTime(2004, 8, 9, 10 /*hour*/, 15, 0);
    _dt1_Monday_noTime = DateTime(2004, 8, 9, 0 /*hour*/, 0, 0);
    _1st_Occur_Tue_0625_after_dt1_DOW123 = DateTime(2004, 8, 17, 6 /*hour*/, 15, 0);
    _2nd_Occur_Tue_0635_after_dt1_DOW123 = DateTime(2004, 8, 24, 6 /*hour*/, 15, 0);
    _2nd_Occur_Tue_0915_after_dt1_DOW123 = DateTime(2004, 8, 24, 9 /*hour*/, 15, 0);
    _2nd_Occur_Thu_0645_after_dt1_DOW123 = DateTime(2004, 8, 19, 6 /*hour*/, 15, 0);
    _2nd_Occur_Thu_noTime_after_dt1_DOW123 = DateTime(2004, 8, 19, 0 /*hour*/, 0, 0);
    _2nd_Occur_Mon_0615_after_dt1_DOW123 = DateTime(2004, 8, 23 /*Monday*/, 6 /*hour*/, 15, 0);
    _2nd_Occur_Mon_noTime_after_dt1_DOW123 = DateTime(2004, 8, 23, 0 /*hour*/, 0, 0);
    fallback::value::apo45023ApplyCat2DefaultsInOOJPU.set(true);
  }

  void tearDown() { _memHandle.clear(); }

  void clearDayTimeAppInfo(DayTimeAppInfo& record3)
  {
    record3.dayTimeAppl() = ' ';
    record3.dayTimeNeg() = ' ';
    record3.dowSame() = ' ';
    record3.todAppl() = ' ';
    record3.geoTblItemNo() = 0;
    record3.dowOccur() = 0;
  }

  void testDOWoccurrence_softpassFCValidation()
  {
    _airSeg1->departureDT() = ptime(date(2004, 1, 20), hours(7) + minutes(0));

    setUpRuleDOW123Occu2(_record3);
    // can not validate DOWoccurrence at FM scope, should expect SOFTPASS
    CPPUNIT_ASSERT_EQUAL(SOFTPASS, _validator.validate(_trx, _itin, *_ptf1, &_record3, *_fm1));
  }

  void testDOWoccurrence_passPUValidWithGoodOccurredDOW()
  {
    setUpRuleDOW123Occu2(_record3);

    _airSeg1->departureDT() = _dt1_Monday_0615;
    _airSeg2->departureDT() = _2nd_Occur_Tue_0635_after_dt1_DOW123;

    CPPUNIT_ASSERT_EQUAL(PASS, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu1));
  }

  void testDOWoccurrence_failPUValidWithBadOccurredDOW()
  {
    setUpRuleDOW123Occu2(_record3);

    _airSeg1->departureDT() = _dt1_Monday_0615;
    _airSeg2->departureDT() = _1st_Occur_Tue_0625_after_dt1_DOW123;

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu1));

    _airSeg2->departureDT() = _2nd_Occur_Thu_0645_after_dt1_DOW123;

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu1));
  }

  void testDOWoccurrence_failPUValidWithGoodOccurredDOWBadTODOnFirstFU_RuleOnFirstFU()
  {
    setUpRuleDOW123Occu2TOD8To18(_record3);

    _airSeg1->departureDT() = _dt1_Monday_0615;
    _airSeg2->departureDT() = _2nd_Occur_Tue_0915_after_dt1_DOW123;

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu1));

    _record3.dayTimeAppl() = 'X'; // pu scope, always against origin of PU, even FU is not
    CPPUNIT_ASSERT_EQUAL(FAIL, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu2));
  }

  void testDOWoccurrence_passPUValidWithGoodOccurredDOWBadTODOnFirstFU_RuleOnSecondFU()
  {
    setUpRuleDOW123Occu2TOD8To18(_record3);

    _airSeg1->departureDT() = _dt1_Monday_0615;
    _airSeg2->departureDT() = _2nd_Occur_Tue_0915_after_dt1_DOW123;

    CPPUNIT_ASSERT_EQUAL(PASS, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu2));
  }

  void testDOWoccurrence_passPUValidWithGoodOccurredDOWBadTODOnSecondFU_RuleOnFirstFU()
  {
    setUpRuleDOW123Occu2TOD8To18(_record3);

    _airSeg1->departureDT() = _dt1_Monday_1015;
    _airSeg2->departureDT() = _2nd_Occur_Mon_0615_after_dt1_DOW123;

    CPPUNIT_ASSERT_EQUAL(PASS, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu1));

    _record3.dayTimeAppl() = 'X'; // pu scope, always against origin of PU, even FU is not
    CPPUNIT_ASSERT_EQUAL(PASS, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu2));
  }

  void testDOWoccurrence_failPUValidWithGoodOccurredDOWBadTODOnSecondFU_RuleOnSecondFU()
  {
    setUpRuleDOW123Occu2TOD8To18(_record3);

    _airSeg1->departureDT() = _dt1_Monday_1015;
    _airSeg2->departureDT() = _2nd_Occur_Mon_0615_after_dt1_DOW123;

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu2));
  }

  void testDOWoccurrence_passPUValidWithGoodOccurredDOWGoodTODOnAllFU()
  {
    setUpRuleDOW123Occu2TOD8To18(_record3);

    _airSeg1->departureDT() = _dt1_Monday_1015;
    _airSeg2->departureDT() = _2nd_Occur_Tue_0915_after_dt1_DOW123;

    CPPUNIT_ASSERT_EQUAL(PASS, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu1));
  }

  void testDOWoccurrence_passPUValidWhenFirstFUWithOpenDate()
  {
    setUpRuleDOW123Occu2TOD8To18(_record3);
    _airSeg1->departureDT() = DateTime::openDate();
    setAirSegAsOpenDateTime(*_airSeg1);

    _airSeg2->departureDT() = _2nd_Occur_Tue_0915_after_dt1_DOW123;

    CPPUNIT_ASSERT_EQUAL(PASS, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu1));
  }

  void testDOWoccurrence_passPUValidWhenFirstFUWithOpenTime()
  {
    setUpRuleDOW123Occu2TOD8To18(_record3);

    _airSeg1->departureDT() = _dt1_Monday_noTime;
    setAirSegAsOpenTime(*_airSeg1);
    _airSeg2->departureDT() = _2nd_Occur_Tue_0915_after_dt1_DOW123;

    CPPUNIT_ASSERT_EQUAL(PASS, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu1));
  }

  void testDOWoccurrence_passPUValidWhenSecondFUWithOpenDate()
  {
    setUpRuleDOW123Occu2TOD8To18(_record3);

    _airSeg1->departureDT() = _dt1_Monday_0615;
    _airSeg2->departureDT() = _2nd_Occur_Mon_noTime_after_dt1_DOW123;
    setAirSegAsOpenDateTime(*_airSeg2);

    CPPUNIT_ASSERT_EQUAL(PASS, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu2));
  }

  void testDOWoccurrence_passPUValidWhenSecondFUWithOpenTimePassDOW()
  {
    setUpRuleDOW123Occu2TOD8To18(_record3);

    _airSeg1->departureDT() = _dt1_Monday_1015;
    _airSeg2->departureDT() = _2nd_Occur_Mon_noTime_after_dt1_DOW123;
    setAirSegAsOpenTime(*_airSeg2);

    CPPUNIT_ASSERT_EQUAL(PASS, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu1));
  }

  void testDOWoccurrence_failPUValidWhenSecondFUWithOpenTimeFailDOW()
  {
    setUpRuleDOW123Occu2TOD8To18(_record3);

    _airSeg1->departureDT() = _dt1_Monday_1015;
    _airSeg2->departureDT() = _2nd_Occur_Thu_noTime_after_dt1_DOW123;
    setAirSegAsOpenTime(*_airSeg2);

    CPPUNIT_ASSERT_EQUAL(FAIL, _validator.validate(_trx, &_record3, 0, *_puWithFU1FU2, *_fu1));
  }

  void setUpRuleDOW123Occu2(DayTimeAppInfo& record3)
  {
    record3.todAppl() = 'R';
    record3.dow() = "123";
    record3.dowOccur() = 2;
    record3.dayTimeAppl() = ' ';
    record3.startTime() = 0;
    record3.stopTime() = 0;
  }

  void setUpRuleDOW123Occu2TOD8To18(DayTimeAppInfo& record3)
  {
    setUpRuleDOW123Occu2(record3);
    record3.startTime() = 8 * 60; // 8:00
    record3.stopTime() = 18 * 60; // 18:00
  }

  void setAirSegAsOpenDateTime(AirSeg& airSeg)
  {
    airSeg.segmentType() = Open;
    airSeg.openSegAfterDatedSeg() = true;
  }

  void setAirSegAsOpenTime(AirSeg& airSeg)
  {
    airSeg.segmentType() = Open;
    airSeg.openSegAfterDatedSeg() = false;
  }

  void startStopClearDayTimeAppInfo(DayTimeAppInfo& record3)
  {
    record3.dayTimeAppl() = ' ';
    record3.dayTimeNeg() = ' ';
    record3.dowSame() = ' ';
    record3.todAppl() = ' ';
    record3.geoTblItemNo() = 0;
    record3.dowOccur() = 0;
  }

  void startStopSimple()
  {
    // Data common for all test cases
    DayTimeAppInfo record3;
    clearDayTimeAppInfo(record3);
    record3.startTime() = 6 * 60;
    record3.stopTime() = 7 * 60;

    PricingTrx trx;

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->vendor() = "ATP";

    {
      // Required data items
      Fare fare;
      fare.setFareInfo(fareInfo);
      std::vector<TravelSeg*> tvlSegs;
      AirSeg airSeg0;
      airSeg0.departureDT() = ptime(date(2004, 6, 8), hours(6) + minutes(0));

      tvlSegs.push_back(&airSeg0);
      AirSeg airSeg1;
      airSeg1.departureDT() = ptime(date(2004, 6, 8), hours(8) + minutes(0));

      tvlSegs.push_back(&airSeg1);

      FareMarket fm;
      fm.travelSeg() = tvlSegs;
      PaxType* paxTypeAdult = TestPaxTypeFactory::create("data/PaxTypeAdult.xml");
      PaxTypeFare ptf;
      ptf.initialize(&fare, paxTypeAdult, &fm);

      DayTimeApplication validator;
      validator.initialize(&record3);

      // -----------------------
      // Test call
      // -----------------------
      Itin itin; // not really used
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == PASS);
    }
  }

  void startStopRange()
  {
    // Data common for all test cases
    PricingTrx trx;

    DayTimeAppInfo record3;
    clearDayTimeAppInfo(record3);
    record3.startTime() = 8 * 60;
    record3.stopTime() = 11 * 60;
    record3.geoTblItemNo() = 0;
    record3.todAppl() = 'R';
    record3.dow() = "1234";

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->vendor() = "ATP";
    Fare fare;
    fare.setFareInfo(fareInfo);
    {
      // Required data items
      Loc loc0;
      loc0.loc() = "FAR";

      Loc loc1;
      loc1.loc() = "LON";

      std::vector<TravelSeg*> tvlSegs;
      AirSeg airSeg0;
      airSeg0.departureDT() = ptime(date(2004, 6, 8), hours(14) + minutes(0));
      airSeg0.origin() = &loc0;
      airSeg0.destination() = &loc1;

      tvlSegs.push_back(&airSeg0);

      FareMarket fm;
      fm.travelSeg() = tvlSegs;
      PaxType* paxTypeAdult = TestPaxTypeFactory::create("data/PaxTypeAdult.xml");
      PaxTypeFare ptf;
      ptf.initialize(&fare, paxTypeAdult, &fm);

      DayTimeApplication validator;
      validator.initialize(&record3);

      // -----------------------
      // Test call
      // -----------------------
      Itin itin; // not really used
      Record3ReturnTypes rtn = validator.validate(trx, itin, ptf, &record3, fm);
      CPPUNIT_ASSERT(rtn == PASS);
      record3.dayTimeNeg() = 'X';
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == FAIL);
      record3.dayTimeNeg() = ' ';
    }
    {
      // Required data items
      Loc loc2;
      loc2.loc() = "LON";

      Loc loc3;
      loc3.loc() = "FAR";

      std::vector<TravelSeg*> tvlSegs;
      AirSeg airSeg1;
      airSeg1.origin() = &loc2;
      airSeg1.destination() = &loc3;

      tvlSegs.push_back(&airSeg1);

      FareMarket fm;
      fm.travelSeg() = tvlSegs;
      PaxType* paxTypeAdult = TestPaxTypeFactory::create("data/PaxTypeAdult.xml");
      PaxTypeFare ptf;
      ptf.initialize(&fare, paxTypeAdult, &fm);

      DayTimeApplication validator;
      validator.initialize(&record3);

      // -----------------------
      // Test call
      // -----------------------
      Itin itin; // not really used
      record3.dayTimeNeg() = ' ';
      airSeg1.departureDT() = ptime(date(2004, 6, 7), hours(8) + minutes(0));
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == PASS);
      airSeg1.departureDT() = ptime(date(2004, 6, 10), hours(11) + minutes(0));
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == PASS);

      airSeg1.departureDT() = ptime(date(2004, 6, 7), hours(7) + minutes(59));
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == FAIL);
      airSeg1.departureDT() = ptime(date(2004, 6, 10), hours(11) + minutes(1));
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == FAIL);

      record3.dayTimeNeg() = 'X';
      airSeg1.departureDT() = ptime(date(2004, 6, 7), hours(8) + minutes(0));
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == FAIL);
      airSeg1.departureDT() = ptime(date(2004, 6, 10), hours(11) + minutes(0));
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == FAIL);

      airSeg1.departureDT() = ptime(date(2004, 6, 7), hours(7) + minutes(59));
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == PASS);
      airSeg1.departureDT() = ptime(date(2004, 6, 10), hours(11) + minutes(1));
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == PASS);

      record3.dayTimeNeg() = ' ';
    }
    {
      // Required data items
      Loc loc2;
      loc2.loc() = "LON";

      Loc loc3;
      loc3.loc() = "FAR";

      std::vector<TravelSeg*> tvlSegs;
      AirSeg airSeg1;
      airSeg1.departureDT() = ptime(date(2004, 6, 10), hours(14) + minutes(0));
      airSeg1.origin() = &loc2;
      airSeg1.destination() = &loc3;

      tvlSegs.push_back(&airSeg1);

      FareMarket fm;
      fm.travelSeg() = tvlSegs;
      PaxType* paxTypeAdult = TestPaxTypeFactory::create("data/PaxTypeAdult.xml");
      PaxTypeFare ptf;
      ptf.initialize(&fare, paxTypeAdult, &fm);

      DayTimeApplication validator;
      validator.initialize(&record3);

      // -----------------------
      // Test call
      // -----------------------
      Itin itin; // not really used
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == FAIL);
      record3.dayTimeNeg() = 'X';
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == PASS);
      record3.dayTimeNeg() = ' ';
    }
  }

  void startStopDaily()
  {
    // Data common for all test cases
    PricingTrx trx;

    DayTimeAppInfo record3;
    clearDayTimeAppInfo(record3);
    record3.startTime() = 8 * 60;
    record3.stopTime() = 11 * 60;
    record3.geoTblItemNo() = 0;
    record3.todAppl() = 'D';
    record3.dow() = "1234";

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->vendor() = "ATP";
    Fare fare;
    fare.setFareInfo(fareInfo);
    {
      // Required data items
      Loc loc0;
      loc0.loc() = "FAR";

      Loc loc1;
      loc1.loc() = "LON";

      std::vector<TravelSeg*> tvlSegs;
      AirSeg airSeg0;
      airSeg0.departureDT() = ptime(date(2004, 6, 8), hours(0) + minutes(0));
      airSeg0.origin() = &loc0;
      airSeg0.destination() = &loc1;
      airSeg0.segmentType() = Open;

      tvlSegs.push_back(&airSeg0);

      FareMarket fm;
      fm.travelSeg() = tvlSegs;
      PaxType* paxTypeAdult = TestPaxTypeFactory::create("data/PaxTypeAdult.xml");
      PaxTypeFare ptf;
      ptf.initialize(&fare, paxTypeAdult, &fm);

      DayTimeApplication validator;
      validator.initialize(&record3);

      // -----------------------
      // Test call for OpenTime
      // -----------------------
      Itin itin; // not really used
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == PASS);
      record3.dayTimeNeg() = 'X';
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == PASS);
      record3.dayTimeNeg() = ' ';
    }
    {
      // Required data items
      Loc loc0;
      loc0.loc() = "FAR";

      Loc loc1;
      loc1.loc() = "LON";

      std::vector<TravelSeg*> tvlSegs;
      AirSeg airSeg0;
      airSeg0.departureDT() = ptime(date(2004, 6, 8), hours(14) + minutes(0));
      airSeg0.origin() = &loc0;
      airSeg0.destination() = &loc1;

      tvlSegs.push_back(&airSeg0);

      FareMarket fm;
      fm.travelSeg() = tvlSegs;
      PaxType* paxTypeAdult = TestPaxTypeFactory::create("data/PaxTypeAdult.xml");
      PaxTypeFare ptf;
      ptf.initialize(&fare, paxTypeAdult, &fm);

      DayTimeApplication validator;
      validator.initialize(&record3);

      // -----------------------
      // Test call
      // -----------------------
      Itin itin; // not really used
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == FAIL);
      record3.dayTimeNeg() = 'X';
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == PASS);
      record3.dayTimeNeg() = ' ';
    }
    {
      // Required data items
      Loc loc2;
      loc2.loc() = "LON";

      Loc loc3;
      loc3.loc() = "FAR";

      std::vector<TravelSeg*> tvlSegs;
      AirSeg airSeg1;
      airSeg1.departureDT() = ptime(date(2004, 6, 10), hours(14) + minutes(0));
      airSeg1.origin() = &loc2;
      airSeg1.destination() = &loc3;

      tvlSegs.push_back(&airSeg1);

      FareMarket fm;
      fm.travelSeg() = tvlSegs;
      PaxType* paxTypeAdult = TestPaxTypeFactory::create("data/PaxTypeAdult.xml");
      PaxTypeFare ptf;
      ptf.initialize(&fare, paxTypeAdult, &fm);

      DayTimeApplication validator;
      validator.initialize(&record3);

      // -----------------------
      // Test call
      // -----------------------
      Itin itin; // not really used
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == FAIL);
      record3.dayTimeNeg() = 'X';
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == PASS);
      record3.dayTimeNeg() = ' ';
    }
    {
      // Required data items
      Loc loc4;
      loc4.loc() = "FAR";

      Loc loc5;
      loc5.loc() = "LON";

      std::vector<TravelSeg*> tvlSegs;
      AirSeg airSeg2;
      airSeg2.departureDT() = ptime(date(2004, 6, 7), hours(9) + minutes(0));
      airSeg2.origin() = &loc4;
      airSeg2.destination() = &loc5;

      tvlSegs.push_back(&airSeg2);

      FareMarket fm;
      fm.travelSeg() = tvlSegs;
      PaxType* paxTypeAdult = TestPaxTypeFactory::create("data/PaxTypeAdult.xml");
      PaxTypeFare ptf;
      ptf.initialize(&fare, paxTypeAdult, &fm);

      DayTimeApplication validator;
      validator.initialize(&record3);

      // -----------------------
      // Test call
      // -----------------------
      Itin itin; // not really used
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == PASS);
      record3.dayTimeNeg() = 'X';
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == FAIL);
      record3.dayTimeNeg() = ' ';
    }
  }

  void startStopSameDOW()
  {
    // Data common for all test cases
    PricingTrx trx;

    DayTimeAppInfo record3;
    clearDayTimeAppInfo(record3);
    //    record3.todAppl() = 'R';
    // cout << "  appl / neg  : " <<  record3.todAppl() << " / "  << record3.dayTimeNeg() << endl;
    record3.startTime() = 1;
    record3.stopTime() = 24 * 60;
    record3.todAppl() = 'D';
    record3.dowSame() = 'X';
    record3.dow() = "7123";
    record3.dayTimeAppl() = DayTimeApplication::subJourneyBased;

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->vendor() = "ATP";

    {
      // Required data items
      Fare fare;
      fare.setFareInfo(fareInfo);

      Loc loc0;
      loc0.loc() = "MSP";

      Loc loc1;
      loc1.loc() = "LON";

      std::vector<TravelSeg*> tvlSegs;
      AirSeg airSeg0;
      airSeg0.departureDT() = ptime(date(2004, 8, 9), hours(14) + minutes(0));
      airSeg0.origin() = &loc0;
      airSeg0.destination() = &loc1;

      tvlSegs.push_back(&airSeg0);

      FareUsage fu0;
      fu0.travelSeg().swap(tvlSegs);

      PricingUnit pu0;
      pu0.fareUsage().push_back(&fu0);
      pu0.travelSeg().push_back(&airSeg0);

      DayTimeApplication validator;
      validator.initialize(&record3);

      // -----------------------
      // Test call, only one FareUsage, should pass
      // -----------------------
      FarePath fp;
      PaxType* paxTypeAdult = TestPaxTypeFactory::create("data/PaxTypeAdult.xml");
      FareMarket fm;
      PaxTypeFare ptf;
      ptf.initialize(&fare, paxTypeAdult, &fm);
      fu0.paxTypeFare() = &ptf;
      CPPUNIT_ASSERT(validator.validate(trx, &record3, &fp, pu0, fu0) == PASS);

      // Required data items

      Loc loc2;
      loc2.loc() = "LON";

      Loc loc3;
      loc3.loc() = "MSP";

      std::vector<TravelSeg*> tvlSegs1;
      AirSeg airSeg1;
      airSeg1.departureDT() = ptime(date(2004, 8, 17), hours(14) + minutes(0));
      airSeg1.origin() = &loc2;
      airSeg1.destination() = &loc3;

      tvlSegs1.push_back(&airSeg1);

      FareUsage fu1;
      fu1.travelSeg().swap(tvlSegs1);

      pu0.fareUsage().push_back(&fu1);
      pu0.travelSeg().push_back(&airSeg1);

      // -----------------------
      // Test call
      // -----------------------
      CPPUNIT_ASSERT(validator.validate(trx, &record3, &fp, pu0, fu0) == FAIL);

      // ----------------------
      // One of the two FareUsages travels on OpenDate, should pass
      // ----------------------
      airSeg1.departureDT() = DateTime::openDate();
      airSeg1.segmentType() = Open;
      CPPUNIT_ASSERT(validator.validate(trx, &record3, &fp, pu0, fu0) == PASS);

      // -----------------------
      // Have another FareUsage travel on different DOW, should fail
      AirSeg airSeg2;
      airSeg2.departureDT() = ptime(date(2004, 8, 17), hours(14) + minutes(0));
      airSeg2.origin() = &loc3;
      airSeg2.destination() = &loc1;
      airSeg2.segmentType() = Air;

      std::vector<TravelSeg*> tvlSegs2;
      tvlSegs2.push_back(&airSeg2);

      FareUsage fu2;
      fu2.travelSeg().swap(tvlSegs2);

      pu0.fareUsage().push_back(&fu2);
      pu0.travelSeg().push_back(&airSeg2);
      CPPUNIT_ASSERT(validator.validate(trx, &record3, &fp, pu0, fu0) == FAIL);

      // -----------------------
      // Change to travel on same DOW, should PASS
      airSeg2.departureDT() = airSeg0.departureDT();
      CPPUNIT_ASSERT(validator.validate(trx, &record3, &fp, pu0, fu0) == PASS);
    }
  }

  void startStopSameDOWtimes()
  {
    // Data common for all test cases
    PricingTrx trx;

    DayTimeAppInfo record3;
    clearDayTimeAppInfo(record3);
    record3.dowSame() = 'X';
    record3.startTime() = 6 * 60;
    record3.stopTime() = 7 * 60;
    record3.dayTimeAppl() = DayTimeApplication::subJourneyBased;

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->vendor() = "ATP";

    Fare fare;
    fare.setFareInfo(fareInfo);

    PaxType* paxTypeAdult = TestPaxTypeFactory::create("data/PaxTypeAdult.xml");
    FareMarket fm;
    PaxTypeFare ptf;
    ptf.initialize(&fare, paxTypeAdult, &fm);

    {
      // Required data items
      Loc loc0;
      loc0.loc() = "CHI";

      Loc loc1;
      loc1.loc() = "LON";

      std::vector<TravelSeg*> tvlSegs;
      AirSeg airSeg0;
      airSeg0.departureDT() = ptime(date(2004, 8, 9), hours(6) + minutes(15));
      airSeg0.origin() = &loc0;
      airSeg0.destination() = &loc1;

      tvlSegs.push_back(&airSeg0);

      FareUsage fu0;
      fu0.travelSeg().swap(tvlSegs);
      fu0.paxTypeFare() = &ptf;

      PricingUnit pu0;
      pu0.fareUsage().push_back(&fu0);
      pu0.travelSeg().push_back(&airSeg0);

      DayTimeApplication validator;
      validator.initialize(&record3);

      // -----------------------
      // Test call
      // -----------------------
      FarePath fp;
      CPPUNIT_ASSERT(validator.validate(trx, &record3, &fp, pu0, fu0) == PASS);
    }
    {
      // Required data items
      Loc loc2;
      loc2.loc() = "LON";

      Loc loc3;
      loc3.loc() = "CHI";

      std::vector<TravelSeg*> tvlSegs;
      AirSeg airSeg1;
      airSeg1.departureDT() = ptime(date(2004, 8, 9), hours(7) + minutes(0));
      airSeg1.origin() = &loc2;
      airSeg1.destination() = &loc3;

      tvlSegs.push_back(&airSeg1);

      FareMarket fm;
      fm.travelSeg() = tvlSegs;

      DayTimeApplication validator;
      validator.initialize(&record3);

      // -----------------------
      // Test call
      // -----------------------
      record3.dayTimeAppl() = ' ';

      // -------------------------------------
      // dayTimeAppl is still subJourneyBased
      // -------------------------------------
      Itin itin; // not really used
      CPPUNIT_ASSERT(validator.validate(trx, itin, ptf, &record3, fm) == SOFTPASS);
    }
  }

  void startStopOpenSeg()
  {
    // Data common for all test cases
    PricingTrx trx;

    DayTimeAppInfo record3;
    clearDayTimeAppInfo(record3);
    record3.startTime() = 8 * 60;
    record3.stopTime() = 11 * 60;
    record3.todAppl() = 'R';
    record3.dow() = "12";
    record3.dowOccur() = 2;

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->vendor() = "ATP";

    {
      // Required data items
      Fare fare;
      fare.setFareInfo(fareInfo);

      Loc loc0;
      loc0.loc() = "FAR";

      Loc loc1;
      loc1.loc() = "LON";

      std::vector<TravelSeg*> tvlSegs;
      AirSeg airSeg0;
      airSeg0.departureDT() = ptime(date(2005, 4, 5), hours(6) + minutes(15));
      airSeg0.origin() = &loc0;
      airSeg0.destination() = &loc1;
      airSeg0.openSegAfterDatedSeg() = false;

      tvlSegs.push_back(&airSeg0);

      AirSeg airSeg1;
      airSeg1.departureDT() = ptime(date(2005, 4, 6), hours(6) + minutes(15));
      airSeg1.origin() = &loc1;
      airSeg1.destination() = &loc0;
      airSeg1.openSegAfterDatedSeg() = false;

      tvlSegs.push_back(&airSeg1);

      FareUsage fu0;
      fu0.travelSeg().swap(tvlSegs);

      PricingUnit pu0;
      pu0.fareUsage().push_back(&fu0);
      pu0.travelSeg().push_back(&airSeg0);
      pu0.travelSeg().push_back(&airSeg1);

      DayTimeApplication validator;
      validator.initialize(&record3);

      // -----------------------
      // Test call
      // -----------------------
      FarePath fp;
      PaxType* paxTypeAdult = TestPaxTypeFactory::create("data/PaxTypeAdult.xml");
      FareMarket fm;
      fm.travelSeg().push_back(&airSeg1);
      PaxTypeFare ptf;
      ptf.initialize(&fare, paxTypeAdult, &fm);
      fu0.paxTypeFare() = &ptf;
      CPPUNIT_ASSERT(validator.validate(trx, &record3, &fp, pu0, fu0) == FAIL);

      airSeg1.openSegAfterDatedSeg() = true;
      CPPUNIT_ASSERT(validator.validate(trx, &record3, &fp, pu0, fu0) == PASS);
    }
  }

private:
  DayTimeApplication _validator;
  DayTimeAppInfo _record3;
  PricingTrx _trx;
  Itin _itin;
  Loc _loc0, _loc1, _loc2, _loc3;
  AirSeg* _airSeg1, *_airSeg2;
  FareMarket* _fm1, *_fm2;
  FareInfo* _fareInfo1, *_fareInfo2;
  Fare* _fare1, *_fare2;
  PaxTypeFare* _ptf1, *_ptf2;
  FareUsage* _fu1, *_fu2;
  PricingUnit* _puWithFU1FU2;
  DateTime _dt1_Monday_0615;
  DateTime _dt1_Monday_1015;
  DateTime _dt1_Monday_noTime;
  DateTime _1st_Occur_Tue_0625_after_dt1_DOW123;
  DateTime _2nd_Occur_Tue_0635_after_dt1_DOW123;
  DateTime _2nd_Occur_Tue_0915_after_dt1_DOW123;
  DateTime _2nd_Occur_Thu_0645_after_dt1_DOW123;
  DateTime _2nd_Occur_Thu_noTime_after_dt1_DOW123;
  DateTime _2nd_Occur_Mon_0615_after_dt1_DOW123;
  DateTime _2nd_Occur_Mon_noTime_after_dt1_DOW123;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DayTimeApplicationTest);
}
