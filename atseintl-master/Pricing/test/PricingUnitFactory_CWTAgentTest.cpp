#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/FareMarket.h"
#include "Pricing/MergedFareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/PUPQItem.h"
#include "Pricing/PricingUnitFactory.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/Agent.h"
#include "DBAccess/Customer.h"
#include "Rules/RuleConst.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
class PricingUnitFactory_CWTAgentTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingUnitFactory_CWTAgentTest);

  CPPUNIT_TEST(testBuildFareUsage_ADT_NationFrance15And35);
  CPPUNIT_TEST(testBuildFareUsage_ADT_NationFrance15);
  CPPUNIT_TEST(testBuildFareUsage_ADT_NationFrance35);
  CPPUNIT_TEST(testBuildFareUsage_ADT_NoNationFrance);
  CPPUNIT_TEST(testBuildFareUsage_ADT_NationFrance15And35_MatchedCorpId);
  CPPUNIT_TEST(testBuildFareUsage_ADT_NationFrance15And35_NotPrivate);
  CPPUNIT_TEST(testBuildFareUsage_ADT_NationFrance15And35_NotCwt);

  CPPUNIT_TEST(testIsFareValid_ADT_NationFrance15And35);
  CPPUNIT_TEST(testIsFareValid_ADT_NationFrance15);
  CPPUNIT_TEST(testIsFareValid_ADT_NationFrance35);
  CPPUNIT_TEST(testIsFareValid_ADT_NoNationFrance);
  CPPUNIT_TEST(testIsFareValid_ADT_NationFrance15And35_MatchedCorpId);
  CPPUNIT_TEST(testIsFareValid_ADT_NationFrance15And35_NotPrivate);
  CPPUNIT_TEST(testIsFareValid_ADT_NationFrance15And35_NotCwt);

  CPPUNIT_TEST_SUITE_END();

  class MockPricingUnitFactory;
  TestMemHandle _memHandle;
  MockPricingUnitFactory* _puf;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testBuildFareUsage_ADT_NationFrance15And35()
  {
    createPricingUnitFactory(
        "ADT", true, true, false, RuleConst::PRIVATE_TARIFF, Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(!_puf->mockBuildFareUsage());
  }

  void testBuildFareUsage_ADT_NationFrance15()
  {
    createPricingUnitFactory(
        "ADT", true, false, false, RuleConst::PRIVATE_TARIFF, Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(!_puf->mockBuildFareUsage());
  }

  void testBuildFareUsage_ADT_NationFrance35()
  {
    createPricingUnitFactory(
        "ADT", false, true, false, RuleConst::PRIVATE_TARIFF, Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(!_puf->mockBuildFareUsage());
  }

  void testBuildFareUsage_ADT_NoNationFrance()
  {
    createPricingUnitFactory(
        "ADT", false, false, false, RuleConst::PRIVATE_TARIFF, Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(_puf->mockBuildFareUsage());
  }

  void testBuildFareUsage_ADT_NationFrance15And35_MatchedCorpId()
  {
    createPricingUnitFactory(
        "ADT", true, true, true, RuleConst::PRIVATE_TARIFF, Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(_puf->mockBuildFareUsage());
  }

  void testBuildFareUsage_ADT_NationFrance15And35_NotPrivate()
  {
    createPricingUnitFactory("ADT", true, true, false, 0, Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(_puf->mockBuildFareUsage());
  }

  void testBuildFareUsage_ADT_NationFrance15And35_NotCwt()
  {
    createPricingUnitFactory("ADT", true, true, false, RuleConst::PRIVATE_TARIFF, 0);
    CPPUNIT_ASSERT(_puf->mockBuildFareUsage());
  }

  void testIsFareValid_ADT_NationFrance15And35()
  {
    createPricingUnitFactory(
        "ADT", true, true, false, RuleConst::PRIVATE_TARIFF, Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(!_puf->mockIsFareValid());
  }

  void testIsFareValid_ADT_NationFrance15()
  {
    createPricingUnitFactory(
        "ADT", true, false, false, RuleConst::PRIVATE_TARIFF, Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(!_puf->mockIsFareValid());
  }

  void testIsFareValid_ADT_NationFrance35()
  {
    createPricingUnitFactory(
        "ADT", false, true, false, RuleConst::PRIVATE_TARIFF, Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(!_puf->mockIsFareValid());
  }

  void testIsFareValid_ADT_NoNationFrance()
  {
    createPricingUnitFactory(
        "ADT", false, false, false, RuleConst::PRIVATE_TARIFF, Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(_puf->mockIsFareValid());
  }

  void testIsFareValid_ADT_NationFrance15And35_MatchedCorpId()
  {
    createPricingUnitFactory(
        "ADT", true, true, true, RuleConst::PRIVATE_TARIFF, Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(_puf->mockIsFareValid());
  }

  void testIsFareValid_ADT_NationFrance15And35_NotPrivate()
  {
    createPricingUnitFactory("ADT", true, true, false, 0, Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(_puf->mockIsFareValid());
  }

  void testIsFareValid_ADT_NationFrance15And35_NotCwt()
  {
    createPricingUnitFactory("ADT", true, true, false, RuleConst::PRIVATE_TARIFF, 0);
    CPPUNIT_ASSERT(_puf->mockIsFareValid());
  }

private:
  void createPricingUnitFactory(const PaxTypeCode& paxTypeCode,
                                bool nationFRInCat15,
                                bool nationFRInCat35,
                                bool matchedCorpId,
                                TariffCategory tariffCat,
                                int agentGroupNumber)
  {
    _puf = _memHandle.create<MockPricingUnitFactory>();
    PaxType* paxType = getPaxType(paxTypeCode);
    _puf->_fareMarket =
        getFareMarket(paxType, nationFRInCat15, nationFRInCat35, matchedCorpId, tariffCat);
    //      _puf->_mergedMareMarket->pushBack(_puf->_fareMarket);
    MergedFareMarket* mfm = _memHandle.create<MergedFareMarket>();
    mfm->mergedFareMarket().push_back(_puf->_fareMarket);
    PU* pu = _memHandle.create<PU>();
    pu->fareMarket().push_back(mfm);
    pu->fareDirectionality().push_back(FROM);
    _puf->pu() = pu;
    _puf->paxType() = paxType;
    _puf->trx() = getPricingTrx(agentGroupNumber);
    _puf->itin() = _memHandle.create<Itin>();
  }

  PaxType* getPaxType(PaxTypeCode paxTypeCode)
  {
    PaxType* paxType = _memHandle.create<PaxType>();
    paxType->number() = 1;
    paxType->paxType() = paxTypeCode;
    paxType->paxTypeInfo() = _memHandle.create<PaxTypeInfo>();
    return paxType;
  }

  PricingTrx* getPricingTrx(int agentGroupNumber)
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setRequest(_memHandle.create<PricingRequest>());
    trx->setOptions(_memHandle.create<PricingOptions>());
    trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    Customer* customer = _memHandle.create<Customer>();
    customer->ssgGroupNo() = agentGroupNumber;
    trx->getRequest()->ticketingAgent()->agentTJR() = customer;
    return trx;
  }

  FareMarket* getFareMarket(PaxType* paxType,
                            bool nationFRInCat15,
                            bool nationFRInCat35,
                            bool matchedCorpId,
                            TariffCategory tariffCat)
  {
    const std::string fareClass = "WMLUQOW";
    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    fareMarket->fareBasisCode() = fareClass;
    fareMarket->fareCalcFareAmt() =
        "100"; // Making fare dummy so that isValidFare returns true all the time when testing
    // buildFareUsage. It will be cleared before testing isValidFare.
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->_fareClass = fareClass;
    fareInfo->_directionality = BOTH;
    fareInfo->_currency = USD;
    TariffCrossRefInfo* tariffRefInfo = _memHandle.create<TariffCrossRefInfo>();
    tariffRefInfo->_tariffCat = tariffCat;
    Fare* fare = _memHandle.create<Fare>();
    fare->setNationFRInCat15(nationFRInCat15);
    fare->initialize(Fare::FS_International, fareInfo, *fareMarket, tariffRefInfo);
    PaxTypeFare* paxTypeFare = _memHandle.create<PaxTypeFare>();
    paxTypeFare->initialize(fare, paxType, fareMarket);
    paxTypeFare->setFare(fare);
    if (nationFRInCat35)
      paxTypeFare->setNationFRInCat35();
    if (matchedCorpId)
      paxTypeFare->setMatchedCorpID();
    FareClassAppSegInfo* fareClassAppSegInfo = _memHandle.create<FareClassAppSegInfo>();
    fareClassAppSegInfo->_paxType = paxType->paxType();
    paxTypeFare->fareClassAppSegInfo() = fareClassAppSegInfo;
    PaxTypeBucket paxTypeCortege;
    paxTypeCortege.requestedPaxType() = paxType;
    paxTypeCortege.paxTypeFare().push_back(paxTypeFare);
    fareMarket->paxTypeCortege().push_back(paxTypeCortege);
    return fareMarket;
  }

  class MockPricingUnitFactory : public PricingUnitFactory
  {
  public:
    MockPricingUnitFactory()
    {
      _pupqItem.pricingUnit() = &_pricingUnit;
      _pupqItem.isValid() = false;
    }

    bool mockBuildFareUsage()
    {
      bool b = false;
      return PricingUnitFactory::buildFareUsage(0, 0, 0, b, _pupqItem, 0, false, _fareType, _diag);
    }

    bool mockIsFareValid()
    {
      bool b = false;

      PaxTypeBucket& paxTypeCortege = _fareMarket->paxTypeCortege().front();
      PaxTypeFare* paxTypeFare = paxTypeCortege.paxTypeFare().front();

      // Remove fare's "dumminess" so that we can test isFareValid
      _fareMarket->fareCalcFareAmt().clear();

      return PricingUnitFactory::isFareValid(
          *paxTypeFare, paxTypeCortege, *_fareMarket, 0, b, _diag, true);
    }

    // Fake data to be used in testing
    PUPQItem _pupqItem;
    PricingUnit _pricingUnit;
    FareType _fareType;
    DiagCollector _diag;
    FareMarket* _fareMarket;
  };
};
CPPUNIT_TEST_SUITE_REGISTRATION(PricingUnitFactory_CWTAgentTest);
}
