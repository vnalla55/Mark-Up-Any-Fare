#include "test/include/CppUnitHelperMacros.h"
#include <string>

#include "DBAccess/TariffCrossRefInfo.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "Rules/FDAdvanceResTkt.h"
#include "Rules/RuleConst.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class FcDispFareUsageTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FcDispFareUsageTest);
  CPPUNIT_TEST(testNeedReturnSegment);
  CPPUNIT_TEST(testNeedReturnSegmentFailsWhenNotIsOneWayDoubled);
  CPPUNIT_TEST(testNeedReturnSegmentFailsWhenInvalidReturnDate);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fdAdvRes = _memHandle.create<FDAdvanceResTkt>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testNeedReturnSegment()
  {
    Indicator owrt = ONE_WAY_MAY_BE_DOUBLED;
    FareDisplayRequest request;
    request.returnDate() = DateTime::localTime();
    FareDisplayTrx fdtrx;
    fdtrx.setRequest(&request);
    CPPUNIT_ASSERT(_fdAdvRes->needReturnSegment(fdtrx, owrt));
  }

  void testNeedReturnSegmentFailsWhenNotIsOneWayDoubled()
  {
    Indicator owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    FareDisplayRequest request;
    request.returnDate() = DateTime::localTime();
    FareDisplayTrx fdtrx;
    fdtrx.setRequest(&request);
    CPPUNIT_ASSERT(!_fdAdvRes->needReturnSegment(fdtrx, owrt));
  }

  void testNeedReturnSegmentFailsWhenInvalidReturnDate()
  {
    Indicator owrt = ONE_WAY_MAY_BE_DOUBLED;
    FareDisplayRequest request;
    FareDisplayTrx fdtrx;
    fdtrx.setRequest(&request);
    CPPUNIT_ASSERT(!_fdAdvRes->needReturnSegment(fdtrx, owrt));
  }
private:
  TestMemHandle _memHandle;
  FDAdvanceResTkt* _fdAdvRes;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FcDispFareUsageTest);
};
