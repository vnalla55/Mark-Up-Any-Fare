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

#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/SwapperBasicValidationStrategy.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/test/DiversityTestUtil.h"

#include "test/include/LegsBuilder.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#include "test/include/CppUnitHelperMacros.h"
#include <boost/range.hpp>

namespace tse
{

// =================================
// LEGS DATA
// =================================

static DateTime obDate = DateTime(2013, 06, 01);
static DateTime ibDate = DateTime(2013, 06, 02);

#define DT(date, hrs) DateTime(date, boost::posix_time::hours(hrs))
const LegsBuilder::Segment segments[] = {
  // leg, sop, gov,  org,   dst,   car,  dep,            arr
  { 0, 0, "LH", "JFK", "DFW", "LH", DT(obDate, 6), DT(obDate, 7) },
  { 0, 1, "AA", "JFK", "LAX", "AA", DT(obDate, 5), DT(obDate, 7) },
  { 0, 1, "AA", "LAX", "DFW", "AA", DT(obDate, 7), DT(obDate, 9) },
  { 1, 0, "LH", "DFW", "JFK", "LH", DT(ibDate, 12), DT(ibDate, 13) },
  { 1, 1, "AA", "DFW", "JFK", "AA", DT(ibDate, 12), DT(ibDate, 13) }
};
#undef DT

class MockItinStatistic : public ItinStatistic
{
public:
  MockItinStatistic(ShoppingTrx& trx) : ItinStatistic(trx) {}

  void setMinPrice(Diversity::BucketType bucket, MoneyAmount price)
  {
    _minPricePerBucket[bucket] = price;
  }
  void setMinPricedItinCount(const Diversity::BucketType bucket, size_t cnt)
  {
    _minPricedItinCount[bucket] = cnt;
  }
  void setNumOfItinsForCarrier(const CarrierCode& carrier, size_t num)
  {
    _carrierDiversity[carrier] = num;
  }
};

class MockDiversityModel : public DiversityModel
{
public:
  virtual PQItemAction getPQItemAction(const shpq::SoloPQItem* pqItem) { return USE; }
  virtual SOPCombinationList::iterator
  getDesiredSOPCombination(SOPCombinationList& combinations, MoneyAmount score, size_t fpKey)
  {
    return combinations.begin();
  }
  virtual bool getIsNewCombinationDesired(const SOPCombination& combination, MoneyAmount score)
  {
    return true;
  }
  virtual bool isNonStopNeededOnly() { return false; }
  virtual bool addSolution(const ShoppingTrx::FlightMatrix::value_type& solution,
                           ShoppingTrx::FlightMatrix& flightMatrix,
                           size_t farePathKey,
                           const DatePair* datePair)
  {
    return true;
  }
  virtual int getBucketStatus(const Diversity::BucketType bucket) const { return 0; }
  virtual bool isAdditionalNonStopEnabled() const { return false; }
  virtual bool isAdditionalNonStopOptionNeeded() const { return false; }

  void setNonStopOptionNeeded(bool nsneeded) { _nonStopNeeded = nsneeded; }
  virtual bool isNonStopOptionNeeded() const { return _nonStopNeeded; }

private:
  bool _nonStopNeeded;
};

// ==================================
// TEST CLASS
// ==================================

class SwapperBasicValidationStrategyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SwapperBasicValidationStrategyTest);
  CPPUNIT_TEST(testCheckCarrierDiversity);
  CPPUNIT_TEST(testCheckNonStopDiversity);
  CPPUNIT_TEST(testCheckLastMinPricedItin);
  CPPUNIT_TEST(testCheckCustomSolution);
  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  MockItinStatistic* _stats;
  MockDiversityModel* _model;
  NewSolutionAttributes _newSolution;
  TestMemHandle _memHandle;

