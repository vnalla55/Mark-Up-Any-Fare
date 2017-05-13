// -------------------------------------------------------------------
//
//! \author       Michal Mlynek
//! \date         11-06-2013
//! \file         IbfTest.cpp
//! \brief
//!
//!  Copyright (C) Sabre 2013
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
// -------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include "Pricing/Shopping/Diversity/DiversityModelBasic.h"
#include "Pricing/Shopping/Diversity/DmcBrandedFaresRCOnlineRequirement.h"
#include "Pricing/Shopping/PQ/SoloBrandedFaresFlightOnlySolutions.h"
#include "DataModel/Billing.h"
#include "Pricing/Shopping/Diversity/DmcRequirementsFacade.h"
#include "Pricing/Shopping/PQ/SOPCombination.h"
#include "Common/TseConsts.h"

#include "Pricing/Shopping/PQ/test/TestPQItem.h"

namespace tse
{
namespace shpq
{

struct MockSOPCombination : public SOPCombination
{
public:
  MockSOPCombination() {}

  void setOSopVec(std::vector<int> solution)
  {
    oSopVec.clear();
    oSopVec.resize(solution.size());
    for (uint32_t i = 0; i < solution.size(); i++)
    {
      oSopVec[i] = solution[i];
    }
  }
};

class MockItinStatistic : public ItinStatistic
{
  friend class IbfTest;

public:
  MockItinStatistic(ShoppingTrx& trx) : ItinStatistic(trx) {}

protected:
  void addNonStopCarrier(CarrierCode cxr, size_t noAlreadyFoundSolutions = 0)
  {
    _nonStopCarriers[cxr] = noAlreadyFoundSolutions;
  }
};
class MockDiversity : public Diversity
{
  friend class IbfTest;

protected:
  void addMaxNonStopCountPerCarrier(CarrierCode cxr, size_t noWantedSolutions = 1)
  {
    _maxNonStopCountPerCarrier[cxr] = noWantedSolutions;
  }
};

class IbfTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(IbfTest);
  CPPUNIT_TEST(correctStatsEnabled);
  CPPUNIT_TEST(RCcombinationCouldSatisfyTest);
  CPPUNIT_TEST(GenerateRCOnlineAsFos);
  CPPUNIT_TEST_SUITE_END();

public:
  void correctStatsEnabled()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    _trx->getRequest()->setBrandedFaresRequest(true);
    tse::Billing billing;
    _trx->billing() = &billing;
    _trx->billing()->partitionID() = "AA";

    ItinStatistic* stats = _memHandle.create<ItinStatistic>(*_trx);
    DiagCollector* dc = 0;
    DiversityModelBasic* model = _memHandle.create<DiversityModelBasic>(*_trx, *stats, dc);
    int32_t stats_enabled = stats->getEnabledStatistics(model);

    CPPUNIT_ASSERT(stats_enabled & ItinStatistic::STAT_RC_ONLINES);
    CPPUNIT_ASSERT(stats_enabled & ItinStatistic::STAT_UNUSED_SOPS);
  }

