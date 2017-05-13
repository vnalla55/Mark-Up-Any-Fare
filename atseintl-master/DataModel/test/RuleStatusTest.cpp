#include "DBAccess/AddonFareInfo.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "Common/TseConsts.h"
#include "DBAccess/Loc.h"
#include "DataModel/PaxType.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{
class RuleStatusTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RuleStatusTest);
  CPPUNIT_TEST(testRuleStatusSetGet);
  CPPUNIT_TEST_SUITE_END();

  void testRuleStatusSetGet()
  {
    // create Fare object

    FareInfo fareInfo;
    AddonFareInfo addonFareInfo;
    TariffCrossRefInfo tariffCrossRefInfo;

    Fare fare;

    PaxTypeFare paxTypeFare;
    PaxType actualPaxType;
    FareMarket fareMarket;
    FareClassAppInfo fareClassAppInfo;
    FareClassAppSegInfo fareClassAppSegInfo;
    // UnknownGeoTravelType
    CPPUNIT_ASSERT(
        fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffCrossRefInfo));

    // test rule status get/set

    int i;

    // check default rule state. by default all pass

    for (i = 1; i <= 16; i++)
    {
      if (i != 1 && i != 13)
      {
        CPPUNIT_ASSERT(fare.isCategoryValid(i));
      }
    }

    // set all categories to fail and check the status

    for (i = 1; i <= 16; i++)
    {
      if (i != 1 && i != 13)
      {
        CPPUNIT_ASSERT(!fare.setCategoryValid(i, false));
      }
    }

    for (i = 1; i <= 16; i++)
    {
      if (i != 1 && i != 13)
      {
        CPPUNIT_ASSERT(!fare.isCategoryValid(i));
      }
    }

    // set half of categories to pass and check the status

    for (i = 1; i <= 16; i += 2)
    {
      if (i != 1 && i != 13)
      {
        CPPUNIT_ASSERT(fare.setCategoryValid(i, true));
      }
    }

    bool expectedStatus = true;
    for (i = 1; i <= 16; i++)
    {
      if (i != 1 && i != 13)
      {
        if (expectedStatus)
          CPPUNIT_ASSERT(fare.isCategoryValid(i));
        else
          CPPUNIT_ASSERT(!fare.isCategoryValid(i));
      }

      expectedStatus = !expectedStatus;
    }

    // reset rule status so paxTypeFare.initialize will return true

    for (i = 1; i <= 16; i++)
    {
      if (i != 1 && i != 13)
      {
        CPPUNIT_ASSERT(fare.setCategoryValid(i, true));
      }
    }

    // create PaxTypeFare

    paxTypeFare.fareClassAppInfo() = &fareClassAppInfo;
    paxTypeFare.fareClassAppSegInfo() = &fareClassAppSegInfo;

    CPPUNIT_ASSERT(paxTypeFare.initialize(&fare, &actualPaxType, &fareMarket));

    // check default rule state. by default all pass

    for (i = 1; i <= 35; i++)
    {
      if (i != 17 && i != 18 && i != 23 && i != 24 && i != 26 && i != 27 && i != 28 && i != 29 &&
          i != 30 && i != 31 && i != 32 && i != 33 && i != 34)
      {
        CPPUNIT_ASSERT(paxTypeFare.isCategoryValid(i));
      }
    }

    // set all categories to fail and check the status

    for (i = 1; i <= 35; i++)
    {
      if (i != 17 && i != 18 && i != 23 && i != 24 && i != 26 && i != 27 && i != 28 && i != 29 &&
          i != 30 && i != 31 && i != 32 && i != 33 && i != 34)
      {
        CPPUNIT_ASSERT(!paxTypeFare.setCategoryValid(i, false));
      }
    }

    for (i = 1; i <= 35; i++)
    {
      if (i != 17 && i != 18 && i != 23 && i != 24 && i != 26 && i != 27 && i != 28 && i != 29 &&
          i != 30 && i != 31 && i != 32 && i != 33 && i != 34)
      {
        CPPUNIT_ASSERT(!paxTypeFare.isCategoryValid(i));
      }
    }

    // set half of categories to pass and check the status

    for (i = 1; i <= 35; i += 2)
    {
      if (i != 17 && i != 18 && i != 23 && i != 24 && i != 26 && i != 27 && i != 28 && i != 29 &&
          i != 30 && i != 31 && i != 32 && i != 33 && i != 34)
      {
        CPPUNIT_ASSERT(paxTypeFare.setCategoryValid(i, true));
      }
    }

    expectedStatus = true;
    for (i = 1; i <= 16; i++)
    {
      if (i != 17 && i != 18 && i != 23 && i != 24 && i != 26 && i != 27 && i != 28 && i != 29 &&
          i != 30 && i != 31 && i != 32 && i != 33 && i != 34)
      {
        if (expectedStatus)
          CPPUNIT_ASSERT(paxTypeFare.isCategoryValid(i));
        else
          CPPUNIT_ASSERT(!paxTypeFare.isCategoryValid(i));
      }

      expectedStatus = !expectedStatus;
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RuleStatusTest);
}
