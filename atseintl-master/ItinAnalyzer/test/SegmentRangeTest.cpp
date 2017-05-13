#include "ItinAnalyzer/SegmentRange.h"

#include "test/include/CppUnitHelperMacros.h"

namespace tse
{
class SegmentRangeTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(SegmentRangeTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testContains);
  CPPUNIT_TEST(testIntersects);
  CPPUNIT_TEST(testGetIntersection);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor();
  void testContains();
  void testIntersects();
  void testGetIntersection();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SegmentRangeTest);

void
SegmentRangeTest::testConstructor()
{
  SegmentRange range1(0, 1);
  CPPUNIT_ASSERT_EQUAL(size_t(0), range1.getStartIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(1), range1.getEndIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(1), range1.getSize());

  SegmentRange range2(1, 1);
  CPPUNIT_ASSERT_EQUAL(size_t(1), range2.getStartIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(1), range2.getEndIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(0), range2.getSize());

  SegmentRange range3(1, 4);
  CPPUNIT_ASSERT_EQUAL(size_t(1), range3.getStartIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(4), range3.getEndIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(3), range3.getSize());
}

void
SegmentRangeTest::testContains()
{
  SegmentRange range1(0, 3);
  SegmentRange range2(1, 3);
  SegmentRange range3(2, 4);
  SegmentRange range4(0, 2);
  CPPUNIT_ASSERT(range1.contains(range2));
  CPPUNIT_ASSERT(!range2.contains(range1));
  CPPUNIT_ASSERT(!range1.contains(range3));
  CPPUNIT_ASSERT(!range3.contains(range1));
  CPPUNIT_ASSERT(!range4.contains(range3));
}

void
SegmentRangeTest::testIntersects()
{
  SegmentRange range1(0, 3);
  SegmentRange range2(0, 2);
  SegmentRange range3(3, 5);
  SegmentRange range4(2, 5);

  CPPUNIT_ASSERT(range1.intersects(range2));
  CPPUNIT_ASSERT(range2.intersects(range1));
  CPPUNIT_ASSERT(!range1.intersects(range3));
  CPPUNIT_ASSERT(!range3.intersects(range1));
  CPPUNIT_ASSERT(range1.intersects(range4));
  CPPUNIT_ASSERT(range4.intersects(range1));
}

void
SegmentRangeTest::testGetIntersection()
{
  SegmentRange range1(0, 4);
  SegmentRange range2(0, 3);
  SegmentRange range3(2, 5);
  SegmentRange range4(3, 5);

  SegmentRange range = range1.getIntersection(range2);
  CPPUNIT_ASSERT_EQUAL(range2, range);

  range = range2.getIntersection(range1);
  CPPUNIT_ASSERT_EQUAL(range2, range);

  range = range2.getIntersection(range3);
  CPPUNIT_ASSERT_EQUAL(size_t(2), range.getStartIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(3), range.getEndIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(1), range.getSize());

  range = range3.getIntersection(range2);
  CPPUNIT_ASSERT_EQUAL(size_t(2), range.getStartIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(3), range.getEndIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(1), range.getSize());

  range = range2.getIntersection(range4);
  CPPUNIT_ASSERT_EQUAL(size_t(0), range.getStartIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(0), range.getEndIdx());
  CPPUNIT_ASSERT_EQUAL(size_t(0), range.getSize());
}

} // namespace tse