public:
  SwapperBasicValidationStrategyTest()
  {
    _newSolution.bucket = Diversity::GOLD;
    _newSolution.carrier = "";
    _newSolution.todBucket = 0;
    _newSolution.isNonStop = false;
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    _stats = _memHandle.create<MockItinStatistic>(*_trx);
    _model = _memHandle.create<MockDiversityModel>();
    CPPUNIT_ASSERT(_trx);
    initTrx();
  }

  void tearDown() { _memHandle.clear(); }

  void testCheckCarrierDiversity()
  {
    SwapperBasicValidationStrategy strategy(*_trx, *_stats, _model);
    strategy.setNewSolutionAttributes(&_newSolution);
    strategy.setCurrentBucket(Diversity::GOLD);

    SopIdVec online(2, 0);
    SopIdVec interline(2, 0);
    interline[1] = 1;

    _trx->diversity().setOptionsPerCarrier("LH", 10.0f);
    _stats->setNumOfItinsForCarrier("LH", 9);
    _newSolution.carrier = "";

    CPPUNIT_ASSERT(SwapperEvaluationResult::CARRIERS == strategy.checkCarrierDiversity(online));
    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkCarrierDiversity(interline));

    _stats->setNumOfItinsForCarrier("LH", 11);
    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkCarrierDiversity(online));
  }

  void testCheckNonStopDiversity()
  {
    SwapperBasicValidationStrategy strategy(*_trx, *_stats, _model);
    strategy.setNewSolutionAttributes(&_newSolution);
    strategy.setCurrentBucket(Diversity::GOLD);

    shpq::DiversityTestUtil divUtil(_trx->diversity());
    SopIdVec nonstop(2, 0);
    SopIdVec notANonStop(2, 0);
    notANonStop[0] = 1;

    divUtil.setHighDensityMarket(true);
    CPPUNIT_ASSERT(SwapperEvaluationResult::NON_STOPS == strategy.checkNonStopDiversity(nonstop));
    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkNonStopDiversity(notANonStop));

    divUtil.setHighDensityMarket(false);
    _model->setNonStopOptionNeeded(true);
    _newSolution.isNonStop = false;
    CPPUNIT_ASSERT(SwapperEvaluationResult::NON_STOPS == strategy.checkNonStopDiversity(nonstop));
    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkNonStopDiversity(notANonStop));

    _newSolution.isNonStop = true;
    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkNonStopDiversity(nonstop));

    _newSolution.isNonStop = false;
    _model->setNonStopOptionNeeded(false);
    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkNonStopDiversity(nonstop));
  }

  void testCheckLastMinPricedItin()
  {
    SwapperBasicValidationStrategy strategy(*_trx, *_stats, _model);
    strategy.setNewSolutionAttributes(&_newSolution);
    strategy.setCurrentBucket(Diversity::GOLD);

    _stats->setMinPrice(Diversity::GOLD, 100.0);
    _stats->setMinPricedItinCount(Diversity::GOLD, 1);

    CPPUNIT_ASSERT(SwapperEvaluationResult::LAST_MIN_PRICED == strategy.checkLastMinPricedItin(100.0));
    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkLastMinPricedItin(200.0));

    _stats->setMinPricedItinCount(Diversity::GOLD, 2);
    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkLastMinPricedItin(100.0));
  }

  void testCheckCustomSolution()
  {
    SwapperBasicValidationStrategy strategy(*_trx, *_stats, _model);
    strategy.setNewSolutionAttributes(&_newSolution);
    strategy.setCurrentBucket(Diversity::GOLD);
    SopIdVec comb(2, 0);

    CPPUNIT_ASSERT(SwapperEvaluationResult::SELECTED == strategy.checkCustomSolution(comb));

    _trx->legs()[0].setCustomLeg(true);
    _trx->legs()[1].setCustomLeg(true);
    _trx->legs()[0].sop()[0].setCustomSop(true);
    _trx->legs()[1].sop()[0].setCustomSop(true);
    _trx->diversity().setNumOfCustomSolutions(1);
    CPPUNIT_ASSERT(SwapperEvaluationResult::CUSTOM_SOLUTION == strategy.checkCustomSolution(comb));
  }

private:
  void initTrx()
  {
    // init legs
    LegsBuilder builder(*_trx, _memHandle);
    builder.addSegments(segments, boost::size(segments));
    builder.endBuilding();
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwapperBasicValidationStrategyTest);

} /* namespace tse */
