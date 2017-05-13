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

#include "test/include/CppUnitHelperMacros.h"
#include <boost/range.hpp>
#include <sstream>

#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"

#include "test/include/LegsBuilder.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{
namespace fos
{

// =================================
// LEGS DATA
// =================================

static DateTime obDate = DateTime(2013, 06, 01);
static DateTime ibDate = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment Segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // 1 h
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) } // 1 h
};
#undef DT

// ==================================
// TEST CLASS
// ==================================

class FosStatisticTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FosStatisticTest);
  CPPUNIT_TEST(testInitialization);
  CPPUNIT_TEST(testCounters);
  CPPUNIT_TEST(testLackingValidators);
  CPPUNIT_TEST(testCarrierCounters);
  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx);

    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments, boost::size(Segments));
    builder.endBuilding();
  }

  void tearDown() { _memHandle.clear(); }

  void testInitialization()
  {
    FosStatistic stats(*_trx);

    for (uint32_t i = 0; i <= VALIDATOR_LAST; i++)
    {
      std::ostringstream counterMsg;
      counterMsg << "Validator " << i << ": counter not initialized";
      std::ostringstream counterLimitMsg;
      counterLimitMsg << "Validator " << i << ": counter limit not initialized";

      ValidatorType vt = static_cast<ValidatorType>(i);
      CPPUNIT_ASSERT_EQUAL_MESSAGE(counterMsg.str(), uint32_t(0), stats.getCounter(vt));
      CPPUNIT_ASSERT_EQUAL_MESSAGE(counterLimitMsg.str(), uint32_t(0), stats.getCounterLimit(vt));
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Lacking validators not initialized", ValidatorBitMask(0), stats.getLackingValidators());

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Carrier counter not initialized", uint32_t(0), stats.getCarrierCounter("LH").value);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Carrier counter limit not initialized", uint32_t(0), stats.getCarrierCounter("LH").limit);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Direct carrier counter not initialized",
                                 uint32_t(0),
                                 stats.getDirectCarrierCounter("LH").value);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Direct carrier counter limit not initialized",
                                 uint32_t(0),
                                 stats.getDirectCarrierCounter("LH").limit);
  }

  void testCounters()
  {
    SopIdVec sopIdVec(2, 0);

    ValidatorBitMask countersBitMask =
        validatorBitMask(VALIDATOR_ONLINE) | validatorBitMask(VALIDATOR_SNOWMAN) |
        validatorBitMask(VALIDATOR_DIAMOND) | validatorBitMask(VALIDATOR_TRIANGLE);

    FosStatistic stats(*_trx);
    stats.addFOS(countersBitMask, sopIdVec);

    CPPUNIT_ASSERT_EQUAL(uint32_t(1), stats.getCounter(VALIDATOR_ONLINE));
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), stats.getCounter(VALIDATOR_SNOWMAN));
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), stats.getCounter(VALIDATOR_DIAMOND));
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), stats.getCounter(VALIDATOR_TRIANGLE));
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), stats.getCounter(VALIDATOR_CUSTOM));
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), stats.getCounter(VALIDATOR_LONGCONX));
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), stats.getCounter(VALIDATOR_NONSTOP));
  }

  void testLackingValidators()
  {
    SopIdVec comb(2, 0);

    FosStatistic stats(*_trx);
    stats.setCounterLimit(VALIDATOR_SNOWMAN, 1);
    CPPUNIT_ASSERT_EQUAL(validatorBitMask(VALIDATOR_SNOWMAN), stats.getLackingValidators());

    stats.addFOS(validatorBitMask(VALIDATOR_SNOWMAN), comb);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), stats.getLackingValidators());

    stats.setCounterLimit(VALIDATOR_SNOWMAN, 2);
    CPPUNIT_ASSERT_EQUAL(validatorBitMask(VALIDATOR_SNOWMAN), stats.getLackingValidators());
  }

  void testCarrierCounters()
  {
    SopIdVec sopIdVec(2, 0);

    ValidatorBitMask countersBitMask =
        validatorBitMask(VALIDATOR_ONLINE) | validatorBitMask(VALIDATOR_NONSTOP);

    FosStatistic stats(*_trx);
    stats.addFOS(countersBitMask, sopIdVec);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Online carrier counter", uint32_t(1), stats.getCarrierCounter("LH").value);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Direct carrier counter", uint32_t(1), stats.getDirectCarrierCounter("LH").value);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FosStatisticTest);

} // fos
} // tse
