// -------------------------------------------------------------------
//
//
//  Copyright (C) Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "test/include/CppUnitHelperMacros.h"

#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/Diversity/DiversityUtil.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

#include "test/include/LegsBuilder.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{
namespace shpq
{

typedef DiversityUtil::CompoundCarrier CompoundCarrier;

class DiversityUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DiversityUtilTest);
  CPPUNIT_TEST(testOW_isSolutionSimilar);
  CPPUNIT_TEST(testOW_isSolutionNotSimilar);
  CPPUNIT_TEST(testOW_isSolutionSimilarEvenIfOperatingCarrier);
  CPPUNIT_TEST(testOW_isSolutionNotSimilarBySegmentCount);
  CPPUNIT_TEST(testRT_isSolutionSimilar);
  CPPUNIT_TEST(testRT_isSolutionNotSimilarBySegmentCount);

  CPPUNIT_TEST(testOW_isNeverSnowman);
  CPPUNIT_TEST(testRT_isSnowman);
  CPPUNIT_TEST(testRT_isSnowmanSegCount_3_vs_4);
  CPPUNIT_TEST(testRT_isSnowmanSegCount_3_vs_2);
  CPPUNIT_TEST(testRT_isSnowmanSegCount_3_vs_1);
  CPPUNIT_TEST(testRT_isSnowmanEvenIfOperatingCarrier);
  CPPUNIT_TEST(testRT_isNotSnowmanConnectingCity);

  CPPUNIT_TEST(testCompoundCarrier_isNotInitialized);
  CPPUNIT_TEST(testCompoundCarrier_isOnlineOW);
  CPPUNIT_TEST(testCompoundCarrier_isOnlineRT);
  CPPUNIT_TEST(testCompoundCarrier_isInterline);
  CPPUNIT_TEST(testCompoundCarrier_isLessOperatorForUniqueness);
  CPPUNIT_TEST(testCompoundCarrier_isLessOperatorForElementsOrder);
  CPPUNIT_TEST_SUITE_END();

