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
#include "Pricing/Shopping/FOS/NonStopValidator.h"

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
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 1, "AA", "DFW", "JFK", "AA", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 2, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 2, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }
};
#undef DT

// ==================================
// TEST CLASS
// ==================================

class NonStopValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NonStopValidatorTest);
  CPPUNIT_TEST(testValidOnline);
  CPPUNIT_TEST(testValidInterline);
  CPPUNIT_TEST(testDeferred);
  CPPUNIT_TEST(testInvalidCases);
  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  TestMemHandle _memHandle;

  FosFilterComposite* _fosFilterComposite;
  SolFosGenerator* _fosGenerator;
  FosStatistic* _fosStatistic;
  NonStopValidator* _nonStopValidator;

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

    _fosStatistic = _memHandle.create<FosStatistic>(*_trx);
    _fosFilterComposite = _memHandle.create<FosFilterComposite>(*_fosStatistic);
    _fosGenerator = _memHandle.create<SolFosGenerator>(*_trx, *_fosFilterComposite);
    _nonStopValidator = _memHandle.create<NonStopValidator>(*_trx, *_fosGenerator, *_fosStatistic);
  }

  void tearDown() { _memHandle.clear(); }

  void testValidOnline()
  {
    SopIdVec online(2, 0);

    _fosStatistic->setCounterLimit(VALIDATOR_NONSTOP, 1);
    _fosStatistic->getDirectCarrierCounter("LH").limit = 1;

    CPPUNIT_ASSERT_EQUAL(BaseValidator::VALID, _nonStopValidator->validate(online));
    CPPUNIT_ASSERT(!_nonStopValidator->isThrowAway(online, 0));
  }

  void testValidInterline()
  {
    SopIdVec interline(2, 0);
    interline[1] = 1;

    _fosStatistic->setCounterLimit(VALIDATOR_NONSTOP, 1);
    _fosStatistic->getDirectCarrierCounter("").limit = 1;

    CPPUNIT_ASSERT_EQUAL(BaseValidator::VALID, _nonStopValidator->validate(interline));
    CPPUNIT_ASSERT(!_nonStopValidator->isThrowAway(interline, 0));
  }

  void testDeferred()
  {
    SopIdVec online(2, 0);

    _fosStatistic->setCounterLimit(VALIDATOR_NONSTOP, 1);
    _fosStatistic->getDirectCarrierCounter("LH").limit = 0;

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID, _nonStopValidator->validate(online));
    CPPUNIT_ASSERT(!_nonStopValidator->isThrowAway(online, 0));

    _nonStopValidator->setDeferredProcessing(true);
    CPPUNIT_ASSERT_EQUAL(BaseValidator::DEFERRED, _nonStopValidator->validate(online));
  }

  void testInvalidCases()
  {
    SopIdVec online(2, 0);
    SopIdVec onlineIndirect(2, 0);
    onlineIndirect[1] = 2;

    _fosStatistic->setCounterLimit(VALIDATOR_NONSTOP, 1);
    _fosStatistic->getDirectCarrierCounter("LH").limit = 1;

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID, _nonStopValidator->validate(onlineIndirect));
    CPPUNIT_ASSERT(!_nonStopValidator->isThrowAway(onlineIndirect, 0));

    _fosStatistic->addFOS(validatorBitMask(VALIDATOR_NONSTOP), online);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID, _nonStopValidator->validate(online));
    CPPUNIT_ASSERT(!_nonStopValidator->isThrowAway(online, 0));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NonStopValidatorTest);

} // fos
} // tse
