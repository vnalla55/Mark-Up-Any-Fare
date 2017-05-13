#include "Taxes/Common/TaxUtility.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/Loc.h"
#include "DataModel/AirSeg.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class TaxUtility_hasHiddenStopTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxUtility_hasHiddenStopTest);
  CPPUNIT_TEST(hasHiddenStop_NoHiddenStops_Test);
  CPPUNIT_TEST(hasHiddenStop_NoHiddenStopInLoc_Test);
  CPPUNIT_TEST(hasHiddenStop_IsHiddenStopInLoc_Test);
  CPPUNIT_TEST_SUITE_END();

  LocCode NRT;
  LocCode JFK;
  LocCode KRK;
  Loc _locNRT;
  Loc _locJFK;
  Loc _locKRK;
  AirSeg _airSeg;

public:
  void setUp()
  {
    NRT = "NRT";
    JFK = "JFK";
    KRK = "KRK";

    _locNRT.loc() = NRT;
    _locJFK.loc() = JFK;
    _locKRK.loc() = KRK;
  }

  void hasHiddenStop_NoHiddenStops_Test()
  {
    CPPUNIT_ASSERT(!taxUtil::hasHiddenStopInLoc(&_airSeg, NRT));
  }

  void hasHiddenStop_NoHiddenStopInLoc_Test()
  {
    _airSeg.hiddenStops().push_back(&_locJFK);
    _airSeg.hiddenStops().push_back(&_locKRK);

    CPPUNIT_ASSERT(!taxUtil::hasHiddenStopInLoc(&_airSeg, NRT));
  }

  void hasHiddenStop_IsHiddenStopInLoc_Test()
  {
    _airSeg.hiddenStops().push_back(&_locNRT);

    CPPUNIT_ASSERT(taxUtil::hasHiddenStopInLoc(&_airSeg, NRT));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxUtility_hasHiddenStopTest);
};
