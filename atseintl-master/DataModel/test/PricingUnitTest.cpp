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
#include "DBAccess/DataHandle.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"

namespace tse
{

class PricingUnitTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingUnitTest);
  CPPUNIT_TEST(testIsAnyFareUsageAcrossTurnaroundPoint);
  CPPUNIT_TEST_SUITE_END();

public:
  void testIsAnyFareUsageAcrossTurnaroundPoint()
  {
    CPPUNIT_ASSERT(!makePricingUnit(false, false)->isAnyFareUsageAcrossTurnaroundPoint());
    CPPUNIT_ASSERT(makePricingUnit(true, false)->isAnyFareUsageAcrossTurnaroundPoint());
    CPPUNIT_ASSERT(makePricingUnit(false, true)->isAnyFareUsageAcrossTurnaroundPoint());
    CPPUNIT_ASSERT(makePricingUnit(true, true)->isAnyFareUsageAcrossTurnaroundPoint());
  }

private:
  tse::DataHandle _allocator;

  FareUsage* makeFareUsage(bool acrossTurnaroundPoint)
  {
    FareUsage* result(&_allocator.safe_create<FareUsage>());
    AirSeg* seg1(&_allocator.safe_create<AirSeg>());
    AirSeg* seg2(&_allocator.safe_create<AirSeg>());
    seg1->legId() = 0;
    seg2->legId() = acrossTurnaroundPoint ? 1 : 0;
    result->travelSeg().push_back(seg1);
    result->travelSeg().push_back(seg2);
    return result;
  }

  PricingUnit* makePricingUnit(bool acrossTurnaroundPoint1, bool acrossTurnaroundPoint2)
  {
    PricingUnit* result(&_allocator.safe_create<PricingUnit>());
    result->fareUsage().push_back(makeFareUsage(acrossTurnaroundPoint1));
    result->fareUsage().push_back(makeFareUsage(acrossTurnaroundPoint2));
    return result;
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(PricingUnitTest);
}
