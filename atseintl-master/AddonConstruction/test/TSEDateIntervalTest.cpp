#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "test/include/CppUnitHelperMacros.h"

#include "DBAccess/TSEDateInterval.h"

using namespace std;
namespace tse
{
class TSEDateIntervalTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TSEDateIntervalTest);
  CPPUNIT_TEST(testDateInterval);
  CPPUNIT_TEST_SUITE_END();

public:
  void testDateInterval()
  {
    TSEDateInterval i1, i2, r;

    //   |=== i1 ===|
    //                |=== i2 ===|

    i1.effDate() = DateTime(2005, 3, 5);
    i1.expireDate() = DateTime(2005, 3, 10);
    i1.discDate() = DateTime(2005, 3, 8);

    i2.effDate() = DateTime(2005, 3, 15);
    i2.expireDate() = DateTime(2005, 3, 25);
    i2.discDate() = DateTime(2005, 3, 28);

    CPPUNIT_ASSERT(!r.defineIntersection(i1, i2));

    //   |=== i1 ===|
    //              |=== i2 ===|

    i1.effDate() = DateTime(2005, 3, 5);
    i1.expireDate() = DateTime(2005, 3, 10);
    i1.discDate() = DateTime(2005, 3, 8);

    i2.effDate() = DateTime(2005, 3, 8);
    i2.expireDate() = DateTime(2005, 3, 25);
    i2.discDate() = DateTime(2005, 3, 28);

    CPPUNIT_ASSERT(r.defineIntersection(i1, i2));

    //   |=== i1 ======|
    //              |=== i2 ===|

    i1.effDate() = DateTime(2005, 3, 5);
    i1.expireDate() = DateTime(2005, 3, 10);
    i1.discDate() = DateTime(2005, 3, 12);

    i2.effDate() = DateTime(2005, 3, 8);
    i2.expireDate() = DateTime(2005, 3, 25);
    i2.discDate() = DateTime(2005, 3, 28);

    CPPUNIT_ASSERT(r.defineIntersection(i1, i2));

    //   |=== i1 ==============|
    //              |=== i2 ===|

    i1.effDate() = DateTime(2005, 3, 5);
    i1.expireDate() = DateTime(2005, 3, 27);
    i1.discDate() = DateTime(2005, 3, 25);

    i2.effDate() = DateTime(2005, 3, 8);
    i2.expireDate() = DateTime(2005, 3, 25);
    i2.discDate() = DateTime(2005, 3, 28);

    CPPUNIT_ASSERT(r.defineIntersection(i1, i2));

    //   |=== i1 ==================|
    //              |=== i2 ===|

    i1.effDate() = DateTime(2005, 3, 5);
    i1.expireDate() = DateTime(2005, 4, 27);
    i1.discDate() = DateTime(2005, 4, 25);

    i2.effDate() = DateTime(2005, 3, 8);
    i2.expireDate() = DateTime(2005, 3, 25);
    i2.discDate() = DateTime(2005, 3, 28);

    CPPUNIT_ASSERT(r.defineIntersection(i1, i2));

    //              |=== i1 =========|
    //              |=== i2 ===|

    i1.effDate() = DateTime(2005, 3, 8);
    i1.expireDate() = DateTime(2005, 4, 27);
    i1.discDate() = DateTime(2005, 4, 25);

    i2.effDate() = DateTime(2005, 3, 8);
    i2.expireDate() = DateTime(2005, 3, 25);
    i2.discDate() = DateTime(2005, 3, 28);

    CPPUNIT_ASSERT(r.defineIntersection(i1, i2));

    //                  |=== i1 ======|
    //              |=== i2 ===|

    i1.effDate() = DateTime(2005, 3, 15);
    i1.expireDate() = DateTime(2005, 4, 27);
    i1.discDate() = DateTime(2005, 4, 25);

    i2.effDate() = DateTime(2005, 3, 8);
    i2.expireDate() = DateTime(2005, 3, 25);
    i2.discDate() = DateTime(2005, 3, 28);

    CPPUNIT_ASSERT(r.defineIntersection(i1, i2));

    //                         |=== i1 ===|
    //              |=== i2 ===|

    i1.effDate() = DateTime(2005, 3, 25);
    i1.expireDate() = DateTime(2005, 4, 27);
    i1.discDate() = DateTime(2005, 4, 25);

    i2.effDate() = DateTime(2005, 3, 8);
    i2.expireDate() = DateTime(2005, 3, 25);
    i2.discDate() = DateTime(2005, 3, 28);

    CPPUNIT_ASSERT(r.defineIntersection(i1, i2));

    //                            |=== i1 ===|
    //              |=== i2 ===|

    i1.effDate() = DateTime(2005, 3, 26);
    i1.expireDate() = DateTime(2005, 4, 27);
    i1.discDate() = DateTime(2005, 4, 25);

    i2.effDate() = DateTime(2005, 3, 8);
    i2.expireDate() = DateTime(2005, 3, 25);
    i2.discDate() = DateTime(2005, 3, 28);

    CPPUNIT_ASSERT(!r.defineIntersection(i1, i2));
    /*
     cout << "\ni1.effDate()" << i1.effDate() << "\n"
     << "i1.expireDate()" << i1.expireDate() << "\n"
     << "i1.discDate()" << i1.discDate() << "\n\n";

     cout << "i2.effDate()" << i2.effDate() << "\n"
     << "i2.expireDate()" << i2.expireDate() << "\n"
     << "i2.discDate()" << i2.discDate() << "\n\n";

     TSEDateInterval r2;
     r.clone( r2 );

     cout << "r2.effDate()" << r.effDate() << "\n"
     << "r2.expireDate()" << r.expireDate() << "\n"
     << "r2.discDate()" << r2.discDate() << endl;
     */
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TSEDateIntervalTest);
}
