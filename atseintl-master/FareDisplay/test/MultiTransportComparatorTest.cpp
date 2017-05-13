#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/Fare.h"
#include "DataModel/PaxTypeFare.h"
#include "Common/TseEnums.h"
#include "test/include/MockGlobal.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/TestMemHandle.h"
#include "FareDisplay/MultiTransportComparator.h"

using namespace std;

namespace tse
{
class MultiTransportComparatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MultiTransportComparatorTest);
  CPPUNIT_TEST(testgetPriority);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    pFare1 = _memHandle.create<PaxTypeFare>();
    pFare2 = _memHandle.create<PaxTypeFare>();
  }
  void tearDown() { _memHandle.clear(); }

  void testgetPriority()
  {
    MultiTransportComparator comparator;
    CPPUNIT_ASSERT(comparator.getPriority("DFW", "LAX") == 2);
    CPPUNIT_ASSERT(comparator.getPriority("DFW", "PHX") == 3);
    CPPUNIT_ASSERT(comparator.getPriority("DFW", "PHX") != 4);
    CPPUNIT_ASSERT(comparator.getPriority("LAX", "DFW") == 2);
    CPPUNIT_ASSERT(comparator.getPriority("DFW", "LAX") == 2);
  }

private:
  PaxTypeFare* pFare1;
  PaxTypeFare* pFare2;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MultiTransportComparatorTest);
}
