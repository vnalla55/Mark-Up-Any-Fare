#include "test/include/CppUnitHelperMacros.h"
#include "Common/ErrorResponseException.h"

using namespace tse;
using namespace std;

namespace tse
{
class ErrorResponseExceptionTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(ErrorResponseExceptionTest);
  CPPUNIT_TEST(testThrow);
  CPPUNIT_TEST(testThrowWithMessage);
  CPPUNIT_TEST_SUITE_END();

public:
  void testThrow()
  {
    // Try not setting msg
    try { throw ErrorResponseException(ErrorResponseException::NO_FARE_REQUESTED); }
    catch (ErrorResponseException& ere)
    {
      CPPUNIT_ASSERT(ere.code() == ErrorResponseException::NO_FARE_REQUESTED);
      CPPUNIT_ASSERT(ere.message() == "NO FARE REQUESTED");
    }
  }

  void testThrowWithMessage()
  {
    // Now override the msg
    string mymsg("My own message");
    try { throw ErrorResponseException(ErrorResponseException::NO_FARE_REQUESTED, mymsg.c_str()); }
    catch (ErrorResponseException& ere)
    {
      CPPUNIT_ASSERT(ere.code() == ErrorResponseException::NO_FARE_REQUESTED);
      CPPUNIT_ASSERT(ere.message() == mymsg);
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ErrorResponseExceptionTest);
}
