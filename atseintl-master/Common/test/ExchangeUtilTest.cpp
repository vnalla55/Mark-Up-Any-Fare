#include "Common/ExchangeUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/RexBaseRequest.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <iostream>

namespace tse
{
class ExchangeUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ExchangeUtilTest);
  CPPUNIT_TEST(testCategoriesSetup);
  CPPUNIT_TEST(testExcTotalNucAmtFP);
  CPPUNIT_TEST(testExcTotalNucAmtC5Atrue);
  CPPUNIT_TEST(testExcTotalNucAmtC5Afalse);
  CPPUNIT_TEST(testExcTotalBaseCurrAmtFP);
  CPPUNIT_TEST(testExcTotalBaseCurrAmtC5Atrue);
  CPPUNIT_TEST(testExcTotalBaseCurrAmtC5Afalse);
  CPPUNIT_TEST(testExcNonRefundableNucAmtFP);
  CPPUNIT_TEST(testExcNonRefundableNucAmtC5Atrue);
  CPPUNIT_TEST(testExcNonRefundableNucAmtC5Afalse);
  CPPUNIT_TEST(testExcNonRefundableBaseCurrAmtFP);
  CPPUNIT_TEST(testExcNonRefundableBaseCurrAmtC5Atrue);
  CPPUNIT_TEST(testExcNonRefundableBaseCurrAmtC5Afalse);
  CPPUNIT_TEST(testFixedFirstLeg);
  CPPUNIT_TEST(testFixedFixedNormalNormal);
  CPPUNIT_TEST(testFlownFixedFixedNormal);
  CPPUNIT_TEST(testNotShoppedFixedFixedNormal);
  CPPUNIT_TEST(testFlownNotShoppedFixedNormal);
  CPPUNIT_TEST(testNotShoppedNotShoppedFixedNormal);
  CPPUNIT_TEST(testNotShoppedFixedNormalNotShopped);
  CPPUNIT_TEST(testFlownFixedNotShoppedNormal);
  CPPUNIT_TEST(testFixedNormalFixedNormal);
  CPPUNIT_TEST(testNormalNormalFixedNormal);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  NonRefundableUtil* _nru;
  RexPricingTrx* _trx;

