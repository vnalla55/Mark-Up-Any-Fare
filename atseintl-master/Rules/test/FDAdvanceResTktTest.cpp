#include "test/include/CppUnitHelperMacros.h"
#include <string>
#include "Rules/FDAdvanceResTkt.h"
#include "DataModel/FareDisplayRequest.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DataModel/FareDisplayTrx.h"
#include "Rules/RuleConst.h"

namespace tse
{
class FDAdvanceResTktTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FDAdvanceResTktTest);
  CPPUNIT_TEST(testNeedReturnSegment);
  CPPUNIT_TEST(testNeedReturnSegmentFailsWhenNotIsOneWayDoubled);
  CPPUNIT_TEST(testNeedReturnSegmentFailsWhenInvalidReturnDate);
  CPPUNIT_TEST_SUITE_END();

public:
  FDAdvanceResTkt fdAdvRes;

  void testNeedReturnSegment()
  {
    Indicator owrt = ONE_WAY_MAY_BE_DOUBLED;
    FareDisplayRequest request;
    request.returnDate() = DateTime::localTime();
    FareDisplayTrx fdtrx;
    fdtrx.setRequest(&request);
    CPPUNIT_ASSERT(fdAdvRes.needReturnSegment(fdtrx, owrt));
  }

  void testNeedReturnSegmentFailsWhenNotIsOneWayDoubled()
  {
    Indicator owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    FareDisplayRequest request;
    request.returnDate() = DateTime::localTime();
    FareDisplayTrx fdtrx;
    fdtrx.setRequest(&request);
    CPPUNIT_ASSERT(!fdAdvRes.needReturnSegment(fdtrx, owrt));
  }

  void testNeedReturnSegmentFailsWhenInvalidReturnDate()
  {
    Indicator owrt = ONE_WAY_MAY_BE_DOUBLED;
    FareDisplayRequest request;
    FareDisplayTrx fdtrx;
    fdtrx.setRequest(&request);
    CPPUNIT_ASSERT(!fdAdvRes.needReturnSegment(fdtrx, owrt));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FDAdvanceResTktTest);
} // tse