public:
  DiversityUtilTest() {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = TestShoppingTrxFactory::create(
        "/vobs/atseintl/Pricing/Shopping/PQ/test/testdata/ShoppingNGSTrx.xml", true);
  }

  void tearDown() { _memHandle.clear(); }

  void testOW_isSolutionSimilar()
  {
    setupOneway123();

    CPPUNIT_ASSERT(DiversityUtil::isSolutionSimilar(*_trx, makeOW(SOP_1), makeOW(SOP_3)));
  }

  void testOW_isSolutionNotSimilar()
  {
    setupOneway123();

    CPPUNIT_ASSERT(!DiversityUtil::isSolutionSimilar(*_trx, makeOW(SOP_1), makeOW(SOP_2)));
  }

  void testOW_isSolutionSimilarEvenIfOperatingCarrier()
  {
    setupOneway123();

    CPPUNIT_ASSERT(DiversityUtil::isSolutionSimilar(*_trx, makeOW(SOP_1), makeOW(SOP_3)));
  }

  void testOW_isSolutionNotSimilarBySegmentCount()
  {
    setupOneway123();

    CPPUNIT_ASSERT(!DiversityUtil::isSolutionSimilar(*_trx, makeOW(SOP_1), makeOW(SOP_4)));
  }

  void testRT_isSolutionSimilar()
  {
    setupOneway123();

    LegsBuilder builder(*_trx, _memHandle, true);
    builder.addIBFromReversedOB(SOP_1, SOP_1);
    builder.addIBFromReversedOB(SOP_2, SOP_3);

    CPPUNIT_ASSERT(
        DiversityUtil::isSolutionSimilar(*_trx, makeRT(SOP_1, SOP_1), makeRT(SOP_1, SOP_2)));
  }

  void testRT_isSolutionNotSimilarBySegmentCount()
  {
    setupOneway123();

    LegsBuilder builder(*_trx, _memHandle, true);
    builder.addIBFromReversedOB(SOP_1, SOP_1);
    builder.addIBFromReversedOB(SOP_2, SOP_4);

    CPPUNIT_ASSERT(
        !DiversityUtil::isSolutionSimilar(*_trx, makeRT(SOP_1, SOP_1), makeRT(SOP_1, SOP_2)));
  }

  void testOW_isNeverSnowman()
  {
    setupSnowman123();

    CPPUNIT_ASSERT_MESSAGE("One-way can never be snowman",
                           !DiversityUtil::detectSnowman(*_trx, makeOW(SOP_1)));
  }

  void testRT_isSnowman()
  {
    setupSnowman123();

    LegsBuilder builder(*_trx, _memHandle, true);
    builder.addIBFromReversedOB(SOP_1, SOP_1);

    CPPUNIT_ASSERT(DiversityUtil::detectSnowman(*_trx, makeRT(SOP_1, SOP_1)));
  }

  void testRT_isSnowmanSegCount_3_vs_4()
  {
    setupSnowman123();

    LegsBuilder builder(*_trx, _memHandle, true);
    builder.addIBFromReversedOB(SOP_1, SOP_2);

    CPPUNIT_ASSERT(DiversityUtil::detectSnowman(*_trx, makeRT(SOP_1, SOP_1)));
  }

  void testRT_isSnowmanSegCount_3_vs_2()
  {
    setupSnowman123();

    LegsBuilder builder(*_trx, _memHandle, true);
    builder.addIBFromReversedOB(SOP_1, SOP_6);

    CPPUNIT_ASSERT(DiversityUtil::detectSnowman(*_trx, makeRT(SOP_1, SOP_1)));
  }

  void testRT_isSnowmanSegCount_3_vs_1()
  {
    setupSnowman123();

    LegsBuilder builder(*_trx, _memHandle, true);
    builder.addIBFromReversedOB(SOP_1, SOP_5);

    CPPUNIT_ASSERT(!DiversityUtil::detectSnowman(*_trx, makeRT(SOP_1, SOP_1)));
  }

  void testRT_isSnowmanEvenIfOperatingCarrier()
  {
    setupSnowman123();

    LegsBuilder builder(*_trx, _memHandle, true);
    builder.addIBFromReversedOB(SOP_1, SOP_2);

    CPPUNIT_ASSERT(DiversityUtil::detectSnowman(*_trx, makeRT(SOP_1, SOP_1)));
  }

  void testRT_isNotSnowmanConnectingCity()
  {
    setupSnowman123();

    LegsBuilder builder(*_trx, _memHandle, true);
    builder.addIBFromReversedOB(SOP_1, SOP_4);

    CPPUNIT_ASSERT(!DiversityUtil::detectSnowman(*_trx, makeRT(SOP_1, SOP_1)));
  }

  void testCompoundCarrier_isNotInitialized()
  {
    CompoundCarrier cxr;

    CPPUNIT_ASSERT_EQUAL(static_cast<std::string>(INVALID_CARRIERCODE),
                         boost::lexical_cast<std::string>(cxr));
  }

  void testCompoundCarrier_isOnlineOW()
  {
    CompoundCarrier cxr("AA");

    CPPUNIT_ASSERT(cxr.isOnline());
    CPPUNIT_ASSERT(!cxr.isInterline());
  }

  void testCompoundCarrier_isOnlineRT()
  {
    CompoundCarrier cxr("AA", "AA");

    CPPUNIT_ASSERT(cxr.isOnline());
    CPPUNIT_ASSERT(!cxr.isInterline());
  }

  void testCompoundCarrier_isInterline()
  {
    CompoundCarrier cxr("UA", "AA");

    CPPUNIT_ASSERT(!cxr.isOnline());
    CPPUNIT_ASSERT(cxr.isInterline());
  }

  void testCompoundCarrier_isLessOperatorForUniqueness()
  {
    using namespace boost;
    using namespace boost::assign;

    const size_t expectUniqueElementsNum = 5;

    const CompoundCarrier notInitialized;
    const CompoundCarrier aa("AA", "AA");
    const CompoundCarrier interline1("AA", "UA");
    const CompoundCarrier interline2("UA", "AA");
    const CompoundCarrier us("US", "US");

    // mix them up!
    std::set<CompoundCarrier> set;
    set += us, interline2, interline1, notInitialized, aa, notInitialized, notInitialized,
        interline1, interline2;

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "\"operator<\" fails to sort out unique elements", expectUniqueElementsNum, set.size());
  }

  void testCompoundCarrier_isLessOperatorForElementsOrder()
  {
    using namespace boost;
    using namespace boost::assign;

    const CompoundCarrier notInitialized;
    const CompoundCarrier aa("AA", "AA");
    const CompoundCarrier interline1("AA", "UA");
    const CompoundCarrier interline2("UA", "AA");
    const CompoundCarrier us("US", "US");

    // mix them up!
    std::set<CompoundCarrier> set;
    set += us, interline2, interline1, notInitialized, aa;

    std::vector<CompoundCarrier> sortedVec;
    sortedVec += notInitialized, aa, interline1, interline2, us;

    CPPUNIT_ASSERT_EQUAL(sortedVec.size(), set.size());

    std::set<CompoundCarrier>::iterator setIter = set.begin();
    for (size_t pos = 0; pos < sortedVec.size(); ++pos, ++setIter)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE(
          str(format("Elements order does not match starting from position %1%") % pos),
          sortedVec[pos],
          *setIter);
    }
  }

