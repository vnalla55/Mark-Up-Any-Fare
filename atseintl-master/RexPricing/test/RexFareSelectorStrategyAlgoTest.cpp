#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "RexPricing/test/RexFareSelectorStrategyTestUtils.h"
#include "RexPricing/RexFareSelectorStrategyAlgo.h"
#include "test/include/PrintCollection.h"

namespace tse
{

using boost::assign::operator+=;

class RexFareSelectorStrategyAlgoTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexFareSelectorStrategyAlgoTest);

  CPPUNIT_TEST(testCopyIteratorIf);
  CPPUNIT_TEST(testCopyIf);

  CPPUNIT_TEST(testStablePartition);
  CPPUNIT_TEST(testStablePartition_Begin);
  CPPUNIT_TEST(testStablePartition_End);

  CPPUNIT_TEST(testSequentialSelect_First);
  CPPUNIT_TEST(testSequentialSelect_Second);
  CPPUNIT_TEST(testSequentialSelect_None);

  CPPUNIT_TEST_SUITE_END();

public:
  void testCopyIteratorIf()
  {
    int t[] = { 1, 20, 3, 40, 5, 60 };
    std::vector<int*> sel;
    copyIteratorIf(t, t + 6, std::back_inserter(sel), std::bind2nd(std::less<int>(), 10));

    CPPUNIT_ASSERT_EQUAL(std::size_t(3), sel.size());

    CPPUNIT_ASSERT_EQUAL(t + 0, sel[0]);
    CPPUNIT_ASSERT_EQUAL(t + 2, sel[1]);
    CPPUNIT_ASSERT_EQUAL(t + 4, sel[2]);
    std::vector<int*> expect;
    expect += t, t + 2, t + 4;
    CPPUNIT_ASSERT_EQUAL(expect, sel);
  }

  void testCopyIf()
  {
    int t[] = { 1, 20, 3, 40, 5, 60 };
    std::vector<int> sel, expect;
    copyIf(t, t + 6, std::back_inserter(sel), std::bind2nd(std::less<int>(), 10));

    expect += 1, 3, 5;
    CPPUNIT_ASSERT_EQUAL(expect, sel);
  }

  void testStablePartition()
  {
    std::vector<int> sel, expect;
    sel += 1, 2, 3, 4, 5, 6, 7, 8, 9;
    expect += 1, 3, 5, 7, 9, 2, 4, 6, 8;

    std::vector<int>::iterator p =
        stablePartition(sel.begin(), sel.end(), std::bind2nd(std::modulus<int>(), 2));

    CPPUNIT_ASSERT_EQUAL(expect, sel);
    CPPUNIT_ASSERT(sel.begin() + 5 == p);
  }

  void testStablePartition_Begin()
  {
    std::vector<int> sel, expect;
    sel += 2, 4, 6, 8;
    expect = sel;

    std::vector<int>::iterator p =
        stablePartition(sel.begin(), sel.end(), std::bind2nd(std::modulus<int>(), 2));

    CPPUNIT_ASSERT_EQUAL(expect, sel);
    CPPUNIT_ASSERT(sel.begin() == p);
  }

  void testStablePartition_End()
  {
    std::vector<int> sel, expect;
    sel += 1, 3, 5, 7, 9;
    expect = sel;

    std::vector<int>::iterator p =
        stablePartition(sel.begin(), sel.end(), std::bind2nd(std::modulus<int>(), 2));

    CPPUNIT_ASSERT_EQUAL(expect, sel);
    CPPUNIT_ASSERT(sel.end() == p);
  }

  class CheckPercent : public std::unary_function<double, bool>
  {
  public:
    CheckPercent(double amount, double percent) : _percent(1.0 + percent), _amount(amount) {}

    bool operator()(double val) const { return (val > _amount) && (val < _amount * _percent); }

  protected:
    const double _percent, _amount;
  };

  static const double BASE;

  std::vector<double> populate()
  {
    std::vector<double> sel;
    sel += 0.9 * BASE, 1.03 * BASE, 1.09 * BASE, 1.13 * BASE, 1.15 * BASE, 1.25 * BASE, 1.5 * BASE;
    return sel;
  }

  void testSequentialSelect_First()
  {
    const double var[] = { 0.1, 0.2 };

    std::vector<double> result, expect, src = populate();

    expect += src[1], src[2];

    CPPUNIT_ASSERT(sequentialSelect<CheckPercent>(src.begin(), src.end(), BASE, var, result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSequentialSelect_Second()
  {
    const double var[] = { 0.01, 0.2 };

    std::vector<double> result, expect, src = populate();

    expect += src[1], src[2], src[3], src[4];

    CPPUNIT_ASSERT(sequentialSelect<CheckPercent>(src.begin(), src.end(), BASE, var, result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSequentialSelect_None()
  {
    const double var[] = { 0.01, 0.02 };

    std::vector<double> result, expect, src = populate();

    CPPUNIT_ASSERT(!sequentialSelect<CheckPercent>(src.begin(), src.end(), BASE, var, result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }
};

const double RexFareSelectorStrategyAlgoTest::BASE = 100.0;

CPPUNIT_TEST_SUITE_REGISTRATION(RexFareSelectorStrategyAlgoTest);

} // tse
