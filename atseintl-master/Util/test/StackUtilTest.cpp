#include <iostream>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Util/StackUtil.h"

namespace tse
{
class StackUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StackUtilTest);
  CPPUNIT_TEST(testGetStackTrace);
  CPPUNIT_TEST(testGetRawStackTrace);
  CPPUNIT_TEST(testOutputStackTrace);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testGetStackTrace()
  {
    CPPUNIT_ASSERT(StackUtil::getStackTrace().find("testGetStackTrace") != std::string::npos);
  }

  void testGetRawStackTrace()
  {
    static const int MAX_DEPTH = 50;
    void* frames[MAX_DEPTH];

    const int depth = StackUtil::getRawStackTrace(frames, MAX_DEPTH);
    CPPUNIT_ASSERT(StackUtil::getStackTrace(frames, depth).find("testGetRawStackTrace") != std::string::npos);
  }

  void testOutputStackTrace()
  {
    int mypipe[2];
    pipe(mypipe);

    int savstdout = dup(STDERR_FILENO); // save original sterr
    dup2(mypipe[1], STDERR_FILENO);

    StackUtil::outputStackTrace();
    fflush(stderr);

    dup2(savstdout, STDERR_FILENO); // restore original sterr

    char buf[2048];
    read(mypipe[0], buf, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    CPPUNIT_ASSERT(strstr(buf, "testOutputStackTrace") > buf);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(StackUtilTest);
}
