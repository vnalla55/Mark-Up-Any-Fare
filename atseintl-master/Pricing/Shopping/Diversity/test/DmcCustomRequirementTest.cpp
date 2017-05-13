// -------------------------------------------------------------------
//
//! \author       Natalia Walus
//! \date         25-03-2013
//! \file         DmcCustomRequirementTest.cpp
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

#include "Pricing/Shopping/Diversity/DmcCustomRequirement.h"
#include "Pricing/Shopping/Diversity/DmcRequirementsFacade.h"
#include "Pricing/Shopping/PQ/SOPCombination.h"
#include "Common/TseConsts.h"

#include "Pricing/Shopping/PQ/test/TestPQItem.h"

namespace tse
{
namespace shpq
{

class CustomTestPQItem;

typedef CustomTestPQItem* CustomTestPQItemPtr;
typedef FareMarket* FareMarketPtr;
typedef std::vector<FareMarketPtr> FMVector;

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

class CustomTestPQItem : public test::TestPQItem
{
public:
  CustomTestPQItem(const MoneyAmount score,
                   const SoloPQItemLevel level,
                   const SolutionPattern* sp = 0)
    : TestPQItem(score, level, sp)
  {
    _fmVector.clear();
  }

  static CustomTestPQItemPtr
  create(const MoneyAmount score, const SoloPQItemLevel level, const SolutionPattern* sp = 0)
  {
    return CustomTestPQItemPtr(new CustomTestPQItem(score, level, sp));
  }

  void visitFareMarkets(FareMarketVisitor& visitor) const
  {
    FMVector::const_iterator it = _fmVector.begin();
    for (; it != _fmVector.end(); ++it)
    {
      visitor.visit(*it);
    }
  }

