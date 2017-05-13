#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/ComparatorFactory.h"

#include "Common/Global.h"
#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/Comparator.h"

#include "FareDisplay/RoutingNumberComparator.h"
#include "FareDisplay/FareAmountComparator.h"
#include "FareDisplay/PublicPrivateComparator.h"
#include "FareDisplay/NormalSpecialComparator.h"
#include "FareDisplay/OneWayRoundTripComparator.h"
#include "FareDisplay/GlobalDirectionComparator.h"
#include "FareDisplay/PsgTypeComparator.h"
#include "FareDisplay/FareBasisComparator.h"
#include "FareDisplay/CharCombinationsComparator.h"
#include "FareDisplay/TravelDiscontinueDateComparator.h"
#include "FareDisplay/ScheduleCountComparator.h"
#include "FareDisplay/MultiTransportComparator.h"
#include "FareDisplay/S8BrandComparator.h"
#include "FareDisplay/NullComparator.h"
#include "FareDisplay/CabinGroupComparator.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{
class ComparatorFactoryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ComparatorFactoryTest);
  CPPUNIT_TEST(testGetComparator);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    DiskCache::initialize(Global::config());
    _memHandle.create<MockDataManager>();
  }

  void tearDown() { _memHandle.clear(); }

  void testGetComparator()
  {
    FareDisplayTrx trx;
    ComparatorFactory cf(trx);
    Comparator* pCmp = NULL;
    Group::GroupType gt;

    gt = Group::GROUP_BY_SCHEDULE_COUNT;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<ScheduleCountComparator*>(pCmp));

    gt = Group::GROUP_BY_ROUTING;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<RoutingNumberComparator*>(pCmp));

    gt = Group::GROUP_BY_GLOBAL_DIR;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<GlobalDirectionComparator*>(pCmp));

    gt = Group::GROUP_BY_PSG_TYPE;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<PsgTypeComparator*>(pCmp));

    gt = Group::GROUP_BY_NORMAL_SPECIAL;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<NormalSpecialComparator*>(pCmp));

    gt = Group::GROUP_BY_PUBLIC_PRIVATE;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<PublicPrivateComparator*>(pCmp));

    gt = Group::GROUP_BY_FARE_AMOUNT;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<FareAmountComparator*>(pCmp));

    gt = Group::GROUP_BY_OW_RT;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<OneWayRoundTripComparator*>(pCmp));

    gt = Group::GROUP_BY_FARE_BASIS_CODE;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<FareBasisComparator*>(pCmp));

    gt = Group::GROUP_BY_FARE_BASIS_CHAR_COMB;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<CharCombinationsComparator*>(pCmp));

    gt = Group::GROUP_BY_TRAVEL_DISCONTINUE_DATE;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<TravelDiscontinueDateComparator*>(pCmp));

    gt = Group::GROUP_BY_MULTITRANSPORT;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<MultiTransportComparator*>(pCmp));

    gt = Group::GROUP_BY_S8BRAND;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<S8BrandComparator*>(pCmp));

    gt = Group::GROUP_BY_CABIN;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<CabinGroupComparator*>(pCmp));

    gt = Group::GROUP_NOT_REQUIRED;
    pCmp = cf.getComparator(gt);
    CPPUNIT_ASSERT(dynamic_cast<NullComparator*>(pCmp));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(ComparatorFactoryTest);
}
