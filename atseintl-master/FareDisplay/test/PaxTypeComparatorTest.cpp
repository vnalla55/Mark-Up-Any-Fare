#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/PsgTypeComparator.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/Fare.h"
#include "DataModel/PaxTypeFare.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseEnums.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareDisplayRequest.h"
#include "DBAccess/FareClassAppSegInfo.h"

#include "FareDisplay/GroupingStrategy.h"
#include "FareDisplay/Group.h"
#include "FareDisplay/StrategyBuilder.h"

#include "DataModel/Agent.h"
#include "DataModel/Itin.h"
#include "DataModel/Billing.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "Common/DateTime.h"
#include "test/include/MockGlobal.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
class PaxTypeComparatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PaxTypeComparatorTest);
  CPPUNIT_TEST(testTranslate);
  CPPUNIT_TEST(testAlphabeticalComparison);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _pFare1 = _memHandle.create<PaxTypeFare>();
    _pFare2 = _memHandle.create<PaxTypeFare>();
  }
  void tearDown() { _memHandle.clear(); }

  void testTranslate()
  {
    PsgTypeComparator comparator;
    PaxTypeCode paxType;
    CPPUNIT_ASSERT(comparator.translate(paxType) == "ADT");
    paxType = "CNN";
    CPPUNIT_ASSERT(comparator.translate(paxType) == "CNN");
  }
  void testAlphabeticalComparison()
  {

    FareClassAppSegInfo appSeg1, appSeg2;
    appSeg1._paxType = "ADT";
    appSeg2._paxType = "CNN";

    _pFare1->fareClassAppSegInfo() = &appSeg1;
    _pFare2->fareClassAppSegInfo() = &appSeg2;
    PsgTypeComparator comparator;
    Group group;
    group.sortType() = Group::APPLY_PRIORITY_LIST;
    comparator.group() = &group;
    Comparator::Result result;
    result = comparator.alphabeticalComparison(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::TRUE);

    appSeg1._paxType = "UNN";
    appSeg2._paxType = "CNN";

    result = comparator.alphabeticalComparison(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::FALSE);
  }

private:
  PaxTypeFare* _pFare1;
  PaxTypeFare* _pFare2;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(PaxTypeComparatorTest);
}