private:
  enum
  {
    OUTBOUND_LEG = 0
  };

  // sop ids
  enum
  {
    SOP_1,
    SOP_2,
    SOP_3,
    SOP_4,
    SOP_5,
    SOP_6
  };

  RootLoggerGetOff _loggerGetOff;
  ShoppingTrx* _trx;
  TestMemHandle _memHandle;

  void setupOneway123()
  {
    LegsBuilder builder(*_trx, _memHandle);

    //-- SOLUTION 1: connecting city airport DTT

    builder.addSegment(OUTBOUND_LEG, SOP_1, "ATW", "ATW", "DTW", "DTT", "DL", "DL")
        .addSegment(OUTBOUND_LEG, SOP_1, "DTW", "DTT", "PHX", "PHX", "DL", "DL");

    //-- SOLUTION 2: connecting city and airport MSP

    builder.addSegment(OUTBOUND_LEG, SOP_2, "ATW", "ATW", "MSP", "MSP", "DL", "DL")
        .addSegment(OUTBOUND_LEG, SOP_2, "MSP", "MSP", "PHX", "PHX", "DL", "DL");

    //-- SOLUTION 3: connecting city airport DTT but another operating carrier AA

    builder.addSegment(OUTBOUND_LEG, SOP_3, "ATW", "ATW", "DTW", "DTT", "DL", "AA")
        .addSegment(OUTBOUND_LEG, SOP_3, "DTW", "DTT", "PHX", "PHX", "DL", "AA");

    //-- SOLUTION 4: direct

    builder.addSegment(OUTBOUND_LEG, SOP_4, "ATW", "ATW", "PHX", "PHX", "AA", "AA");
  }

  void setupSnowman123()
  {
    LegsBuilder builder(*_trx, _memHandle);

    //-- SOLUTION 1: 3-seg with carrier change to UA in the middle
    //
    builder.addSegment(OUTBOUND_LEG, SOP_1, "CUN", "CUN", "DFW", "DFW", "DL", "DL")
        .addSegment(OUTBOUND_LEG, SOP_1, "DFW", "DFW", "ORD", "CHI", "DL", "DL")
        .addSegment(OUTBOUND_LEG, SOP_1, "ORD", "CHI", "YYZ", "YTO", "UA", "UA");

    //-- SOLUTION 2: 4-seg with carrier change to UA in the middle
    //
    builder.addSegment(OUTBOUND_LEG, SOP_2, "CUN", "CUN", "DFW", "DFW", "DL", "DL")
        .addSegment(OUTBOUND_LEG, SOP_2, "DFW", "DFW", "FFF", "FFF", "DL", "DL")
        .addSegment(OUTBOUND_LEG, SOP_2, "GGG", "GGG", "ORD", "CHI", "DL", "DL")
        .addSegment(OUTBOUND_LEG, SOP_2, "ORD", "CHI", "YYZ", "YTO", "UA", "UA");

    //-- SOLUTION 3: 4-seg but random operating carrier
    //
    builder.addSegment(OUTBOUND_LEG, SOP_3, "CUN", "CUN", "DFW", "DFW", "DL", "DL")
        .addSegment(OUTBOUND_LEG, SOP_3, "DFW", "DFW", "FFF", "FFF", "DL", "SW")
        .addSegment(OUTBOUND_LEG, SOP_3, "GGG", "GGG", "ORD", "CHI", "DL", "US")
        .addSegment(OUTBOUND_LEG, SOP_3, "ORD", "CHI", "YYZ", "YTO", "UA", "IK");

    //-- SOLUTION 4: 2-seg with carrier change to UA in the middle but different connecting city
    //
    builder.addSegment(OUTBOUND_LEG, SOP_4, "CUN", "CUN", "DFW", "DFW", "DL", "DL")
        .addSegment(OUTBOUND_LEG, SOP_4, "DFW", "DFW", "YYZ", "YTO", "UA", "UA");

    //-- SOLUTION 5: direct
    //
    builder.addSegment(OUTBOUND_LEG, SOP_5, "CUN", "CUN", "YYZ", "YTO", "UA", "DL");

    //-- SOLUTION 6: 2-seg with carrier change to UA in the middle same connecting city
    //
    builder.addSegment(OUTBOUND_LEG, SOP_6, "CUN", "CUN", "ORD", "CHI", "DL", "DL")
        .addSegment(OUTBOUND_LEG, SOP_6, "ORD", "CHI", "YYZ", "YTO", "UA", "UA");
  }

  /**
   * make sop index vector from OB sop
   */
  static SopIdxVec makeOW(int sop)
  {
    SopIdxVec result(1);
    result[0] = sop;
    return result;
  }

  /**
   * make sop index vector from OB and IB sop
   */
  static SopIdxVec makeRT(int obSop, int ibSop)
  {
    SopIdxVec result(2);
    result[0] = obSop;
    result[1] = ibSop;
    return result;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DiversityUtilTest);

} /* namespace shpq */
} /* namespace tse */
