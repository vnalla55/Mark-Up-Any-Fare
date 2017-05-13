#include "test/include/CppUnitHelperMacros.h"

#include "DBAccess/OptionalServicesInfo.h"
#include "FreeBagService/BaggageAncillaryChargesValidator.h"
#include "FreeBagService/test/S7Builder.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class BaggageAncillaryChargesValidatorByOccurrenceFilterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageAncillaryChargesValidatorByOccurrenceFilterTest);

  CPPUNIT_TEST(testReset);
  CPPUNIT_TEST(testSelect_ContinuousSequence);
  CPPUNIT_TEST(testSelect_CurrentFirstPcGreaterMoreThatOneThatPrevLastPc);
  CPPUNIT_TEST(testSelect_CurrentFirstPcSmallerThatPrevLastPc);
  CPPUNIT_TEST(testSelect_BlankBlankAtEnd_ContinuousSequence);
  CPPUNIT_TEST(testSelect_BlankBlankAtEnd_NonContinuousSequence);
  CPPUNIT_TEST(testSelect_SequenceWithBlank);
  CPPUNIT_TEST(testSelect_SequenceWithBlankAndBlankBlank);

  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
  BaggageAncillaryChargesValidator::ByOccurrenceFilter _filter;

  void testReset()
  {
    _filter.reset();

    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_FIRST,
                         _filter._state);
  }

  void testSelect_ContinuousSequence()
  {
    const OptionalServicesInfo* s7[3];

    s7[0] = S7Builder(&_memHandle).withSequenceNo(1).withBaggageOccurrence(1, 1).build();

    s7[1] = S7Builder(&_memHandle).withSequenceNo(2).withBaggageOccurrence(2, 3).build();

    s7[2] = S7Builder(&_memHandle).withSequenceNo(3).withBaggageOccurrence(4, 9).build();

    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[0]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_SEQUENCE,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[1]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_SEQUENCE,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[2]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_SEQUENCE,
                         _filter._state);
  }

  void testSelect_CurrentFirstPcSmallerThatPrevLastPc()
  {
    const OptionalServicesInfo* s7[2];

    s7[0] = S7Builder(&_memHandle).withSequenceNo(2).withBaggageOccurrence(2, 3).build();

    s7[1] = S7Builder(&_memHandle).withSequenceNo(3).withBaggageOccurrence(1, 1).build();

    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[0]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_SEQUENCE,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(false, _filter.select(*s7[1]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_BLANKBLANK,
                         _filter._state);
  }

  void testSelect_CurrentFirstPcGreaterMoreThatOneThatPrevLastPc()
  {
    const OptionalServicesInfo* s7[2];

    s7[0] = S7Builder(&_memHandle).withSequenceNo(1).withBaggageOccurrence(2, 3).build();

    s7[1] = S7Builder(&_memHandle).withSequenceNo(2).withBaggageOccurrence(5, 6).build();

    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[0]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_SEQUENCE,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(false, _filter.select(*s7[1]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_BLANKBLANK,
                         _filter._state);
  }

  void testSelect_BlankBlankAtEnd_ContinuousSequence()
  {
    const OptionalServicesInfo* s7[3];

    s7[0] = S7Builder(&_memHandle).withSequenceNo(1).withBaggageOccurrence(1, 1).build();

    s7[1] = S7Builder(&_memHandle).withSequenceNo(2).withBaggageOccurrence(2, 3).build();

    s7[2] = S7Builder(&_memHandle).withSequenceNo(3).withBaggageOccurrence(-1, -1).build();

    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[0]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_SEQUENCE,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[1]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_SEQUENCE,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[2]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::SKIP,
                         _filter._state);
  }

  void testSelect_BlankBlankAtEnd_NonContinuousSequence()
  {
    const OptionalServicesInfo* s7[3];

    s7[0] = S7Builder(&_memHandle).withSequenceNo(1).withBaggageOccurrence(1, 1).build();

    s7[1] = S7Builder(&_memHandle).withSequenceNo(2).withBaggageOccurrence(1, 8).build();

    s7[2] = S7Builder(&_memHandle).withSequenceNo(3).withBaggageOccurrence(-1, -1).build();

    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[0]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_SEQUENCE,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(false, _filter.select(*s7[1]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_BLANKBLANK,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[2]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::SKIP,
                         _filter._state);
  }

  void testSelect_SequenceWithBlank()
  {
    const OptionalServicesInfo* s7[3];

    s7[0] = S7Builder(&_memHandle).withSequenceNo(1).withBaggageOccurrence(1, 1).build();

    s7[1] = S7Builder(&_memHandle).withSequenceNo(2).withBaggageOccurrence(2, -1).build();

    s7[2] = S7Builder(&_memHandle).withSequenceNo(3).withBaggageOccurrence(3, 4).build();

    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[0]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_SEQUENCE,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[1]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_BLANKBLANK,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(false, _filter.select(*s7[2]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_BLANKBLANK,
                         _filter._state);
  }

  void testSelect_SequenceWithBlankAndBlankBlank()
  {
    const OptionalServicesInfo* s7[4];

    s7[0] = S7Builder(&_memHandle).withSequenceNo(1).withBaggageOccurrence(1, 1).build();

    s7[1] = S7Builder(&_memHandle).withSequenceNo(2).withBaggageOccurrence(2, -1).build();

    s7[2] = S7Builder(&_memHandle).withSequenceNo(3).withBaggageOccurrence(3, 4).build();

    s7[3] = S7Builder(&_memHandle).withSequenceNo(4).withBaggageOccurrence(-1, -1).build();

    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[0]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_SEQUENCE,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[1]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_BLANKBLANK,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(false, _filter.select(*s7[2]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::MATCHING_BLANKBLANK,
                         _filter._state);
    CPPUNIT_ASSERT_EQUAL(true, _filter.select(*s7[3]));
    CPPUNIT_ASSERT_EQUAL(BaggageAncillaryChargesValidator::ByOccurrenceFilter::SKIP,
                         _filter._state);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaggageAncillaryChargesValidatorByOccurrenceFilterTest);
}
