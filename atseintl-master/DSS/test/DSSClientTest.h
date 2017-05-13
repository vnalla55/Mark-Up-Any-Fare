#ifndef DSS_CLIENT_TEST_H
#define DSS_CLIENT_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

namespace tse
{
class DSSClientTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DSSClientTest);
  CPPUNIT_TEST(testgetScheduleCount);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();
  void testgetScheduleCount();

private:
};
}

#endif
