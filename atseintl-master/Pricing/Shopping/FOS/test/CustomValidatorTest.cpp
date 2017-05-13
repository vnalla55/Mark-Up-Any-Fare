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
#include "Pricing/Shopping/FOS/CustomValidator.h"

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
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // custom sop
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }, // custom sop
  { 1, 1, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }
};
#undef DT

// ==================================
// TEST CLASS
// ==================================

class CustomValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CustomValidatorTest);
  CPPUNIT_TEST(testCustomSolution);
  CPPUNIT_TEST(testNonCustomSolution);
  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  TestMemHandle _memHandle;

  FosFilterComposite* _fosFilterComposite;
  SolFosGenerator* _fosGenerator;
  FosStatistic* _fosStatistic;
  CustomValidator* _customValidator;

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

    _trx->legs()[0].setCustomLeg(true);
    _trx->legs()[0].sop()[0].setCustomSop(true);
    _trx->legs()[1].setCustomLeg(true);
    _trx->legs()[1].sop()[0].setCustomSop(true);

    _fosStatistic = _memHandle.create<FosStatistic>(*_trx);
    _fosFilterComposite = _memHandle.create<FosFilterComposite>(*_fosStatistic);
    _fosGenerator = _memHandle.create<SolFosGenerator>(*_trx, *_fosFilterComposite);
    _customValidator = _memHandle.create<CustomValidator>(*_trx, *_fosGenerator, *_fosStatistic);
  }

  void tearDown() { _memHandle.clear(); }

  void testCustomSolution()
  {
    SopIdVec custom(2, 0);

    _fosStatistic->setCounterLimit(VALIDATOR_CUSTOM, 1);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::VALID, _customValidator->validate(custom));
    CPPUNIT_ASSERT(!_customValidator->isThrowAway(custom, 0));

    _fosStatistic->addFOS(validatorBitMask(VALIDATOR_CUSTOM), custom);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID, _customValidator->validate(custom));
    CPPUNIT_ASSERT(_customValidator->isThrowAway(custom, 0));
    CPPUNIT_ASSERT(_customValidator->isThrowAway(custom, ~uint32_t(0)));
  }

  void testNonCustomSolution()
  {
    SopIdVec nonCustom;
    nonCustom.push_back(0);
    nonCustom.push_back(1);

    _fosStatistic->setCounterLimit(VALIDATOR_CUSTOM, 1);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID, _customValidator->validate(nonCustom));
    CPPUNIT_ASSERT(!_customValidator->isThrowAway(nonCustom, 0));

    _fosStatistic->addFOS(validatorBitMask(VALIDATOR_CUSTOM), nonCustom);

    CPPUNIT_ASSERT(!_customValidator->isThrowAway(nonCustom, 0));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CustomValidatorTest);

} // fos
} // tse
