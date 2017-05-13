#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include "test/include/TestMemHandle.h"

#include "Rules/NetRemitPscMatchUtil.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "Rules/RuleConst.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/NegFareRestExtSeq.h"

namespace tse
{
class NetRemitPscMatchUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NetRemitPscMatchUtilTest);
  CPPUNIT_TEST(testMatchCarrier_Pass_Any);
  CPPUNIT_TEST(testMatchCarrier_Pass_Blank);
  CPPUNIT_TEST(testMatchCarrier_Pass_Equal);
  CPPUNIT_TEST(testMatchCarrier_Fail);
  CPPUNIT_TEST(testMatchCarrier_Pass_Range);
  CPPUNIT_TEST(testMatchCarrier_Fail_Range);

  CPPUNIT_TEST(testMatchVia_Fail_Empty);
  CPPUNIT_TEST(testMatchVia_Pass_AnyCity);
  CPPUNIT_TEST(testMatchVia_Pass_Exact);
  CPPUNIT_TEST(testMatchVia_Fail);

  CPPUNIT_TEST(testMatchToFrom_Pass_Any);
  CPPUNIT_TEST(testMatchToFrom_FailFrom);
  CPPUNIT_TEST(testMatchToFrom_PassFrom);
  CPPUNIT_TEST(testMatchToFrom_FailTo);
  CPPUNIT_TEST(testMatchToFrom_PassTo);
  CPPUNIT_TEST(testMatchToFrom_FailFromInbound);
  CPPUNIT_TEST(testMatchToFrom_PassFromInbound);
  CPPUNIT_TEST(testMatchToFrom_FailToInbound);
  CPPUNIT_TEST(testMatchToFrom_PassToInbound);

  CPPUNIT_TEST(testStoreMatchedInFareUsage_Empty);
  CPPUNIT_TEST(testStoreMatchedInFareUsage_NotEmpty);

  CPPUNIT_TEST(testMatchFareComp_Fail_OnlyOneSeg);
  CPPUNIT_TEST(testMatchFareComp_Pass_MatchFirstToLast);
  CPPUNIT_TEST(testMatchFareComp_Pass_MatchFirstToNotLast);
  CPPUNIT_TEST(testMatchFareComp_Pass_MatchNotFirstToLast);
  CPPUNIT_TEST(testMatchFareComp_Fail_Via);

  CPPUNIT_TEST(testFindMatchedSeq_Pass);
  CPPUNIT_TEST(testFindMatchedSeq_Fail);

  CPPUNIT_TEST(testProcess_Pass_WithSkipR3);
  CPPUNIT_TEST(testProcess_Pass_WithSkipR3Ext);
  CPPUNIT_TEST(testProcess_Pass);
  CPPUNIT_TEST(testProcess_Fail);
  CPPUNIT_TEST(testProcess_Fail_NotCat35);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _pricingUnit = _memHandle.create<PricingUnit>();
    _fareUsage = _memHandle.create<FareUsage>();
    _pricingUnit->fareUsage().push_back(_fareUsage);

    _negFareRest = _memHandle.create<NegFareRest>();
    _negFareRestExt = _memHandle.create<NegFareRestExt>();
    _negFareRestExt->fareBasisAmtInd() = 'A';
    _negFareRestExtSeq = _memHandle.create<NegFareRestExtSeq>();
    _ptf = _memHandle.create<PaxTypeFare>();
    _fareUsage->paxTypeFare() = _ptf;

    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->carrier() = "CR";
    fare->setFareInfo(fareInfo);
    _ptf->setFare(fare);

    _airSeg = _memHandle.create<AirSeg>();
    _airSeg->carrier() = "OK";
    _airSeg->offMultiCity() = "VIA";
    _airSeg->destAirport() = "VIA";
    _fareUsage->travelSeg().push_back(_airSeg);

    _matchUtil = _memHandle.insert(new NetRemitPscMatchUtil(*_pricingUnit));
    _matchUtil->_fu = _fareUsage;

    _ruleData = _memHandle.create<NegPaxTypeFareRuleData>();
    _matchUtil->_negFareRuleData = _ruleData;
    _ruleData->negFareRestExtSeq().push_back(_negFareRestExtSeq);

    _negFareRestExtSeq->cityFrom() = NetRemitPscMatchUtil::ANY_CITY;
    _negFareRestExtSeq->cityTo() = NetRemitPscMatchUtil::ANY_CITY;
    _negFareRestExtSeq->viaCity1() = NetRemitPscMatchUtil::ANY_CITY;
  }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
  PricingUnit* _pricingUnit;
  FareUsage* _fareUsage;
  PaxTypeFare* _ptf;
  NegFareRest* _negFareRest;
  NegFareRestExt* _negFareRestExt;
  NegFareRestExtSeq* _negFareRestExtSeq;
  NetRemitPscMatchUtil* _matchUtil;
  NegPaxTypeFareRuleData* _ruleData;
  AirSeg* _airSeg;

  NegPaxTypeFareRuleData* makeCat35Fare()
  {
    _ptf->status().set(PaxTypeFare::PTF_Negotiated);
    NegPaxTypeFareRuleData* ruleData = _memHandle.create<NegPaxTypeFareRuleData>();
    PaxTypeFare::PaxTypeFareAllRuleData* allRules =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();

    allRules->chkedRuleData = true;
    allRules->chkedGfrData = false;
    allRules->fareRuleData = ruleData;
    allRules->gfrRuleData = 0;

    ruleData->ruleItemInfo() = _negFareRest;
    ruleData->negFareRestExt() = _negFareRestExt;
    ruleData->negFareRestExtSeq().push_back(_negFareRestExtSeq);
    (*_ptf->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE] = allRules;

    return ruleData;
  }

  void testMatchCarrier_Pass_Any() { CPPUNIT_ASSERT(_matchUtil->matchCarrier(ANY_CARRIER, "QQ")); }

  void testMatchCarrier_Pass_Blank() { CPPUNIT_ASSERT(_matchUtil->matchCarrier("", "CR")); }

  void testMatchCarrier_Pass_Equal() { CPPUNIT_ASSERT(_matchUtil->matchCarrier("QQ", "QQ")); }

  void testMatchCarrier_Fail() { CPPUNIT_ASSERT(!_matchUtil->matchCarrier("QQ", "RR")); }

  void testMatchCarrier_Pass_Range()
  {
    CPPUNIT_ASSERT(_matchUtil->matchCarrier(
        _fareUsage->travelSeg().begin(), _fareUsage->travelSeg().begin(), "OK"));
  }

  void testMatchCarrier_Fail_Range()
  {
    CPPUNIT_ASSERT(!_matchUtil->matchCarrier(
        _fareUsage->travelSeg().begin(), _fareUsage->travelSeg().begin(), "NO"));
  }

  void testMatchVia_Fail_Empty()
  {
    _negFareRestExtSeq->viaCity1() = "";
    CPPUNIT_ASSERT(!_matchUtil->matchVia(
        *_negFareRestExtSeq, _fareUsage->travelSeg().begin(), _fareUsage->travelSeg().end()));
  }

  void testMatchVia_Pass_AnyCity()
  {
    _negFareRestExtSeq->viaCity1() = NetRemitPscMatchUtil::ANY_CITY;
    CPPUNIT_ASSERT(_matchUtil->matchVia(
        *_negFareRestExtSeq, _fareUsage->travelSeg().begin(), _fareUsage->travelSeg().end()));
  }

  void testMatchVia_Pass_Exact()
  {
    _negFareRestExtSeq->viaCity1() = "VIA";
    CPPUNIT_ASSERT(_matchUtil->matchVia(
        *_negFareRestExtSeq, _fareUsage->travelSeg().begin(), _fareUsage->travelSeg().end()));
  }

  void testMatchVia_Fail()
  {
    _negFareRestExtSeq->viaCity1() = "BAD";
    CPPUNIT_ASSERT(!_matchUtil->matchVia(
        *_negFareRestExtSeq, _fareUsage->travelSeg().begin(), _fareUsage->travelSeg().end()));
  }

  void testMatchToFrom_Pass_Any()
  {
    CPPUNIT_ASSERT(_matchUtil->matchToFrom(*_airSeg, *_negFareRestExtSeq));
  }

  void testMatchToFrom_FailFrom()
  {
    _negFareRestExtSeq->cityFrom() = "FRO";
    _airSeg->boardMultiCity() = "BAD";
    CPPUNIT_ASSERT(!_matchUtil->matchToFrom(*_airSeg, *_negFareRestExtSeq));
  }

  void testMatchToFrom_PassFrom()
  {
    _negFareRestExtSeq->cityFrom() = "FRO";
    _airSeg->boardMultiCity() = "FRO";
    CPPUNIT_ASSERT(_matchUtil->matchToFrom(*_airSeg, *_negFareRestExtSeq));
  }

  void testMatchToFrom_FailTo()
  {
    _negFareRestExtSeq->cityTo() = "TOO";
    _airSeg->offMultiCity() = "BAD";
    CPPUNIT_ASSERT(!_matchUtil->matchToFrom(*_airSeg, *_negFareRestExtSeq));
  }

  void testMatchToFrom_PassTo()
  {
    _negFareRestExtSeq->cityTo() = "TOO";
    _airSeg->offMultiCity() = "TOO";
    CPPUNIT_ASSERT(_matchUtil->matchToFrom(*_airSeg, *_negFareRestExtSeq));
  }

  void testMatchToFrom_FailFromInbound()
  {
    _negFareRestExtSeq->cityFrom() = "FRO";
    _airSeg->offMultiCity() = "BAD";
    _fareUsage->inbound() = true;
    CPPUNIT_ASSERT(!_matchUtil->matchToFrom(*_airSeg, *_negFareRestExtSeq));
  }

  void testMatchToFrom_PassFromInbound()
  {
    _negFareRestExtSeq->cityFrom() = "FRO";
    _airSeg->offMultiCity() = "FRO";
    _fareUsage->inbound() = true;
    CPPUNIT_ASSERT(_matchUtil->matchToFrom(*_airSeg, *_negFareRestExtSeq));
  }

  void testMatchToFrom_FailToInbound()
  {
    _negFareRestExtSeq->cityTo() = "TOO";
    _airSeg->boardMultiCity() = "BAD";
    _fareUsage->inbound() = true;
    CPPUNIT_ASSERT(!_matchUtil->matchToFrom(*_airSeg, *_negFareRestExtSeq));
  }

  void testMatchToFrom_PassToInbound()
  {
    _negFareRestExtSeq->cityTo() = "TOO";
    _airSeg->boardMultiCity() = "TOO";
    _fareUsage->inbound() = true;
    CPPUNIT_ASSERT(_matchUtil->matchToFrom(*_airSeg, *_negFareRestExtSeq));
  }

  void testStoreMatchedInFareUsage_Empty()
  {
    std::vector<const NegFareRestExtSeq*> matched;
    matched.push_back(_negFareRestExtSeq);
    std::vector<const TravelSeg*> ts;
    ts.push_back(_airSeg);
    _matchUtil->storeMatchedInFareUsage(matched, ts, ts);
    CPPUNIT_ASSERT(!_fareUsage->netRemitPscResults().empty());
  }

  void testStoreMatchedInFareUsage_NotEmpty()
  {
    std::vector<const NegFareRestExtSeq*> matched;
    std::vector<const TravelSeg*> ts;
    _matchUtil->storeMatchedInFareUsage(matched, ts, ts);
    CPPUNIT_ASSERT(_fareUsage->netRemitPscResults().empty());
  }

  void testMatchFareComp_Fail_OnlyOneSeg()
  {
    std::vector<TravelSeg*>::const_iterator end = _fareUsage->travelSeg().end();
    CPPUNIT_ASSERT(
        !_matchUtil->matchFareComp(_fareUsage->travelSeg().begin(), end, *_negFareRestExtSeq));
  }

  void testMatchFareComp_Pass_MatchFirstToLast()
  {
    AirSeg seg;
    seg.carrier() = "CR";
    _fareUsage->travelSeg().push_back(&seg);

    std::vector<TravelSeg*>::const_iterator end = _fareUsage->travelSeg().end();
    CPPUNIT_ASSERT(
        _matchUtil->matchFareComp(_fareUsage->travelSeg().begin(), end, *_negFareRestExtSeq));
  }

  void testMatchFareComp_Pass_MatchFirstToNotLast()
  {
    AirSeg seg;
    seg.offMultiCity() = "TO";
    seg.carrier() = "CR";
    _negFareRestExtSeq->cityTo() = "TO";
    _fareUsage->travelSeg().push_back(&seg);
    _fareUsage->travelSeg().push_back(_memHandle.create<AirSeg>()); // last

    std::vector<TravelSeg*>::const_iterator end = _fareUsage->travelSeg().end();
    CPPUNIT_ASSERT(
        _matchUtil->matchFareComp(_fareUsage->travelSeg().begin(), end, *_negFareRestExtSeq));
  }

  void testMatchFareComp_Pass_MatchNotFirstToLast()
  {
    AirSeg seg;
    seg.boardMultiCity() = "FRO";
    seg.carrier() = "CR";
    _negFareRestExtSeq->cityFrom() = "FRO";
    _fareUsage->travelSeg().insert(_fareUsage->travelSeg().begin(), &seg);
    _fareUsage->travelSeg().insert(_fareUsage->travelSeg().begin(),
                                   _memHandle.create<AirSeg>()); // first

    std::vector<TravelSeg*>::const_iterator end = _fareUsage->travelSeg().end();
    CPPUNIT_ASSERT(
        _matchUtil->matchFareComp(_fareUsage->travelSeg().begin() + 1, end, *_negFareRestExtSeq));
  }

  void testMatchFareComp_Fail_Via()
  {
    _fareUsage->travelSeg().push_back(_memHandle.create<AirSeg>());
    _negFareRestExtSeq->viaCity1() = "";

    std::vector<TravelSeg*>::const_iterator end = _fareUsage->travelSeg().end();
    CPPUNIT_ASSERT(
        !_matchUtil->matchFareComp(_fareUsage->travelSeg().begin(), end, *_negFareRestExtSeq));
  }

  void testFindMatchedSeq_Pass()
  {
    _negFareRestExtSeq->viaCity1() = "";
    _airSeg->carrier() = "CR";
    std::vector<TravelSeg*>::const_iterator end = _fareUsage->travelSeg().end();
    CPPUNIT_ASSERT(_ruleData->negFareRestExtSeq().end() !=
                   _matchUtil->findMatchedSeq(_fareUsage->travelSeg().begin(), end));
  }

  void testFindMatchedSeq_Fail()
  {
    _airSeg->carrier() = "CR";
    std::vector<TravelSeg*>::const_iterator end = _fareUsage->travelSeg().end();
    CPPUNIT_ASSERT(_ruleData->negFareRestExtSeq().end() ==
                   _matchUtil->findMatchedSeq(_fareUsage->travelSeg().begin(), end));
  }

  void testProcess_Pass_WithSkipR3()
  {
    makeCat35Fare();
    _airSeg->carrier() = "CR";
    _negFareRest->tktFareDataInd1() = 'X';
    CPPUNIT_ASSERT(_matchUtil->process());
  }

  void testProcess_Pass_WithSkipR3Ext()
  {
    makeCat35Fare();
    _airSeg->carrier() = "CR";
    _negFareRestExt->tktFareDataSegExistInd() = 'N';
    CPPUNIT_ASSERT(_matchUtil->process());
  }

  void testProcess_Pass()
  {
    makeCat35Fare();
    _airSeg->carrier() = "CR";
    _negFareRestExtSeq->viaCity1() = "";
    CPPUNIT_ASSERT(_matchUtil->process());
  }

  void testProcess_Fail()
  {
    makeCat35Fare();
    _airSeg->carrier() = "CR";
    CPPUNIT_ASSERT(!_matchUtil->process());
  }

  void testProcess_Fail_NotCat35()
  {
    _airSeg->carrier() = "CR";
    CPPUNIT_ASSERT(!_matchUtil->process());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetRemitPscMatchUtilTest);
}
