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
#include "Pricing/Shopping/FOS/TriangleValidator.h"
#include "Pricing/Shopping/FOS/test/FosGeneratorMock.h"

#include "test/include/LegsBuilder.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{
namespace fos
{

class TriangleValidatorMock : public TriangleValidator
{
public:
  TriangleValidatorMock(ShoppingTrx& trx, SolFosGenerator& generator, FosStatistic& stats)
    : TriangleValidator(trx, generator, stats)
  {
  }

  void setCheapestThruFM(FareMarket* fm) { _cheapestThruFM = fm; }

  void addTriangleCandidate(int sopId) { _triangleCandidates.insert(sopId); }
};

// =================================
// LEGS DATA
// =================================

static DateTime obDate = DateTime(2013, 06, 01);
static DateTime ibDate = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment Segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "LH", "JFK", "SFO", "AA", DT(obDate, 10), DT(obDate, 11) }, // ob valid
  { 0, 0, "LH", "SFO", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) },
  { 0, 1, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // ob direct
  { 1, 0, "AA", "DFW", "SFO", "LH", DT(ibDate, 10), DT(ibDate, 11) }, // ib valid
  { 1, 0, "AA", "SFO", "JFK", "AA", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 1, "AA", "DFW", "SFO", "AA", DT(ibDate, 10), DT(ibDate, 11) }, // ib diff carrier
  { 1, 1, "AA", "SFO", "JFK", "AA", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 2, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) } // ib direct
};
#undef DT

// ==================================
// TEST CLASS
// ==================================

class TriangleValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TriangleValidatorTest);
  CPPUNIT_TEST(testValid_CheapestLeg0);
  CPPUNIT_TEST(testValid_CheapestLeg1);
  CPPUNIT_TEST(testInvalid_Statistic);
  CPPUNIT_TEST(testInvalid_NotACandidate);
  CPPUNIT_TEST(testInvalid_DiffCarrier);
  CPPUNIT_TEST(testInvalid_Direct);

  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  TestMemHandle _memHandle;

  FosFilterComposite* _fosFilterComposite;
  FosGeneratorMock* _fosGenerator;
  FosStatistic* _fosStatistic;
  TriangleValidatorMock* _triangleValidator;

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
    _triangleValidator =
        _memHandle.create<TriangleValidatorMock>(*_trx, *_fosGenerator, *_fosStatistic);

    SopDetailsPtrVec sopDetails00, sopDetails10, sopDetails11, empty;
    sopDetails00.push_back(createSopDetails("SFO", "AA", "LH"));
    sopDetails10.push_back(createSopDetails("SFO", "LH", "AA"));
    sopDetails11.push_back(createSopDetails("SFO", "AA", "AA"));

    _fosGenerator->addDetailedSop(0, 0, sopDetails00);
    _fosGenerator->addDetailedSop(1, 0, sopDetails10);
    _fosGenerator->addDetailedSop(1, 1, sopDetails11);
    _fosGenerator->addDetailedSop(1, 2, empty);
  }

  void tearDown() { _memHandle.clear(); }

  void testValid_CheapestLeg0()
  {
    SopIdVec triangle(2, 0);

    _fosStatistic->setCounterLimit(VALIDATOR_TRIANGLE, 1);
    _triangleValidator->setCheapestThruFM(createCheapestThruFM(0));
    _triangleValidator->addTriangleCandidate(0);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::VALID, _triangleValidator->validate(triangle));
    CPPUNIT_ASSERT(!_triangleValidator->isThrowAway(triangle, 0));
  }

  void testValid_CheapestLeg1()
  {
    SopIdVec triangle(2, 0);

    _fosStatistic->setCounterLimit(VALIDATOR_TRIANGLE, 1);
    _triangleValidator->setCheapestThruFM(createCheapestThruFM(1));
    _triangleValidator->addTriangleCandidate(0);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::VALID, _triangleValidator->validate(triangle));
    CPPUNIT_ASSERT(!_triangleValidator->isThrowAway(triangle, 0));
  }

  void testInvalid_Statistic()
  {
    SopIdVec triangle(2, 0);

    _fosStatistic->setCounterLimit(VALIDATOR_TRIANGLE, 1);
    _fosStatistic->addFOS(validatorBitMask(VALIDATOR_TRIANGLE), triangle);
    _triangleValidator->setCheapestThruFM(createCheapestThruFM(0));
    _triangleValidator->addTriangleCandidate(0);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID, _triangleValidator->validate(triangle));
    CPPUNIT_ASSERT(!_triangleValidator->isThrowAway(triangle, 0));
  }

  void testInvalid_NotACandidate()
  {
    SopIdVec triangle(2, 0);

    _fosStatistic->setCounterLimit(VALIDATOR_TRIANGLE, 1);
    _fosStatistic->addFOS(validatorBitMask(VALIDATOR_TRIANGLE), triangle);
    _triangleValidator->setCheapestThruFM(createCheapestThruFM(0));

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID, _triangleValidator->validate(triangle));
    CPPUNIT_ASSERT(!_triangleValidator->isThrowAway(triangle, 0));
  }

  void testInvalid_DiffCarrier()
  {
    SopIdVec diffCarrier(2, 0);
    diffCarrier[1] = 1;

    _fosStatistic->setCounterLimit(VALIDATOR_TRIANGLE, 1);
    _triangleValidator->setCheapestThruFM(createCheapestThruFM(0));
    _triangleValidator->addTriangleCandidate(0);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID_SOP_DETAILS,
                         _triangleValidator->validate(diffCarrier));
    CPPUNIT_ASSERT(!_triangleValidator->isThrowAway(diffCarrier, 0));
  }

  void testInvalid_Direct()
  {
    SopIdVec direct(2, 1);
    direct[1] = 2;

    _fosStatistic->setCounterLimit(VALIDATOR_TRIANGLE, 1);
    _triangleValidator->setCheapestThruFM(createCheapestThruFM(0));
    _triangleValidator->addTriangleCandidate(0);

    CPPUNIT_ASSERT_EQUAL(BaseValidator::INVALID, _triangleValidator->validate(direct));
    CPPUNIT_ASSERT(!_triangleValidator->isThrowAway(direct, 0));
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

  FareMarket* createCheapestThruFM(uint16_t legId)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->legIndex() = legId;
    return fm;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TriangleValidatorTest);

} // fos
} // tse
