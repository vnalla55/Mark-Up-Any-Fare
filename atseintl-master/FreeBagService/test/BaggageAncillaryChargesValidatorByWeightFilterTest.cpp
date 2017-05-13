#include "test/include/CppUnitHelperMacros.h"

#include "DBAccess/OptionalServicesInfo.h"
#include "FreeBagService/BaggageAncillaryChargesValidator.h"
#include "FreeBagService/test/S7Builder.h"
#include "test/include/TestMemHandle.h"


namespace tse
{

class BaggageAncillaryChargesValidatorByWeightFilterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageAncillaryChargesValidatorByWeightFilterTest);

  CPPUNIT_TEST(testReset);
  CPPUNIT_TEST(testSelect_WeightZero);
  CPPUNIT_TEST(testSelect_WeightBlank_FirstTime);
  CPPUNIT_TEST(testSelect_WeightBlank_OtherTimes);
  CPPUNIT_TEST(testSelect_WeightLargerThanMaxWeight);
  CPPUNIT_TEST(testSelect_WeightSmallerThanMaxWeight);

  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
  BaggageAncillaryChargesValidator::ByWeightFilter _filter;

  void testReset()
  {
    _filter.reset();

    CPPUNIT_ASSERT_EQUAL(false, _filter._blankWeight);
    CPPUNIT_ASSERT_EQUAL(0, _filter._maxWeight);
  }

  void testSelect_WeightZero()
  {
    CPPUNIT_ASSERT_EQUAL(true,
                         _filter.select(S7Builder(&_memHandle).withBaggageWeight(0).buildRef()));
  }

  void testSelect_WeightBlank_FirstTime()
  {
    _filter._blankWeight = false;

    CPPUNIT_ASSERT_EQUAL(true,
                         _filter.select(S7Builder(&_memHandle).withBaggageWeight(-1).buildRef()));
    CPPUNIT_ASSERT_EQUAL(true, _filter._blankWeight);
  }

  void testSelect_WeightBlank_OtherTimes()
  {
    _filter._blankWeight = true;

    CPPUNIT_ASSERT_EQUAL(false,
                         _filter.select(S7Builder(&_memHandle).withBaggageWeight(-1).buildRef()));
    CPPUNIT_ASSERT_EQUAL(true, _filter._blankWeight);
  }

  void testSelect_WeightLargerThanMaxWeight()
  {
    _filter._maxWeight = 0;

    CPPUNIT_ASSERT_EQUAL(true,
                         _filter.select(S7Builder(&_memHandle).withBaggageWeight(10).buildRef()));
    CPPUNIT_ASSERT_EQUAL(false, _filter._blankWeight);
  }

  void testSelect_WeightSmallerThanMaxWeight()
  {
    _filter._maxWeight = 10;

    CPPUNIT_ASSERT_EQUAL(false,
                         _filter.select(S7Builder(&_memHandle).withBaggageWeight(1).buildRef()));
    CPPUNIT_ASSERT_EQUAL(false, _filter._blankWeight);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BaggageAncillaryChargesValidatorByWeightFilterTest);
}
