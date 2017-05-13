// -------------------------------------------------------------------
//
//! \author       Artur de Sousa Rocha
//! \date         08-05-2013
//! \file         DmcLimitLongConxRequirementTest.cpp
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

#include "Pricing/Shopping/Diversity/DmcLimitLongConxRequirement.h"
#include "Pricing/Shopping/Diversity/DmcRequirementsFacade.h"
#include "Pricing/Shopping/PQ/SOPCombination.h"
#include "Pricing/Shopping/PQ/test/TestPQItem.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
namespace shpq
{

struct MockSOPCombination : public SOPCombination
{
public:
  void setOSopVec(std::vector<int> solution)
  {
    oSopVec.resize(solution.size());
    for (uint32_t i = 0; i < solution.size(); i++)
    {
      oSopVec[i] = solution[i];
    }
  }
};

class MockItinStatistic : public ItinStatistic
{
  friend class DmcLimitLongConxRequirementTest;

public:
  MockItinStatistic(ShoppingTrx& trx) : ItinStatistic(trx) {}
};

class DmcLimitLongConxRequirementTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DmcLimitLongConxRequirementTest);
  CPPUNIT_TEST(getThrowAwayCombinationTest);
  CPPUNIT_TEST(getThrowAwaySopTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void getThrowAwayCombinationTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    typedef PricingOrchestratorTestShoppingCommon TestCommon;
    ShoppingTrx* _trx =
        TestCommon::createOwTrx(*dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);

    TestCommon::createSOP(*dataHandle, _trx, _trx->legs()[0], 0, "AA", 0, false, true);
    TestCommon::addSegmentToItinerary(*dataHandle, _trx->legs()[0], "SYD", "FRA", "AA");

    TestCommon::createSOP(*dataHandle, _trx, _trx->legs()[0], 1, "IB", 0);
    TestCommon::addSegmentToItinerary(*dataHandle, _trx->legs()[0], "SYD", "FRA", "IB");

    TestCommon::createSOP(*dataHandle, _trx, _trx->legs()[0], 0, "LH", 0);
    TestCommon::addSegmentToItinerary(*dataHandle, _trx->legs()[0], "SYD", "FRA", "LH");

    _trx->maxNumOfLngCnxSolutions() = 2;
    _trx->diversity().setMaxLongConnectionSolutions(_trx->maxNumOfLngCnxSolutions());
    DiagCollector* diag = NULL;
    MockItinStatistic* stats = _memHandle.create<MockItinStatistic>(*_trx);
    stats->_longConnectionCount = 3;

    DmcRequirementsFacade* sharedCtx =
        _memHandle.create<DmcRequirementsFacade>(*stats, diag, *_trx);
    DmcLimitLongConxRequirement* dmcLongConxReq =
        _memHandle.create<DmcLimitLongConxRequirement>(*sharedCtx);

    std::vector<int> customSolution;
    customSolution.push_back(0);

    MockSOPCombination* sopCombination = _memHandle.create<MockSOPCombination>();
    sopCombination->setOSopVec(customSolution);

    bool throwAway = dmcLongConxReq->getThrowAwayCombination(sopCombination->oSopVec);
    CPPUNIT_ASSERT(throwAway);

    stats->_longConnectionCount = 1;
    throwAway = dmcLongConxReq->getThrowAwayCombination(sopCombination->oSopVec);
    CPPUNIT_ASSERT(!throwAway);
  }

  void getThrowAwaySopTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    typedef PricingOrchestratorTestShoppingCommon TestCommon;

    ShoppingTrx* trx =
        TestCommon::createOwTrx(*dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    TestCommon::createSOP(*dataHandle, trx, trx->legs()[0], 0, "AA", 0, false, true);
    ShoppingTrx::SchedulingOption& sop = trx->legs()[0].sop()[0];
    trx->diversity().setMaxLongConnectionSolutions(1);

    MockItinStatistic stats(*trx);
    stats._longConnectionCount = 0;

    DmcRequirementsFacade sharedCtx(stats, 0, *trx);
    DmcLimitLongConxRequirement dmcLongConxReq(sharedCtx);

    CPPUNIT_ASSERT(!dmcLongConxReq.getThrowAwaySop(0, 0));

    stats._longConnectionCount = 1;
    CPPUNIT_ASSERT(dmcLongConxReq.getThrowAwaySop(0, 0));

    sop.isLngCnxSop() = false;
    CPPUNIT_ASSERT(!dmcLongConxReq.getThrowAwaySop(0, 0));
  }

  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DmcLimitLongConxRequirementTest);
} // shpq
} // tse
