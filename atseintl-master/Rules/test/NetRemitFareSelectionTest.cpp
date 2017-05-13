#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/NegFareRest.h"
#include "Diagnostic/DCFactory.h"
#include "Rules/NetRemitFareSelection.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestPricingTrxFactory.h"

namespace tse
{
using boost::assign::operator+=;

const std::string testDataPath = "/vobs/atseintl/test/testdata/data/";

class NetRemitFareSelectionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NetRemitFareSelectionTest);
  CPPUNIT_TEST(testSelectPublicAgainstPrivateFare_NoFares);
  CPPUNIT_TEST(testSelectPublicAgainstPrivateFare_OneFare);
  CPPUNIT_TEST(testSelectPublicAgainstPrivateFare_TwoPublicFares_DifferentAmounts);
  CPPUNIT_TEST(testSelectPublicAgainstPrivateFare_TwoPublicFares_SameAmounts);
  CPPUNIT_TEST(testSelectPublicAgainstPrivateFare_TwoPrivateFares_DifferentAmounts);
  CPPUNIT_TEST(testSelectPublicAgainstPrivateFare_TwoPrivateFares_SameAmounts);
  CPPUNIT_TEST(testSelectPublicAgainstPrivateFare_PublicFarePrivateFare_SameAmounts);
  CPPUNIT_TEST(testSelectPublicAgainstPrivateFare_PrivateFarePublicFare_SameAmounts);

  CPPUNIT_TEST(testCheckAndBetwDontChangeWhenBetwNotEmpty);
  CPPUNIT_TEST(testCheckAndBetwDontChangeWhenAndNotEmpty);
  CPPUNIT_TEST(testCheckAndBetwDontChangeWhenBothNotEmpty);
  CPPUNIT_TEST(testCheckAndBetwDontChangeWhenAllMarketsMatch);
  CPPUNIT_TEST(testCheckAndBetwEliminateOneWhenOneDirectionNotMatch);
  CPPUNIT_TEST(testFindFaresForPaxTypeReturnEmptyWhenFmNotValid);
  CPPUNIT_TEST(testFindFaresForPaxTypeReturnEmptyWhePaxCodeNoMatch);
  CPPUNIT_TEST(testFindFaresForPaxTypeReturnFareWhePaxCodeMatch);
  CPPUNIT_TEST(tesCollectFaresFromFareMarketsReturnEmptyWhenFareMarketsNotValid);
  CPPUNIT_TEST(tesCollectFaresFromFareMarketsReturnEmptyWhenPaxCodeNoMatch);
  CPPUNIT_TEST(tesCollectFaresFromFareMarketsReturnFareWhenPaxCodeMatch);
  CPPUNIT_TEST(testDisplayPotentialFaresDoNothingWhenDiagnosticIsNotValid);
  CPPUNIT_TEST(testDisplayPotentialFares);
  CPPUNIT_TEST(testIsFareFamilySelectionWhenFareClassIsEmpty);
  CPPUNIT_TEST(testIsFareFamilySelectionWhenFareClassHasDash);

  CPPUNIT_TEST(testCheckMileageRoutingTktSelectedWhenRoutingIsMapPass);
  CPPUNIT_TEST(testCheckMileageRoutingTktSelectedWhenRoutingIsMapFail);
  CPPUNIT_TEST(testCheckMileageRoutingTktNotSelectedWhenMileageFail);
  CPPUNIT_TEST(testCheckMileageRoutingTktSelectedWhenMileagePass);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = TestPricingTrxFactory::create(testDataPath + "pricingTrx.xml");
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->getRequest()->ticketingAgent()->agentTJR() = NULL;
    _farePath = _memHandle.create<FarePath>();
    _pricingUnit = _memHandle.create<PricingUnit>();
    _fareUsage = _memHandle.create<FareUsage>();
    _negFareRest = _memHandle.create<NegFareRest>();
    _fareUsage->paxTypeFare() = createPaxTypeFare(FROM, "NYC", "DFW");

    _fareSelection = _memHandle.insert(
        new NetRemitFareSelection(*_trx, *_farePath, *_pricingUnit, *_fareUsage, *_negFareRest));
  }

  void tearDown() { _memHandle.clear(); }

  void testSelectPublicAgainstPrivateFare_NoFares()
  {
    std::vector<PaxTypeFare*> fares;

    CPPUNIT_ASSERT_EQUAL((const PaxTypeFare*)0,
                         _fareSelection->selectPublicAgainstPrivateFare(fares));
  }

  void testSelectPublicAgainstPrivateFare_OneFare()
  {
    std::vector<PaxTypeFare*> fares;
    MockPaxTypeFare paxTypeFare;

    fares.push_back(&paxTypeFare);

    CPPUNIT_ASSERT_EQUAL(static_cast<const PaxTypeFare*>(&paxTypeFare),
                         _fareSelection->selectPublicAgainstPrivateFare(fares));
  }

  void testSelectPublicAgainstPrivateFare_TwoPublicFares_DifferentAmounts()
  {
    std::vector<PaxTypeFare*> fares;
    MockPaxTypeFare paxTypeFare1(10);
    MockPaxTypeFare paxTypeFare2(100);

    fares.push_back(&paxTypeFare1);
    fares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(static_cast<const PaxTypeFare*>(&paxTypeFare1),
                         _fareSelection->selectPublicAgainstPrivateFare(fares));
  }

  void testSelectPublicAgainstPrivateFare_TwoPublicFares_SameAmounts()
  {
    std::vector<PaxTypeFare*> fares;
    MockPaxTypeFare paxTypeFare1(10);
    MockPaxTypeFare paxTypeFare2(100);

    fares.push_back(&paxTypeFare1);
    fares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(static_cast<const PaxTypeFare*>(&paxTypeFare1),
                         _fareSelection->selectPublicAgainstPrivateFare(fares));
  }

  void testSelectPublicAgainstPrivateFare_TwoPrivateFares_DifferentAmounts()
  {
    std::vector<PaxTypeFare*> fares;
    MockPaxTypeFare paxTypeFare1(10, true);
    MockPaxTypeFare paxTypeFare2(100, true);

    fares.push_back(&paxTypeFare1);
    fares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(static_cast<const PaxTypeFare*>(&paxTypeFare1),
                         _fareSelection->selectPublicAgainstPrivateFare(fares));
  }

  void testSelectPublicAgainstPrivateFare_TwoPrivateFares_SameAmounts()
  {
    std::vector<PaxTypeFare*> fares;
    MockPaxTypeFare paxTypeFare1(10, true);
    MockPaxTypeFare paxTypeFare2(100, true);

    fares.push_back(&paxTypeFare1);
    fares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(static_cast<const PaxTypeFare*>(&paxTypeFare1),
                         _fareSelection->selectPublicAgainstPrivateFare(fares));
  }

  void testSelectPublicAgainstPrivateFare_PublicFarePrivateFare_SameAmounts()
  {
    std::vector<PaxTypeFare*> fares;
    MockPaxTypeFare paxTypeFare1(100);
    MockPaxTypeFare paxTypeFare2(100, true);

    fares.push_back(&paxTypeFare1);
    fares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(static_cast<const PaxTypeFare*>(&paxTypeFare1),
                         _fareSelection->selectPublicAgainstPrivateFare(fares));
  }

  void testSelectPublicAgainstPrivateFare_PrivateFarePublicFare_SameAmounts()
  {
    std::vector<PaxTypeFare*> fares;
    MockPaxTypeFare paxTypeFare1(100, true);
    MockPaxTypeFare paxTypeFare2(100, false);

    fares.push_back(&paxTypeFare1);
    fares.push_back(&paxTypeFare2);

    CPPUNIT_ASSERT_EQUAL(static_cast<const PaxTypeFare*>(&paxTypeFare2),
                         _fareSelection->selectPublicAgainstPrivateFare(fares));
  }

  void testCheckAndBetwDontChangeWhenBetwNotEmpty()
  {
    std::vector<PaxTypeFare*> selFares;
    populatePaxTypeFares(selFares);
    _negFareRest->betwCity1() = "NYC";
    _fareSelection->checkAndBetw(selFares);
    CPPUNIT_ASSERT_EQUAL(size_t(3), selFares.size());
  }

  void testCheckAndBetwDontChangeWhenAndNotEmpty()
  {
    std::vector<PaxTypeFare*> selFares;
    populatePaxTypeFares(selFares);
    _negFareRest->andCity1() = "NYC";
    _fareSelection->checkAndBetw(selFares);
    CPPUNIT_ASSERT_EQUAL(size_t(3), selFares.size());
  }

  void testCheckAndBetwDontChangeWhenBothNotEmpty()
  {
    std::vector<PaxTypeFare*> selFares;
    populatePaxTypeFares(selFares);
    _negFareRest->betwCity1() = "NYC";
    _negFareRest->andCity1() = "NYC";
    _fareSelection->checkAndBetw(selFares);
    CPPUNIT_ASSERT_EQUAL(size_t(3), selFares.size());
  }

  void testCheckAndBetwDontChangeWhenAllMarketsMatch()
  {
    std::vector<PaxTypeFare*> selFares;
    populatePaxTypeFares(selFares);
    _fareSelection->checkAndBetw(selFares);
    CPPUNIT_ASSERT_EQUAL(size_t(3), selFares.size());
  }

  void testCheckAndBetwEliminateOneWhenOneDirectionNotMatch()
  {
    std::vector<PaxTypeFare*> selFares;
    populatePaxTypeFares(selFares);
    setDirectionality(selFares.front(), TO);
    _fareSelection->checkAndBetw(selFares);
    CPPUNIT_ASSERT_EQUAL(size_t(2), selFares.size());
  }

  void testFindFaresForPaxTypeReturnEmptyWhenFmNotValid()
  {
    std::vector<PaxTypeFare*> fares;
    _fareSelection->findFaresForPaxType(fares, ADULT, NULL);
    CPPUNIT_ASSERT(fares.empty());
  }

  void testFindFaresForPaxTypeReturnEmptyWhePaxCodeNoMatch()
  {
    std::vector<PaxTypeFare*> fares;
    createFareMarket(ADULT);

    _fareSelection->findFaresForPaxType(fares, CHILD, _fareMarket);
    CPPUNIT_ASSERT(fares.empty());
  }

  void testFindFaresForPaxTypeReturnFareWhePaxCodeMatch()
  {
    std::vector<PaxTypeFare*> fares;
    createFareMarket(ADULT);

    _fareSelection->findFaresForPaxType(fares, ADULT, _fareMarket);
    CPPUNIT_ASSERT_EQUAL(size_t(1), fares.size());
  }

  void tesCollectFaresFromFareMarketsReturnEmptyWhenFareMarketsNotValid()
  {
    std::vector<PaxTypeFare*> fares;
    _farePath->paxType() = _memHandle.create<PaxType>();
    _farePath->paxType()->paxType() = ADULT;

    fares = _fareSelection->collectFaresFromFareMarkets(NULL, NULL, NULL, NULL);
    CPPUNIT_ASSERT(fares.empty());
  }

  void tesCollectFaresFromFareMarketsReturnEmptyWhenPaxCodeNoMatch()
  {
    std::vector<PaxTypeFare*> fares;
    _farePath->paxType() = _memHandle.create<PaxType>();
    _farePath->paxType()->paxType() = ADULT;
    createFareMarket(CHILD);

    fares = _fareSelection->collectFaresFromFareMarkets(_fareMarket, NULL, NULL, NULL);
    CPPUNIT_ASSERT(fares.empty());
  }

  void tesCollectFaresFromFareMarketsReturnFareWhenPaxCodeMatch()
  {
    std::vector<PaxTypeFare*> fares;
    _farePath->paxType() = _memHandle.create<PaxType>();
    _farePath->paxType()->paxType() = ADULT;
    createFareMarket(ADULT);

    fares = _fareSelection->collectFaresFromFareMarkets(_fareMarket, NULL, NULL, NULL);
    CPPUNIT_ASSERT_EQUAL(size_t(1), fares.size());
  }

  void testDisplayPotentialFaresDoNothingWhenDiagnosticIsNotValid()
  {
    _fareSelection->displayPotentialFares(NULL, NULL, NULL, NULL);
    CPPUNIT_ASSERT(_trx->diagnostic().toString().empty());
  }

  void testDisplayPotentialFares()
  {
    _farePath->paxType() = _memHandle.create<PaxType>();
    _farePath->paxType()->paxType() = CHILD;
    createFareMarket(ADULT);
    createDiagnostic();
    _fareSelection->displayPotentialFares(_fareMarket, _fareMarket, _fareMarket, _fareMarket);
    _fareSelection->_diag->flushMsg();
    CPPUNIT_ASSERT_EQUAL(std::string("NO MATCHING FARES FOUND"),
                         _trx->diagnostic().toString().substr(255, 23));
  }

  void testIsFareFamilySelectionWhenFareClassIsEmpty()
  {
    FareClassCode fareClass;
    CPPUNIT_ASSERT(!_fareSelection->isFareFamilySelection(fareClass));
  }

  void testIsFareFamilySelectionWhenFareClassHasDash()
  {
    FareClassCode fareClass = "A-B";
    CPPUNIT_ASSERT(_fareSelection->isFareFamilySelection(fareClass));
  }

  void testIsFareFamilySelectionWhenFareClassHasntDash()
  {
    FareClassCode fareClass = "AB";
    CPPUNIT_ASSERT(!_fareSelection->isFareFamilySelection(fareClass));
  }

  void testCheckMileageRoutingTktSelectedWhenRoutingIsMapPass()
  {
    PaxTypeFare* ptf = createPaxTypeFare(FROM, "NYC", "DFW");
    ptf->setRoutingValid(true);
    ptf->setIsRouting(true);
    _fareUsage->tktNetRemitFare() = ptf;
    _fareSelection->checkMileageRouting();

    CPPUNIT_ASSERT(_fareUsage->tktNetRemitFare() != 0);
  }

  void testCheckMileageRoutingTktSelectedWhenRoutingIsMapFail()
  {
    PaxTypeFare* ptf = createPaxTypeFare(FROM, "NYC", "DFW");
    ptf->setRoutingValid(false);
    ptf->setIsRouting(true);
    _fareUsage->tktNetRemitFare() = ptf;
    _fareSelection->checkMileageRouting();

    CPPUNIT_ASSERT(_fareUsage->tktNetRemitFare() != 0);
  }

  void testCheckMileageRoutingTktNotSelectedWhenMileageFail()
  {
    PaxTypeFare* ptf = createPaxTypeFare(FROM, "NYC", "DFW");
    ptf->setRoutingValid(false);
    ptf->setIsRouting(false);
    _fareUsage->tktNetRemitFare() = ptf;
    _fareSelection->checkMileageRouting();

    CPPUNIT_ASSERT(_fareUsage->tktNetRemitFare() == 0);
  }

  void testCheckMileageRoutingTktSelectedWhenMileagePass()
  {
    PaxTypeFare* ptf = createPaxTypeFare(FROM, "NYC", "DFW");
    ptf->setRoutingValid(true);
    ptf->setIsRouting(false);
    _fareUsage->tktNetRemitFare() = ptf;
    _fareSelection->checkMileageRouting();

    CPPUNIT_ASSERT(_fareUsage->tktNetRemitFare() != 0);
  }

