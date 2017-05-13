#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/FareBasisComparator.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/Fare.h"
#include "DataModel/PaxTypeFare.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseEnums.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareDisplayRequest.h"
#include "DBAccess/FDSSorting.h"

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

class FareBasisComparatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareBasisComparatorTest);

  CPPUNIT_TEST(testAllAscending);
  CPPUNIT_TEST(testUV39);
  CPPUNIT_TEST(testAllDescending);
  CPPUNIT_TEST(testSizeInequality);
  CPPUNIT_TEST(testAscendingDescendingAscendingComb);
  CPPUNIT_TEST(testRemoveCorpID);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    pFare1 = _memHandle.create<PaxTypeFare>();
    pFare2 = _memHandle.create<PaxTypeFare>();
    fdsNoList = _memHandle.create<FDSSorting>();

    fdsNoList->sortFareBasisChar1() = 'A';
    fdsNoList->sortFareBasisChar2() = 'A';
    fdsNoList->sortFareBasisChar3() = 'A';

    fdswithList = _memHandle.create<FDSSorting>();

    fdswithList->sortFareBasisChar1() = 'A';
    fdswithList->sortFareBasisChar2() = 'D';
    fdswithList->sortFareBasisChar3() = 'A';

    fdswithList->fareBasisChar1() = "DECMXP1R7B";
    fdswithList->fareBasisChar2() = "BELQYC1276";

    fdswithBlankList = _memHandle.create<FDSSorting>();

    fdswithBlankList->fareBasisChar1() = "DECMXP1R7B*";
    fdswithBlankList->fareBasisChar2() = "*BELQYC1276";
    fdswithBlankList->fareBasisChar2() = "BELQY*C1276";

    fdswithBlankList->sortFareBasisChar1() = 'D';
    fdswithBlankList->sortFareBasisChar2() = 'A';
    fdswithBlankList->sortFareBasisChar3() = 'A';

    fdsUV39 = _memHandle.create<FDSSorting>();

    fdsUV39->sortFareBasisChar1() = 'A';
    fdsUV39->sortFareBasisChar2() = 'D';
    fdsUV39->sortFareBasisChar3() = 'A';
  }
  void tearDown() {}

  void testAllDescending()
  {
    FareBasisComparator comparator;
    comparator.fdsItem() = fdswithBlankList;

    Fare fare1, fare2;
    FareInfo info1, info2;

    info1.fareClass() = "DBA";
    info2.fareClass() = "DBB";

    fare1.setFareInfo(&info1);
    fare2.setFareInfo(&info2);

    pFare1->setFare(&fare1);
    pFare2->setFare(&fare2);

    Comparator::Result rc = Comparator::EQUAL;

    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::TRUE);

    info1.fareClass() = "DBB";
    info2.fareClass() = "DB";
    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::FALSE);
  }
  void testAllAscending()
  {
    FareBasisComparator comparator;
    comparator.fdsItem() = fdsNoList;

    Fare fare1, fare2;
    FareInfo info1, info2;

    info1.fareClass() = "ABC";
    info2.fareClass() = "ABD";

    fare1.setFareInfo(&info1);
    fare2.setFareInfo(&info2);

    pFare1->setFare(&fare1);
    pFare2->setFare(&fare2);

    Comparator::Result rc = Comparator::EQUAL;

    CPPUNIT_ASSERT(!comparator.lessBySize(info1.fareClass(), info2.fareClass(), 2));
    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::TRUE);

    info1.fareClass() = "ABE";
    info2.fareClass() = "ABD";

    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::FALSE);

    info1.fareClass() = "CBE";
    info2.fareClass() = "CBE/Y456";
    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::EQUAL);

    info1.fareClass() = "CBF";
    info2.fareClass() = "CBE";
    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc != Comparator::EQUAL);

    info1.fareClass() = "C1F/687xx";
    info2.fareClass() = "CBE";
    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::TRUE);

    info1.fareClass() = "C1F";
    info2.fareClass() = "C17";
    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::FALSE);
  }

  void testSizeInequality()
  {
    FareBasisComparator comparator;
    comparator.fdsItem() = fdsNoList;

    Fare fare1, fare2;
    FareInfo info1, info2;

    info1.fareClass() = "DB";
    info2.fareClass() = "D";

    fare1.setFareInfo(&info1);
    fare2.setFareInfo(&info2);

    pFare1->setFare(&fare1);
    pFare2->setFare(&fare2);

    Comparator::Result rc = Comparator::EQUAL;

    rc = comparator.compare(*pFare1, *pFare2);

    CPPUNIT_ASSERT(rc == Comparator::FALSE);

    info1.fareClass() = "DB";
    info2.fareClass() = "DBB";
    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::TRUE);

    info1.fareClass() = "Y";
    info2.fareClass() = "C";
    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::FALSE);
  }

  void testAscendingDescendingAscendingComb()
  {
    FareBasisComparator comparator;
    comparator.fdsItem() = fdswithList;

    Fare fare1, fare2;
    FareInfo info1, info2;

    info1.fareClass() = "DBA";
    info2.fareClass() = "DBB";

    fare1.setFareInfo(&info1);
    fare2.setFareInfo(&info2);

    pFare1->setFare(&fare1);
    pFare2->setFare(&fare2);

    Comparator::Result rc = Comparator::EQUAL;

    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::TRUE);

    info1.fareClass() = "MC4";
    info2.fareClass() = "BC4";

    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::TRUE);

    info1.fareClass() = "PMT";
    info2.fareClass() = "XMT/Y456";

    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::FALSE);

    info1.fareClass() = "PCY";
    info2.fareClass() = "PQY";
    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::TRUE);
  }

  void testRemoveCorpID()
  {
    FareBasisComparator comparator;
    const FareClassCode s1 = "ABC/56JF";
    FareClassCode s2, s4, s6;
    comparator.removeCorpId(s1, s2);
    CPPUNIT_ASSERT(s2.size() == 3);
    CPPUNIT_ASSERT(s2 == "ABC");

    const FareClassCode s3 = "XXXYYY";
    comparator.removeCorpId(s3, s4);
    CPPUNIT_ASSERT(s4.size() == 6);
    CPPUNIT_ASSERT(s4 == s3);
    CPPUNIT_ASSERT(s6 != s4);
  }

  void testUV39()
  {
    FareBasisComparator comparator;
    comparator.fdsItem() = fdsUV39;

    Fare fare1, fare2;
    FareInfo info1, info2;

    info1.fareClass() = "F";
    info2.fareClass() = "FR";

    fare1.setFareInfo(&info1);
    fare2.setFareInfo(&info2);

    pFare1->setFare(&fare1);
    pFare2->setFare(&fare2);

    Comparator::Result rc = Comparator::EQUAL;

    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::FALSE);

    info1.fareClass() = "FR";
    info2.fareClass() = "F";
    rc = comparator.compare(*pFare1, *pFare2);
    CPPUNIT_ASSERT(rc == Comparator::TRUE);
  }

private:
  PaxTypeFare* pFare1;
  PaxTypeFare* pFare2;
  FDSSorting* fdsNoList;
  FDSSorting* fdswithList;
  FDSSorting* fdswithBlankList;
  FDSSorting* fdsUV39;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareBasisComparatorTest);
}
