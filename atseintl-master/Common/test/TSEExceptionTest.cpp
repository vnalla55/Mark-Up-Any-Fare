#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Common/TSEException.h"

using namespace std;
namespace tse
{
class TSEExceptionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TSEExceptionTest);
  CPPUNIT_TEST(testConstructorWithErrorCodeAndMessage);
  CPPUNIT_TEST(testConstructorWithErrorCode0AndNullMessage);
  CPPUNIT_TEST(testConstructorWithErrorCode1AndNullMessage);
  CPPUNIT_TEST(testConstructorWithErrorCode2AndNullMessage);
  CPPUNIT_TEST(testConstructorWithErrorCode3AndNullMessage);
  CPPUNIT_TEST(testConstructorWithErrorCode4AndNullMessage);
  CPPUNIT_TEST(testCopyConstructor);
  CPPUNIT_TEST(testOperatorEquals);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

private:
  void testConstructorWithErrorCodeAndMessage()
  {
    string msg("XYZ Unkown");
    try { throw TSEException(TSEException::UNKNOWN_TICKETING_CODE_MODIFIER, msg.c_str()); }
    catch (TSEException& ex)
    {
      CPPUNIT_ASSERT(ex.what() == msg);
      CPPUNIT_ASSERT(ex.where() != 0);
    }
  }

  void testConstructorWithErrorCode0AndNullMessage()
  {
    TSEException ex = TSEException(TSEException::NO_ERROR);

    CPPUNIT_ASSERT_EQUAL((std::string) "", (std::string)ex.what());
  }

  void testConstructorWithErrorCode1AndNullMessage()
  {
    TSEException ex = TSEException(TSEException::SITA_FIELD_REQUESTED_FROM_ATPCO_FARE);

    CPPUNIT_ASSERT_EQUAL((std::string) "SITA FIELD REQUESTED FROM ATPCO FARE",
                         (std::string)ex.what());
  }

  void testConstructorWithErrorCode2AndNullMessage()
  {
    TSEException ex = TSEException(TSEException::UNKNOWN_TICKETING_CODE_MODIFIER);

    CPPUNIT_ASSERT_EQUAL((std::string) "UNKNOWN TICKETING CODE MODIFIER", (std::string)ex.what());
  }

  void testConstructorWithErrorCode3AndNullMessage()
  {
    TSEException ex = TSEException(TSEException::INVALID_FLIGHT_APPLICATION_DATA);

    CPPUNIT_ASSERT_EQUAL((std::string) "INVALID FLIGHT APPLICATION DATA", (std::string)ex.what());
  }

  void testConstructorWithErrorCode4AndNullMessage()
  {
    TSEException ex = TSEException(TSEException::UNKNOWN_DATABASE);

    CPPUNIT_ASSERT_EQUAL((std::string) "", (std::string)ex.what());
  }

  void testCopyConstructor()
  {
    TSEException ex = TSEException(TSEException::INVALID_FLIGHT_APPLICATION_DATA);
    TSEException ex2(ex);

    CPPUNIT_ASSERT_EQUAL((std::string) "INVALID FLIGHT APPLICATION DATA", (std::string)ex2.what());
    // where is not copied in the copy constructor
    CPPUNIT_ASSERT(!((std::string)ex.where() == (std::string)ex2.where()));
    CPPUNIT_ASSERT(&ex2 != &ex);
  }

  void testOperatorEquals()
  {
    TSEException ex = TSEException(TSEException::INVALID_FLIGHT_APPLICATION_DATA);
    TSEException ex2 = TSEException(TSEException::NO_ERROR);
    ex2 = ex;

    // Limited public values that can be validated
    CPPUNIT_ASSERT_EQUAL((std::string) "INVALID FLIGHT APPLICATION DATA", (std::string)ex2.what());
    CPPUNIT_ASSERT_EQUAL((std::string)ex.where(), (std::string)ex2.where());
    CPPUNIT_ASSERT(&ex2 != &ex);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TSEExceptionTest);
}
