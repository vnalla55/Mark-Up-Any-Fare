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
#include "Pricing/Shopping/FOS/SnowmanValidator.h"
#include "Pricing/Shopping/FOS/test/FosGeneratorMock.h"

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
  { 0, 0, "LH", "JFK", "SFO", "LH", DT(obDate, 10), DT(obDate, 11) }, // ob valid
  { 0, 0, "LH", "SFO", "DFW", "AA", DT(obDate, 10), DT(obDate, 11) },
  { 0, 1, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // ob direct
  { 1, 0, "LH", "DFW", "SFO", "DL", DT(ibDate, 10), DT(ibDate, 11) }, // ib valid
  { 1, 0, "LH", "SFO", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 1, "LH", "DFW", "LAX", "AA", DT(ibDate, 10), DT(ibDate, 11) }, // ib diff airport
  { 1, 1, "LH", "LAX", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 2, "LH", "DFW", "LAX", "AA", DT(ibDate, 10), DT(ibDate, 11) }, // ib diff carrier
  { 1, 2, "LH", "LAX", "JFK", "AA", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 3, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) } // ib direct
};
#undef DT

// ==================================
// TEST CLASS
// ==================================

class SnowmanValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SnowmanValidatorTest);
  CPPUNIT_TEST(testValidCase);
  CPPUNIT_TEST(testInvalid_Statistic);
  CPPUNIT_TEST(testInvalid_DiffAirport);
  CPPUNIT_TEST(testInvalid_DiffCarrier);
  CPPUNIT_TEST(testInvalid_Direct);

  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  TestMemHandle _memHandle;

  FosFilterComposite* _fosFilterComposite;
  FosGeneratorMock* _fosGenerator;
  FosStatistic* _fosStatistic;
  SnowmanValidator* _snowmanValidator;

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
    _fosGenerator = _memHandle.create<FosGeneratorMock>(*_trx, *_fosFilterComposite);
    _snowmanValidator = _memHandle.create<SnowmanValidator>(*_trx, *_fosGenerator, *_fosStatistic);

    SopDetailsPtrVec sopDetails00, sopDetails10, sopDetails11, sopDetails12, empty;
    sopDetails00.push_back(createSopDetails("SFO", "LH", "AA"));
    sopDetails10.push_back(createSopDetails("SFO", "DL", "LH"));
    sopDetails11.push_back(createSopDetails("LAX", "AA", "LH"));
    sopDetails12.push_back(createSopDetails("SFO", "AA", "AA"));

    _fosGenerator->addDetailedSop(0, 0, sopDetails00);
    _fosGenerator->addDetailedSop(0, 1, empty);
    _fosGenerator->addDetailedSop(1, 0, sopDetails10);
    _fosGenerator->addDetailedSop(1, 1, sopDetails11);
    _fosGenerator->addDetailedSop(1, 2, sopDetails12);
    _fosGenerator->addDetailedSop(1, 3, empty);
  }

  void tearDown() { _memHandle.clear(); }

  void testValidCase()
  {
    SopIdVec snowman(2, 0);

    _fosStatistic->setCounterLimit(VALIDATOR_SNOWMAN, 1);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::VALID, _snowmanValidator->validate(snowman));
    CPPUNIT_ASSERT(!_snowmanValidator->isThrowAway(snowman, 0));
  }

  void testInvalid_Statistic()
  {
    SopIdVec snowman(2, 0);

    _fosStatistic->setCounterLimit(VALIDATOR_SNOWMAN, 1);
    _fosStatistic->addFOS(validatorBitMask(VALIDATOR_SNOWMAN), snowman);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID, _snowmanValidator->validate(snowman));
    CPPUNIT_ASSERT(!_snowmanValidator->isThrowAway(snowman, 0));
  }

  void testInvalid_DiffAirport()
  {
    SopIdVec diffAirport(2, 0);
    diffAirport[1] = 1;

    _fosStatistic->setCounterLimit(VALIDATOR_SNOWMAN, 1);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID_SOP_DETAILS,
                         _snowmanValidator->validate(diffAirport));
    CPPUNIT_ASSERT(!_snowmanValidator->isThrowAway(diffAirport, 0));
  }

  void testInvalid_DiffCarrier()
  {
    SopIdVec diffCarrier(2, 0);
    diffCarrier[1] = 2;

    _fosStatistic->setCounterLimit(VALIDATOR_SNOWMAN, 1);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID_SOP_DETAILS,
                         _snowmanValidator->validate(diffCarrier));
    CPPUNIT_ASSERT(!_snowmanValidator->isThrowAway(diffCarrier, 0));
  }

  void testInvalid_Direct()
  {
    SopIdVec direct(2, 1);
    direct[1] = 3;

    _fosStatistic->setCounterLimit(VALIDATOR_SNOWMAN, 1);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID, _snowmanValidator->validate(direct));
    CPPUNIT_ASSERT(!_snowmanValidator->isThrowAway(direct, 0));
  }

private:
  SopDetails*
  createSopDetails(const LocCode& airport, const CarrierCode& obCxr, const CarrierCode& ibCxr)
  {
    SopDetails* sd = _memHandle.create<SopDetails>();
    sd->destAirport = airport;
    sd->cxrCode[0] = obCxr;
    sd->cxrCode[1] = ibCxr;
    return sd;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SnowmanValidatorTest);

} // fos
} // tse
