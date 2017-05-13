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
#include "Pricing/Shopping/FOS/FosCommonUtil.h"
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
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) },
  { 0, 1, "AA", "JFK", "DFW", "AA", DT(obDate, 10), DT(obDate, 11) },
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 1, "AA", "JFK", "LAX", "AA", DT(obDate, 10), DT(obDate, 11) },
  { 1, 1, "AA", "JFK", "LAX", "AA", DT(obDate, 10), DT(obDate, 11) }
};

const LegsBuilder::Segment ApplyNonRestrictionOWSeg[] = { { 0, 0, "LH", "JFK", "DFW", "LH",
                                                            DT(obDate, 10), DT(obDate, 11) } };

const LegsBuilder::Segment ApplyNonRestrictionRTSeg[] = {
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) },
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }
};
#undef DT

// ==================================
// TEST CLASS
// ==================================

class FosCommonUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FosCommonUtilTest);
  CPPUNIT_TEST(testDetectCarrier);
  CPPUNIT_TEST(testCollectOnlineCarriers);
  CPPUNIT_TEST(testCollectOnlineCarriers_OneCxrPerLeg);
  CPPUNIT_TEST(testCollectOnlineCarriers_OneLegInvalid);
  CPPUNIT_TEST(testCalcNumOfValidSops);
  CPPUNIT_TEST(testCheckNumOfTravelSegs);
  CPPUNIT_TEST(testApplyNonRestrictionFilter_OW);
  CPPUNIT_TEST(testApplyNonRestrictionFilter_RT);
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
  }

  void tearDown() { _memHandle.clear(); }

  void testDetectCarrier()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments, boost::size(Segments));
    builder.endBuilding();

    SopIdVec online(2, 0);
    SopIdVec interline(2, 0);
    interline[1] = 1;
    SopIdVec oneway(1, 0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Online combination", CarrierCode("LH"), FosCommonUtil::detectCarrier(*_trx, online));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Interline combination",
                                 FosCommonUtil::INTERLINE_CARRIER,
                                 FosCommonUtil::detectCarrier(*_trx, interline));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "One way", CarrierCode("LH"), FosCommonUtil::detectCarrier(*_trx, online));
  }

  void testCollectOnlineCarriers()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments, boost::size(Segments));
    builder.endBuilding();

    std::set<CarrierCode> onlineCarriers;
    bool isInterlineApplicable = false;
    FosCommonUtil::collectOnlineCarriers(*_trx, onlineCarriers, isInterlineApplicable);

    CPPUNIT_ASSERT_EQUAL(size_t(2u), onlineCarriers.size());
    CPPUNIT_ASSERT(isInterlineApplicable);
  }

  void testCollectOnlineCarriers_OneCxrPerLeg()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments, boost::size(Segments));
    builder.endBuilding();

    _trx->legs()[0].sop()[0].cabinClassValid() = false; // invalidate LH
    _trx->legs()[1].sop()[1].cabinClassValid() = false; // invalidate AA

    std::set<CarrierCode> onlineCarriers;
    bool isInterlineApplicable = false;
    FosCommonUtil::collectOnlineCarriers(*_trx, onlineCarriers, isInterlineApplicable);

    CPPUNIT_ASSERT_EQUAL(size_t(0u), onlineCarriers.size());
    CPPUNIT_ASSERT(isInterlineApplicable);
  }

  void testCollectOnlineCarriers_OneLegInvalid()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments, boost::size(Segments));
    builder.endBuilding();

    _trx->legs()[1].sop()[0].cabinClassValid() = false;
    _trx->legs()[1].sop()[1].cabinClassValid() = false;

    std::set<CarrierCode> onlineCarriers;
    bool isInterlineApplicable = false;
    FosCommonUtil::collectOnlineCarriers(*_trx, onlineCarriers, isInterlineApplicable);

    CPPUNIT_ASSERT_EQUAL(size_t(0u), onlineCarriers.size());
    CPPUNIT_ASSERT(!isInterlineApplicable);
  }

  void testCalcNumOfValidSops()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments, boost::size(Segments));
    builder.endBuilding();

    CPPUNIT_ASSERT_EQUAL(size_t(4u), FosCommonUtil::calcNumOfValidSops(*_trx));

    _trx->legs()[1].sop()[0].cabinClassValid() = false;
    _trx->legs()[1].sop()[1].cabinClassValid() = false;

    CPPUNIT_ASSERT_EQUAL(size_t(2u), FosCommonUtil::calcNumOfValidSops(*_trx));
  }

  void testCheckNumOfTravelSegs()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(Segments, boost::size(Segments));
    builder.endBuilding();

    SopIdVec comb(2, 0);
    CPPUNIT_ASSERT(!FosCommonUtil::checkNumOfTravelSegsPerLeg(*_trx, comb, 1));
    CPPUNIT_ASSERT(!FosCommonUtil::checkTotalNumOfTravelSegs(*_trx, comb, 2));

    comb[1] = 1;
    CPPUNIT_ASSERT(FosCommonUtil::checkNumOfTravelSegsPerLeg(*_trx, comb, 1));
    CPPUNIT_ASSERT(FosCommonUtil::checkTotalNumOfTravelSegs(*_trx, comb, 2));
  }

  void testApplyNonRestrictionFilter_OW()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(ApplyNonRestrictionOWSeg, boost::size(ApplyNonRestrictionOWSeg));
    builder.endBuilding();

    FosStatistic stats(*_trx);

    // there aren't FOS generated so far
    CPPUNIT_ASSERT(FosCommonUtil::applyNonRestrictionFilter(*_trx, stats, false, true));

    stats.setCounterLimit(VALIDATOR_ONLINE, 1u);
    stats.getCarrierCounter("LH").limit = 1u;
    stats.addFOS(validatorBitMask(VALIDATOR_ONLINE), SopIdVec(1, 0));
    CPPUNIT_ASSERT(!FosCommonUtil::applyNonRestrictionFilter(*_trx, stats, false, true));

    // PQ override
    CPPUNIT_ASSERT(FosCommonUtil::applyNonRestrictionFilter(*_trx, stats, true, true));

    // priced solutions exist
    CPPUNIT_ASSERT(FosCommonUtil::applyNonRestrictionFilter(*_trx, stats, false, false));

    // online solutions lacking
    stats.setCounterLimit(VALIDATOR_ONLINE, 2u);
    stats.getCarrierCounter("LH").limit = 2u;
    CPPUNIT_ASSERT(FosCommonUtil::applyNonRestrictionFilter(*_trx, stats, false, true));
  }

  void testApplyNonRestrictionFilter_RT()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(ApplyNonRestrictionRTSeg, boost::size(ApplyNonRestrictionRTSeg));
    builder.endBuilding();

    FosStatistic stats(*_trx);

    CPPUNIT_ASSERT(!FosCommonUtil::applyNonRestrictionFilter(*_trx, stats, false, false));

    // PQ override
    CPPUNIT_ASSERT(FosCommonUtil::applyNonRestrictionFilter(*_trx, stats, true, false));

    // online solutions lacking
    stats.setCounterLimit(VALIDATOR_ONLINE, 1u);
    stats.getCarrierCounter("LH").limit = 1u;
    CPPUNIT_ASSERT(FosCommonUtil::applyNonRestrictionFilter(*_trx, stats, false, false));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FosCommonUtilTest);

} // fos
} // tse
