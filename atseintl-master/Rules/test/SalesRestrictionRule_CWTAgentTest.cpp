#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Customer.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/SalesRestriction.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/MergedFareMarket.h"
#include "Rules/RuleConst.h"
#include "Rules/SalesRestrictionRule.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class TestableSalesRestrictionRule : public SalesRestrictionRule
{
public:
  bool checkLocaleItems(PricingTrx& trx,
                        PaxTypeFare& paxTypeFare,
                        const CategoryRuleInfo& cri,
                        const SalesRestriction* salesRestrictionRule)
  {
    return tse::checkLocaleItems(*this,
        trx, 0, paxTypeFare, cri, salesRestrictionRule, false);
  }
};

class CWTAgentTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CWTAgentTest);
  CPPUNIT_TEST(testCwtAgentBlockFaresSITI);
  CPPUNIT_TEST(testCwtAgentBlockFaresNotCWTUser);
  CPPUNIT_TEST(testCwtAgentBlockFaresNotPrivateTariff);
  CPPUNIT_TEST(testCwtAgentBlockFaresMatchSaleAndMatchTktRestrictionsFalse);
  CPPUNIT_TEST(testCwtAgentBlockFaresMatchSaleAndMatchTktRestrictionsFalseTktsMayNotBeIssued);
  CPPUNIT_TEST(testCwtAgentBlockFaresNoNationLocation);
  CPPUNIT_TEST(testCwtAgentBlockFaresOneLocNationIsFrance);
  CPPUNIT_TEST(testCwtAgentBlockFaresNoNationFranceLocation);
  CPPUNIT_TEST(testCwtAgentBlockFaresOneLocNationIsFranceLocTypeStateNation);
  CPPUNIT_TEST(testCwtAgentBlockFaresOneLocNationIsFranceLocTypeNationState);
  CPPUNIT_TEST(testCwtAgentBlockFaresOneLocNationIsFranceLocTypeNationNation);
  CPPUNIT_TEST(testCwtAgentBlockFaresOneLocNationIsFranceNationMonaco);
  CPPUNIT_TEST(testCwtAgentBlockFaresFootNoteExists);
  CPPUNIT_TEST(testCwtAgentBlockFaresGeneralRuleExists);
  CPPUNIT_TEST(testCwtAgentBlockFaresNoGeneralRuleOrFootnote);
  CPPUNIT_TEST_SUITE_END();