public:
  ShoppingTrx::Leg* createLeg(bool isShopped, bool isFlown)
  {
    ShoppingTrx::Leg* leg = _memHandle.create<ShoppingTrx::Leg>();
    Itin* itin = _memHandle.create<Itin>();
    ShoppingTrx::SchedulingOption* sop = _memHandle.create<ShoppingTrx::SchedulingOption>(itin, 0);
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->isShopped() = isShopped;
    seg->unflown() = !isFlown;

    itin->travelSeg().push_back(seg);
    sop->itin() = itin;
    leg->sop().push_back(*sop);

    return leg;
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle(new RexPricingTrx);
    _trx->setExcTrxType(PricingTrx::PORT_EXC_TRX);
    ExcItin* ei = _memHandle(new ExcItin);
    ei->rexTrx() = _trx;
    FarePath* efp = _memHandle(new FarePath);
    RexPricingOptions* rpo = _memHandle(new RexPricingOptions);

    _trx->setOptions(rpo);
    _trx->exchangeItin().push_back(ei);
    efp->itin() = ei;
    ei->originationCurrency() = NUC;
    _trx->exchangeItin().front()->farePath().push_back(efp);
    _trx->setRequest(_memHandle.create<RexBaseRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();

    _nru = _memHandle(new NonRefundableUtil(*_trx));
  }

  void tearDown() { _memHandle.clear(); }

  void testCategoriesSetup()
  {
    std::vector<PaxTypeFare*> ptfv;
    PaxTypeFare* ptf = _memHandle(new PaxTypeFare);
    ptf->setFare(_memHandle(new Fare));
    ptf->resetRuleStatus();
    ptf->fare()->resetRuleStatus();
    ptfv.push_back(ptf);
    ptfv.push_back(ptf);

    static const unsigned cats[] = { 6, 7, 10, 13, 18, 27 };

    for (const unsigned cat : cats)
      CPPUNIT_ASSERT(!ptf->isCategoryProcessed(cat));

    ExchangeUtil::avoidValidationOfCategoriesInMinFares(*_trx, ptfv);

    for (const unsigned cat : cats)
      CPPUNIT_ASSERT(ptfv[1]->isCategoryProcessed(cat));
  }

  void testExcTotalNucAmtFP() { CPPUNIT_ASSERT_EQUAL(0.0, _nru->excTotalNucAmt()); }

  void testExcTotalNucAmtC5Atrue()
  {
    _trx->setExcTktNonRefundable(true);
    CPPUNIT_ASSERT_EQUAL(-EPSILON, _nru->excTotalNucAmt());
  }

  void testExcTotalNucAmtC5Afalse()
  {
    _trx->setExcTktNonRefundable(false);
    CPPUNIT_ASSERT_EQUAL(-EPSILON, _nru->excTotalNucAmt());
  }

  void testExcTotalBaseCurrAmtFP() { CPPUNIT_ASSERT_EQUAL(0.0, _nru->excTotalBaseCurrAmt()); }

  void testExcTotalBaseCurrAmtC5Atrue()
  {
    _trx->setExcTktNonRefundable(true);
    CPPUNIT_ASSERT_EQUAL(-EPSILON, _nru->excTotalBaseCurrAmt());
  }

  void testExcTotalBaseCurrAmtC5Afalse()
  {
    _trx->setExcTktNonRefundable(false);
    CPPUNIT_ASSERT_EQUAL(-EPSILON, _nru->excTotalBaseCurrAmt());
  }

  void testExcNonRefundableNucAmtFP() { CPPUNIT_ASSERT_EQUAL(0.0, _nru->excNonRefundableNucAmt()); }

  void testExcNonRefundableNucAmtC5Atrue()
  {
    _trx->setExcTktNonRefundable(true);
    CPPUNIT_ASSERT_EQUAL(-EPSILON, _nru->excNonRefundableNucAmt());
  }

  void testExcNonRefundableNucAmtC5Afalse()
  {
    _trx->setExcTktNonRefundable(false);
    CPPUNIT_ASSERT_EQUAL(0.0, _nru->excNonRefundableNucAmt());
  }

  void testExcNonRefundableBaseCurrAmtFP()
  {
    _trx->exchangeItin().front()->rexTrx() = _trx;
    _trx->itin().push_back(_trx->exchangeItin().front());
    CPPUNIT_ASSERT_EQUAL(0.0, _nru->excNonRefundableBaseCurrAmt());
  }

  void testExcNonRefundableBaseCurrAmtC5Atrue()
  {
    _trx->setExcTktNonRefundable(true);
    CPPUNIT_ASSERT_EQUAL(-EPSILON, _nru->excNonRefundableBaseCurrAmt());
  }

  void testExcNonRefundableBaseCurrAmtC5Afalse()
  {
    _trx->setExcTktNonRefundable(false);
    CPPUNIT_ASSERT_EQUAL(0.0, _nru->excNonRefundableBaseCurrAmt());
  }

  void testFixedFirstLeg()
  {
    std::vector<ShoppingTrx::Leg> legs;

    legs.push_back(*createLeg(false, false));
    legs.push_back(*createLeg(false, false));
    legs.push_back(*createLeg(false, false));
    legs.push_back(*createLeg(false, false));

    _trx->getMutableFixedLegs() = {true, false, false, false};

    CPPUNIT_ASSERT(ExchangeUtil::validateFixedLegsInCEXS(*_trx, legs));
  }

  void testFixedFixedNormalNormal()
  {
    std::vector<ShoppingTrx::Leg> legs;

    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));

    _trx->getMutableFixedLegs() = {true, true, false, false};

    CPPUNIT_ASSERT(ExchangeUtil::validateFixedLegsInCEXS(*_trx, legs));
  }

  void testFlownFixedFixedNormal()
  {
    std::vector<ShoppingTrx::Leg> legs;

    legs.push_back(*createLeg(true, true));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));

    _trx->getMutableFixedLegs() = {false, true, true, false};

    CPPUNIT_ASSERT(ExchangeUtil::validateFixedLegsInCEXS(*_trx, legs));
  }

  void testNotShoppedFixedFixedNormal()
  {
    std::vector<ShoppingTrx::Leg> legs;

    legs.push_back(*createLeg(false, false));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));

    _trx->getMutableFixedLegs() = {false, true, true, false};

    CPPUNIT_ASSERT(ExchangeUtil::validateFixedLegsInCEXS(*_trx, legs));
  }

  void testFlownNotShoppedFixedNormal()
  {
    std::vector<ShoppingTrx::Leg> legs;

    legs.push_back(*createLeg(true, true));
    legs.push_back(*createLeg(false, false));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));

    _trx->getMutableFixedLegs() = {false, false, true, false};

    CPPUNIT_ASSERT(ExchangeUtil::validateFixedLegsInCEXS(*_trx, legs));
  }

  void testNotShoppedNotShoppedFixedNormal()
  {
    std::vector<ShoppingTrx::Leg> legs;

    legs.push_back(*createLeg(false, false));
    legs.push_back(*createLeg(false, false));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));

    _trx->getMutableFixedLegs() = {false, false, true, false};

    CPPUNIT_ASSERT(ExchangeUtil::validateFixedLegsInCEXS(*_trx, legs));
  }

  void testNotShoppedFixedNormalNotShopped()
  {
    std::vector<ShoppingTrx::Leg> legs;

    legs.push_back(*createLeg(false, false));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(false, false));

    _trx->getMutableFixedLegs() = {false, true, false, false};

    CPPUNIT_ASSERT(ExchangeUtil::validateFixedLegsInCEXS(*_trx, legs));
  }

  void testFlownFixedNotShoppedNormal()
  {
    std::vector<ShoppingTrx::Leg> legs;

    legs.push_back(*createLeg(true, true));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(false, false));
    legs.push_back(*createLeg(true, false));

    _trx->getMutableFixedLegs() = {false, true, false, false};

    CPPUNIT_ASSERT(ExchangeUtil::validateFixedLegsInCEXS(*_trx, legs));
  }

  void testFixedNormalFixedNormal()
  {
    std::vector<ShoppingTrx::Leg> legs;

    legs.push_back(*createLeg(true, true));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));

    _trx->getMutableFixedLegs() = {true, false, true, false};

    CPPUNIT_ASSERT(!ExchangeUtil::validateFixedLegsInCEXS(*_trx, legs));
  }

  void testNormalNormalFixedNormal()
  {
    std::vector<ShoppingTrx::Leg> legs;

    legs.push_back(*createLeg(true, true));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));
    legs.push_back(*createLeg(true, false));

    _trx->getMutableFixedLegs() = {false, false, true, false};

    CPPUNIT_ASSERT(!ExchangeUtil::validateFixedLegsInCEXS(*_trx, legs));
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(ExchangeUtilTest);
}
