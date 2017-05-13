#ifndef __NOPNR_PRICING_TRX_TEST_H__
#define __NOPNR_PRICING_TRX_TEST_H__

#include <cppunit/TestFixture.h>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class NoPNRPricingTrx;

class NoPNRPricingTrxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NoPNRPricingTrxTest);
  CPPUNIT_TEST(testIsNoMatchXMCase);
  CPPUNIT_TEST(testIsNoMatchRBDCase);
  CPPUNIT_TEST(testIsNoMatchReprocessCase);

  CPPUNIT_TEST(testFTGroupFromFareTypeEmpty);
  CPPUNIT_TEST(testFTGroupFromFareTypeNotFound);
  CPPUNIT_TEST(testFTGroupFromFareTypeFound);

  CPPUNIT_TEST(testSolutionsMax);
  CPPUNIT_TEST(testSolutionsLimit);
  CPPUNIT_TEST(testSolutionsLimitWithFound);
  CPPUNIT_TEST(testSolutionsNone);
  CPPUNIT_TEST(testSolutionsAll);

  CPPUNIT_TEST(testIsXMMatch);
  CPPUNIT_TEST(testIsXMNoMatch);

  CPPUNIT_TEST(testPrepareNoMatchItinEmpty);
  CPPUNIT_TEST(testPrepareNoMatchItinOneSeg);
  CPPUNIT_TEST(testPrepareNoMatchItinManySegs);
  CPPUNIT_TEST(testPrepareNoMatchItinFilled);

  CPPUNIT_TEST(testSetFullFBCItinNo);
  CPPUNIT_TEST(testSetFullFBCItinNoPartially);
  CPPUNIT_TEST(testSetFullFBCItinYes);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testIsNoMatchXMCase();
  void testIsNoMatchRBDCase();
  void testIsNoMatchReprocessCase();

  void testFTGroupFromFareTypeEmpty();
  void testFTGroupFromFareTypeNotFound();
  void testFTGroupFromFareTypeFound();

  void testSolutionsMax();
  void testSolutionsLimit();
  void testSolutionsLimitWithFound();
  void testSolutionsNone();
  void testSolutionsAll();

  void testIsXMMatch();
  void testIsXMNoMatch();

  void testNoMatchItin();
  void testPrepareNoMatchItinEmpty();
  void testPrepareNoMatchItinOneSeg();
  void testPrepareNoMatchItinManySegs();
  void testPrepareNoMatchItinFilled();

  void setUpFullFBCItin();
  void testSetFullFBCItinNo();
  void testSetFullFBCItinNoPartially();
  void testSetFullFBCItinYes();

private:
  NoPNRPricingTrx* _trx;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(NoPNRPricingTrxTest);
}
#endif //__NOPNR_PRICING_TRX_TEST_H__
