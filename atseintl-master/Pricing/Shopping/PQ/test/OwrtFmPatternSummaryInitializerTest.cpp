// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseConsts.h"
#include "DataModel/OwrtFareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "Pricing/Combinations.h"
#include "Pricing/Shopping/PQ/OwrtFmPatternSummaryInitializer.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{
namespace shpq
{

class OwrtFmPatternSummaryInitializerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OwrtFmPatternSummaryInitializerTest);
  CPPUNIT_TEST(testEoeRequired);
  CPPUNIT_TEST(testEoeNotPermitted);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx);
    _initializer = _memHandle.create<OwrtFmPatternSummaryInitializer>(*_trx);
  }

  void tearDown() { _memHandle.clear(); }

  void testAllValid()
  {
    typedef OwrtFmPatternSummary OFPS;

    FareMarket fm;
    addPTF(fm, 100.0);
    addPTF(fm, 101.0);
    OwrtFareMarket owrtFm(OW, &fm, 0);
    const OwrtFmPatternSummary& summary = owrtFm.getSolPatternSummary();

    CPPUNIT_ASSERT(!summary.isInitialized());

    _initializer->initSummary(owrtFm);
    CPPUNIT_ASSERT(summary.isInitialized());
    CPPUNIT_ASSERT(!summary.isInvalidForPattern(OFPS::EOE));
    CPPUNIT_ASSERT(!summary.isInvalidForPattern(OFPS::NOT_EOE));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, summary.lowerBoundForPattern(OFPS::EOE), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, summary.lowerBoundForPattern(OFPS::NOT_EOE), EPSILON);
  }

  void testEoeRequired()
  {
    typedef OwrtFmPatternSummary OFPS;

    FareMarket fm;
    addPTF(fm, 101.0, Combinations::REQUIRED);
    addPTF(fm, 102.0, Combinations::EOE_REQUIRED_SIDE_TRIP_NOT_PERMITTED);
    addPTF(fm, 103.0);
    OwrtFareMarket owrtFm(OW, &fm, 0);
    const OwrtFmPatternSummary& summary = owrtFm.getSolPatternSummary();

    CPPUNIT_ASSERT(!summary.isInitialized());

    _initializer->initSummary(owrtFm);
    CPPUNIT_ASSERT(summary.isInitialized());
    CPPUNIT_ASSERT(!summary.isInvalidForPattern(OFPS::EOE));
    CPPUNIT_ASSERT(!summary.isInvalidForPattern(OFPS::NOT_EOE));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(101.0, summary.lowerBoundForPattern(OFPS::EOE), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(103.0, summary.lowerBoundForPattern(OFPS::NOT_EOE), EPSILON);
  }

  void testEoeNotPermitted()
  {
    typedef OwrtFmPatternSummary OFPS;

    FareMarket fm;
    addPTF(fm, 102.0, Combinations::NOT_PERMITTED);
    addPTF(fm, 103.0, Combinations::EOE_NOT_PERMITTED_SIDE_TRIP_NOT_PERMITTED);
    addPTF(fm, 104.0);
    OwrtFareMarket owrtFm(OW, &fm, 0);
    const OwrtFmPatternSummary& summary = owrtFm.getSolPatternSummary();

    CPPUNIT_ASSERT(!summary.isInitialized());

    _initializer->initSummary(owrtFm);
    CPPUNIT_ASSERT(summary.isInitialized());
    CPPUNIT_ASSERT(!summary.isInvalidForPattern(OFPS::EOE));
    CPPUNIT_ASSERT(!summary.isInvalidForPattern(OFPS::NOT_EOE));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(104.0, summary.lowerBoundForPattern(OFPS::EOE), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(102.0, summary.lowerBoundForPattern(OFPS::NOT_EOE), EPSILON);
  }

private:
  TestMemHandle _memHandle;
  ShoppingTrx* _trx;

  OwrtFmPatternSummaryInitializer* _initializer;

  void addPTF(FareMarket& fm, MoneyAmount amt, char eoeInd = Combinations::ALWAYS_APPLIES)
  {
    CombinabilityRuleInfo* combRuleInfo = _memHandle.create<CombinabilityRuleInfo>();
    combRuleInfo->eoeInd() = eoeInd;

    PaxTypeFare* paxTypeFare = TestPaxTypeFareFactory::create("testdata/PaxTypeFare.xml", true);
    paxTypeFare->setIsShoppingFare();
    paxTypeFare->fare()->nucFareAmount() = amt;
    paxTypeFare->rec2Cat10() = combRuleInfo;

    fm.allPaxTypeFare().push_back(paxTypeFare);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OwrtFmPatternSummaryInitializerTest);
}
}
