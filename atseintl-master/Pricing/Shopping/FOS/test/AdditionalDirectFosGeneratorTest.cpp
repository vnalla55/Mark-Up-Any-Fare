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

#include "Pricing/Shopping/FOS/AdditionalDirectFosGenerator.h"

#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/Shopping/FOS/FosFilter.h"
#include "Pricing/Shopping/FOS/FosFilterComposite.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"
#include "Pricing/Shopping/FOS/FosTypes.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/LegsBuilder.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TseServerStub.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include <boost/assign/std/vector.hpp>
#include <boost/range.hpp>

#include <sstream>
namespace tse
{
namespace fos
{

// =================================
// LEGS DATA
// =================================

static DateTime obDate = DateTime(2013, 06, 01);
static DateTime ibDate = DateTime(2013, 06, 02);
static DateTime ibDate2 = DateTime(2013, 06, 03);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment AlreadyGeneratedSegments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "AA", "JFK", "DFW", "AA", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 1, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
};

const LegsBuilder::Segment MinimumConnectTimeSegments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(obDate, 11), DT(ibDate, 11) }, // direct SOP
  { 1, 1, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
};

const LegsBuilder::Segment Segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "AA", "JFK", "DFW", "AA", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 0, 1, "LH", "JFK", "DFW", "LH", DT(obDate, 10), DT(obDate, 11) }, // direct SOP
  { 1, 0, "AA", "DFW", "JFK", "AA", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 1, "LH", "DFW", "JFK", "LH", DT(ibDate, 10), DT(ibDate, 11) }, // direct SOP
  { 1, 2, "LH", "DFW", "EWR", "LH", DT(ibDate, 10), DT(ibDate, 11) },
  { 1, 2, "LH", "EWR", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) }
};
#undef DT

const ValidatorBitMask DummyValidatorBit = 12451431u;
const ValidatorBitMask UnusedValidatorBit = 1700000u;

// ==================================
// TEST CLASS
// ==================================

class AdditionalDirectFosGeneratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AdditionalDirectFosGeneratorTest);

  CPPUNIT_TEST(testGetNextCombinationNoValidators);
  CPPUNIT_TEST(testGetNextCombinationAlreadyGenerated);
  CPPUNIT_TEST(testGetNextCombinationMinimumConnect);

  CPPUNIT_TEST_SUITE_END();

private:
  // MOCKS
  class FosFilterMock : public FosFilter
  {
  public:
    FilterType getType() const { return FILTER_NONRESTRICTION; }
    ValidatorBitMask getAffectedValidators() const { return DummyValidatorBit; }
    bool isApplicableSolution(const SopCombination& sopCombination) const { return true; }
  };

  TestMemHandle _memHandle;
  ShoppingTrx* _trx;
  FosFilterMock* _filterMock;
  FosFilterComposite* _filterComposite;
  AdditionalDirectFosGenerator* _generator;
  FosStatistic* _fosStatistic;

  CarrierCode _cxr;
  PricingOrchestratorDerived* _po;
  BitmapOpOrderer* _orderer;
  ShoppingPQDerived* _pq;

  void initAlreadyGeneratedSegments()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(AlreadyGeneratedSegments, boost::size(AlreadyGeneratedSegments));
    builder.endBuilding();
  }

  void initMinimumConnectTimeSegments()
  {
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(MinimumConnectTimeSegments, boost::size(AlreadyGeneratedSegments));
    builder.endBuilding();
  }

  void initGenerator()
  {
    _generator->initGenerators();
    _generator->addPredicates();
  }

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx);

    _cxr = "DL";
    _po = new PricingOrchestratorDerived(*_memHandle.create<TseServerStub>());
    _orderer = new BitmapOpOrderer(Global::config());
    _pq = new ShoppingPQDerived(*_po, *_trx, 4, 0, &_cxr, *_orderer);

    _fosStatistic = _memHandle.create<FosStatistic>(*_trx);
    _filterMock = _memHandle.create<FosFilterMock>();
    _filterComposite = _memHandle.create<FosFilterComposite>(*_fosStatistic);

    _filterComposite->addFilter(*_filterMock);
    _generator = _memHandle.create<AdditionalDirectFosGenerator>(*_trx, *_filterComposite);
  }

  void tearDown()
  {
    delete _po;
    _po = NULL;
    delete _pq;
    _pq = NULL;
    delete _orderer;
    _orderer = NULL;
    _memHandle.clear();
  }

  void testGetNextCombinationNoValidators()
  {
    std::vector<int> comb;

    initAlreadyGeneratedSegments();
    initGenerator();

    CPPUNIT_ASSERT(!_generator->getNextCombination(0, comb));
  }

  void testGetNextCombinationAlreadyGenerated()
  {
    using namespace boost::assign;

    FosFilterMock dummyFilter;

    std::vector<int> generatedComb, comb;
    generatedComb += 0, 0;
    GroupFarePath* gfp = _memHandle.create<GroupFarePath>();

    _trx->shoppingPQVector() += (tse::ShoppingTrx::PQPtr)_pq;
    insert(_trx->shoppingPQVector()[0]->flightMatrix())
    (generatedComb, gfp);

    initAlreadyGeneratedSegments();
    initGenerator();

    CPPUNIT_ASSERT(_generator->getNextCombination(DummyValidatorBit, comb));
    CPPUNIT_ASSERT_EQUAL(0, comb[0]);
    CPPUNIT_ASSERT_EQUAL(1, comb[1]);

    comb.clear();
    CPPUNIT_ASSERT(!_generator->getNextCombination(DummyValidatorBit, comb));
  }

  void testGetNextCombinationMinimumConnect()
  {
    std::vector<int> comb;

    initMinimumConnectTimeSegments();
    _trx->getOptions()->setMinConnectionTimeDomestic(1800); // half an hour
    initGenerator();

    CPPUNIT_ASSERT(_generator->getNextCombination(DummyValidatorBit, comb));
    CPPUNIT_ASSERT_EQUAL(0, comb[0]);
    CPPUNIT_ASSERT_EQUAL(1, comb[1]);

    comb.clear();
    CPPUNIT_ASSERT(!_generator->getNextCombination(DummyValidatorBit, comb));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(AdditionalDirectFosGeneratorTest);

} // fos
} // tse
