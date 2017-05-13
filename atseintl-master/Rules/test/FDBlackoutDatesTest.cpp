#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/BlackoutInfo.h"
#include "Rules/FareDisplayBlackoutDates.h"

#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FDBlackoutDatesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FDBlackoutDatesTest);
  CPPUNIT_TEST(testProcess_RangePass);
  CPPUNIT_TEST(testProcess_RangeFail);
  CPPUNIT_TEST(testProcess_RangeWithEarliestPass);
  CPPUNIT_TEST(testProcess_RangeWithEarliestFail);
  CPPUNIT_TEST(testProcess_DatesPass);
  CPPUNIT_TEST(testProcess_DatesFail);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  BlackoutInfo* _blackoutInfo;
  FareDisplayBlackoutDates* _fdbl;
  FareDisplayTrx* _fdTrx;
  PaxTypeFare* _ptf;
  AirSeg* _airSeg;
  RootLoggerGetOff _loggerOff;

public:
  void setUp()
  {
    _blackoutInfo = _memHandle.create<BlackoutInfo>();
    _fdbl = _memHandle.create<FareDisplayBlackoutDates>();
    _fdTrx = _memHandle.create<FareDisplayTrx>();
    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    PaxType* paxTypeAdult = _memHandle.create<PaxType>();
    paxTypeAdult->paxType() = ADULT;
    _airSeg = _memHandle.create<AirSeg>();

    // create an itinery object - also attach to transaction object
    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(_airSeg);
    itin->fareMarket().push_back(fareMarket);
    fareMarket->travelSeg().push_back(_airSeg);

    // populate transaction object
    _fdTrx->travelSeg().push_back(_airSeg);
    _fdTrx->fareMarket().push_back(fareMarket);
    _fdTrx->paxType().push_back(paxTypeAdult);
    _fdTrx->itin().push_back(itin);

    // create PaxTypeFare object
    _ptf = _memHandle.create<PaxTypeFare>();
    _ptf->fareMarket() = fareMarket;
  }

  void tearDown() { _memHandle.clear(); }

  void setBlackoutValues(int startYear,
                         int startMonth,
                         int startDay,
                         int stopYear,
                         int stopMonth,
                         int stopDay,
                         Indicator appl)
  {
    _blackoutInfo->intlRest() = BlackoutDates::BLANK;
    _blackoutInfo->overrideDateTblItemNo() = 0;
    _blackoutInfo->geoTblItemNoBetween() = 0;
    _blackoutInfo->geoTblItemNoAnd() = 0;

    _blackoutInfo->tvlStartYear() = startYear;
    _blackoutInfo->tvlStartMonth() = startMonth;
    _blackoutInfo->tvlStartDay() = startDay;

    _blackoutInfo->blackoutAppl() = appl;

    _blackoutInfo->tvlStopYear() = stopYear;
    _blackoutInfo->tvlStopMonth() = stopMonth;
    _blackoutInfo->tvlStopDay() = stopDay;

    _fdbl->initialize(*_fdTrx, _blackoutInfo);
  }

  void setAirSegDates(int year, int month, int day, bool setEarliestLates)
  {
    _airSeg->departureDT() = DateTime(year, month, day);
    if (setEarliestLates)
    {
      _airSeg->earliestDepartureDT() = DateTime(year, month, day);
      _airSeg->latestDepartureDT() = DateTime(year, month, day + 3);
    }
  }

  // TESTS
  void testProcess_RangePass()
  {
    setBlackoutValues(10, 8, 1, 10, 8, 5, BlackoutDates::RANGE);
    setAirSegDates(2010, 8, 6, false);
    CPPUNIT_ASSERT_EQUAL(PASS, _fdbl->process(*_ptf, *_fdTrx, false));
  }

  void testProcess_RangeFail()
  {
    setBlackoutValues(10, 8, 1, 10, 8, 5, BlackoutDates::RANGE);
    setAirSegDates(2010, 8, 3, false);
    CPPUNIT_ASSERT_EQUAL(FAIL, _fdbl->process(*_ptf, *_fdTrx, false));
  }

  void testProcess_RangeWithEarliestPass()
  {
    setBlackoutValues(10, 8, 3, 10, 8, 5, BlackoutDates::RANGE);
    setAirSegDates(2010, 8, 3, true);
    CPPUNIT_ASSERT_EQUAL(PASS, _fdbl->process(*_ptf, *_fdTrx, false));
  }

  void testProcess_RangeWithEarliestFail()
  {
    setBlackoutValues(10, 8, 3, 10, 8, 6, BlackoutDates::RANGE);
    setAirSegDates(2010, 8, 3, true);
    CPPUNIT_ASSERT_EQUAL(FAIL, _fdbl->process(*_ptf, *_fdTrx, false));
  }

  void testProcess_DatesPass()
  {
    setBlackoutValues(10, 8, 1, 10, 8, 5, BlackoutDates::DATES);
    setAirSegDates(2010, 8, 3, false);
    CPPUNIT_ASSERT_EQUAL(PASS, _fdbl->process(*_ptf, *_fdTrx, false));
  }

  void testProcess_DatesFail()
  {
    setBlackoutValues(10, 8, 1, 10, 8, 5, BlackoutDates::DATES);
    setAirSegDates(2010, 8, 1, false);
    CPPUNIT_ASSERT_EQUAL(FAIL, _fdbl->process(*_ptf, *_fdTrx, false));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FDBlackoutDatesTest);
}
