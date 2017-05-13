#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Pricing/EstimatedSeatValue.h"
#include "Pricing/ESVPQ.h"
#include "Pricing/ESVPQItem.h"

using namespace tse;

namespace tse
{

class ESVTest : public CppUnit::TestFixture
{
  TestMemHandle _memHandle;

public:
  CPPUNIT_TEST_SUITE(ESVTest);
  CPPUNIT_TEST(createPQ);
  CPPUNIT_TEST(PQempty);
  CPPUNIT_TEST(PQemptyAfterInit);
  CPPUNIT_TEST(PQemptyWhenOneLegEmpty);
  CPPUNIT_TEST(PQemptyWhenTwoLegsEmpty);
  CPPUNIT_TEST(PQemptyWhenFirstLegEmptySecondLegNotEmpty);
  CPPUNIT_TEST(PQemptyWhenFirstLegNotEmptySecondLegEmpty);
  CPPUNIT_TEST(PQnotEmptyWhenFirstLegNotEmptySecondLegNotEmpty);
  CPPUNIT_TEST(oneElement);
  CPPUNIT_TEST(fewElementsInOneLeg);
  CPPUNIT_TEST(fewElementsInTwoLegs);
  CPPUNIT_TEST(PQItemNullWhenPQEmpty);
  CPPUNIT_TEST(PQItemEmptyWhenNoMoreElements);
  CPPUNIT_TEST(owFares);
  CPPUNIT_TEST(owFares2);
  CPPUNIT_TEST(owFares3);
  CPPUNIT_TEST(rtFares);
  CPPUNIT_TEST(rtFares2);
  CPPUNIT_TEST(rtFares3);
  CPPUNIT_TEST(owrtFares);
  CPPUNIT_TEST(owrtFaresInOneLeg);
  CPPUNIT_TEST(checkPQItemsWhenNoPQAnymore);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _outSopVec.clear();
    _inSopVec.clear();
  }

  void tearDown()
  {
    if (_trxP)
    {
      _trxP->paxType().clear();
      _trxP->legs().clear();
    }
    _outSopVec.clear();
    _inSopVec.clear();
    _memHandle.clear();
  }

  void createPQ()
  {
    createTrx();
    ESVPQ pq(_trxP, "a");
    CPPUNIT_ASSERT(true);
  }

  void PQempty()
  {
    createTrx();
    ESVPQ pq(_trxP, "a");
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
  }

  void PQemptyAfterInit()
  {
    createTrxWithNoLegs();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    CPPUNIT_ASSERT(_trxP->legs().size() == 0);
    CPPUNIT_ASSERT(!pq.allLegsNotEmpty());
    CPPUNIT_ASSERT(pq._pqItemPQ.size() == 0);
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
  }

  void PQemptyWhenOneLegEmpty()
  {
    createTrxWithFirstLegEmpty();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
  }

  void PQemptyWhenTwoLegsEmpty()
  {
    createTrx();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
  }

  void PQemptyWhenFirstLegEmptySecondLegNotEmpty()
  {
    createTrx();
    createSOP(_trxP->legs()[1], 0, "AA");
    CPPUNIT_ASSERT(_trxP->legs()[0].sop().size() == 0);
    CPPUNIT_ASSERT(_trxP->legs()[1].sop().size() == 1);
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
  }

  void PQemptyWhenFirstLegNotEmptySecondLegEmpty()
  {
    createTrx();
    createSOP(_trxP->legs()[0], 0, "AA", 1);
    CPPUNIT_ASSERT(_trxP->legs()[0].sop().size() == 1);
    CPPUNIT_ASSERT(_trxP->legs()[1].sop().size() == 0);
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
  }

  void PQnotEmptyWhenFirstLegNotEmptySecondLegNotEmpty()
  {
    createTrx();
    createSOP(_trxP->legs()[0], 0, "AA", 1);
    createSOP(_trxP->legs()[1], 1, "AA", 1);
    ESVPQ pq(_trxP, "a");
    copySopVecs();
    CPPUNIT_ASSERT(!_outSopVec.empty());
    CPPUNIT_ASSERT(!_inSopVec.empty());
    CPPUNIT_ASSERT(_outSopVec.size() == 1);
    CPPUNIT_ASSERT(_inSopVec.size() == 1);
    CPPUNIT_ASSERT(&(*_outSopVec[0]) == &(_trxP->legs()[0].sop()[0]));
    CPPUNIT_ASSERT(&*_inSopVec[0] == &_trxP->legs()[1].sop()[0]);
    CPPUNIT_ASSERT(!_outSopVec[0]->getDummy());
    CPPUNIT_ASSERT(!_inSopVec[0]->getDummy());
    CPPUNIT_ASSERT(_trxP->legs()[0].sop()[0].itin()->travelSeg().size() == 1);
    CPPUNIT_ASSERT(_trxP->legs()[1].sop()[0].itin()->travelSeg().size() == 1);
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    CPPUNIT_ASSERT(!pq._legVec.empty());
    CPPUNIT_ASSERT(pq._legVec.size() != 0);
    CPPUNIT_ASSERT(!pq._legVec[0]->empty());
    CPPUNIT_ASSERT(!pq._legVec[1]->empty());
    CPPUNIT_ASSERT(pq.allLegsNotEmpty());
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
  }

