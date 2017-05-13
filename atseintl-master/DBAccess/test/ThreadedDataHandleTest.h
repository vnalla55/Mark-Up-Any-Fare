#ifndef THREADED_DATA_HANDLE_TEST_H
#define THREADED_DATA_HANDLE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

namespace tse
{

class ThreadedDataHandleTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(ThreadedDataHandleTest);
  CPPUNIT_TEST(testMultiThreaded);
  CPPUNIT_TEST_SUITE_END();

  MockDataManager* dm;

public:
  /** Retrieve Tax Code AA */
  void testMultiThreaded();

  // void setUp();

  // void tearDown();
};

} // namespace tse
#endif // THREADED_DATA_HANDLE_TEST_H
