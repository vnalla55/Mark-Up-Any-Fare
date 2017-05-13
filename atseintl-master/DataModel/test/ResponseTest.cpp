#include "DataModel/Response.h"
#include "Common/TseConsts.h"
#include "DBAccess/Loc.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;
namespace tse
{
class ResponseTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ResponseTest);
  CPPUNIT_TEST(testSetGet);
  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testSet()
  //---------------------------------------------------------------------
  void testSetGet()
  {

    Response response;

    response.output() = "ABC";

    CPPUNIT_ASSERT(response.output() == "ABC");
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(ResponseTest);
}