  void oneElement()
  {
    createTrx();
    createSOP(_trxP->legs()[0], 0, "AA", 1);
    createSOP(_trxP->legs()[1], 1, "AA", 1);
    ESVPQ pq(_trxP, "a");
    copySopVecs();
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    ESVPQItem* pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
    CPPUNIT_ASSERT(pqItem != NULL);
    std::vector<ShoppingTrx::SchedulingOption*> sopVec;
    CPPUNIT_ASSERT(pqItem->totalAmt() == 2);
  }

  void fewElementsInOneLeg()
  {
    createTrxWithFirstLegEmpty();
    createSOP(_trxP->legs()[0], 0, "AA", 2);
    createSOP(_trxP->legs()[0], 1, "AA", 3);
    createSOP(_trxP->legs()[0], 2, "AA", 1);
    ESVPQ pq(_trxP, "a");
    copySopVecs();
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    ESVPQItem* pqItem = 0;

    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pqItem != NULL);
    CPPUNIT_ASSERT(pqItem->totalAmt() == 1);

    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    CPPUNIT_ASSERT(pqItem->totalAmt() == 2);

    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
    CPPUNIT_ASSERT(pqItem->totalAmt() == 3);
  }

  void fewElementsInTwoLegs()
  {
    createTrx();
    createSOP(_trxP->legs()[0], 0, "AA", 2);
    createSOP(_trxP->legs()[0], 1, "AA", 3);
    createSOP(_trxP->legs()[1], 0, "AA", 30);
    createSOP(_trxP->legs()[1], 1, "AA", 20);
    ESVPQ pq(_trxP, "a");
    copySopVecs();
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    ESVPQItem* pqItem = 0;

    // 22
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    CPPUNIT_ASSERT(pqItem->totalAmt() == 22);

    // 23
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    CPPUNIT_ASSERT(pqItem->totalAmt() == 23);

    // 32
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    CPPUNIT_ASSERT(pqItem->totalAmt() == 32);

    // 33
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
    CPPUNIT_ASSERT(pqItem->totalAmt() == 33);
  }

  void PQItemNullWhenPQEmpty()
  {
    createTrx();
    ESVPQ pq(_trxP, "a");
    copySopVecs();
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
    ESVPQItem* pqItem = 0;
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pqItem == NULL);
  }

  void PQItemEmptyWhenNoMoreElements()
  {
    createTrx();
    createSOP(_trxP->legs()[0], 0, "AA", 2);
    createSOP(_trxP->legs()[0], 1, "AA", 1);
    createSOP(_trxP->legs()[1], 0, "AA", 30);
    createSOP(_trxP->legs()[1], 1, "AA", 20);
    copySopVecs();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    ESVPQItem* pqItem = 0;

    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    CPPUNIT_ASSERT(pqItem != NULL);

    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    CPPUNIT_ASSERT(pqItem != NULL);

    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    CPPUNIT_ASSERT(pqItem != NULL);

    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
    CPPUNIT_ASSERT(pqItem != NULL);

    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
    CPPUNIT_ASSERT(pqItem == NULL);
  }

  void owFares()
  {
    createTrx();
    createSOP(_trxP->legs()[0], 0, "AA");
    createSOP(_trxP->legs()[0], 1, "AA");
    createSOP(_trxP->legs()[1], 0, "AA");
    createSOP(_trxP->legs()[1], 1, "AA");
    addFarePath(0, 0, 1, ESVSopWrapper::OW);
    addFarePath(1, 0, 1, ESVSopWrapper::OW);
    copySopVecs();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    ESVPQItem* pqItem = 0;

    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pqItem != NULL);
    CPPUNIT_ASSERT(pqItem->totalAmt() == 2);
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
  }

  void owFares2()
  {
    createTrx();
    createSOP(_trxP->legs()[0], 0, "AA");
    createSOP(_trxP->legs()[1], 0, "AA");
    addFarePath(0, 0, 1, ESVSopWrapper::OW);
    addFarePath(0, 0, 2, ESVSopWrapper::OW);
    addFarePath(1, 0, 1, ESVSopWrapper::OW);
    addFarePath(1, 0, 2, ESVSopWrapper::OW);
    copySopVecs();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    ESVPQItem* pqItem = 0;

    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pqItem != NULL);
    CPPUNIT_ASSERT(pqItem->totalAmt() == 2);
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());

    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pqItem == NULL);
  }

  void owFares3()
  {
    createTrx();
    createSOP(_trxP->legs()[0], 0, "AA");
    createSOP(_trxP->legs()[0], 1, "AA");
    createSOP(_trxP->legs()[1], 0, "AA");
    createSOP(_trxP->legs()[1], 1, "AA");
    addFarePath(0, 0, 1, ESVSopWrapper::OW);
    addFarePath(0, 0, 2, ESVSopWrapper::OW);
    addFarePath(0, 1, 3, ESVSopWrapper::OW);
    addFarePath(0, 1, 4, ESVSopWrapper::OW);
    addFarePath(1, 0, 1, ESVSopWrapper::OW);
    addFarePath(1, 0, 2, ESVSopWrapper::OW);
    addFarePath(1, 1, 3, ESVSopWrapper::OW);
    addFarePath(1, 1, 4, ESVSopWrapper::OW);
    copySopVecs();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");

    checkItems(pq, 2);
    checkItems(pq, 4, 2);
    checkItems(pq, 6);

    ESVPQItem* pqItem = 0;
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pqItem == NULL);
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
  }

  void rtFares()
  {
    createTrx();
    createSOP(_trxP->legs()[0], 0, "AA");
    createSOP(_trxP->legs()[0], 1, "AA");
    createSOP(_trxP->legs()[1], 0, "AA");
    createSOP(_trxP->legs()[1], 1, "AA");
    addFarePath(0, 0, 1, ESVSopWrapper::RT);
    addFarePath(1, 0, 1, ESVSopWrapper::RT);
    copySopVecs();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    ESVPQItem* pqItem = 0;

    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pqItem != NULL);
    CPPUNIT_ASSERT(pqItem->totalAmt() == 2);
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
  }

  void rtFares2()
  {
    createTrx();
    createSOP(_trxP->legs()[0], 0, "AA");
    createSOP(_trxP->legs()[1], 0, "AA");
    addFarePath(0, 0, 1, ESVSopWrapper::RT);
    addFarePath(0, 0, 2, ESVSopWrapper::RT);
    addFarePath(1, 0, 1, ESVSopWrapper::RT);
    addFarePath(1, 0, 2, ESVSopWrapper::RT);
    copySopVecs();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");
    ESVPQItem* pqItem = 0;

    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pqItem != NULL);
    CPPUNIT_ASSERT(pqItem->totalAmt() == 2);
    CPPUNIT_ASSERT(!pq._pqItemPQ.empty());

    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pqItem == NULL);
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
  }

  void rtFares3()
  {
    createTrx();
    createSOP(_trxP->legs()[0], 0, "AA");
    createSOP(_trxP->legs()[0], 1, "AA");
    createSOP(_trxP->legs()[1], 0, "AA");
    createSOP(_trxP->legs()[1], 1, "AA");
    addFarePath(0, 0, 1, ESVSopWrapper::RT);
    addFarePath(0, 0, 2, ESVSopWrapper::RT);
    addFarePath(0, 1, 3, ESVSopWrapper::RT);
    addFarePath(0, 1, 4, ESVSopWrapper::RT);
    addFarePath(1, 0, 1, ESVSopWrapper::RT);
    addFarePath(1, 0, 2, ESVSopWrapper::RT);
    addFarePath(1, 1, 3, ESVSopWrapper::RT);
    addFarePath(1, 1, 4, ESVSopWrapper::RT);
    copySopVecs();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");

    checkItems(pq, 2);
    checkItems(pq, 4, 2);
    checkItems(pq, 6);

    ESVPQItem* pqItem = 0;
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pqItem == NULL);
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
  }

  void owrtFares()
  {
    createTrx();
    createSOP(_trxP->legs()[0], 0, "AA");
    createSOP(_trxP->legs()[0], 1, "AA");
    createSOP(_trxP->legs()[1], 0, "AA");
    createSOP(_trxP->legs()[1], 1, "AA");
    addFarePath(0, 0, 1, ESVSopWrapper::RT);
    addFarePath(0, 0, 2, ESVSopWrapper::OW);
    addFarePath(0, 1, 3, ESVSopWrapper::RT);
    addFarePath(0, 1, 4, ESVSopWrapper::OW);
    addFarePath(1, 0, 1, ESVSopWrapper::OW);
    addFarePath(1, 0, 2, ESVSopWrapper::RT);
    addFarePath(1, 1, 3, ESVSopWrapper::OW);
    addFarePath(1, 1, 4, ESVSopWrapper::RT);
    copySopVecs();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");

    checkItems(pq, 3);
    checkItems(pq, 5, 2);
    checkItems(pq, 7);

    ESVPQItem* pqItem = 0;
    pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pqItem == NULL);
    CPPUNIT_ASSERT(pq._pqItemPQ.empty());
  }

  void owrtFaresInOneLeg()
  {
    createTrxWithFirstLegEmpty();
    createSOP(_trxP->legs()[0], 0, "AA");
    createSOP(_trxP->legs()[0], 1, "AA");
    createSOP(_trxP->legs()[0], 2, "AA");
    createSOP(_trxP->legs()[0], 3, "AA");
    addFarePath(0, 0, 1, ESVSopWrapper::RT);
    addFarePath(0, 1, 2, ESVSopWrapper::OW);
    addFarePath(0, 2, 3, ESVSopWrapper::RT);
    addFarePath(0, 3, 4, ESVSopWrapper::OW);
    addFarePath(0, 0, 1, ESVSopWrapper::OW);
    addFarePath(0, 1, 2, ESVSopWrapper::RT);
    addFarePath(0, 2, 3, ESVSopWrapper::OW);
    addFarePath(0, 3, 4, ESVSopWrapper::RT);
    copySopVecs();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");

    checkItems(pq, 1);
    checkItems(pq, 2);
    checkItems(pq, 3);
    checkItems(pq, 4);

    ESVPQItem* pqItem = pq.getNextItem(NULL);
    CPPUNIT_ASSERT(pqItem == NULL);
  }

  void checkPQItemsWhenNoPQAnymore()
  {
    std::vector<ESVPQItem*> pqItemVec;
    fillInPQItemVec(pqItemVec);

    CPPUNIT_ASSERT(pqItemVec.size() == 4);
  }

  void fillInPQItemVec(std::vector<ESVPQItem*>& pqItemVec)
  {
    createTrxWithFirstLegEmpty();
    createSOP(_trxP->legs()[0], 0, "AA");
    createSOP(_trxP->legs()[0], 1, "AA");
    createSOP(_trxP->legs()[0], 2, "AA");
    createSOP(_trxP->legs()[0], 3, "AA");
    addFarePath(0, 0, 1, ESVSopWrapper::RT);
    addFarePath(0, 1, 2, ESVSopWrapper::OW);
    addFarePath(0, 2, 3, ESVSopWrapper::RT);
    addFarePath(0, 3, 4, ESVSopWrapper::OW);
    addFarePath(0, 0, 1, ESVSopWrapper::OW);
    addFarePath(0, 1, 2, ESVSopWrapper::RT);
    addFarePath(0, 2, 3, ESVSopWrapper::OW);
    addFarePath(0, 3, 4, ESVSopWrapper::RT);
    copySopVecs();
    ESVPQ pq(_trxP, "a");
    pq.init(_outSopVec, _inSopVec, LFSRemaining, "", "");

    ESVPQItem* pqItem = pq.getNextItem(NULL);
    while (pqItem)
    {
      pqItemVec.push_back(pqItem);
      pqItem = pq.getNextItem(NULL);
    }
  }

  void checkItems(ESVPQ& pq, MoneyAmount a, int loopNo = 1)
  {
    ESVPQItem* pqItem = 0;
    std::vector<ShoppingTrx::SchedulingOption*> sopVec;

    for (int i = 0; i < loopNo; ++i)
    {
      CPPUNIT_ASSERT(!pq._pqItemPQ.empty());
      pqItem = pq.getNextItem(NULL);
      CPPUNIT_ASSERT(pqItem != NULL);
      std::stringstream msg;
      msg << "a = " << a << ", amt = " << pqItem->totalAmt() << ", i = " << i << std::endl;
      CPPUNIT_ASSERT_MESSAGE(msg.str(), pqItem->totalAmt() == a);
    }
  }

  void addFarePath(int legIdx, int sopIdx, MoneyAmount amt, ESVSopWrapper::SOPFareListType fareType)
  {
    SOPFarePath* sopFarePath = 0;
    _memHandle.get(sopFarePath);
    sopFarePath->totalAmount() = amt;
    if (fareType == ESVSopWrapper::OW)
    {
      _trxP->legs()[legIdx]
          .sop()[sopIdx]
          .itin()
          ->paxTypeSOPFareListMap()[_trxP->paxType()[0]]
          .owSopFarePaths()
          .push_back(sopFarePath);
    }
    else if (fareType == ESVSopWrapper::RT)
    {
      _trxP->legs()[legIdx]
          .sop()[sopIdx]
          .itin()
          ->paxTypeSOPFareListMap()[_trxP->paxType()[0]]
          .rtSopFarePaths()
          .push_back(sopFarePath);
    }
  }

  void createTrx()
  {
    _memHandle.get(_trxP);
    ShoppingTrx::Leg* pLeg1 = 0;
    ShoppingTrx::Leg* pLeg2 = 0;
    _memHandle.get(pLeg1);
    _memHandle.get(pLeg2);
    _trxP->legs().push_back(*pLeg1);
    _trxP->legs().push_back(*pLeg2);
    PaxType* paxType = 0;
    _memHandle.get(paxType);
    _trxP->paxType().push_back(paxType);
    pLeg1->sop().reserve(10);
    pLeg2->sop().reserve(10);

    PricingOptions* options = 0;
    _memHandle.get(options);
    _trxP->setOptions(options);
  }

  void createTrxWithNoLegs()
  {
    _memHandle.get(_trxP);
    PaxType* paxType = 0;
    _memHandle.get(paxType);
    _trxP->paxType().push_back(paxType);
  }

  void createTrxWithFirstLegEmpty()
  {
    _memHandle.get(_trxP);
    ShoppingTrx::Leg* pLeg1 = 0;
    _memHandle.get(pLeg1);
    _trxP->legs().push_back(*pLeg1);
    PaxType* paxType = 0;
    _memHandle.get(paxType);
    _trxP->paxType().push_back(paxType);
  }

  void createSOP(ShoppingTrx::Leg& leg, int idx, const std::string& carrier, MoneyAmount amt = 0)
  {
    // create itinCellInfo
    ItinIndex::ItinCellInfo* itinCellInfo;
    _memHandle.get(itinCellInfo);

    // create sopItinerary
    Itin* sopItinerary;
    _memHandle.get(sopItinerary);

    // onlineCarrier
    sopItinerary->onlineCarrier() = carrier;

    AirSeg* as = 0;
    _memHandle.get(as);
    sopItinerary->travelSeg().push_back(as);

    ShoppingTrx::SchedulingOption sop(sopItinerary, idx, true);
    sop.governingCarrier() = carrier;

    // generate crx key
    ItinIndex::Key cxrKey;
    ShoppingUtil::createCxrKey(carrier, cxrKey);

    // generate schedule key
    ItinIndex::Key scheduleKey;
    ShoppingUtil::createScheduleKey(scheduleKey);

    leg.carrierIndex().addItinCell(sopItinerary, *itinCellInfo, cxrKey, scheduleKey);
    leg.sop().push_back(sop);

    if (amt != 0)
    {
      SOPFarePath* sopFarePath = 0;
      _memHandle.get(sopFarePath);
      sopFarePath->totalAmount() = amt;
      sopItinerary->paxTypeSOPFareListMap()[_trxP->paxType()[0]].owSopFarePaths().push_back(
          sopFarePath);
    }
  }

  void copySopVecs()
  {
    copyVec(_trxP->legs()[0], _outSopVec);
    if (_trxP->legs().size() > 1)
    {
      copyVec(_trxP->legs()[1], _inSopVec);
    }
  }

  void copyVec(ShoppingTrx::Leg& leg, std::vector<ShoppingTrx::SchedulingOption*>& sopVec)
  {
    sopVec.clear();
    for (size_t i = 0; i < leg.sop().size(); ++i)
    {
      sopVec.push_back(&leg.sop()[i]);
    }
  }

private:
  ShoppingTrx* _trxP;
  std::vector<ShoppingTrx::SchedulingOption*> _outSopVec;
  std::vector<ShoppingTrx::SchedulingOption*> _inSopVec;
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(ESVTest);
