//-------------------------------------------------------------------
//
//  File:        FDSeasonalApplicationTest.cpp
//  Authors:     Lipika Bardalai
//  Created:     March 2005
//  Description: This class contains implements the Unit Test case
//               suite.
//  Copyright Sabre 2001
//               The copyright to the computer program(s) herein
//               is the property of Sabre.
//               The program(s) may be used and/or copied only with
//               the written permission of Sabre or in accordance
//               with the terms and conditions stipulated in the
//               agreement/contract under which the program(s)
//               have been supplied.
//
//---------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"

#include "Common/DateTime.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/SeasonalAppl.h"
#include "Rules/FDSeasonalApplication.h"

#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestPaxTypeFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"

namespace tse
{

class FDSeasonalApplicationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FDSeasonalApplicationTest);

  CPPUNIT_TEST(testValidate_case1);
  CPPUNIT_TEST(testValidate_case2);
  CPPUNIT_TEST(testValidate_case3);
  CPPUNIT_TEST(testValidate_case4);
  CPPUNIT_TEST(testValidate_case5);
  CPPUNIT_TEST(testValidate_case6);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    PaxType* paxTypeAdult =
                 TestPaxTypeFactory::create("FDSeasonalApplicationData/PaxTypeAdult.xml"),
             *paxTypeChild =
                 TestPaxTypeFactory::create("FDSeasonalApplicationData/PaxTypeChild.xml");

    AirSeg* airSeg0 = TestAirSegFactory::create("FDSeasonalApplicationData/AirSegDFWJFK.xml"),
            *airSeg1 = TestAirSegFactory::create("FDSeasonalApplicationData/AirSegJFKDFW.xml");

    _ptfChild = TestPaxTypeFareFactory::create("FDSeasonalApplicationData/PaxTypeFare1.xml");
    _fm = TestFareMarketFactory::create("FDSeasonalApplicationData/FareMarket.xml");

    _fdTrx = _memH.create<FareDisplayTrx>();
    _fdTrx->travelSeg().push_back(airSeg0);
    _fdTrx->travelSeg().push_back(airSeg1);
    _fdTrx->paxType().push_back(paxTypeAdult);
    _fdTrx->paxType().push_back(paxTypeChild);
    _fdTrx->fareMarket().push_back(_fm);

    _itin = _memH.create<Itin>();
    _itin->travelSeg().push_back(airSeg0);
    _itin->travelSeg().push_back(airSeg1);
    _itin->fareMarket().push_back(_fm);

    _fdTrx->itin().push_back(_itin);

    _seasonsInfo = _memH.create<SeasonalAppl>();
    _seasonsInfo->unavailtag() = ' ';
    _seasonsInfo->seasonDateAppl() = 'H';
    _seasonsInfo->assumptionOverride() = ' ';
    _seasonsInfo->geoTblItemNo() = 1;

    _rule = _memH.create<CategoryRuleItemInfo>();

    _seasonalAppl = _memH.create<FDSeasonalApplication>(_rule);
  }

  void tearDown() { _memH.clear(); }

  void testValidate_case1()
  {
    _rule->setInOutInd(RuleConst::ALWAYS_APPLIES);
    populateDates(DateTime(2004, 6, 1), DateTime(2004, 12, 30));

    Record3ReturnTypes ret = FAIL;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _seasonalAppl->validate(*_fdTrx, *_itin, *_ptfChild, _seasonsInfo, *_fm));
    CPPUNIT_ASSERT_EQUAL(SOFTPASS, ret);
  }

  void testValidate_case2()
  {
    _rule->setInOutInd(FDSeasonalApplication::OUTBOUND);
    populateDates(DateTime(1400, 6, 1), DateTime(1400, 12, 30));

    Record3ReturnTypes ret = FAIL;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _seasonalAppl->validate(*_fdTrx, *_itin, *_ptfChild, _seasonsInfo, *_fm));
    CPPUNIT_ASSERT_EQUAL(SOFTPASS, ret);
  }

  void testValidate_case3()
  {
    _rule->setInOutInd(FDSeasonalApplication::OUTBOUND);
    populateDates(DateTime(2004, 6, 1), DateTime(2004, 6, 30));

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _seasonalAppl->validate(*_fdTrx, *_itin, *_ptfChild, _seasonsInfo, *_fm));
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidate_case4()
  {
    _rule->setInOutInd(FDSeasonalApplication::OUTBOUND);
    populateDates(DateTime(2004, 6, 1), DateTime(2004, 6, 30));
    _seasonsInfo->unavailtag() = 'X';

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _seasonalAppl->validate(*_fdTrx, *_itin, *_ptfChild, _seasonsInfo, *_fm));
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidate_case5()
  {
    _rule->setInOutInd(FDSeasonalApplication::INBOUND);
    populateDates(DateTime(2004, 6, 1), DateTime(2004, 6, 30));
    _seasonsInfo->unavailtag() = 'Y';

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _seasonalAppl->validate(*_fdTrx, *_itin, *_ptfChild, _seasonsInfo, *_fm));
    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  void testValidate_case6()
  {
    _rule->setInOutInd(FDSeasonalApplication::INBOUND);
    populateDates(DateTime(2004, 6, 1), DateTime(2004, 6, 30));
    _seasonsInfo->unavailtag() = ' ';
    _seasonsInfo->seasonDateAppl() = ' ';
    _seasonsInfo->assumptionOverride() = ' ';
    _seasonsInfo->geoTblItemNo() = 1;

    Record3ReturnTypes ret = PASS;
    CPPUNIT_ASSERT_NO_THROW(
        ret = _seasonalAppl->validate(*_fdTrx, *_itin, *_ptfChild, _seasonsInfo, *_fm));
    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void populateDates(const DateTime& start, const DateTime& stop)
  {
    _seasonsInfo->tvlstartyear() = start.year() < 2000 ? 0 : uint(start.year());
    _seasonsInfo->tvlstartmonth() = start.month();
    _seasonsInfo->tvlstartDay() = start.day();
    _seasonsInfo->tvlStopyear() = stop.year() < 2000 ? 0 : uint(stop.year());
    _seasonsInfo->tvlStopmonth() = stop.month();
    _seasonsInfo->tvlStopDay() = stop.day();
  }

protected:
  FareMarket* _fm;
  FareDisplayTrx* _fdTrx;
  PaxTypeFare* _ptfChild;
  Itin* _itin;
  SeasonalAppl* _seasonsInfo;
  CategoryRuleItemInfo* _rule;
  SeasonalApplication* _seasonalAppl;

  TestMemHandle _memH;
  RootLoggerGetOff _loggerOff;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FDSeasonalApplicationTest);

} // tse
