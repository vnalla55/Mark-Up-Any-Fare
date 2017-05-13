//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/AccompaniedTravelInfo.h"
#include "Rules/AccompaniedTravel.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/DiscountSegInfo.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/Fare.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "FareCalc/FcMessage.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Common/Vendor.h"
#include "Common/AccTvlDetailOut.h"
#include "Common/AccTvlDetailIn.h"

#include "test/testdata/TestFactoryManager.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestFareUsageFactory.h"
#include "test/testdata/TestPaxTypeFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockGlobal.h"

namespace tse
{

class AccompaniedTravelTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AccompaniedTravelTest);
  CPPUNIT_TEST(testFareMarket);
  CPPUNIT_TEST(testDiscountFare);
  CPPUNIT_TEST(testFareUsage1);
  CPPUNIT_TEST(testFareUsage2);
  CPPUNIT_SKIP_TEST(testWPA);
  CPPUNIT_TEST(testWPN);
  CPPUNIT_TEST(testFareUsageSameRule);
  CPPUNIT_TEST(testCheckFareClassBkgCodeReturnTrueIfAccompaniedTravelInfoEmpty);
  CPPUNIT_TEST(testCheckFareClassBkgCodeReturnTrueIfFareClassIndBlank);
  CPPUNIT_TEST(testCheckFareClassBkgCodeReturnFalseIfFarePointerZero);
  CPPUNIT_TEST(testCheckFareClassBkgCodeReturnTrueIfFareClassMatches);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  ConfigMan* _configMan;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _configMan = _memHandle.create<ConfigMan>();
  }

  void tearDown() { _memHandle.clear(); }

  void testFareMarket()
  {
    PricingTrx trx;

    trx.diagnostic().diagnosticType() = Diagnostic313;
    trx.diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic313);

    PaxType* paxTypeAdult = TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypeAdult.xml");
    CPPUNIT_ASSERT(paxTypeAdult != 0);
    PaxType* paxTypeChild = TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypeChild.xml");
    CPPUNIT_ASSERT(paxTypeChild != 0);
    AirSeg* airSeg0 = TestAirSegFactory::create("AccompaniedTravelData/AirSegDFWJFK.xml");
    CPPUNIT_ASSERT(airSeg0 != 0);
    AirSeg* airSeg1 = TestAirSegFactory::create("AccompaniedTravelData/AirSegJFKDFW.xml");
    CPPUNIT_ASSERT(airSeg1 != 0);
    PaxTypeFare* ptfChild =
        TestPaxTypeFareFactory::create("AccompaniedTravelData/PaxTypeFare1.xml");

    FareMarket* fm = TestFareMarketFactory::create("AccompaniedTravelData/FareMarket.xml");
    CPPUNIT_ASSERT(fm != 0);

    trx.travelSeg().push_back(airSeg0);
    trx.travelSeg().push_back(airSeg1);
    trx.fareMarket().push_back(fm);
    trx.paxType().push_back(paxTypeAdult);
    trx.paxType().push_back(paxTypeChild);

    // Create the itin
    Itin itin;

    itin.travelSeg().push_back(airSeg0);
    itin.travelSeg().push_back(airSeg1);
    itin.fareMarket().push_back(fm);

    trx.itin().push_back(&itin);

    // Rule Item Info
    AccompaniedTravel accTvl;
    AccompaniedTravelInfo accTvlInfo;

    accTvlInfo.vendor() = "ATP";
    accTvlInfo.unavailTag() = ' ';
    accTvlInfo.accPsgAppl() = ' '; // MUST
    accTvlInfo.accPsgType() = ADULT;
    accTvlInfo.accPsgId() = 'X';
    accTvlInfo.minAge() = 25;
    accTvlInfo.maxAge() = 65;
    accTvlInfo.minNoPsg() = 1;
    accTvlInfo.maxNoPsg() = 2;
    accTvlInfo.accTvlAllSectors() = 'X';
    accTvlInfo.accTvlOut() = ' ';
    accTvlInfo.accTvlOneSector() = ' ';
    accTvlInfo.accTvlSameCpmt() = 'X';
    accTvlInfo.accTvlSameRule() = 'X';
    accTvlInfo.geoTblItemNoVia1() = 0;
    accTvlInfo.geoTblItemNoVia2() = 0;

    // Array of Fare Class/Generic
    const FareClassCode fcArray1[] = { "F", "J1FL", "REUNBAO", "YFLBAO" };
    const int fcArraySize1 = sizeof(fcArray1) / sizeof(fcArray1[0]);

    std::vector<FareClassCode> fc1(fcArray1, fcArray1 + fcArraySize1);
    accTvlInfo.fareClassBkgCds() = fc1;
    accTvlInfo.fareClassBkgCdInd() = accTvl.fareClass;

    //--------------------------------------------------------
    // Should PASS, MUST ADT
    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validate(trx, itin, *ptfChild, &accTvlInfo, *fm));
    CPPUNIT_ASSERT(ret == PASS);

    //--------------------------------------------------------
    // Should PASS
    accTvlInfo.accPsgAppl() = 'X'; // MUST NOT
    accTvlInfo.accPsgType() = "MIL";

    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validate(trx, itin, *ptfChild, &accTvlInfo, *fm));
    CPPUNIT_ASSERT(ret == PASS);

    //--------------------------------------------------------
    // Should FAIL
    accTvlInfo.accPsgAppl() = 'X'; // MUST NOT
    accTvlInfo.accPsgType() = "ADT";

    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validate(trx, itin, *ptfChild, &accTvlInfo, *fm));
    CPPUNIT_ASSERT(ret != PASS);

    //--------------------------------------------------------
    // Should FAIL
    accTvlInfo.accPsgAppl() = ' '; // MUST
    accTvlInfo.accPsgType() = "MIL";

    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validate(trx, itin, *ptfChild, &accTvlInfo, *fm));
    CPPUNIT_ASSERT(ret != PASS);

    diag.flushMsg();
  }

  void testFareUsage1()
  {
    PricingTrx trx;
    trx.diagnostic().diagnosticType() = Diagnostic313;
    trx.diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic313);

    PaxType* paxTypeAdult = TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypeAdult.xml");
    CPPUNIT_ASSERT(paxTypeAdult != 0);
    PaxType* paxTypeChild = TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypeChild.xml");
    CPPUNIT_ASSERT(paxTypeChild != 0);
    AirSeg* airSeg0 = TestAirSegFactory::create("AccompaniedTravelData/AirSegDFWJFK.xml");
    CPPUNIT_ASSERT(airSeg0 != 0);
    AirSeg* airSeg1 = TestAirSegFactory::create("AccompaniedTravelData/AirSegJFKDFW.xml");
    CPPUNIT_ASSERT(airSeg1 != 0);

    FareUsage* fu0 = TestFareUsageFactory::create("AccompaniedTravelData/FareUsage0.xml");
    FareUsage* fu1 = TestFareUsageFactory::create("AccompaniedTravelData/FareUsage1.xml");
    // FareMarket* fareMarket = TestFareMarketFactory::create(
    // "AccompaniedTravelData/fareMarket.xml");

    CPPUNIT_ASSERT(fu0 != 0);
    CPPUNIT_ASSERT(fu1 != 0);

    trx.travelSeg().push_back(airSeg0);
    trx.travelSeg().push_back(airSeg1);
    // trx.fareMarket().push_back(fareMarket);
    trx.paxType().push_back(paxTypeAdult);
    trx.paxType().push_back(paxTypeChild);

    // Rule Item Info
    AccompaniedTravel accTvl;
    AccompaniedTravelInfo accTvlInfo;

    accTvlInfo.vendor() = "ATP";
    accTvlInfo.unavailTag() = ' ';
    accTvlInfo.accPsgAppl() = ' '; // MUST
    accTvlInfo.accPsgType() = ADULT;
    accTvlInfo.accPsgId() = 'X';
    accTvlInfo.minAge() = 25;
    accTvlInfo.maxAge() = 65;
    accTvlInfo.minNoPsg() = 1;
    accTvlInfo.maxNoPsg() = 2;
    accTvlInfo.accTvlAllSectors() = 'X';
    accTvlInfo.accTvlOut() = ' ';
    accTvlInfo.accTvlOneSector() = ' ';
    accTvlInfo.accTvlSameCpmt() = 'X';
    accTvlInfo.accTvlSameRule() = ' ';
    accTvlInfo.geoTblItemNoVia1() = 0;
    accTvlInfo.geoTblItemNoVia2() = 0;

    // Array of Fare Class/Generic
    const FareClassCode fcArray1[] = { "F", "J1FL", "REUNBAO", "YFLBAO" };
    const int fcArraySize1 = sizeof(fcArray1) / sizeof(fcArray1[0]);

    std::vector<FareClassCode> fc1(fcArray1, fcArray1 + fcArraySize1);
    accTvlInfo.fareClassBkgCds() = fc1;
    accTvlInfo.fareClassBkgCdInd() = accTvl.fareClass;

    //--------------------------------------------------------

    std::vector<FareUsage*> fareUsages;
    fareUsages.push_back(fu1);
    fareUsages.push_back(fu0);

    fu1->paxTypeFare()->setAccSameFareBreak(true); // Revalidation during
    // combinability check

    PaxTypeFareRuleData* paxTypeFareRuleData = _memHandle.create<PaxTypeFareRuleData>();
    paxTypeFareRuleData->ruleItemInfo() = dynamic_cast<RuleItemInfo*>(&accTvlInfo);
    fu1->paxTypeFare()->setRuleData(
        RuleConst::ACCOMPANIED_PSG_RULE, trx.dataHandle(), paxTypeFareRuleData);

    //--------------------------------------------------------
    // Should fail because fareclass not match
    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validate(trx, fareUsages));
    CPPUNIT_ASSERT(ret != true);

    //--------------------------------------------------------
    // Should PASS
    accTvlInfo.accPsgAppl() = 'X'; // MUST NOT
    accTvlInfo.accPsgType() = "MIL";

    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validate(trx, fareUsages));
    CPPUNIT_ASSERT(ret == true);

    diag.flushMsg();
  }

  void testDiscountFare()
  {
    PricingTrx trx;
    trx.diagnostic().diagnosticType() = Diagnostic319;
    trx.diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic319);

    PaxType* paxTypeAdult = TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypeAdult.xml");
    PaxType* paxTypeChild = TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypeChild.xml");
    PaxTypeFare* paxTypeFareChild =
        TestPaxTypeFareFactory::create("AccompaniedTravelData/PaxTypeFare1.xml");
    CPPUNIT_ASSERT(paxTypeAdult != 0);
    CPPUNIT_ASSERT(paxTypeChild != 0);
    CPPUNIT_ASSERT(paxTypeFareChild != 0);

    trx.paxType().push_back(paxTypeAdult);
    trx.paxType().push_back(paxTypeChild);

    // Rule Item Info
    AccompaniedTravel accTvl;
    DiscountInfo discInfo;
    // Memory will be freed by DiscountInfo object
    DiscountSegInfo* discountSegInfo1 = new DiscountSegInfo();
    DiscountSegInfo& discSegInfo1 = *discountSegInfo1;

    discInfo.vendor() = "ATP";
    discInfo.unavailtag() = ' ';
    discInfo.accPsgAppl() = ' '; // MUST
    discInfo.paxType() = CHILD;
    discInfo.segCnt() = 1;
    discInfo.psgid() = 'X';
    discInfo.minAge() = 5;
    discInfo.maxAge() = 10;
    discInfo.accTvlAllSectors() = 'X';
    discInfo.accTvlOut() = ' ';
    discInfo.accTvlOneSector() = ' ';
    discInfo.accTvlSameCpmt() = 'X';
    discInfo.accTvlSameRule() = ' ';
    discInfo.fareClassBkgCodeInd() = accTvl.notApply;

    discSegInfo1.minNoPsg() = 2;
    discSegInfo1.maxNoPsg() = 1;
    discSegInfo1.minAge1() = 20;
    discSegInfo1.maxAge1() = 54;
    discSegInfo1.minAge2() = 55;
    discSegInfo1.maxAge2() = 60;
    discSegInfo1.minAge3() = 0;
    discSegInfo1.maxAge3() = 0;
    discSegInfo1.accPsgType1() = ADULT;
    discSegInfo1.accPsgType2() = "";
    discSegInfo1.accPsgType3() = "";
    discSegInfo1.fareClass() = "";
    discSegInfo1.bookingCode() = "";

    discInfo.segs().push_back(&discSegInfo1);

    //--------------------------------------------------------
    // Should fail because not enough accompanying passenger
    // Should pass the count validation was removed bu SPR133695
    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validate(trx, *paxTypeChild, discInfo, *paxTypeFareChild));
    CPPUNIT_ASSERT(ret == PASS);

    //--------------------------------------------------------
    // Should PASS
    discInfo.accPsgAppl() = 'X'; // MUST NOT

    discSegInfo1.accPsgType1() = "SEX";

    // Memory will be freed by DiscountInfo object
    DiscountSegInfo* discountSegInfo2 = new DiscountSegInfo();
    DiscountSegInfo& discSegInfo2 = *discountSegInfo2;

    discSegInfo2.minNoPsg() = 1;
    discSegInfo2.maxNoPsg() = 1;
    discSegInfo2.minAge1() = 20;
    discSegInfo2.maxAge1() = 65;
    discSegInfo2.minAge2() = 0;
    discSegInfo2.maxAge2() = 0;
    discSegInfo2.minAge3() = 0;
    discSegInfo2.maxAge3() = 0;
    discSegInfo2.accPsgType1() = "MIL";
    discSegInfo2.accPsgType2() = "";
    discSegInfo2.accPsgType3() = "";
    discSegInfo2.fareClass() = "";
    discSegInfo2.bookingCode() = "";

    discInfo.segs().push_back(&discSegInfo2);

    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validate(trx, *paxTypeChild, discInfo, *paxTypeFareChild));
    CPPUNIT_ASSERT(ret == PASS);

    // INS should be able to use CNN discount fare
    PaxType* paxTypeIns =
        TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypeChildActualINS.xml");
    trx.paxType().clear();
    trx.paxType().push_back(paxTypeAdult);
    trx.paxType().push_back(paxTypeIns);
    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validate(trx, *paxTypeIns, discInfo, *paxTypeFareChild));
    CPPUNIT_ASSERT(ret == PASS);

    diag.flushMsg();
  }

  void testFareUsage2()
  {
    PricingTrx trx;
    trx.diagnostic().diagnosticType() = Diagnostic319;
    trx.diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic319);

    PaxType* paxTypeAdult = TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypeAdult.xml");
    PaxType* paxTypeChild = TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypeChild.xml");
    CPPUNIT_ASSERT(paxTypeAdult != 0);
    CPPUNIT_ASSERT(paxTypeChild != 0);

    trx.paxType().push_back(paxTypeAdult);
    trx.paxType().push_back(paxTypeChild);

    AirSeg* airSeg0 = TestAirSegFactory::create("AccompaniedTravelData/AirSegDFWJFK.xml");
    CPPUNIT_ASSERT(airSeg0 != 0);
    AirSeg* airSeg1 = TestAirSegFactory::create("AccompaniedTravelData/AirSegJFKDFW.xml");
    CPPUNIT_ASSERT(airSeg1 != 0);

    // We need FarePath and PricingUnit for GeoItemTbl validation, although
    // we do not declaim to support GeoItemTbl on accompanied travel yet

    FareUsage* fu0 = TestFareUsageFactory::create("AccompaniedTravelData/FareUsage2.xml");
    FareUsage* fu1 = TestFareUsageFactory::create("AccompaniedTravelData/FareUsage1.xml");
    FareMarket* fareMarket = TestFareMarketFactory::create("AccompaniedTravelData/FareMarket.xml");

    CPPUNIT_ASSERT(fu0 != 0);
    CPPUNIT_ASSERT(fu1 != 0);

    trx.travelSeg().push_back(airSeg0);
    trx.travelSeg().push_back(airSeg1);
    trx.fareMarket().push_back(fareMarket);
    trx.paxType().push_back(paxTypeAdult);
    trx.paxType().push_back(paxTypeChild);

    //--------------------------------------------------------
    std::vector<FareUsage*> fareUsages;
    fareUsages.push_back(fu1);
    fareUsages.push_back(fu0);

    fu1->paxTypeFare()->setAccSameFareBreak(true); // Revalidation during
    // combinability check
    fu1->paxTypeFare()->fareMarket() = fareMarket;

    // fu1->paxTypeFare()->setRuleData(RuleConst::ACCOMPANIED_PSG_RULE, trx.dataHandle(), 0);

    for(auto& a: *fu1->paxTypeFare()->paxTypeFareRuleDataMap())
      a = nullptr;

    // Rule Item Info
    AccompaniedTravel accTvl;
    DiscountInfo discInfo;
    // Memory will be freed by DiscountInfo object
    DiscountSegInfo* discountSegInfo1 = new DiscountSegInfo();
    DiscountSegInfo& discSegInfo1 = *discountSegInfo1;

    discInfo.vendor() = "ATP";
    discInfo.unavailtag() = ' ';
    discInfo.geoTblItemNo() = 0;
    discInfo.accPsgAppl() = ' '; // MUST
    discInfo.paxType() = CHILD;
    discInfo.segCnt() = 1;
    discInfo.psgid() = 'X';
    discInfo.minAge() = 5;
    discInfo.maxAge() = 10;
    discInfo.accTvlAllSectors() = 'X';
    discInfo.accTvlOut() = ' ';
    discInfo.accTvlOneSector() = ' ';
    discInfo.accTvlSameCpmt() = 'X';
    discInfo.accTvlSameRule() = ' ';
    discInfo.fareClassBkgCodeInd() = accTvl.notApply;

    discSegInfo1.minNoPsg() = 2;
    discSegInfo1.maxNoPsg() = 1;
    discSegInfo1.minAge1() = 20;
    discSegInfo1.maxAge1() = 54;
    discSegInfo1.minAge2() = 55;
    discSegInfo1.maxAge2() = 60;
    discSegInfo1.minAge3() = 0;
    discSegInfo1.maxAge3() = 0;
    discSegInfo1.accPsgType1() = ADULT;
    discSegInfo1.accPsgType2() = "";
    discSegInfo1.accPsgType3() = "";
    discSegInfo1.fareClass() = "";
    discSegInfo1.bookingCode() = "";

    discInfo.segs().push_back(&discSegInfo1);

    // Link discount info to PaxTypeFare
    PaxTypeFareRuleData* paxTypeFareRuleData = _memHandle.create<PaxTypeFareRuleData>();
    paxTypeFareRuleData->ruleItemInfo() = dynamic_cast<RuleItemInfo*>(&discInfo);
    fu1->paxTypeFare()->setRuleData(19, trx.dataHandle(), paxTypeFareRuleData);
    fu1->paxTypeFare()->status().set(PaxTypeFare::PTF_Discounted);

    //--------------------------------------------------------
    // Should fail because not correct air segment
    bool ret = true;
    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validate(trx, fareUsages));
    CPPUNIT_ASSERT(ret != true);

    //--------------------------------------------------------
    // Should PASS
    fareUsages.clear();
    fu0 = TestFareUsageFactory::create("AccompaniedTravelData/FareUsage0.xml");
    fareUsages.push_back(fu0);
    fareUsages.push_back(fu1);

    discInfo.accPsgAppl() = 'X'; // MUST NOT
    discSegInfo1.accPsgType1() = "SEX";

    // Memory will be freed by DiscountInfo object
    DiscountSegInfo* discountSegInfo2 = new DiscountSegInfo();
    DiscountSegInfo& discSegInfo2 = *discountSegInfo2;

    discSegInfo2.minNoPsg() = 0;
    discSegInfo2.maxNoPsg() = 0;
    discSegInfo2.minAge1() = 20;
    discSegInfo2.maxAge1() = 65;
    discSegInfo2.minAge2() = 0;
    discSegInfo2.maxAge2() = 0;
    discSegInfo2.minAge3() = 0;
    discSegInfo2.maxAge3() = 0;
    discSegInfo2.accPsgType1() = "MIL";
    discSegInfo2.accPsgType2() = "";
    discSegInfo2.accPsgType3() = "";
    discSegInfo2.fareClass() = "";
    discSegInfo2.bookingCode() = "";

    discInfo.segs().push_back(&discSegInfo2);

    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validate(trx, fareUsages));
    CPPUNIT_ASSERT(ret == true);

    diag.flushMsg();
  }

  void testFareUsageSameRule()
  {
    PricingTrx trx;
    trx.diagnostic().diagnosticType() = Diagnostic313;
    trx.diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic319);

    FareUsage* fu0 = TestFareUsageFactory::create("AccompaniedTravelData/FareUsage0.xml");
    FareUsage* fu1 = TestFareUsageFactory::create("AccompaniedTravelData/FareUsage1.xml");
    FareMarket* fareMarket = TestFareMarketFactory::create("AccompaniedTravelData/FareMarket.xml");

    CPPUNIT_ASSERT(fu0 != 0);
    CPPUNIT_ASSERT(fu1 != 0);

    //--------------------------------------------------------
    std::vector<FareUsage*> fareUsages;
    fareUsages.push_back(fu0);
    fareUsages.push_back(fu1);

    fu1->paxTypeFare()->setAccSameFareBreak(true); // Revalidation during
    // combinability check
    fu1->paxTypeFare()->fareMarket() = fareMarket;

    // Rule Item Info
    AccompaniedTravelInfo accTvlInfo;

    accTvlInfo.vendor() = "ATP";
    accTvlInfo.unavailTag() = ' ';
    accTvlInfo.accPsgAppl() = ' '; // MUST
    accTvlInfo.accPsgType() = "PFA";
    accTvlInfo.accPsgId() = 'X';
    accTvlInfo.minAge() = 0;
    accTvlInfo.maxAge() = 0;
    accTvlInfo.minNoPsg() = 0;
    accTvlInfo.maxNoPsg() = 0;
    accTvlInfo.accTvlAllSectors() = ' ';
    accTvlInfo.accTvlOut() = ' ';
    accTvlInfo.accTvlOneSector() = ' ';
    accTvlInfo.accTvlSameCpmt() = ' ';
    accTvlInfo.accTvlSameRule() = 'X';
    accTvlInfo.geoTblItemNoVia1() = 0;
    accTvlInfo.geoTblItemNoVia2() = 0;
    accTvlInfo.fareClassBkgCdInd() = ' ';

    AccompaniedTravel accTvl;
    Record3ReturnTypes ret = PASS;

    //--------------------------------------------------------
    // Fail because different Tariff
    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validateFareUsages(trx, &accTvlInfo, *fu0, fareUsages));
    CPPUNIT_ASSERT(ret == FAIL);

    FareUsage* fu2 = TestFareUsageFactory::create("AccompaniedTravelData/FareUsage2.xml");
    fu2->paxTypeFare()->fare()->initialize(Fare::FS_International,
                                           fu0->paxTypeFare()->fare()->fareInfo(),
                                           *(fu0->paxTypeFare()->fareMarket()),
                                           fu0->paxTypeFare()->fare()->tariffCrossRefInfo(),
                                           fu0->paxTypeFare()->fare()->constructedFareInfo());
    PaxType* paxTypePFA = TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypePFA.xml");
    CPPUNIT_ASSERT(paxTypePFA != 0);

    fu2->paxTypeFare()->actualPaxType() = paxTypePFA;
    fareUsages.push_back(fu2);

    //--------------------------------------------------------
    // should pass now
    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validateFareUsages(trx, &accTvlInfo, *fu0, fareUsages));

    CPPUNIT_ASSERT(ret == PASS);

    //--------------------------------------------------------
    // should fail if same rule but wrong PaxType
    accTvlInfo.accPsgType() = "FIF";
    CPPUNIT_ASSERT_NO_THROW(ret = accTvl.validateFareUsages(trx, &accTvlInfo, *fu0, fareUsages));
    CPPUNIT_ASSERT(ret == FAIL);

    diag.flushMsg();
  }

  void testCheckFareClassBkgCodeReturnTrueIfAccompaniedTravelInfoEmpty()
  {
    AccompaniedTravel accTvl;
    AccompaniedTravelInfo accTvlInfo;
    PaxTypeFare* ptf = TestPaxTypeFareFactory::create("AccompaniedTravelData/PaxTypeFare1.xml");
    PricingTrx trx;
    CPPUNIT_ASSERT(accTvl.checkFareClassBkgCode(*ptf, accTvlInfo, trx, 0));
  }

  void testCheckFareClassBkgCodeReturnTrueIfFareClassIndBlank()
  {
    AccompaniedTravel accTvl;
    AccompaniedTravelInfo accTvlInfo;
    PaxTypeFare* ptf = TestPaxTypeFareFactory::create("AccompaniedTravelData/PaxTypeFare1.xml");
    accTvlInfo.fareClassBkgCds().push_back(ptf->fare()->fareClass());
    accTvlInfo.fareClassBkgCdInd() = ' ';
    PricingTrx trx;
    CPPUNIT_ASSERT(accTvl.checkFareClassBkgCode(*ptf, accTvlInfo, trx, 0));
  }

  void testCheckFareClassBkgCodeReturnFalseIfFarePointerZero()
  {
    AccompaniedTravel accTvl;
    AccompaniedTravelInfo accTvlInfo;
    PaxTypeFare* ptf = TestPaxTypeFareFactory::create("AccompaniedTravelData/PaxTypeFare1.xml");
    accTvlInfo.fareClassBkgCds().push_back(ptf->fare()->fareClass());
    PaxTypeFare ptf2;
    ptf2.setFare(0);
    accTvlInfo.fareClassBkgCdInd() = AccompaniedTravel::fareClass;
    PricingTrx trx;
    CPPUNIT_ASSERT(!accTvl.checkFareClassBkgCode(ptf2, accTvlInfo, trx, 0));
  }

  void testCheckFareClassBkgCodeReturnTrueIfFareClassMatches()
  {
    AccompaniedTravel accTvl;
    AccompaniedTravelInfo accTvlInfo;
    PaxTypeFare* ptf = TestPaxTypeFareFactory::create("AccompaniedTravelData/PaxTypeFare1.xml");
    accTvlInfo.fareClassBkgCds().push_back(ptf->fare()->fareClass());
    accTvlInfo.fareClassBkgCdInd() = AccompaniedTravel::fareClass;
    PricingTrx trx;
    CPPUNIT_ASSERT(accTvl.checkFareClassBkgCode(*ptf, accTvlInfo, trx, 0));
  }

  void testWPA()
  {
    PricingTrx trx;
    trx.diagnostic().diagnosticType() = Diagnostic860;
    trx.diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic860);

    FareUsage* fu1 = TestFareUsageFactory::create("AccompaniedTravelData/FareUsage1.xml");

    // Rule Item Info
    // Cat19
    DiscountInfo discInfo;
    // Memory will be freed by DiscountInfo object
    DiscountSegInfo* discountSegInfo1 = new DiscountSegInfo();
    DiscountSegInfo& discSegInfo1 = *discountSegInfo1;

    discInfo.vendor() = "ATP";
    discInfo.unavailtag() = ' ';
    discInfo.accPsgAppl() = ' '; // MUST
    discInfo.paxType() = CHILD;
    discInfo.segCnt() = 1;
    discInfo.psgid() = 'X';
    discInfo.minAge() = 5;
    discInfo.maxAge() = 10;
    discInfo.accTvlAllSectors() = 'X';
    discInfo.accTvlOut() = ' ';
    discInfo.accTvlOneSector() = ' ';
    discInfo.accTvlSameCpmt() = 'X';
    discInfo.accTvlSameRule() = ' ';
    discInfo.fareClassBkgCodeInd() = AccompaniedTravel::notApply;

    discSegInfo1.minNoPsg() = 2;
    discSegInfo1.maxNoPsg() = 1;
    discSegInfo1.minAge1() = 20;
    discSegInfo1.maxAge1() = 54;
    discSegInfo1.minAge2() = 55;
    discSegInfo1.maxAge2() = 60;
    discSegInfo1.minAge3() = 0;
    discSegInfo1.maxAge3() = 0;
    discSegInfo1.accPsgType1() = ADULT;
    discSegInfo1.accPsgType2() = "";
    discSegInfo1.accPsgType3() = "";
    discSegInfo1.fareClass() = "";
    discSegInfo1.bookingCode() = "";

    discInfo.segs().push_back(&discSegInfo1);
    // Link discount info to PaxTypeFare
    PaxTypeFareRuleData* paxTypeFareRuleData0 = _memHandle.create<PaxTypeFareRuleData>();
    paxTypeFareRuleData0->ruleItemInfo() = dynamic_cast<RuleItemInfo*>(&discInfo);
    fu1->paxTypeFare()->setRuleData(19, trx.dataHandle(), paxTypeFareRuleData0);
    fu1->paxTypeFare()->status().set(PaxTypeFare::PTF_Discounted);
    fu1->paxTypeFare()->fareMarket()->travelSeg().front()->segmentOrder() = 1;
    fu1->paxTypeFare()->fareMarket()->primarySector()->setBookingCode("S");

    // Cat13
    AccompaniedTravelInfo accTvlInfo;

    accTvlInfo.vendor() = "ATP";
    accTvlInfo.unavailTag() = ' ';
    accTvlInfo.accPsgAppl() = ' '; // MUST
    accTvlInfo.accPsgType() = ADULT;
    accTvlInfo.accPsgId() = 'X';
    accTvlInfo.minAge() = 25;
    accTvlInfo.maxAge() = 65;
    accTvlInfo.minNoPsg() = 1;
    accTvlInfo.maxNoPsg() = 2;
    accTvlInfo.accTvlAllSectors() = 'X';
    accTvlInfo.accTvlOut() = ' ';
    accTvlInfo.accTvlOneSector() = ' ';
    accTvlInfo.accTvlSameCpmt() = 'X';
    accTvlInfo.accTvlSameRule() = ' ';
    accTvlInfo.geoTblItemNoVia1() = 0;
    accTvlInfo.geoTblItemNoVia2() = 0;

    // Array of Fare Class/Generic
    const FareClassCode fcArray1[] = { "F", "J1FL", "REUNBAO", "YFLBAO" };
    const int fcArraySize1 = sizeof(fcArray1) / sizeof(fcArray1[0]);

    std::vector<FareClassCode> fc1(fcArray1, fcArray1 + fcArraySize1);
    accTvlInfo.fareClassBkgCds() = fc1;
    accTvlInfo.fareClassBkgCdInd() = AccompaniedTravel::fareClass;

    fu1->paxTypeFare()->setAccSameFareBreak(true); // Revalidation during
    // combinability check

    PaxTypeFareRuleData* paxTypeFareRuleData1 = _memHandle.create<PaxTypeFareRuleData>();
    paxTypeFareRuleData1->ruleItemInfo() = dynamic_cast<RuleItemInfo*>(&accTvlInfo);
    fu1->paxTypeFare()->setRuleData(
        RuleConst::ACCOMPANIED_PSG_RULE, trx.dataHandle(), paxTypeFareRuleData1);
    fu1->paxTypeFare()->fareMarket()->travelSeg().front()->segmentOrder() = 2;
    fu1->paxTypeFare()->fareMarket()->primarySector()->setBookingCode("H");

    PricingUnit* pu = _memHandle.create<PricingUnit>();
    CPPUNIT_ASSERT(pu != 0);
    pu->fareUsage().push_back(fu1);

    FarePath* fp1 = _memHandle.create<FarePath>();
    CPPUNIT_ASSERT(fp1 != 0);
    fp1->pricingUnit().push_back(pu); // use same pu to make things simple,
    // may not be true with real PNR
    PaxType* paxTypeChild = TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypeChild.xml");
    fp1->paxType() = paxTypeChild;

    AccTvlDetailOut<DiagCollector> accTvlDetail;
    accTvlDetail.printFormat(diag);
    accTvlDetail.storeAccTvlDetail(&trx, diag, paxTypeChild->paxType(), *fp1);

    diag << " \n";
    fu1->paxTypeFare()->setAccSameFareBreak(true);
    accTvlDetail.storeAccTvlDetail(&trx, diag, paxTypeChild->paxType(), *fp1);

    diag.flushMsg();
  }

  void testWPN()
  {
    std::string accTvlDetail0 = "ADT 1 ADT 0 1 1 2 3 SPX 7100 0 H ";
    std::string accTvlDetail1 =
        "CNN 2 CNN 0 1 1 2 3 NLWAA3N 7100 0 H R 1 1 4 F J1FL REUNBAO YFLBAO 1 ADT ";
    std::vector<const std::string*> accTvlData;
    accTvlData.push_back(&accTvlDetail0);
    accTvlData.push_back(&accTvlDetail1);

    PricingTrx trx;
    trx.diagnostic().diagnosticType() = Diagnostic860;
    trx.diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    CPPUNIT_ASSERT(factory != 0);
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic860);

    AccompaniedTravel accTvl;
    std::vector<bool> resultVec;
    bool result = accTvl.validateAccTvl(trx, accTvlData, resultVec);
    // FareClass not match fail
    CPPUNIT_ASSERT(!result);
    CPPUNIT_ASSERT(resultVec[0]);

    // ADT use matched fare, pass
    accTvlDetail0 = "ADT 1 ADT 0 1 1 2 3 YFLBAO 7100 0 H ";
    result = accTvl.validateAccTvl(trx, accTvlData, resultVec);
    CPPUNIT_ASSERT(result);

    diag.flushMsg();
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(AccompaniedTravelTest);
}
