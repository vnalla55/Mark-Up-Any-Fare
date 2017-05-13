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

#include <string>
#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/Agent.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloPQ.h"
#include "Pricing/Shopping/PQ/SoloPQItemManager.h"
#include "Pricing/Shopping/PQ/SoloTrxData.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"
#include "test/include/TseServerStub.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestShoppingTrxFactory.h"

#define ARRSIZE(x) (sizeof(x) / sizeof(x[0]))

namespace tse
{
namespace shpq
{

const LocCode LocStr[] = { "LAX", "SFO", "JFK", "DFW" };
const CarrierCode Carrier[] = { "AA", "DL" };

// ==================================
// TEST CLASS
// ==================================

class SPPQItemTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SPPQItemTest);
  CPPUNIT_TEST(testItemAttributes);
  CPPUNIT_TEST(testExpansion);
  CPPUNIT_TEST_SUITE_END();

private:
  ShoppingTrx* _trx;
  Loc* _loc[ARRSIZE(LocStr)];
  DirFMPathListPtr _outboundOWOW;
  DirFMPathListPtr _inboundOW;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    init();
  }

  void tearDown() { _memHandle.clear(); }

  void testItemAttributes()
  {
    TseServerStub server;
    PricingOrchestrator po(server);
    SoloTrxData soloTrxData(*_trx, po);
    SolutionPatternPQItemPtr item = soloTrxData.getPQItemManager().constructSPPQItem(
        getSP(SolutionPattern::SP30), _outboundOWOW, _inboundOW);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Level", SoloPQItem::SP_LEVEL, item->getLevel());
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Score", 270.0, item->getScore(), EPSILON);
  }

  void testExpansion()
  {
    typedef std::vector<SoloPQItemPtr> SoloPQItemVec;

    TseServerStub server;
    PricingOrchestrator po(server);
    SoloTrxData soloTrxData(*_trx, po);
    ItinStatistic stats(*_trx);
    SoloPQ pq(*_trx, stats, 0);

    SoloPQItemVec expandedItems;

    SolutionPatternPQItemPtr item = soloTrxData.getPQItemManager().constructSPPQItem(
        getSP(SolutionPattern::SP30), _outboundOWOW, _inboundOW);

    pq.enqueue(item);
    while (!pq.empty())
    {
      SoloPQItemPtr newItem = pq.dequeue();
      if (newItem->getLevel() == SoloPQItem::SP_LEVEL)
        newItem->expand(soloTrxData, pq);
      else
        expandedItems.push_back(newItem);
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Number of expanded items", static_cast<size_t>(2u), expandedItems.size());

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Expanded item 0: level", SoloPQItem::CR_LEVEL, expandedItems[0]->getLevel());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Expanded item 1: level", SoloPQItem::CR_LEVEL, expandedItems[1]->getLevel());

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
        "Expanded item 0: score", 270.0, expandedItems[0]->getScore(), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
        "Expanded item 1: score", 350.0, expandedItems[1]->getScore(), EPSILON);
  }

private:
  void init()
  {
    _trx =
        TestShoppingTrxFactory::create("/vobs/atseintl/test/testdata/data/ShoppingTrx.xml", true);
    CPPUNIT_ASSERT(_trx);

    // It's needed by SoloTrxData's member
    _trx->getRequest()->ticketingAgent() = _trx->dataHandle().create<Agent>();
    _trx->getRequest()->ticketingAgent()->agentCity() = "DFW";

    // It's needed by SoloPQ constructor
    _trx->journeyItin() = _trx->dataHandle().create<Itin>();
    _trx->journeyItin()->travelSeg().push_back(_trx->dataHandle().create<AirSeg>());
    _trx->journeyItin()->travelSeg().push_back(_trx->dataHandle().create<AirSeg>());

    for (size_t i = 0; i < ARRSIZE(LocStr); i++)
      _loc[i] = createLoc(LocStr[i]);

    _trx->journeyItin()->travelSeg().front()->origin() = _loc[0];
    _trx->journeyItin()->travelSeg().front()->destination() = _loc[1];

    initOutbound();
    initInbound();
  }

  Loc* createLoc(const LocCode& code)
  {
    Loc* loc = _trx->dataHandle().create<Loc>();
    loc->loc() = code;
    return loc;
  }

  void initOutbound()
  {
    CxrFareMarketsPtr localOwCFM11 = CxrFareMarkets::create(*_trx, OW);
    CxrFareMarketsPtr localOwCFM12 = CxrFareMarkets::create(*_trx, OW);
    CxrFareMarketsPtr localOwCFM21 = CxrFareMarkets::create(*_trx, OW);
    CxrFareMarketsPtr localOwCFM22 = CxrFareMarkets::create(*_trx, OW);

    localOwCFM11->insert(createOwrtFareMarket(OW, _loc[0], _loc[1], Carrier[0], 30.0));
    localOwCFM12->insert(createOwrtFareMarket(OW, _loc[1], _loc[3], Carrier[0], 40.0));
    localOwCFM21->insert(createOwrtFareMarket(OW, _loc[0], _loc[2], Carrier[0], 50.0));
    localOwCFM22->insert(createOwrtFareMarket(OW, _loc[2], _loc[3], Carrier[0], 100.0));

    DirFMPathPtr owOwFMPath1 = DirFMPath::create(*_trx, localOwCFM11, localOwCFM12);
    DirFMPathPtr owOwFMPath2 = DirFMPath::create(*_trx, localOwCFM21, localOwCFM22);

    _outboundOWOW = DirFMPathList::create(*_trx);
    _outboundOWOW->insert(owOwFMPath1);
    _outboundOWOW->insert(owOwFMPath2);
  }

  void initInbound()
  {
    CxrFareMarketsPtr thruOwCFM = CxrFareMarkets::create(*_trx, OW);

    thruOwCFM->insert(createOwrtFareMarket(OW, _loc[3], _loc[0], Carrier[0], 200.0));
    thruOwCFM->insert(createOwrtFareMarket(OW, _loc[3], _loc[0], Carrier[1], 400.0));

    DirFMPathPtr owFMPath = DirFMPath::create(*_trx, thruOwCFM);

    _inboundOW = DirFMPathList::create(*_trx);
    _inboundOW->insert(owFMPath);
  }

  OwrtFareMarketPtr createOwrtFareMarket(
      SolutionType st, const Loc* org, const Loc* dst, const CarrierCode& cxr, MoneyAmount amt)
  {
    FareMarket* fm = _trx->dataHandle().create<FareMarket>();
    fm->origin() = org;
    fm->destination() = dst;
    fm->governingCarrier() = cxr;
    fm->boardMultiCity() = org->loc();
    fm->offMultiCity() = dst->loc();
    fm->allPaxTypeFare().push_back(createPaxTypeFare(amt));
    return OwrtFareMarket::create(*_trx, st, fm, 0);
  }

  PaxTypeFare* createPaxTypeFare(MoneyAmount amt)
  {
    PaxTypeFare* paxTypeFare = TestPaxTypeFareFactory::create("testdata/PaxTypeFare.xml", true);
    paxTypeFare->setIsShoppingFare();
    paxTypeFare->fare()->nucFareAmount() = amt;
    return paxTypeFare;
  }

  const SolutionPattern& getSP(SolutionPattern::SPEnumType spType)
  {
    return SolutionPatternStorage::instance().getSPById(spType);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SPPQItemTest);

} /* namespace shpq */
} /* namespace tse */
