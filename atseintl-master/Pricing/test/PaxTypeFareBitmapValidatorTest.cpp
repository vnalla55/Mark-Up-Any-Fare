//----------------------------------------------------------------------------
//  Copyright Sabre 2013
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
#include <boost/range.hpp>

#include "Common/TseConsts.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/PaxTypeFareBitmapValidator.h"

#include "test/include/LegsBuilder.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{

// =================================
// LEGS DATA
// =================================

static DateTime obDate = DateTime(2013, 06, 01);
static DateTime ibDate = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment IsNonStopItinSegments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) },
  { 1, 0, "LH", "DFW", "SAO", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 0, "LH", "SAO", "JFK", "LH", DT(ibDate, 11), DT(ibDate, 12) }
};
#undef DT

// ==================================
// TEST CLASS
// ==================================

class PaxTypeFareBitmapValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PaxTypeFareBitmapValidatorTest);
  CPPUNIT_TEST(testIsNonStopItin);
  CPPUNIT_TEST(testIsNonStopItin_ASO);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  ShoppingTrx* _trx;
  PaxTypeFareBitmapValidator* _ptfbValidator;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx);
  }

  void tearDown() { _memHandle.clear(); }

  void testIsNonStopItin()
  {
    // prepare legs
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(IsNonStopItinSegments, boost::size(IsNonStopItinSegments));
    builder.endBuilding();

    PaxTypeFareBitmapValidator* validator = createValidator(0);
    ItinIndex::ItinIndexIterator* it = createItinIndexIterator(0);

    CPPUNIT_ASSERT_EQUAL(true, validator->isNonStopItin(*it));
  }

  void testIsNonStopItin_ASO()
  {
    // prepare legs
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(IsNonStopItinSegments, boost::size(IsNonStopItinSegments));
    builder.endBuilding();

    _trx->legs().push_back(ShoppingTrx::Leg());
    _trx->legs().back().stopOverLegFlag() = true;
    _trx->legs().back().jumpedLegIndices().push_back(0);
    _trx->legs().back().jumpedLegIndices().push_back(1);

    PaxTypeFareBitmapValidator* validator = createValidator(2);
    ItinIndex::ItinIndexIterator* it = createItinIndexIteratorASO(0, 0);

    CPPUNIT_ASSERT_EQUAL(false, validator->isNonStopItin(*it));
  }

private:
  PaxTypeFareBitmapValidator* createValidator(uint16_t legIndex)
  {
    AirSeg* ts = _memHandle.create<AirSeg>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    fm->legIndex() = legIndex;
    fm->travelSeg().push_back(ts);
    ptf->fareMarket() = fm;

    const DatePair* dates = 0;
    PaxTypeFareBitmapValidator::SkippedBitValidator* skippedBitValidator = 0;
    FareUsageMatrixMap* fareUsageMatrixMap = 0;
    PaxTypeFareBitmapValidator* validator = _memHandle.create<PaxTypeFareBitmapValidator>(
        *_trx, *ptf, dates, skippedBitValidator, fareUsageMatrixMap);

    return validator;
  }

  ItinIndex::ItinIndexIterator* createItinIndexIterator(uint32_t sopIndex)
  {
    typedef ItinIndex::ItinIndexIterator ItinIndexIterator;

    ItinIndexIterator::IteratorMode mode = ItinIndexIterator::IteratorModeNormalLeg;
    ItinIndex::ItinCell* cell = _memHandle.create<ItinIndex::ItinCell>();
    cell->first.sopIndex() = sopIndex;
    ItinIndexIterator* it = _memHandle.create<ItinIndexIterator>(mode);
    it->_cell = cell;
    return it;
  }

  ItinIndex::ItinIndexIterator* createItinIndexIteratorASO(uint32_t sopIndexOb, uint32_t sopIndexIb)
  {
    typedef ItinIndex::ItinIndexIterator ItinIndexIterator;

    ItinIndexIterator::IteratorMode mode = ItinIndexIterator::IteratorModeAcrossLeg;
    ItinIndexIterator* it = _memHandle.create<ItinIndexIterator>(mode);
    it->_currentSopSet.push_back(sopIndexOb);
    it->_currentSopSet.push_back(sopIndexIb);
    return it;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PaxTypeFareBitmapValidatorTest);

} // tse
