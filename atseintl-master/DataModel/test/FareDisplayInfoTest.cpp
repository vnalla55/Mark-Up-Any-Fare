#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

class FareDisplayInfoTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayInfoTest);
  CPPUNIT_TEST(APNoPrice);
  CPPUNIT_TEST(APSameTime);
  CPPUNIT_TEST(APBadDay);
  CPPUNIT_TEST(APDash);
  CPPUNIT_TEST(APDash2);

  CPPUNIT_TEST(APResDay);
  CPPUNIT_TEST(APResTkt);
  CPPUNIT_TEST(APOnlyTkt);
  CPPUNIT_TEST(APTktConvert);
  CPPUNIT_TEST(APTktConvert2);
  CPPUNIT_TEST(APTktNoConvert);
  CPPUNIT_TEST(APMonth);
  CPPUNIT_TEST(AP0Hour);
  CPPUNIT_TEST(AP0Day);
  CPPUNIT_TEST(APBadHour);
  CPPUNIT_TEST(APBadHour2);
  CPPUNIT_TEST(APMultiTkt);

  CPPUNIT_TEST(testSetPassedInclusion_true);

  CPPUNIT_TEST_SUITE_END();

private:
  FareDisplayInfo* _fdi;
  FareDisplayRequest* _requestFD;
  FareDisplayTrx* _trxFD;

  TestMemHandle _memHandle;
  std::string _APstring;

public:
  void setUp()
  {
    _fdi = _memHandle.create<FareDisplayInfo>();
    _requestFD = _memHandle.create<FareDisplayRequest>();
    _trxFD = _memHandle.create<FareDisplayTrx>();

  }

  void tearDown() { _memHandle.clear(); }


/* AdvPurchace string - text */
void APNoPrice()
{
  _fdi->displayOnly() = true;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" NP"), _APstring.substr(0, 3));

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string("NP"), _APstring);
}
void APSameTime()
{
  _fdi->advTktPeriod() = "000";
  _fdi->advTktUnit() = "N";
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string("SIML "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}
void APBadDay()
{
  _fdi->lastResPeriod() = "xxx";
  _fdi->lastResUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $   "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}
void APDash()
{
  _fdi->lastResPeriod() = ADV_PUR_TKT_3BLANKSPACE;
  _fdi->advTktPeriod() = ADV_PUR_TKT_3BLANKSPACE;
  _fdi->ticketed() = 'Y';
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" -/S "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}
void APDash2()
{
  _fdi->lastResPeriod() = ADV_PUR_TKT_3BLANKSPACE;
  _fdi->advTktPeriod() = ADV_PUR_TKT_3BLANKSPACE;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" -   "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" -"), _APstring);
}

/* AdvPurchace string - real data */
void APResDay()
{
  _fdi->lastResPeriod() = "030";
  _fdi->lastResUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string("30   "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string("30"), _APstring);
}
void APResTkt()
{
  _fdi->lastResPeriod() = "045";
  _fdi->lastResUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->advTktPeriod() = "010";
  _fdi->advTktUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string("45/10"), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}
void APOnlyTkt()
{
  _fdi->lastResPeriod() = ADV_PUR_TKT_3BLANKSPACE;
  _fdi->advTktPeriod() = "001";
  _fdi->advTktUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" -/1 "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}

void APTktConvert()
{
  _fdi->lastResPeriod() = "029";
  _fdi->lastResUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->advTktPeriod() = "048";
  _fdi->advTktUnit() = ADV_PUR_TKT_UNITS_HOUR;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string("29/2 "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}
void APTktConvert2()
{
  _fdi->lastResPeriod() = "021";
  _fdi->lastResUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->advTktPeriod() = "192";
  _fdi->advTktUnit() = ADV_PUR_TKT_UNITS_HOUR;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string("21/8 "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}

void APTktNoConvert()
{
  _fdi->lastResPeriod() = "021";
  _fdi->lastResUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->advTktPeriod() = "004";
  _fdi->advTktUnit() = ADV_PUR_TKT_UNITS_HOUR;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string("21/4H"), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}

void APMonth()
{
  _fdi->lastResPeriod() = "090";
  _fdi->lastResUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->advTktPeriod() = "001";
  _fdi->advTktUnit() = ADV_PUR_TKT_UNITS_MONTH;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string("90/1M"), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}

void AP0Hour()
{
  _fdi->lastResPeriod() = "037";
  _fdi->lastResUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->advTktPeriod() = "000";
  _fdi->advTktUnit() = ADV_PUR_TKT_UNITS_HOUR;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string("37/$ "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}
void AP0Day()
{
  _fdi->lastResPeriod() = ADV_PUR_TKT_3BLANKSPACE;
  _fdi->advTktPeriod() = "000";
  _fdi->advTktUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" -/0 "), _APstring);
  //  old req  CPPUNIT_ASSERT_EQUAL(std::string(" -/    "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}

void APBadHour()
{
  _fdi->lastResPeriod() = "003";
  _fdi->lastResUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->advTktPeriod() = "025";
  _fdi->advTktUnit() = ADV_PUR_TKT_UNITS_HOUR;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" 3/$ "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}
void APBadHour2()
{
  _fdi->lastResPeriod() = ADV_PUR_TKT_3BLANKSPACE;
  _fdi->advTktPeriod() = "666";
  _fdi->advTktUnit() = ADV_PUR_TKT_UNITS_HOUR;
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" -/$ "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}

void APMultiTkt()
{
  _fdi->lastResPeriod() = "014";
  _fdi->lastResUnit() = ADV_PUR_TKT_UNITS_DAY;
  _fdi->advTktPeriod() = "$$$";
  _fdi->getAPString(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string("14/$ "), _APstring);

  _fdi->getAPStringShort(_APstring);
  CPPUNIT_ASSERT_EQUAL(std::string(" $"), _APstring);
}

void testSetPassedInclusion_true()
{
  _trxFD->setRequest(_requestFD);
  _fdi->setFareDisplayTrx(_trxFD);
  CPPUNIT_ASSERT(_fdi->inclusionCabinNum() == 0);

  uint8_t inclusionNumber = 6; // YB
  _fdi->setPassedInclusion(inclusionNumber);

  CPPUNIT_ASSERT(_fdi->inclusionCabinNum() != 0);
}

};

CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayInfoTest);
} // tse
