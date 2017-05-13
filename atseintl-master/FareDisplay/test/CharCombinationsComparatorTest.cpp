#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/CharCombinationsComparator.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/Fare.h"
#include "DataModel/PaxTypeFare.h"
#include "Common/TseEnums.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "FareDisplay/Group.h"

#include "Common/DateTime.h"
#include "test/include/MockGlobal.h"
#include "DataModel/PaxTypeFare.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
class CharCombinationsComparatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CharCombinationsComparatorTest);
  CPPUNIT_TEST(testCombPriority);
  CPPUNIT_TEST(testCompare);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _pFare1 = _memHandle.create<PaxTypeFare>();
    _pFare2 = _memHandle.create<PaxTypeFare>();
  }
  void tearDown() { _memHandle.clear(); }

  void testCombPriority()
  {
    CharCombinationsComparator comparator;

    comparator.priorityMap().insert(make_pair("AB", 1));
    comparator.priorityMap().insert(make_pair("BB", 2));
    comparator.priorityMap().insert(make_pair("CB", 2));

    std::string a("ABC"), b("BBC"), c("AECBF");
    CPPUNIT_ASSERT(comparator.combPriority(a) == 1);
    CPPUNIT_ASSERT(comparator.combPriority(b) == 2);
    CPPUNIT_ASSERT(comparator.combPriority(c) == 2);
    CPPUNIT_ASSERT(comparator.combPriority("XXXX") == 4);
  }
  void testCompare()
  {
    CharCombinationsComparator comparator;

    comparator.priorityMap().insert(make_pair("AB", 1));
    comparator.priorityMap().insert(make_pair("BB", 2));
    comparator.priorityMap().insert(make_pair("CB", 2));

    Fare fare1, fare2;
    FareInfo info1, info2;

    info1.fareClass() = "DEFAB";
    info2.fareClass() = "BBC";

    fare1.setFareInfo(&info1);
    fare2.setFareInfo(&info2);

    _pFare1->setFare(&fare1);
    _pFare2->setFare(&fare2);

    Group group;
    group.sortType() = Group::ASCENDING;
    comparator.group() = &group;
    Comparator::Result result;

    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::TRUE);

    info1.fareClass() = "DEFAB";
    info2.fareClass() = "ABC";

    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::EQUAL);

    info1.fareClass() = "YYYY";
    info2.fareClass() = "ABC";

    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::FALSE);

    // how about no child data

    comparator.priorityMap().clear();

    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::EQUAL);
  }

private:
  PaxTypeFare* _pFare1;
  PaxTypeFare* _pFare2;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CharCombinationsComparatorTest);
}
