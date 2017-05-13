#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestFactoryManager.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/RexPricingTrx.h"
#include "RexPricing/PrepareRexFareRules.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "test/include/TestMemHandle.h"
#include "RexPricing/test/MockRexRuleController.h"
#include "test/testdata/TestPaxTypeFareFactory.h"

using namespace std;

namespace tse
{

class RexFarePrepareRuleTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexFarePrepareRuleTest);
  CPPUNIT_TEST(testFoundNoCat31Rule);
  CPPUNIT_TEST(testGotAndMatchedCat31Rule);
  CPPUNIT_TEST(testGotAndUnmatchedCat31Rule);
  CPPUNIT_TEST(testCat25BaseFareGotAndMatchedCat31Rule);
  CPPUNIT_TEST(testCat25BaseFareGotAndUnmatchedCat31Rule);
  CPPUNIT_TEST(testCat25FareGotAndMatchedCat31Rule);
  CPPUNIT_TEST(testCat25FareGotAndUnmatchedCat31Rule);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<RexPricingTrx>();

    ExcItin* excItin = _memHandle.create<ExcItin>();
    CPPUNIT_ASSERT(excItin != 0);
    _trx->exchangeItin().push_back(excItin);

    _paxTypeFare = _memHandle.create<PaxTypeFare>();

    Fare* fare = _memHandle.create<Fare>();

    FareInfo* fareInfo = _memHandle.create<FareInfo>();

    fareInfo->vendor() = "ATP";
    fareInfo->carrier() = "AA";
    fareInfo->fareTariff() = 0;
    fareInfo->ruleNumber() = "DUMM";

    fare->setFareInfo(fareInfo);
    _paxTypeFare->setFare(fare);

    _fareInfo = fareInfo;

    // optional baseFare for FBR
    _baseFare = _memHandle.create<PaxTypeFare>();

    Fare* fareB = _memHandle.create<Fare>();

    FareInfo* fareInfoB = _memHandle.create<FareInfo>();

    fareInfoB->vendor() = "ATP";
    fareInfoB->carrier() = "AA";
    fareInfoB->fareTariff() = 0;
    fareInfoB->ruleNumber() = "DUMM";

    fareB->setFareInfo(fareInfoB);
    _baseFare->setFare(fareB);

    _baseFareInfo = fareInfoB;
    _cat31Rule = 0;

    CombinabilityRuleInfo* rec2Cat10 = _memHandle.create<CombinabilityRuleInfo>();
    _paxTypeFare->rec2Cat10() = rec2Cat10;
  }

  void tearDown()
  {
    _memHandle.clear();
    TestFactoryManager::instance()->destroyAll();
  }

  void createNoCat31RulePaxTypeFare()
  {
    _paxTypeFare->resetRuleStatus();
    _paxTypeFare->status().set(PaxTypeFare::PTF_FareByRule, false);
    _paxTypeFare->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    _fareInfo->ruleNumber() = "NRUL";
    Fare* fare = _paxTypeFare->fare();
    fare->setFareInfo(_fareInfo);
    _paxTypeFare->setFare(fare);
  }

  void createNoMatchCat31RulePaxTypeFare()
  {
    _paxTypeFare->resetRuleStatus();
    _paxTypeFare->status().set(PaxTypeFare::PTF_FareByRule, false);
    _paxTypeFare->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    _fareInfo->ruleNumber() = "NMAT";
    Fare* fare = _paxTypeFare->fare();
    fare->setFareInfo(_fareInfo);
    _paxTypeFare->setFare(fare);
  }

  void createMatchCat31RulePaxTypeFare()
  {
    _paxTypeFare->resetRuleStatus();
    _paxTypeFare->status().set(PaxTypeFare::PTF_FareByRule, false);
    _paxTypeFare->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    _fareInfo->ruleNumber() = "MATC";
    Fare* fare = _paxTypeFare->fare();
    fare->setFareInfo(_fareInfo);
    _paxTypeFare->setFare(fare);
  }

  void createFBRPaxTypeFareNoMatchCat31RuleBaseFare()
  {
    _paxTypeFare->resetRuleStatus();
    _paxTypeFare->status().set(PaxTypeFare::PTF_FareByRule, true);
    _paxTypeFare->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    _fareInfo->ruleNumber() = "BASF"; // simulating FBR override_cat31 = 'B'
    Fare* fare = _paxTypeFare->fare();
    fare->setFareInfo(_fareInfo);
    _paxTypeFare->setFare(fare);

    _baseFare->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    _baseFareInfo->ruleNumber() = "NRUL";
    Fare* fare1 = _baseFare->fare();
    fare1->setFareInfo(_baseFareInfo);
    _baseFare->setFare(fare1);

    FBRPaxTypeFareRuleData* ruleData = _paxTypeFare->getFbrRuleData(RuleConst::FARE_BY_RULE);

    if (ruleData == 0)
    {
      ruleData = _memHandle.create<FBRPaxTypeFareRuleData>();
    }

    CPPUNIT_ASSERT(ruleData != 0);

    ruleData->baseFare() = _baseFare;
    _paxTypeFare->setRuleData(25, _trx->dataHandle(), ruleData);
  }

  void createFBRPaxTypeFareMatchCat31RuleBaseFare()
  {
    _paxTypeFare->resetRuleStatus();
    _paxTypeFare->status().set(PaxTypeFare::PTF_FareByRule, true);
    _paxTypeFare->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    _fareInfo->ruleNumber() = "BASF"; // simulating FBR override_cat31 = 'B'
    Fare* fare = _paxTypeFare->fare();
    fare->setFareInfo(_fareInfo);
    _paxTypeFare->setFare(fare);

    _baseFare->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    _baseFareInfo->ruleNumber() = "MATC";
    Fare* fare1 = _baseFare->fare();
    fare1->setFareInfo(_baseFareInfo);
    _baseFare->setFare(fare1);

    FBRPaxTypeFareRuleData* ruleData = _paxTypeFare->getFbrRuleData(RuleConst::FARE_BY_RULE);

    if (ruleData == 0)
    {
      ruleData = _memHandle.create<FBRPaxTypeFareRuleData>();
    }

    CPPUNIT_ASSERT(ruleData != 0);

    ruleData->baseFare() = _baseFare;
    _paxTypeFare->setRuleData(25, _trx->dataHandle(), ruleData);
  }

  void createFBRPaxTypeFareNoMatchCat31Rule()
  {
    _paxTypeFare->resetRuleStatus();
    _paxTypeFare->status().set(PaxTypeFare::PTF_FareByRule, true);
    _paxTypeFare->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    _fareInfo->ruleNumber() = "NMAT";
    Fare* fare = _paxTypeFare->fare();
    fare->setFareInfo(_fareInfo);
    _paxTypeFare->setFare(fare);

    _baseFare->setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);
    _baseFareInfo->ruleNumber() = "NRUL";
    Fare* fare1 = _baseFare->fare();
    fare1->setFareInfo(_baseFareInfo);
    _baseFare->setFare(fare1);

    FBRPaxTypeFareRuleData* ruleData = _paxTypeFare->getFbrRuleData(RuleConst::FARE_BY_RULE);

    if (ruleData == 0)
    {
      ruleData = _memHandle.create<FBRPaxTypeFareRuleData>();
    }

    CPPUNIT_ASSERT(ruleData != 0);

    ruleData->baseFare() = _baseFare;
    _paxTypeFare->setRuleData(25, _trx->dataHandle(), ruleData);
  }

  void createFBRPaxTypeFareMatchCat31Rule() {}

  void testFoundNoCat31Rule()
  {
    createNoCat31RulePaxTypeFare();

    MockRexRuleController ruleController(VolunExcPrevalidation);
    PrepareRexFareRules prepareRexFareRules(*_trx, &ruleController);
    prepareRexFareRules.process(_paxTypeFare);

    CPPUNIT_ASSERT(_paxTypeFare->paxTypeFareRuleData(RuleConst::VOLUNTARY_EXCHANGE_RULE) == 0);
    CPPUNIT_ASSERT(!_paxTypeFare->isCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE));
  }

  void testGotAndUnmatchedCat31Rule()
  {
    createNoMatchCat31RulePaxTypeFare();

    MockRexRuleController ruleController(VolunExcPrevalidation);
    PrepareRexFareRules prepareRexFareRules(*_trx, &ruleController);
    prepareRexFareRules.process(_paxTypeFare);

    CPPUNIT_ASSERT(!_paxTypeFare->isCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE));
    CPPUNIT_ASSERT(_paxTypeFare->paxTypeFareRuleData(RuleConst::VOLUNTARY_EXCHANGE_RULE) != 0);
  }

  void testGotAndMatchedCat31Rule()
  {
    createMatchCat31RulePaxTypeFare();

    MockRexRuleController ruleController(VolunExcPrevalidation);
    PrepareRexFareRules prepareRexFareRules(*_trx, &ruleController);
    prepareRexFareRules.process(_paxTypeFare);

    CPPUNIT_ASSERT(_paxTypeFare->isCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE));
    CPPUNIT_ASSERT(_paxTypeFare->paxTypeFareRuleData(RuleConst::VOLUNTARY_EXCHANGE_RULE) != 0);
    CPPUNIT_ASSERT(_paxTypeFare->isCategorySoftPassed(RuleConst::VOLUNTARY_EXCHANGE_RULE));
  }

  void testCat25BaseFareGotAndMatchedCat31Rule()
  {
    createFBRPaxTypeFareMatchCat31RuleBaseFare();

    MockRexRuleController ruleController(VolunExcPrevalidation);
    PrepareRexFareRules prepareRexFareRules(*_trx, &ruleController);
    prepareRexFareRules.process(_paxTypeFare);

    CPPUNIT_ASSERT(_paxTypeFare->isCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE));
    CPPUNIT_ASSERT(_paxTypeFare->paxTypeFareRuleData(RuleConst::VOLUNTARY_EXCHANGE_RULE) == 0);
    CPPUNIT_ASSERT(_paxTypeFare->isCategorySoftPassed(RuleConst::VOLUNTARY_EXCHANGE_RULE));
    CPPUNIT_ASSERT(
        _paxTypeFare->baseFare()->isCategorySoftPassed(RuleConst::VOLUNTARY_EXCHANGE_RULE));
  }

  void testCat25BaseFareGotAndUnmatchedCat31Rule()
  {
    createFBRPaxTypeFareNoMatchCat31RuleBaseFare();

    MockRexRuleController ruleController(VolunExcPrevalidation);
    PrepareRexFareRules prepareRexFareRules(*_trx, &ruleController);
    prepareRexFareRules.process(_paxTypeFare);

    CPPUNIT_ASSERT(!_paxTypeFare->isCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE));
    CPPUNIT_ASSERT(_paxTypeFare->paxTypeFareRuleData(RuleConst::VOLUNTARY_EXCHANGE_RULE) == 0);
  }

  void testCat25FareGotAndMatchedCat31Rule()
  {
    createFBRPaxTypeFareNoMatchCat31RuleBaseFare();

    MockRexRuleController ruleController(VolunExcPrevalidation);
    PrepareRexFareRules prepareRexFareRules(*_trx, &ruleController);
    prepareRexFareRules.process(_paxTypeFare);

    CPPUNIT_ASSERT(!_paxTypeFare->isCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE));
    CPPUNIT_ASSERT(_paxTypeFare->paxTypeFareRuleData(RuleConst::VOLUNTARY_EXCHANGE_RULE) == 0);
  }

  void testCat25FareGotAndUnmatchedCat31Rule()
  {
    createFBRPaxTypeFareNoMatchCat31RuleBaseFare();

    MockRexRuleController ruleController(VolunExcPrevalidation);
    PrepareRexFareRules prepareRexFareRules(*_trx, &ruleController);
    prepareRexFareRules.process(_paxTypeFare);

    CPPUNIT_ASSERT(!_paxTypeFare->isCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE));
    CPPUNIT_ASSERT(_paxTypeFare->paxTypeFareRuleData(RuleConst::VOLUNTARY_EXCHANGE_RULE) == 0);
  }

private:
  TestMemHandle _memHandle;
  RexPricingTrx* _trx;

  PaxTypeFare* _paxTypeFare;
  FareInfo* _fareInfo;
  PaxTypeFare* _baseFare;
  FareInfo* _baseFareInfo;
  GeneralFareRuleInfo* _cat31Rule;
};
CPPUNIT_TEST_SUITE_REGISTRATION(RexFarePrepareRuleTest);
}
