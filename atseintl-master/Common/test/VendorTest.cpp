#include "test/include/CppUnitHelperMacros.h"
#include "Common/Vendor.h"

using namespace std;
namespace tse
{
class VendorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(VendorTest);
  CPPUNIT_TEST(testDisplayChar);
  CPPUNIT_TEST_SUITE_END();

  void testDisplayChar()
  {
    CPPUNIT_ASSERT_EQUAL('A', Vendor::displayChar(Vendor::ATPCO));
    CPPUNIT_ASSERT_EQUAL('S', Vendor::displayChar(Vendor::SITA));
    CPPUNIT_ASSERT_EQUAL('S', Vendor::displayChar(Vendor::SABRE));
    CPPUNIT_ASSERT_EQUAL('F', Vendor::displayChar(Vendor::FMS));
    CPPUNIT_ASSERT_EQUAL('W', Vendor::displayChar(Vendor::WN));
    CPPUNIT_ASSERT_EQUAL('P', Vendor::displayChar(Vendor::POFO));
    CPPUNIT_ASSERT_EQUAL('D', Vendor::displayChar(Vendor::SABD));
    CPPUNIT_ASSERT_EQUAL('H', Vendor::displayChar(Vendor::HIKE));
    CPPUNIT_ASSERT_EQUAL('*', Vendor::displayChar(Vendor::EMPTY));
    // unknown values should default to *
    CPPUNIT_ASSERT_EQUAL('*', Vendor::displayChar("test"));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(VendorTest);
};
