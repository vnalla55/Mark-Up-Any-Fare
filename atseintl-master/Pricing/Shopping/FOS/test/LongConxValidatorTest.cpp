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
#include "Pricing/Shopping/FOS/FosFilterComposite.h"
#include "Pricing/Shopping/FOS/SolFosGenerator.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"
#include "Pricing/Shopping/FOS/LongConxValidator.h"

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
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 1, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) } // long conx
};
#undef DT

// ==================================
// TEST CLASS
// ==================================

class LongConxValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(LongConxValidatorTest);
  CPPUNIT_TEST(testLongConxSolution);
  CPPUNIT_TEST(testNormalSolution);
  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  TestMemHandle _memHandle;

  FosFilterComposite* _fosFilterComposite;
  SolFosGenerator* _fosGenerator;
  FosStatistic* _fosStatistic;
  LongConxValidator* _longConxValidator;

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

    _trx->legs()[1].sop()[1].isLngCnxSop() = true;

    _fosStatistic = _memHandle.create<FosStatistic>(*_trx);
    _fosFilterComposite = _memHandle.create<FosFilterComposite>(*_fosStatistic);
    _fosGenerator = _memHandle.create<SolFosGenerator>(*_trx, *_fosFilterComposite);
    _longConxValidator =
        _memHandle.create<LongConxValidator>(*_trx, *_fosGenerator, *_fosStatistic);
  }

  void tearDown() { _memHandle.clear(); }

  void testLongConxSolution()
  {
    uint32_t validatorBM = validatorBitMask(VALIDATOR_LONGCONX);
    SopIdVec longConx(2, 0);
    longConx[1] = 1;

    _fosStatistic->setCounterLimit(VALIDATOR_LONGCONX, 1);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::VALID, _longConxValidator->validate(longConx));
    CPPUNIT_ASSERT(!_longConxValidator->isThrowAway(longConx, ~uint32_t(0)));
    CPPUNIT_ASSERT(_longConxValidator->isThrowAway(longConx, validatorBM));

    _fosStatistic->addFOS(validatorBM, longConx);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID, _longConxValidator->validate(longConx));
    CPPUNIT_ASSERT(_longConxValidator->isThrowAway(longConx, ~uint32_t(0)));
  }

  void testNormalSolution()
  {
    uint32_t validatorBM = validatorBitMask(VALIDATOR_LONGCONX);
    SopIdVec normal(2, 0);

    _fosStatistic->setCounterLimit(VALIDATOR_LONGCONX, 1);
    _fosStatistic->addFOS(validatorBM, normal);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID, _longConxValidator->validate(normal));
    CPPUNIT_ASSERT(!_longConxValidator->isThrowAway(normal, ~uint32_t(0)));
    CPPUNIT_ASSERT(!_longConxValidator->isThrowAway(normal, validatorBM));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(LongConxValidatorTest);

} // fos
} // tse