  void GenerateRCOnlineAsFos()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    _trx->getRequest()->setBrandedFaresRequest(true);
    tse::Billing billing;
    _trx->billing() = &billing;
    _trx->billing()->partitionID() = "JL";
    _trx->setAltDates(false);

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "JL", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "JL", DateTime::localTime().addDays(1));
    _trx->legs().front().sop().front().governingCarrier() = "JL";
    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 0, "JL", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "KRK", "SYD", "JL", DateTime::localTime().addDays(3));
    _trx->legs().back().sop().front().governingCarrier() = "JL";

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 1, "JL", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "JL", DateTime::localTime().addDays(1));
    _trx->legs().front().sop().back().governingCarrier() = "JL";
    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 1, "KE", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "KRK", "SYD", "KE", DateTime::localTime().addDays(3));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 2, "LH", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "LH", DateTime::localTime().addDays(1));
    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 2, "LH", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "KRK", "SYD", "LH", DateTime::localTime().addDays(3));

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 3, "LX", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "KRK", "LX", DateTime::localTime().addDays(1));
    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 3, "JL", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "KRK", "SYD", "JL", DateTime::localTime().addDays(3));
    _trx->legs().back().sop().back().governingCarrier() = "JL";

    _trx->legs().front().requestSops() = 4;
    _trx->legs().back().requestSops() = 4;

    /*MockDiversity* diversity = _memHandle.create<MockDiversity>();
    diversity->initialize(0, *_trx, std::map<ItinIndex::Key, CarrierCode>());
    diversity->addMaxNonStopCountPerCarrier("JL", 0);
    diversity->setBucketDistribution(Diversity::GOLD, 0.0);
    diversity->setBucketDistribution(Diversity::LUXURY, 0.0);
    diversity->setBucketDistribution(Diversity::JUNK, 0.0);
    diversity->setBucketDistribution(Diversity::UGLY, 0.0);
    */
    MockItinStatistic* stats = _memHandle.create<MockItinStatistic>(*_trx);
    DiagCollector* dc = 0;
    /*DiversityModelBasic* model = */ _memHandle.create<DiversityModelBasic>(
        *_trx, *stats, dc);
    // diversity->addMaxNonStopCountPerCarrier("JL", 0);
    stats->setMissingRCOnlineOptionsCount(4);
    stats->addNonStopCarrier("JL");

    size_t unusedRCOnlines = stats->getUnusedRCOnlineCombs().size();
    CPPUNIT_ASSERT(unusedRCOnlines == 4);

    // Generate missing flights and missing RCOnlines as Foses
    // fos::SoloBrandedFaresFlightOnlySolutions ibfFosGenerator(*_trx, *stats);
    // fos::MissingRCOnlinesIterator missingRco(*_trx, stats->getUnusedRCOnlineCombs());
    // ibfFosGenerator(missingRco,4);
  }

  void RCcombinationCouldSatisfyTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml");
    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "AA", 0, true);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "FRA", "AA");
    _trx->legs().front().sop().front().governingCarrier() = "AA";

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 1, "IB", 0, true);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "FRA", "IB");
    _trx->legs().front().sop().back().governingCarrier() = "IB";

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 0, "AA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "FRA", "SYD", "AA", DateTime::localTime().addDays(7));
    _trx->legs().back().sop().front().governingCarrier() = "AA";

    tse::Billing billing;
    _trx->billing() = &billing;
    _trx->billing()->partitionID() = "AA";
    _trx->getRequest()->setBrandedFaresRequest(true);

    ItinStatistic* stats = _memHandle.create<ItinStatistic>(*_trx);
    stats->setMissingRCOnlineOptionsCount(1);
    DiagCollector* diag = NULL;
    DmcRequirementsFacade* sharedCtx =
        _memHandle.create<DmcRequirementsFacade>(*stats, diag, *_trx);
    DmcBrandedFaresRCOnlineRequirement* dmcRCOnlineReq =
        _memHandle.create<DmcBrandedFaresRCOnlineRequirement>(*sharedCtx);

    std::vector<int> RCOnlineSolution;
    RCOnlineSolution.push_back(0);
    RCOnlineSolution.push_back(0);

    std::vector<int> NotRCOnlineSolution;
    NotRCOnlineSolution.push_back(1);
    NotRCOnlineSolution.push_back(0);

    MockSOPCombination* sopCombination;
    _memHandle.get(sopCombination);
    sopCombination->setOSopVec(RCOnlineSolution);

    DmcRequirement::Value statusRC =
        dmcRCOnlineReq->getCombinationCouldSatisfy(sopCombination->oSopVec, 15.0);

    CPPUNIT_ASSERT(statusRC == DmcRequirement::NEED_RC_ONLINES);

    stats->setMissingRCOnlineOptionsCount(0);
    statusRC = dmcRCOnlineReq->getCombinationCouldSatisfy(sopCombination->oSopVec, 15.0);
    CPPUNIT_ASSERT(!statusRC);

    stats->setMissingRCOnlineOptionsCount(1);
    sopCombination->setOSopVec(NotRCOnlineSolution);
    DmcRequirement::Value statusNotRC =
        dmcRCOnlineReq->getCombinationCouldSatisfy(sopCombination->oSopVec, 15.0);

    CPPUNIT_ASSERT(!statusNotRC);
  }

  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(IbfTest);

} // shpq
} // tse
