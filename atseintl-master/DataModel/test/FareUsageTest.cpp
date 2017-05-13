//
// Copyright Sabre 2012-03-08
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "DataModel/AirSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"

namespace tse
{
class FareUsageTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareUsageTest);
  CPPUNIT_TEST(testIsAcrossTurnaroundPoint);
  CPPUNIT_TEST(testGetNonRefundableAmt);
  CPPUNIT_TEST_SUITE_END();

  void testIsAcrossTurnaroundPoint()
  {
    {
      FareUsage fu;
      AirSeg seg1, seg2;
      seg1.legId() = 0;
      seg2.legId() = 1;
      fu.travelSeg().push_back(&seg1);
      fu.travelSeg().push_back(&seg2);
      CPPUNIT_ASSERT(fu.isAcrossTurnaroundPoint());
    }
    {
      FareUsage fu;
      AirSeg seg1, seg2;
      seg1.legId() = 1;
      seg2.legId() = 1;
      fu.travelSeg().push_back(&seg1);
      fu.travelSeg().push_back(&seg2);
      CPPUNIT_ASSERT(!fu.isAcrossTurnaroundPoint());
    }
  }

  void testGetNonRefundableAmt()
  {
    PricingTrx trx;
    FareUsage fareUsage;
    fareUsage.paxTypeFare() = _memHandle.create<PaxTypeFare>();
    fareUsage.paxTypeFare()->nucFareAmount() = 10;
    fareUsage.surchargeAmt() = 10;
    fareUsage.stopOverAmt() = 10;
    fareUsage.transferAmt() = 10;
    fareUsage.differentialAmt() = 10;
    CurrencyCode calculationCurrency = NUC;
    bool useInternationalRounding = false;

    Money nonRefundableAmt = fareUsage.getNonRefundableAmt(calculationCurrency,
                                                           trx,
                                                           useInternationalRounding);
    CPPUNIT_ASSERT_EQUAL(nonRefundableAmt.value(), 50.0);
    CPPUNIT_ASSERT(nonRefundableAmt.code() == NUC);

    calculationCurrency = USD;

    nonRefundableAmt = fareUsage.getNonRefundableAmt(calculationCurrency,
                                                     trx,
                                                     useInternationalRounding);
    CPPUNIT_ASSERT_EQUAL(nonRefundableAmt.value(), 40.0);
    CPPUNIT_ASSERT(nonRefundableAmt.code() == USD);
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareUsageTest);
}
