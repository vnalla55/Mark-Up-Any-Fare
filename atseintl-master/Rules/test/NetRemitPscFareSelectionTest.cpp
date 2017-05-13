#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include "test/include/TestMemHandle.h"

#include "Rules/NetRemitPscFareSelection.h"
#include "DataModel/Agent.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{
class NetRemitPscFareSelectionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NetRemitPscFareSelectionTest);

  CPPUNIT_TEST(testGetCarrierCodeWhenPscCarrierIsEmpty);
  CPPUNIT_TEST(testGetCarrierCodeWhenPscCarrierIsSetTo2Stars);
  CPPUNIT_TEST(testGetCarrierCodeWhenPscCarrierIsSpecified);

  CPPUNIT_TEST(testSelectTktPblFareWhenCannotCreateFareMarket);
  CPPUNIT_TEST(testSelectTktPblFareWhenCannotMatchFare);
  CPPUNIT_TEST(testSelectTktPblFareWhenMatchFare);
  CPPUNIT_TEST(testSelectTktPblFareWhenNoMatch);
  CPPUNIT_TEST(testSelectTktPblFareWhenMatch);

  CPPUNIT_TEST(testCreateTravelSegWhenStartSegmentNotFound);
  CPPUNIT_TEST(testCreateTravelSegWhenEndSegmentNotFound);
  CPPUNIT_TEST(testCreateTravelSegWhenWrongOrder);
  CPPUNIT_TEST(testCreateTravelSeg);

  CPPUNIT_TEST(testProcess);

  CPPUNIT_TEST(testIsMatchedFare_Fail);
  CPPUNIT_TEST(testIsMatchedFare_Pass);

  CPPUNIT_TEST(testGetFareMarket_Pass);
  CPPUNIT_TEST(testGetFareMarket_Fail);

  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _farePath = _memHandle.create<FarePath>();
    _farePath->paxType() = _memHandle.create<PaxType>();
    _pricingUnit = _memHandle.create<PricingUnit>();
    _fareUsage = _memHandle.create<FareUsage>();
    _negFareRest = _memHandle.create<NegFareRest>();
    _negFareRestExt = _memHandle.create<NegFareRestExt>();
    _negFareRestExtSeqs = _memHandle.create<std::vector<NegFareRestExtSeq*> >();
    _negFareRestExtSeq = _memHandle.create<NegFareRestExtSeq>();
    _negFareRestExtSeqs->push_back(_negFareRestExtSeq);
    _fareUsage->paxTypeFare() = _memHandle.create<PaxTypeFare>();
    _fareUsage->paxTypeFare()->fareMarket() = _memHandle.create<FareMarket>();

    _airSeg = _memHandle.create<AirSeg>();
    _airSeg->carrier() = "OK";
    _airSeg->boardMultiCity() = "VIA";
    _fareUsage->travelSeg().push_back(_airSeg);

    _trx->diagnostic().diagnosticType() = Diagnostic692;
    _trx->diagnostic().activate();
    _dc = DCFactory::instance()->create(*_trx);
    if (_dc)
      _dc->enable(Diagnostic692);

    _fareSelection = _memHandle.insert(new NetRemitPscFareSelectionMock(*_trx,
                                                                        *_farePath,
                                                                        *_pricingUnit,
                                                                        *_fareUsage,
                                                                        *_negFareRest,
                                                                        *_negFareRestExt,
                                                                        *_negFareRestExtSeqs));
  }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FarePath* _farePath;
  PricingUnit* _pricingUnit;
  FareUsage* _fareUsage;
  NegFareRest* _negFareRest;
  NegFareRestExt* _negFareRestExt;
  NegFareRestExtSeq* _negFareRestExtSeq;
  std::vector<NegFareRestExtSeq*>* _negFareRestExtSeqs;
  NetRemitPscFareSelection* _fareSelection;
  DiagCollector* _dc;
  AirSeg* _airSeg;

  void prepareDisplayNetRemitPscResultsCase()
  {
    FareUsage::TktNetRemitPscResult* netRemitResult1 =
        _memHandle.create<FareUsage::TktNetRemitPscResult>();
    FareUsage::TktNetRemitPscResult* netRemitResult2 =
        _memHandle.create<FareUsage::TktNetRemitPscResult>();

    NegFareRestExtSeq* restExtSeq1 = _memHandle.create<NegFareRestExtSeq>();
    NegFareRestExtSeq* restExtSeq2 = _memHandle.create<NegFareRestExtSeq>();

    restExtSeq1->seqNo() = 100;
    restExtSeq1->cityFrom() = "SYD";
    restExtSeq1->cityTo() = "KUL";
    restExtSeq1->carrier() = "MH";
    restExtSeq1->viaCity1() = "AAA";
    restExtSeq1->viaCity2() = "BBB";
    restExtSeq1->viaCity3() = "CCC";
    restExtSeq1->viaCity4() = "DDD";
    restExtSeq1->publishedFareBasis() = "Y2345678";
    restExtSeq1->uniqueFareBasis() = "U2345678";

    restExtSeq2->seqNo() = 200;
    restExtSeq2->cityFrom() = "BLR";
    restExtSeq2->cityTo() = "SIN";
    restExtSeq2->carrier() = "SQ";
    restExtSeq2->viaCity1() = "EEE";
    restExtSeq2->viaCity2() = "FFF";
    restExtSeq2->publishedFareBasis() = "C-";
    restExtSeq2->uniqueFareBasis() = "TEST1234";

    netRemitResult1->_tfdpscSeqNumber = restExtSeq1;
    netRemitResult2->_tfdpscSeqNumber = restExtSeq2;

    AirSeg* seg1 = _memHandle.create<AirSeg>();
    AirSeg* seg2 = _memHandle.create<AirSeg>();

    seg1->segmentOrder() = 1;
    seg2->segmentOrder() = 2;

    netRemitResult1->_startTravelSeg = seg1;
    netRemitResult1->_endTravelSeg = seg1;
    netRemitResult2->_startTravelSeg = seg2;
    netRemitResult2->_endTravelSeg = seg2;

    _fareUsage->netRemitPscResults().push_back(*netRemitResult1);
    _fareUsage->netRemitPscResults().push_back(*netRemitResult2);
  }

  void createFare(const Directionality& dir = TO)
  {
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->directionality() = dir;
    fare->setFareInfo(fareInfo);
    fareInfo->carrier() = "UA";
    _fareUsage->paxTypeFare()->initialize(fare, 0, 0);
  }

  void testGetCarrierCodeWhenPscCarrierIsEmpty()
  {
    createFare();

    CPPUNIT_ASSERT_EQUAL(_fareUsage->paxTypeFare()->carrier(),
                         _fareSelection->getCarrierCode("", *_airSeg));
  }

  void testGetCarrierCodeWhenPscCarrierIsSetTo2Stars()
  {
    createFare();

    CPPUNIT_ASSERT_EQUAL(_airSeg->carrier(), _fareSelection->getCarrierCode(ANY_CARRIER, *_airSeg));
  }

  void testGetCarrierCodeWhenPscCarrierIsSpecified()
  {
    createFare();

    CPPUNIT_ASSERT_EQUAL(CarrierCode("MH"), _fareSelection->getCarrierCode("MH", *_airSeg));
  }

  void prepareSelectTktPblFare(AirSeg& seg, FareUsage::TktNetRemitPscResult& pscRes)
  {
    NegFareRestExtSeq* restExtSeq = _memHandle.create<NegFareRestExtSeq>();
    pscRes._tfdpscSeqNumber = restExtSeq;
    pscRes._endTravelSeg = &seg;
    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();
    ((NetRemitPscFareSelectionMock*)_fareSelection)->_fareMarket = fareMarket;
    ((NetRemitPscFareSelectionMock*)_fareSelection)->_paxTypeFare = paxTypeFare;
  }

  void testSelectTktPblFareWhenCannotCreateFareMarket()
  {
    FareUsage::TktNetRemitPscResult pscRes;
    prepareSelectTktPblFare(*_airSeg, pscRes);
    ((NetRemitPscFareSelectionMock*)_fareSelection)->_fareMarket = 0;

    CPPUNIT_ASSERT(!_fareSelection->selectTktPblFare(_airSeg, pscRes));
  }

  void testSelectTktPblFareWhenCannotMatchFare()
  {
    FareUsage::TktNetRemitPscResult pscRes;
    prepareSelectTktPblFare(*_airSeg, pscRes);
    ((NetRemitPscFareSelectionMock*)_fareSelection)->_paxTypeFare = 0;

    CPPUNIT_ASSERT(!_fareSelection->selectTktPblFare(_airSeg, pscRes));
  }

  void testSelectTktPblFareWhenMatchFare()
  {
    FareUsage::TktNetRemitPscResult pscRes;
    prepareSelectTktPblFare(*_airSeg, pscRes);

    CPPUNIT_ASSERT_EQUAL(((NetRemitPscFareSelectionMock*)_fareSelection)->_paxTypeFare,
                         _fareSelection->selectTktPblFare(_airSeg, pscRes));
  }

  void testSelectTktPblFareWhenNoMatch()
  {
    prepareDisplayNetRemitPscResultsCase();

    _fareSelection->selectTktPblFare();

    CPPUNIT_ASSERT(!_fareUsage->netRemitPscResults().begin()->_resultFare);
  }

  void testSelectTktPblFareWhenMatch()
  {
    prepareDisplayNetRemitPscResultsCase();
    const FareMarket fareMarket;
    const PaxTypeFare paxTypeFare;
    ((NetRemitPscFareSelectionMock*)_fareSelection)->_fareMarket = &fareMarket;
    ((NetRemitPscFareSelectionMock*)_fareSelection)->_paxTypeFare = &paxTypeFare;

    _fareSelection->selectTktPblFare();

    CPPUNIT_ASSERT_EQUAL(size_t(2), _fareUsage->netRemitPscResults().size());
    CPPUNIT_ASSERT_EQUAL(&paxTypeFare, _fareUsage->netRemitPscResults().begin()->_resultFare);
    CPPUNIT_ASSERT_EQUAL(&paxTypeFare, (_fareUsage->netRemitPscResults().begin() + 1)->_resultFare);
  }

  void testCreateTravelSegWhenStartSegmentNotFound()
  {
    AirSeg startSeg;
    AirSeg endSeg;
    _fareUsage->paxTypeFare()->fareMarket()->travelSeg().push_back(&endSeg);

    _fareSelection->createTravelSeg(&startSeg, &endSeg);
    CPPUNIT_ASSERT_EQUAL(size_t(0), _fareSelection->_tvlSegs.size());
  }

  void testCreateTravelSegWhenEndSegmentNotFound()
  {
    AirSeg startSeg;
    AirSeg endSeg;
    _fareUsage->paxTypeFare()->fareMarket()->travelSeg().push_back(&startSeg);

    _fareSelection->createTravelSeg(&startSeg, &endSeg);
    CPPUNIT_ASSERT_EQUAL(size_t(0), _fareSelection->_tvlSegs.size());
  }

  void testCreateTravelSegWhenWrongOrder()
  {
    AirSeg startSeg;
    AirSeg endSeg;
    _fareUsage->paxTypeFare()->fareMarket()->travelSeg().push_back(&endSeg);
    _fareUsage->paxTypeFare()->fareMarket()->travelSeg().push_back(&startSeg);

    _fareSelection->createTravelSeg(&startSeg, &endSeg);
    CPPUNIT_ASSERT_EQUAL(size_t(0), _fareSelection->_tvlSegs.size());
  }

  void testCreateTravelSeg()
  {
    AirSeg startSeg;
    AirSeg endSeg;
    _fareUsage->paxTypeFare()->fareMarket()->travelSeg().push_back(&startSeg);
    _fareUsage->paxTypeFare()->fareMarket()->travelSeg().push_back(&endSeg);

    _fareSelection->createTravelSeg(&startSeg, &endSeg);
    CPPUNIT_ASSERT_EQUAL(size_t(2), _fareSelection->_tvlSegs.size());
    CPPUNIT_ASSERT(_fareUsage->paxTypeFare()->fareMarket()->travelSeg() ==
                   _fareSelection->_tvlSegs);
  }

  void testProcess()
  {
    createFare();
    _fareSelection->process();

    CPPUNIT_ASSERT_EQUAL(
        std::string(" \n\n"
                    "***************** NET REMIT FARE SELECTION *******************\n"
                    "------------------- CAT35 FARE -------------------------------\n"
                    "    /CXR-/ #GI-XX#  .UNKNWN. REQPAX-\n"
                    "P UA     *      0                 I UNKUNK                0.00\n"
                    "---------- MATCHED SEQUENCES FOR TICKETED FARE DATA ---------- \n"
                    "SEQNO  FRM TO  CXR VIA POINTS      FARE BASIS   SPNV\n"),
        _trx->diagnostic().toString());
  }

  void createNegFareRestExtSeq(const Directionality& dir)
  {
    createFare(dir);
    _fareSelection->_negFareRestExtSeq = _memHandle.create<NegFareRestExtSeq>();
  }

  void testIsMatchedFare_Fail()
  {
    createNegFareRestExtSeq(TO);
    CPPUNIT_ASSERT(!_fareSelection->isMatchedFare(*_fareUsage->paxTypeFare()));
  }

  void testIsMatchedFare_Pass()
  {
    createNegFareRestExtSeq(BOTH);
    CPPUNIT_ASSERT(_fareSelection->isMatchedFare(*_fareUsage->paxTypeFare()));
  }

  void testGetFareMarket_Pass()
  {
    RepricingTrx* trx = _memHandle.create<RepricingTrx>();
    trx->fareMarket().push_back(_fareUsage->paxTypeFare()->fareMarket());
    Itin* itin = _memHandle.create<Itin>();
    trx->itin().push_back(itin);
    _fareUsage->paxTypeFare()->fareMarket()->travelSeg().push_back(_airSeg);
    CPPUNIT_ASSERT_EQUAL((const FareMarket*)_fareUsage->paxTypeFare()->fareMarket(),
                         _fareSelection->getFareMarket(trx, _fareUsage->travelSeg(), ""));
  }

  void testGetFareMarket_Fail()
  {
    RepricingTrx* trx = _memHandle.create<RepricingTrx>();
    trx->fareMarket().push_back(_fareUsage->paxTypeFare()->fareMarket());
    Itin* itin = _memHandle.create<Itin>();
    trx->itin().push_back(itin);
    CPPUNIT_ASSERT_EQUAL((const FareMarket*)NULL,
                         _fareSelection->getFareMarket(trx, _fareUsage->travelSeg(), ""));
  }

  // Mocks
  class NetRemitPscFareSelectionMock : public NetRemitPscFareSelection
  {
    friend class NetRemitPscFareSelectionTest;

  public:
    NetRemitPscFareSelectionMock(PricingTrx& trx,
                                 const FarePath& fPath,
                                 PricingUnit& pu,
                                 FareUsage& fu,
                                 const NegFareRest& negFareRest,
                                 const NegFareRestExt& negFareRestExt,
                                 const std::vector<NegFareRestExtSeq*>& negFareRestExtSeqs)
      : NetRemitPscFareSelection(
            trx, fPath, pu, fu, negFareRest, negFareRestExt, negFareRestExtSeqs),
        _fareMarket(0),
        _paxTypeFare(0)
    {
    }

    using NetRemitPscFareSelection::getPblFareMarket;
    using NetRemitPscFareSelection::selectTktPblFare;

  protected:
    const FareMarket* getPblFareMarket(const Loc* origLoc,
                                       const Loc* destLoc,
                                       const CarrierCode& cxr,
                                       const DateTime& deptDT,
                                       const DateTime& arrDT)
    {
      return _fareMarket;
    }

    const PaxTypeFare* selectTktPblFare(const FareMarket& fareMarket) { return _paxTypeFare; }

  private:
    const FareMarket* _fareMarket;
    const PaxTypeFare* _paxTypeFare;
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetRemitPscFareSelectionTest);
}
