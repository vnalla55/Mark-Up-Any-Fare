//----------------------------------------------------------------------------
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "Pricing/PU.h"
#include "Pricing/MergedFareMarket.h"
#include "DataModel/PricingUnit.h"

using namespace std;

namespace tse
{
class PUTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PUTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testOperatorlessThanFailIfEqual);
  CPPUNIT_TEST(testOperatorlessThanPassForPuType);
  CPPUNIT_TEST(testOperatorlessThanPassForPuSubType);
  CPPUNIT_TEST(testOperatorlessThanPassForDirectionality);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    PU pu;
    CPPUNIT_ASSERT(PricingUnit::Type::UNKNOWN == pu.puType());
    CPPUNIT_ASSERT_EQUAL(PricingUnit::UNKNOWN_SUBTYPE, pu.puSubType());
    CPPUNIT_ASSERT_EQUAL(PricingUnit::NOT_CHECKED, pu.ojSurfaceStatus());
    CPPUNIT_ASSERT(GeoTravelType::UnknownGeoTravelType == pu.itinGeoTvlType());
  }

  void testOperatorlessThanFailIfEqual()
  {
    PU pu1, pu2;
    setupMatchingPU(pu1, pu2);
    CPPUNIT_ASSERT(!(pu1 < pu2));
  }

  void testOperatorlessThanPassForPuType()
  {
    PU pu1, pu2;
    setupMatchingPU(pu1, pu2);
    pu1.puType() = PricingUnit::Type::OPENJAW;
    CPPUNIT_ASSERT(pu1 < pu2);
  }

  void testOperatorlessThanPassForPuSubType()
  {
    PU pu1, pu2;
    setupMatchingPU(pu1, pu2);
    pu1.puSubType() = PricingUnit::ORIG_OPENJAW;
    CPPUNIT_ASSERT(pu1 < pu2);
  }

  void testOperatorlessThanPassForDirectionality()
  {
    PU pu1, pu2;
    setupMatchingPU(pu1, pu2);
    pu1.fareDirectionality().clear();
    pu1.fareDirectionality().push_back(FROM);
    CPPUNIT_ASSERT(pu1 < pu2);
  }

  void setupMatchingPU(PU& pu1, PU& pu2)
  {
    MergedFareMarket* fm1 = new MergedFareMarket();
    pu1.fareMarket().push_back(fm1);
    pu2.fareMarket().push_back(fm1);
    pu1.fareDirectionality().push_back(WITHIN);
    pu2.fareDirectionality().push_back(WITHIN);
    pu1.puType() = PricingUnit::Type::ROUNDTRIP;
    pu2.puType() = PricingUnit::Type::ROUNDTRIP;
    pu1.puSubType() = PricingUnit::DOUBLE_OPENJAW;
    pu2.puSubType() = PricingUnit::DOUBLE_OPENJAW;
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(PUTest);
}