protected:
  // helper functions
  void createFareMarket(const PaxTypeCode& paxTypeCode)
  {
    _fareMarket = _memHandle.create<FareMarket>();
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    PaxTypeBucket* ptCortege = _memHandle.create<PaxTypeBucket>();
    ptCortege->paxTypeFare().push_back(ptf);
    PaxType* paxType = _memHandle.create<PaxType>();
    paxType->paxType() = paxTypeCode;
    ptCortege->requestedPaxType() = paxType;
    _fareMarket->paxTypeCortege().push_back(*ptCortege);
  }

  void createDiagnostic()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic692;
    _trx->diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    _fareSelection->_diag = factory->create(*_trx);
    _fareSelection->_diag->enable(Diagnostic692);
    _fareSelection->_dispAllFares = false;
  }

  void populatePaxTypeFares(std::vector<PaxTypeFare*>& selFares)
  {
    PaxTypeFare* ptf1 = createPaxTypeFare(FROM, "NYC", "DFW");
    PaxTypeFare* ptf2 = createPaxTypeFare(FROM, "NYC", "DFW");
    PaxTypeFare* ptf3 = createPaxTypeFare(FROM, "NYC", "DFW");
    selFares += ptf1, ptf2, ptf3;
  }

  PaxTypeFare*
  createPaxTypeFare(const Directionality& dir, const LocCode& market1, const LocCode& market2)
  {
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->directionality() = dir;
    fareInfo->market1() = market1;
    fareInfo->market2() = market2;
    fareInfo->currency() = "USD";
    fare->setFareInfo(fareInfo);
    ptf->setFare(fare);
    return ptf;
  }

  void setDirectionality(PaxTypeFare* ptf, const Directionality& dir)
  {
    const_cast<FareInfo*>(ptf->fare()->fareInfo())->directionality() = dir;
  }

  void setMarkets(PaxTypeFare* ptf, const LocCode& market1, const LocCode& market2)
  {
    const_cast<FareInfo*>(ptf->fare()->fareInfo())->market1() = market1;
    const_cast<FareInfo*>(ptf->fare()->fareInfo())->market2() = market2;
  }

  class MockPaxTypeFare : public PaxTypeFare
  {
  public:
    MockPaxTypeFare(MoneyAmount fareAmount = 1.0, bool privateFare = false)
    {
      fare_.nucFareAmount() = fareAmount;
      fare_.setFareInfo(&fareInfo_);
      tcrInfo_.tariffCat() = privateFare ? RuleConst::PRIVATE_TARIFF : 0;
      fare_.setTariffCrossRefInfo(&tcrInfo_);
      setFare(&fare_);
    }

  protected:
    FareInfo fareInfo_;
    Fare fare_;
    TariffCrossRefInfo tcrInfo_;
  };

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FarePath* _farePath;
  PricingUnit* _pricingUnit;
  FareUsage* _fareUsage;
  NegFareRest* _negFareRest;
  NetRemitFareSelection* _fareSelection;
  FareMarket* _fareMarket;
  tse::ConfigMan* _configMan;
};

CPPUNIT_TEST_SUITE_REGISTRATION(NetRemitFareSelectionTest);
}
