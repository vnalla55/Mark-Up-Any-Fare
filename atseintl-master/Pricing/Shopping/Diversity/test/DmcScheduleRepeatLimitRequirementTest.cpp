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
  friend class SRLTest;

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

class SRLTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SRLTest);
  CPPUNIT_TEST(SRLByDefaultEqualsZero);
  // CPPUNIT_TEST(getThrowAwayCombinationTest);
  CPPUNIT_TEST(correctStatsEnabled);
  CPPUNIT_TEST_SUITE_END();

public:
  void SRLByDefaultEqualsZero()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx->getRequest()->getScheduleRepeatLimit() == 0);
  }

  void correctStatsEnabled()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    // Diversity Model Basic has to have sop pairing nebled in order for SRL to work
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    tse::Billing billing;
    _trx->billing() = &billing;
    _trx->billing()->partitionID() = "AA";

    ItinStatistic* stats = _memHandle.create<ItinStatistic>(*_trx);
    DiagCollector* dc = 0;
    DiversityModelBasic* model = _memHandle.create<DiversityModelBasic>(*_trx, *stats, dc);
    int32_t stats_enabled = stats->getEnabledStatistics(model);

    CPPUNIT_ASSERT(stats_enabled & ItinStatistic::STAT_SOP_PAIRING);
  }

  void getThrowAwayCombinationTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "AA", 0, true);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "FRA", "AA");
    _trx->legs().front().sop().front().governingCarrier() = "AA";

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 1, "IB", 0, true);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "FRA", "IB");
    _trx->legs().front().sop().back().governingCarrier() = "AA";

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 0, "AA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "FRA", "SYD", "AA", DateTime::localTime().addDays(7));
    _trx->legs().back().sop().front().governingCarrier() = "AA";

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 1, "AA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "FRA", "SYD", "BB", DateTime::localTime().addDays(7));
    _trx->legs().back().sop().front().governingCarrier() = "AA";

    MockItinStatistic* stats = _memHandle.create<MockItinStatistic>(*_trx);
    DiagCollector* diag = NULL;
    DmcRequirementsFacade* sharedCtx =
        _memHandle.create<DmcRequirementsFacade>(*stats, diag, *_trx);

    _trx->getRequest()->setScheduleRepeatLimit(0);

    stats->addNonStopCarrier("AA");
    DmcScheduleRepeatLimitRequirement* dmcSRLReq =
        _memHandle.create<DmcScheduleRepeatLimitRequirement>(*sharedCtx);

    MockSOPCombination* sopCombination;
    _memHandle.get(sopCombination);
    std::vector<int> combination1;
    combination1.push_back(0);
    combination1.push_back(0);
    sopCombination->setOSopVec(combination1);
    stats->setEnabledStatistics(ItinStatistic::STAT_SOP_PAIRING, this);

    // With SRL=0 this requirement should always return false
    bool status = dmcSRLReq->getThrowAwayCombination(sopCombination->oSopVec);
    CPPUNIT_ASSERT(status == false);

    stats->addFOS(combination1);
    status = dmcSRLReq->getThrowAwayCombination(sopCombination->oSopVec);
    CPPUNIT_ASSERT(status == false);

    // Now we set SRL to 1. It should not be possble to add a combination with the sops ids that
    // have already been used

    _trx->getRequest()->setScheduleRepeatLimit(1);
    DmcScheduleRepeatLimitRequirement* dmcSRLReq2 =
        _memHandle.create<DmcScheduleRepeatLimitRequirement>(*sharedCtx);
    status = dmcSRLReq2->getThrowAwayCombination(sopCombination->oSopVec);
    CPPUNIT_ASSERT(status == true);

    // try combination where only one sop fails
    std::vector<int> combination2;
    combination2.push_back(1);
    combination2.push_back(0);
    MockSOPCombination* sopCombination2;
    _memHandle.get(sopCombination2);
    sopCombination2->setOSopVec(combination2);
    status = dmcSRLReq2->getThrowAwayCombination(sopCombination2->oSopVec);
    bool status_operator = dmcSRLReq2->operator()(sopCombination2->oSopVec);

    CPPUNIT_ASSERT(status == true);
    // Operator() in DmcScheduleRepeatLimitRequirement should give an opposite result from
    // getThrowAwayCombination
    CPPUNIT_ASSERT(status_operator == false);

    // now something that has not been used before

    std::vector<int> combination3;
    combination3.push_back(1);
    combination3.push_back(1);
    MockSOPCombination* sopCombination3;
    _memHandle.get(sopCombination3);
    sopCombination3->setOSopVec(combination3);
    status = dmcSRLReq2->getThrowAwayCombination(sopCombination3->oSopVec);

    CPPUNIT_ASSERT(status == false);

    // Set SRL to value higher then 1 ( so first sop can pass )

    _trx->getRequest()->setScheduleRepeatLimit(2);
    DmcScheduleRepeatLimitRequirement* dmcSRLReq3 =
        _memHandle.create<DmcScheduleRepeatLimitRequirement>(*sharedCtx);
    status = dmcSRLReq3->getThrowAwayCombination(sopCombination->oSopVec);

    CPPUNIT_ASSERT(status == false);

    // add combination 3
    stats->addFOS(combination3);
    // now all sops are used once. It should be possible to add combination2 with SRL = 2

    status = dmcSRLReq3->getThrowAwayCombination(sopCombination2->oSopVec);
    CPPUNIT_ASSERT(status == false);

    // but not with SRL =1
    status = dmcSRLReq2->getThrowAwayCombination(sopCombination2->oSopVec);
    CPPUNIT_ASSERT(status == true);

    // it should be possible to add it with SRL disabled ( SRL =0 )
    status = dmcSRLReq->getThrowAwayCombination(sopCombination2->oSopVec);
    status_operator = dmcSRLReq->operator()(sopCombination2->oSopVec);

    CPPUNIT_ASSERT(status == false);
    // Operator() in DmcScheduleRepeatLimitRequirement should give an opposite result from
    // getThrowAwayCombination
    CPPUNIT_ASSERT(status_operator == true);
  }

  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SRLTest);

} // shpq
} // tse
