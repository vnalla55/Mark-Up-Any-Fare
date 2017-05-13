#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Taxes/LegacyTaxes/TaxMap.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "Common/TseCodeTypes.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{

class TaxMapTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxMapTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxMapCase0);
  CPPUNIT_TEST(testTaxMapCase1);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    try { TaxMap taxMap(trx.dataHandle()); }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  /**
   * Case0 is check Map setup,
   * then we return true.
   **/
  void testTaxMapCase0()
  {
    TaxMap taxMap(trx.dataHandle());

    taxMap.initialize();
  }

  /**
   * Case1 is check retrival from the Map setup,
  **/

  void testTaxMapCase1()
  {
    TaxMap taxMap(trx.dataHandle());

    taxMap.initialize();

    uint16_t specialProcessNo = 0;

    Tax& tax = *taxMap.getSpecialTax(specialProcessNo);

    CPPUNIT_ASSERT(&tax != 0);

    specialProcessNo = 8000;

    Tax& tax2 = *taxMap.getSpecialTax(specialProcessNo);

    CPPUNIT_ASSERT(&tax2 != 0);

    specialProcessNo = 999;

    Tax& tax3 = *taxMap.getSpecialTax(specialProcessNo);

    CPPUNIT_ASSERT(&tax3 == 0);
  }

  tse::PricingTrx trx;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxMapTest);
}
