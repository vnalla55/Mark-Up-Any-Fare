#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/Fare.h"
#include "DataModel/PricingTrx.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FareTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(FareTest);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    //  _memHandle.create<TestConfigInitializer>();
  }

  void tearDown()
  {
    //  _memHandle.clear();
  }

private:
  //  TestMemHandle _memHandle;
  PricingTrx _trx;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareTest);
}