  void addFareMarket(FareMarketPtr fm) { _fmVector.push_back(fm); }

private:
  FMVector _fmVector;
};

class MockItinStatistic : public ItinStatistic
{
  friend class DmcCustomRequirementTest;

public:
  MockItinStatistic(ShoppingTrx& trx) : ItinStatistic(trx) {}
};

class DmcCustomRequirementTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DmcCustomRequirementTest);
  CPPUNIT_TEST(combinationCouldSatisfyTest);
  CPPUNIT_TEST(pqItemCouldSatisfyTest);
  CPPUNIT_TEST(getThrowAwayCombinationTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void pqItemCouldSatisfyTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "AA", 0, true);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "FRA", "AA");

    _trx->setNumOfCustomSolutions(1);

    ItinStatistic* stats = _memHandle.create<ItinStatistic>(*_trx);
    DiagCollector* diag = NULL;
    DmcRequirementsFacade* sharedCtx =
        _memHandle.create<DmcRequirementsFacade>(*stats, diag, *_trx);
    DmcCustomRequirement* dmcCustomReq =
        _memHandle.create<DmcCustomRequirement>(*sharedCtx);

    CustomTestPQItemPtr pqItem = CustomTestPQItem::create(50.0, SoloPQItem::CRC_LEVEL);
    FareMarketPtr fm1;
    _memHandle.get(fm1);
    _trx->setCustomSolutionFM(fm1);
    CPPUNIT_ASSERT(_trx->isCustomSolutionFM(fm1));

    pqItem->addFareMarket(fm1);
    DmcRequirement::Value statusCustom = dmcCustomReq->getPQItemCouldSatisfy(pqItem);
    CPPUNIT_ASSERT(statusCustom == DmcRequirement::NEED_CUSTOM);

    FareMarketPtr fm2;
    _memHandle.get(fm2);
    fm2->legIndex() = 0;
    CustomTestPQItemPtr pqItem2 = CustomTestPQItem::create(50.0, SoloPQItem::CRC_LEVEL);
    pqItem2->addFareMarket(fm2);
    statusCustom = dmcCustomReq->getPQItemCouldSatisfy(pqItem2);
    CPPUNIT_ASSERT(!statusCustom);

    CustomTestPQItemPtr pqItem3 = CustomTestPQItem::create(50.0, SoloPQItem::SP_LEVEL);
    statusCustom = dmcCustomReq->getPQItemCouldSatisfy(pqItem3);
    CPPUNIT_ASSERT(statusCustom == DmcRequirement::NEED_CUSTOM);

    _trx->setNumOfCustomSolutions(0);
    CustomTestPQItemPtr pqItem4 = CustomTestPQItem::create(50.0, SoloPQItem::CRC_LEVEL);
    statusCustom = dmcCustomReq->getPQItemCouldSatisfy(pqItem4);
    CPPUNIT_ASSERT(!statusCustom);
  }

  void combinationCouldSatisfyTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "AA", 0, true);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "FRA", "AA");

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 1, "IB", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "FRA", "IB");

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[1], 0, "AA", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[1], "FRA", "SYD", "AA", DateTime::localTime().addDays(7));

    _trx->setNumOfCustomSolutions(1);

    ItinStatistic* stats = _memHandle.create<ItinStatistic>(*_trx);
    DiagCollector* diag = NULL;
    DmcRequirementsFacade* sharedCtx =
        _memHandle.create<DmcRequirementsFacade>(*stats, diag, *_trx);

    std::vector<int> customSolution;
    customSolution.push_back(0);
    customSolution.push_back(0);

    std::vector<int> normalSolution;
    normalSolution.push_back(1);
    normalSolution.push_back(0);

    DmcCustomRequirement* dmcCustomReq =
        _memHandle.create<DmcCustomRequirement>(*sharedCtx);

    MockSOPCombination* sopCombination;
    _memHandle.get(sopCombination);
    sopCombination->setOSopVec(customSolution);

    DmcRequirement::Value statusCustom =
        dmcCustomReq->getCombinationCouldSatisfy(sopCombination->oSopVec);

    sopCombination->setOSopVec(normalSolution);
    DmcRequirement::Value statusNormal =
        dmcCustomReq->getCombinationCouldSatisfy(sopCombination->oSopVec);

    CPPUNIT_ASSERT(statusCustom == DmcRequirement::NEED_CUSTOM);
    CPPUNIT_ASSERT(!statusNormal);
  }

  void getThrowAwayCombinationTest()
  {
    DataHandle* dataHandle = _memHandle.create<DataHandle>();
    ShoppingTrx* _trx = PricingOrchestratorTestShoppingCommon::createOwTrx(
        *dataHandle, "/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "AA", 0, true);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "FRA", "AA");

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 1, "IB", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "FRA", "IB");

    PricingOrchestratorTestShoppingCommon::createSOP(
        *dataHandle, _trx, _trx->legs()[0], 0, "LH", 0);
    PricingOrchestratorTestShoppingCommon::addSegmentToItinerary(
        *dataHandle, _trx->legs()[0], "SYD", "FRA", "LH");

    _trx->setNumOfCustomSolutions(2);
    DiagCollector* diag = NULL;
    MockItinStatistic* stats = _memHandle.create<MockItinStatistic>(*_trx);
    stats->_customSolutionCount = 3;

    DmcRequirementsFacade* sharedCtx =
        _memHandle.create<DmcRequirementsFacade>(*stats, diag, *_trx);
    DmcCustomRequirement* dmcCustomReq =
        _memHandle.create<DmcCustomRequirement>(*sharedCtx);

    std::vector<int> customSolution;
    customSolution.push_back(0);

    MockSOPCombination* sopCombination;
    _memHandle.get(sopCombination);
    sopCombination->setOSopVec(customSolution);

    bool throwAway = dmcCustomReq->getThrowAwayCombination(sopCombination->oSopVec);
    CPPUNIT_ASSERT(throwAway);

    stats->_customSolutionCount = 1;
    throwAway = dmcCustomReq->getThrowAwayCombination(sopCombination->oSopVec);
    CPPUNIT_ASSERT(!throwAway);
  }

  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(DmcCustomRequirementTest);
}
}