protected:
  PricingTrx* _trx;
  PaxTypeFare* _paxTypeFare;
  CategoryRuleInfo* _cri;
  SalesRestriction* _sr;
  TestableSalesRestrictionRule* _srr;
  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testCwtAgentBlockFaresSITI()
  {
    // Vendor SITA, nation france flag should not be set
    getData(SITA_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_NATION,
            NATION_FRANCE,
            NATION_FRANCE,
            RuleConst::SALE_RESTRICTIONS_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            true,
            false,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15());
  }

  void testCwtAgentBlockFaresNotCWTUser()
  {
    // Not CWT user, nation france flag should not be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_NATION,
            NATION_FRANCE,
            NATION_FRANCE,
            RuleConst::SALE_RESTRICTIONS_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            true,
            false,
            RuleConst::PRIVATE_TARIFF,
            0);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15());
  }

  void testCwtAgentBlockFaresNotPrivateTariff()
  {
    // Not private tariff, nation france flag should not be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_NATION,
            NATION_FRANCE,
            NATION_FRANCE,
            RuleConst::SALE_RESTRICTIONS_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            true,
            false,
            0,
            Agent::CWT_GROUP_NUMBER);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15());
  }

  void testCwtAgentBlockFaresMatchSaleAndMatchTktRestrictionsFalse()
  {
    // False matchSaleRestriction and matchTktRestriction, nation france flag should not be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_NATION,
            NATION_FRANCE,
            NATION_FRANCE,
            RuleConst::SALE_RESTRICTIONS_RULE,
            RuleConst::TKTS_MAY_NOT_BE_SOLD,
            true,
            false,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15());
  }

  void testCwtAgentBlockFaresMatchSaleAndMatchTktRestrictionsFalseTktsMayNotBeIssued()
  {
    // False matchSaleRestriction and matchTktRestriction, nation france flag should not be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_NATION,
            NATION_FRANCE,
            NATION_FRANCE,
            RuleConst::SALE_RESTRICTIONS_RULE,
            RuleConst::TKTS_MAY_NOT_BE_ISSUED,
            true,
            false,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15());
  }

  void testCwtAgentBlockFaresNoNationLocation()
  {
    // No nation location type, nation france flag should not be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_STATE,
            LOCTYPE_STATE,
            NATION_FRANCE,
            NATION_FRANCE,
            RuleConst::SALE_RESTRICTIONS_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            true,
            false,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15());
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
  }

  void testCwtAgentBlockFaresNoNationFranceLocation()
  {
    // No nation france location, nation france flag should not be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_NATION,
            NATION_MONACO,
            NATION_MONACO,
            RuleConst::SALE_RESTRICTIONS_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            true,
            false,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15());
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
  }

  void testCwtAgentBlockFaresOneLocNationIsFrance()
  {
    // One loc is nation france, flag should be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_NATION,
            NATION_FRANCE,
            NATION_FRANCE,
            RuleConst::FARE_BY_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            true,
            false,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15());
  }

  void testCwtAgentBlockFaresOneLocNationIsFranceLocTypeStateNation()
  {
    // One loc is nation france, flag should be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_STATE,
            LOCTYPE_NATION,
            NATION_FRANCE,
            NATION_FRANCE,
            RuleConst::FARE_BY_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            true,
            false,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15());
  }

  void testCwtAgentBlockFaresOneLocNationIsFranceLocTypeNationState()
  {
    // One loc is nation france, flag should be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_STATE,
            NATION_FRANCE,
            NATION_FRANCE,
            RuleConst::FARE_BY_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            true,
            false,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15());
  }

  void testCwtAgentBlockFaresOneLocNationIsFranceLocTypeNationNation()
  {

    // One loc is nation france, flag should be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_NATION,
            NATION_MONACO,
            NATION_FRANCE,
            RuleConst::FARE_BY_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            true,
            false,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15());
  }

  void testCwtAgentBlockFaresOneLocNationIsFranceNationMonaco()
  {

    // One loc is nation france, flag should be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_NATION,
            NATION_FRANCE,
            NATION_MONACO,
            RuleConst::FARE_BY_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            true,
            false,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15());
  }

  void testCwtAgentBlockFaresFootNoteExists()
  {

    // Footnote exists, NationFRInCat15Fn should also be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_NATION,
            NATION_FRANCE,
            NATION_MONACO,
            RuleConst::SALE_RESTRICTIONS_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            true,
            true,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15());
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Gr());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fr());
  }

  void testCwtAgentBlockFaresGeneralRuleExists()
  {

    // General rule exists, NationFRInCat15Gr should also be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_NATION,
            NATION_FRANCE,
            NATION_MONACO,
            RuleConst::SALE_RESTRICTIONS_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            true,
            false,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15Gr());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fr());
  }

  void testCwtAgentBlockFaresNoGeneralRuleOrFootnote()
  {

    // No general rule or footnote, NationFRInCat15Fr should also be set
    getData(ATPCO_VENDOR_CODE,
            LOCTYPE_NATION,
            LOCTYPE_NATION,
            NATION_FRANCE,
            NATION_MONACO,
            RuleConst::SALE_RESTRICTIONS_RULE,
            RuleConst::TKTS_MAY_ONLY_BE_SOLD,
            false,
            false,
            RuleConst::PRIVATE_TARIFF,
            Agent::CWT_GROUP_NUMBER);
    _srr->checkLocaleItems(*_trx, *_paxTypeFare, *_cri, _sr);
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Fn());
    CPPUNIT_ASSERT(!_paxTypeFare->fare()->isNationFRInCat15Gr());
    CPPUNIT_ASSERT(_paxTypeFare->fare()->isNationFRInCat15Fr());
  }

  void getData(const std::string& vendorCode,
               Indicator locType1,
               Indicator locType2,
               NationCode nation1,
               NationCode nation2,
               uint16_t categoryNumber,
               Indicator locApp,
               bool isCat15GeneralRuleProcess,
               bool isFootNoteCtrlInfo,
               TariffCategory tariffCat,
               int agentGroupNumber)
  {
    _srr = _memHandle.create<TestableSalesRestrictionRule>();

    FareMarket* fareMarket = _memHandle.create<FareMarket>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    PaxType* paxType = _memHandle.create<PaxType>();

    TariffCrossRefInfo* tariffRefInfo = _memHandle.create<TariffCrossRefInfo>();
    tariffRefInfo->_tariffCat = tariffCat;

    Fare* fare = _memHandle.create<Fare>();
    fare->initialize(Fare::FS_International, fareInfo, *fareMarket, tariffRefInfo);
    fare->setCat15GeneralRuleProcess(isCat15GeneralRuleProcess);

    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _paxTypeFare->initialize(fare, paxType, fareMarket);
    _paxTypeFare->setFare(fare);

    Customer* customer = _memHandle.create<Customer>();
    customer->ssgGroupNo() = agentGroupNumber;

    Loc* agentLoc = _memHandle.create<Loc>();
    agentLoc->loc() = "PAR";
    agentLoc->nation() = "FR";

    Agent* agent = _memHandle.create<Agent>();
    agent->agentTJR() = customer;
    agent->agentLocation() = agentLoc;

    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getRequest()->ticketingAgent() = agent;

    LocKey* loc1 = _memHandle.create<LocKey>();
    loc1->locType() = locType1;
    loc1->loc() = nation1;

    LocKey* loc2 = _memHandle.create<LocKey>();
    loc2->locType() = locType2;
    loc2->loc() = nation2;

    // memory will be deleted by SalesRestriction
    Locale* locale = new Locale();
    locale->locAppl() = locApp;
    locale->loc1() = *loc1;
    locale->loc2() = *loc2;

    _sr = _memHandle.create<SalesRestriction>();
    _sr->vendor() = vendorCode;
    _sr->locales().push_back(locale);

    if (!isFootNoteCtrlInfo)
      _cri = _memHandle.create<CategoryRuleInfo>();
    else
      _cri = _memHandle.create<FootNoteCtrlInfo>();

    _cri->categoryNumber() = categoryNumber;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CWTAgentTest);

} // tse
