//-------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include <algorithm>
#include <set>
#include <vector>

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include <gmock/gmock.h>
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "Fares/FareCollectorOrchestrator.h"
#include "Server/TseServer.h"

#include "test/include/MockTseServer.h"
#include "test/include/TestMemHandle.h"

using ::testing::Return;
using ::testing::ByRef;

namespace tse
{
using namespace boost::assign;

namespace
{
class MockPaxTypeFare : public PaxTypeFare
{
public:
  MockPaxTypeFare(const MoneyAmount& nucFareAmount,
                  const Footnote& footNote1,
                  const Footnote& footNote2)
  {
    _fare._nucFareAmount = nucFareAmount;
    _fareInfo._footnote1 = footNote1;
    _fareInfo._footnote2 = footNote2;
    _fare.initialize(Fare::FS_ForeignDomestic, &_fareInfo, _market);
    setFare(&_fare);
  }
  MockPaxTypeFare(const VendorCode& vendor = "ATP",
                  const CarrierCode& carrier = "LO",
                  const TariffNumber& fareTariff = 0,
                  const RuleNumber& ruleNumber = "",
                  const FareClassCode& fareClass = "",
                  const MoneyAmount& fareAmount = 0.0,
                  const CurrencyCode& currency = "",
                  const Indicator& owrt = 'O',
                  const Directionality& directionality = FROM,
                  const PaxTypeCode& paxTypeCode = "")
  {
    _paxType.paxType() = paxTypeCode;

    _fareInfo._vendor = vendor;
    _fareInfo._carrier = carrier;
    _fareInfo._fareTariff = fareTariff;
    _fareInfo._ruleNumber = ruleNumber;
    _fareInfo._fareClass = fareClass;
    _fareInfo._fareAmount = fareAmount;
    _fareInfo._currency = currency;
    _fareInfo._owrt = owrt;
    _fareInfo._directionality = directionality;

    _fare.initialize(Fare::FS_ForeignDomestic, &_fareInfo, _market);

    if (carrier == "YY")
    {
      _iFare.initialize(_fare, _ifa, nullptr, _dh, false);
      _iFare.status().set(Fare::FS_IndustryFare, true);
      _iFare.changeFareClass() = true;
      setFare(&_iFare);
    }
    else
    {
      setFare(&_fare);
    }
    _actualPaxType = &_paxType;
    _fareMarket = &_market;
  }
  Fare _fare;
  IndustryFare _iFare;
  FareInfo _fareInfo;
  PaxType _paxType;
  FareMarket _market;
  IndustryFareAppl _ifa;
  DataHandle _dh;
};

class MockTravelSeg : public TravelSeg
{
public:
  TravelSeg* clone(DataHandle&) const { return 0; }
};
struct NegFareValidatorStub
{
};
class MockFaresGetter
{
public:
  MOCK_CONST_METHOD0(getFares, std::set<PaxTypeFare*>());
};

class FakeBuckets
{
public:
  FakeBuckets(TestMemHandle& memHandle) : _faresGetter(memHandle.create<MockFaresGetter>()) {}

  template <typename Validator>
  void collectValidNegFares(Validator validator, std::set<PaxTypeFare*>& result) const
  {
    if (_faresGetter)
      result = _faresGetter->getFares();
  }

  MockFaresGetter* _faresGetter;
};
}

class FareCollectorOrchestratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareCollectorOrchestratorTest);
  CPPUNIT_TEST(testcheckAltDateIsEffectiveFare);
  CPPUNIT_TEST(testcheckAltDateIsNOTEffectiveFare);
  CPPUNIT_TEST(testaltDateCAT15PositiveValidation);
  CPPUNIT_TEST(testaltDateCAT15NegativeValidation);

  CPPUNIT_TEST(testIsKeepFare);
  CPPUNIT_TEST(testIsKeepFare_FailOnVendor);
  CPPUNIT_TEST(testIsKeepFare_FailOnCarrier);
  CPPUNIT_TEST(testIsKeepFare_FailOnPaxType);
  CPPUNIT_TEST(testIsKeepFare_FailOnTariff);
  CPPUNIT_TEST(testIsKeepFare_FailOnRule);
  CPPUNIT_TEST(testIsKeepFare_FailOnFareClass);
  CPPUNIT_TEST(testIsKeepFare_FailOnFareAmount);
  CPPUNIT_TEST(testIsKeepFare_FailOnCurrency);
  CPPUNIT_TEST(testIsKeepFare_FailOnDirectionality);
  CPPUNIT_TEST(testForEachKeepFare_Pass);
  CPPUNIT_TEST(testForEachKeepFare_Fail);
  CPPUNIT_TEST(testForEachKeepFare_Empty);
  CPPUNIT_TEST(testGetKeepFareForNewFareMarket_Pass);
  CPPUNIT_TEST(testGetKeepFareForNewFareMarket_Fail);
  CPPUNIT_TEST(testGetKeepFareForNewFareMarket_Empty);

  CPPUNIT_TEST(testRetrieveNegFares_ShoppingTrx);
  CPPUNIT_TEST(testRetrieveNegFares_PricingTrx);
  CPPUNIT_TEST(testRetrieveNegFares_RepricingTrx_flagFalse);
  CPPUNIT_TEST(testRetrieveNegFares_RepricingTrx_flagTrue);

  CPPUNIT_TEST(testFindMaxSegmentOrderForFlownInExcFoundInFirstFareMarket);
  CPPUNIT_TEST(testFindMaxSegmentOrderForFlownInExcFoundInLastFareMarket);
  CPPUNIT_TEST(testInvalidateSomeFareMarketsDontInvalidateWhenNotApplyForFlown);
  CPPUNIT_TEST(testInvalidateSomeFareMarketsDontInvalidateWhenNotApplyForUnflown);
  CPPUNIT_TEST(testInvalidateSomeFareMarketsInvalidateForUnflownWhenRequireCurrentButNone);

  CPPUNIT_TEST(testRemoveDuplicateFares);
  CPPUNIT_TEST(testRemoveDuplicateFares_empty);
  CPPUNIT_TEST(testRemoveDuplicateFares_notSorted);
  CPPUNIT_TEST(testRemoveDuplicateFares_firstAmtDifferent);
  CPPUNIT_TEST(testRemoveDuplicateFares_lastAmtDifferent);
  CPPUNIT_TEST(testRemoveDuplicateFares_firstAmtDifferent_someNoteDiff);
  CPPUNIT_TEST(testRemoveDuplicateFares_lastAmtDifferent_someNoteDiff);
  CPPUNIT_TEST(testRemoveDuplicateFares_sameAmt_note1Diff);
  CPPUNIT_TEST(testRemoveDuplicateFares_sameAmt_note2Diff);
  CPPUNIT_TEST(testRemoveDuplicateFares_sameAmt_note1and2Diff);
  CPPUNIT_TEST(testRemoveDuplicateFares_sameAmt_firstNoteDiff);
  CPPUNIT_TEST(testRemoveDuplicateFares_sameAmt_lastNoteDiff);
  CPPUNIT_TEST(testRemoveDuplicateFares_sameAmt_beforelastNoteDiff);
  CPPUNIT_TEST(testRemoveDuplicateFares_sameAmt_threeKindOfDiffNotes);
  CPPUNIT_TEST(testRemoveDuplicateFares_changedOriginalSortedPTFPositions);
  CPPUNIT_TEST(testRemoveFaresInvalidForHipIfFareIsCat25);
  CPPUNIT_TEST(testRemoveFaresInvalidForHipIfFareIsNegotiated);
  CPPUNIT_TEST(testRemoveFaresInvalidForHipDontRemove);

  CPPUNIT_TEST(testCloneNewFMsWithFareApplication0);
  CPPUNIT_TEST(testCloneNewFMsWithFareApplicationH);
  CPPUNIT_TEST(testCloneNewFMsWithFareApplicationL);
  CPPUNIT_TEST(testCloneNewFMsWithFareApplicationC);
  CPPUNIT_TEST(testCloneNewFMsWithFareApplicationHLC);

  CPPUNIT_TEST(testMergeValidatingCarriers);
  CPPUNIT_TEST(testRestoreOrigValidatingCarriers);
  CPPUNIT_TEST(testRestorePtfVCs);

  CPPUNIT_TEST(testCollectValidNegFares);

  CPPUNIT_TEST(testVerifySpecificFareBasis);
  CPPUNIT_TEST(testVerifySpecificFareBasisYY);
  CPPUNIT_TEST(testVerifySpecificFareBasisYY_FailBookingCode);
  CPPUNIT_TEST(testVerifySpecificFareBasisAllInvalid);
  CPPUNIT_TEST(testVerifySpecificFareBasisNoRFB);
  CPPUNIT_TEST(testSpecificFareBasisAllArgumentsPositive);
  CPPUNIT_TEST(testSpecificFareBasisAllArgumentsNegative);

  CPPUNIT_TEST(testProcessFRRAdjustedSellingLevel);

  CPPUNIT_TEST_SUITE_END();

protected:
  PricingTrx* m_trx;
  PaxTypeFare* m_ptFare;
  Fare* m_oldFare;
  FareMarket* m_dest;
  GeneralFareRuleInfo* m_genInfo;
  CategoryRuleItemInfo* m_itemInfo;
  CategoryRuleItemInfoSet* m_ruleSet;
  PaxTypeFareRuleData* m_paxTypeFareRuleData;
  DateTime m_date;
  FareCollectorOrchestrator* _collector;
  MockTseServer* _server;
  TestMemHandle _memHandle;
  DataHandle _dataHandle;

  std::map<const FareMarket*, const PaxTypeFare*>* _fmToPtf;

public:
  void setUp()
  {
    _server = _memHandle.insert(new MockTseServer("tseserver"));
    m_date = DateTime(2009, 3, 5);

    _collector = _memHandle.insert(new FareCollectorOrchestrator(*_server, ""));
  }

  void tearDown()
  {
    _memHandle.clear();
    _dataHandle.clear();
  }

  void testcheckAltDateIsEffectiveFare()
  {
    DateTime d1(2008, 5, 13);
    setup(d1);

    CPPUNIT_ASSERT(!FareCollectorOrchestrator::checkAltDateIsNOTEffectiveFare(m_ptFare));
  }

  void testcheckAltDateIsNOTEffectiveFare()
  {
    DateTime d1(2008, 5, 11);
    setup(d1);

    CPPUNIT_ASSERT(FareCollectorOrchestrator::checkAltDateIsNOTEffectiveFare(m_ptFare));
  }

  void setup(DateTime& departureDate)
  {
    m_ptFare = _memHandle.create<PaxTypeFare>();
    m_oldFare = _memHandle.create<Fare>();

    setupTrx();

    FareMarket* fareMarketPaxType = _memHandle.create<FareMarket>();
    PaxType* paxType = _memHandle.create<PaxType>();
    Fare* farePaxType = _memHandle.create<Fare>();

    fareMarketPaxType->travelDate() = departureDate;

    m_ptFare->initialize(farePaxType, paxType, fareMarketPaxType, *m_trx);

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    FareMarket* fareMarket = _memHandle.create<FareMarket>();

    TariffCrossRefInfo* tariffRefInfo = _memHandle.create<TariffCrossRefInfo>();

    TSEDateInterval& eff = fareInfo->effInterval();

    eff.effDate() = DateTime(2008, 5, 12);
    eff.expireDate() = DateTime(2008, 6, 12);
    eff.discDate() = DateTime(2008, 6, 22);

    setGeneralRuleInfo();

    m_oldFare->initialize(Fare::FS_International, fareInfo, *fareMarket, tariffRefInfo);
    m_ptFare->setFare(m_oldFare);
  }

  void setupFarmarket(DateTime& travelDate)
  {
    m_dest = _memHandle.create<FareMarket>();
    m_dest->travelDate() = travelDate;
  }

  void setupTrx() { m_trx = _memHandle.create<PricingTrx>(); }

  void setGeneralRuleInfo()
  {
    // delete in destructor
    m_ruleSet = new CategoryRuleItemInfoSet();

    CategoryRuleItemInfo m_itemInfo;

    m_itemInfo.setRelationalInd(CategoryRuleItemInfo::THEN);
    m_itemInfo.setItemNo(165502);
    m_itemInfo.setItemcat(15);

    m_ruleSet->push_back(m_itemInfo);

    m_genInfo = _memHandle.create<GeneralFareRuleInfo>();

    m_genInfo->vendorCode() = "ATP";
    m_genInfo->addItemInfoSetNosync(m_ruleSet);
    m_genInfo->effDate() = DateTime(2008, 6, 12);
    m_genInfo->expireDate() = DateTime(2008, 6, 28);

    m_paxTypeFareRuleData = _memHandle.create<PaxTypeFareRuleData>();
    m_paxTypeFareRuleData->categoryRuleInfo() = m_genInfo;
    m_ptFare->setRuleData(15, _dataHandle, m_paxTypeFareRuleData);
  }

  void testaltDateCAT15NegativeValidation()
  {
    DateTime d1(2008, 5, 13);
    DateTime d2(2008, 6, 10);
    setupFarmarket(d2);
    setup(d1);

    CPPUNIT_ASSERT(!FareCollectorOrchestrator::altDateCAT15Validation(*m_trx, *m_dest, m_ptFare));
  }

  void testaltDateCAT15PositiveValidation()
  {
    DateTime d1(2008, 5, 13);
    DateTime d2(2008, 6, 22);
    setupFarmarket(d2);
    setup(d1);

    CPPUNIT_ASSERT(FareCollectorOrchestrator::altDateCAT15Validation(*m_trx, *m_dest, m_ptFare));
  }

  void testIsKeepFare()
  {
    MockPaxTypeFare excItinFare("ATP", "LO", 12, "RN12", "FCLASS", 100.0, "PLN", 'O', FROM, "ADT"),
        newItinFare("ATP", "LO", 12, "RN12", "FCLASS", 100.0, "PLN", 'O', FROM, "ADT");

    FareCollectorOrchestrator::IsKeepFare isKeepFare(excItinFare, m_date);
    CPPUNIT_ASSERT(isKeepFare(&newItinFare));
  }

  void testIsKeepFare_FailOnVendor()
  {
    MockPaxTypeFare excItinFare("ATP"), newItinFare("SITA");

    FareCollectorOrchestrator::IsKeepFare isKeepFare(excItinFare, m_date);
    CPPUNIT_ASSERT(!isKeepFare(&newItinFare));
  }

  void testIsKeepFare_FailOnCarrier()
  {
    MockPaxTypeFare excItinFare("ATP", "LO"), newItinFare("ATP", "LH");

    FareCollectorOrchestrator::IsKeepFare isKeepFare(excItinFare, m_date);
    CPPUNIT_ASSERT(!isKeepFare(&newItinFare));
  }

  void testIsKeepFare_FailOnTariff()
  {
    MockPaxTypeFare excItinFare("ATP", "LO", 12), newItinFare("ATP", "LO", 6);

    FareCollectorOrchestrator::IsKeepFare isKeepFare(excItinFare, m_date);
    CPPUNIT_ASSERT(!isKeepFare(&newItinFare));
  }

  void testIsKeepFare_FailOnRule()
  {
    MockPaxTypeFare excItinFare("ATP", "LO", 12, "RN12"), newItinFare("ATP", "LO", 12, "AAAA");

    FareCollectorOrchestrator::IsKeepFare isKeepFare(excItinFare, m_date);
    CPPUNIT_ASSERT(!isKeepFare(&newItinFare));
  }

  void testIsKeepFare_FailOnFareClass()
  {
    MockPaxTypeFare excItinFare("ATP", "LO", 12, "RN12", "TEST"),
        newItinFare("ATP", "LO", 12, "RN12", "AABBCC");

    FareCollectorOrchestrator::IsKeepFare isKeepFare(excItinFare, m_date);
    CPPUNIT_ASSERT(!isKeepFare(&newItinFare));
  }

  void testIsKeepFare_FailOnFareAmount()
  {
    MockPaxTypeFare excItinFare("ATP", "LO", 12, "RN12", "TEST", 1000.0),
        newItinFare("ATP", "LO", 12, "RN12", "TEST", 500.0);

    FareCollectorOrchestrator::IsKeepFare isKeepFare(excItinFare, m_date);
    CPPUNIT_ASSERT(!isKeepFare(&newItinFare));
  }

  void testIsKeepFare_FailOnCurrency()
  {
    MockPaxTypeFare excItinFare("ATP", "LO", 12, "RN12", "TEST", 1000.0, "PLN"),
        newItinFare("ATP", "LO", 12, "RN12", "TEST", 1000.0, "USD");

    FareCollectorOrchestrator::IsKeepFare isKeepFare(excItinFare, m_date);
    CPPUNIT_ASSERT(!isKeepFare(&newItinFare));
  }

  void testIsKeepFare_FailOnDirectionality()
  {
    MockPaxTypeFare excItinFare("ATP", "LO", 12, "RN12", "TEST", 1000.0, "PLN", 'O', FROM),
        newItinFare("ATP", "LO", 12, "RN12", "TEST", 1000.0, "USD", 'O', TO);

    FareCollectorOrchestrator::IsKeepFare isKeepFare(excItinFare, m_date);
    CPPUNIT_ASSERT(!isKeepFare(&newItinFare));
  }

  void testIsKeepFare_FailOnPaxType()
  {
    MockPaxTypeFare excItinFare("ATP", "LO", 12, "RN12", "FCLASS", 100.0, "PLN", 'O', FROM, "NEG"),
        newItinFare("ATP", "LO", 12, "RN12", "FCLASS", 100.0, "PLN", 'O', FROM, "ADT");

    FareCollectorOrchestrator::IsKeepFare isKeepFare(excItinFare, m_date);
    CPPUNIT_ASSERT(!isKeepFare(&newItinFare));
  }

  struct KeepFaresCounter
  {
    KeepFaresCounter(int& count_) : count(count_) {}
    void operator()(PaxTypeFare* fare) { ++count; }

  private:
    int& count;
  };

  void testForEachKeepFare_Pass()
  {
    MockPaxTypeFare sourceKeepFare, keepFare1, keepFare2, keepFare3, otherFare1("ATP", "AC"),
        otherFare2("ATP", "LH"), otherFare3("ATP", "AA"), otherFare4("SIT");

    std::vector<PaxTypeFare*> fares;
    fares += &otherFare1, &keepFare1, &keepFare2, &otherFare2, &otherFare3, &keepFare3, &otherFare4;

    int count = 0;
    KeepFaresCounter counter(count);
    _collector->forEachKeepFare(fares.begin(), fares.end(), sourceKeepFare, counter);
    CPPUNIT_ASSERT_EQUAL(3, count);
  }

  void testForEachKeepFare_Fail()
  {
    MockPaxTypeFare sourceKeepFare, otherFare1("ATP", "AC"), otherFare2("ATP", "LH"),
        otherFare3("ATP", "AA"), otherFare4("SIT");

    std::vector<PaxTypeFare*> fares;
    fares += &otherFare1, &otherFare2, &otherFare3, &otherFare4;

    int count = 0;
    KeepFaresCounter counter(count);
    _collector->forEachKeepFare(fares.begin(), fares.end(), sourceKeepFare, counter);
    CPPUNIT_ASSERT_EQUAL(0, count);
  }

  void testForEachKeepFare_Empty()
  {
    MockPaxTypeFare sourceKeepFare;

    std::vector<PaxTypeFare*> fares;

    int count = 0;
    KeepFaresCounter counter(count);
    _collector->forEachKeepFare(fares.begin(), fares.end(), sourceKeepFare, counter);
    CPPUNIT_ASSERT_EQUAL(0, count);
  }

  void testGetKeepFareForNewFareMarket_Pass()
  {
    FareMarket fm1, fm2;
    const PaxTypeFare ptf1, ptf2;
    RexPricingTrx trx;
    trx.newItinKeepFares()[&ptf1] = &fm1;
    trx.newItinKeepFares()[&ptf2] = &fm2;

    CPPUNIT_ASSERT_EQUAL(&ptf1, _collector->getKeepFareForNewFareMarket(trx, &fm1));
  }

  void testGetKeepFareForNewFareMarket_Fail()
  {
    FareMarket fm1, fm2, unStored;
    const PaxTypeFare ptf1, ptf2;
    RexPricingTrx trx;
    trx.newItinKeepFares()[&ptf1] = &fm1;
    trx.newItinKeepFares()[&ptf2] = &fm2;

    CPPUNIT_ASSERT_EQUAL(static_cast<const PaxTypeFare*>(0),
                         _collector->getKeepFareForNewFareMarket(trx, &unStored));
  }

  void testGetKeepFareForNewFareMarket_Empty()
  {
    FareMarket unStored;
    RexPricingTrx trx;

    CPPUNIT_ASSERT_EQUAL(static_cast<const PaxTypeFare*>(0),
                         _collector->getKeepFareForNewFareMarket(trx, &unStored));
  }

  void testRetrieveNegFares_ShoppingTrx()
  {
    ShoppingTrx* trx = _memHandle.create<ShoppingTrx>();
    CPPUNIT_ASSERT(FareCollectorOrchestrator::retrieveNegFares(*trx));
  }
  void testRetrieveNegFares_PricingTrx()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    CPPUNIT_ASSERT(FareCollectorOrchestrator::retrieveNegFares(*trx));
  }
  void testRetrieveNegFares_RepricingTrx_flagFalse()
  {
    RepricingTrx* trx = _memHandle.create<RepricingTrx>();
    CPPUNIT_ASSERT(!FareCollectorOrchestrator::retrieveNegFares(*trx));
  }
  void testRetrieveNegFares_RepricingTrx_flagTrue()
  {
    RepricingTrx* trx = _memHandle.create<RepricingTrx>();
    trx->retrieveNegFares() = true;
    CPPUNIT_ASSERT(FareCollectorOrchestrator::retrieveNegFares(*trx));
  }
  void testFindMaxSegmentOrderForFlownInExcFoundInFirstFareMarket()
  {
    ExcItin* excItin = createExcItinWithTwoFareMarets();
    excItin->fareMarket().front()->changeStatus() = FL;
    excItin->fareMarket().back()->changeStatus() = UU;
    int16_t maxFlownSegOrder = _collector->findMaxSegmentOrderForFlownInExc(excItin);
    int16_t lastFlownSeg = 1;
    CPPUNIT_ASSERT_EQUAL(lastFlownSeg, maxFlownSegOrder);
  }
  void testFindMaxSegmentOrderForFlownInExcFoundInLastFareMarket()
  {
    ExcItin* excItin = createExcItinWithTwoFareMarets();
    excItin->fareMarket().front()->changeStatus() = UU;
    excItin->fareMarket().back()->changeStatus() = FL;
    int16_t maxFlownSegOrder = _collector->findMaxSegmentOrderForFlownInExc(excItin);
    int16_t lastFlownSeg = 2;
    CPPUNIT_ASSERT_EQUAL(lastFlownSeg, maxFlownSegOrder);
  }
  void testInvalidateSomeFareMarketsDontInvalidateWhenNotApplyForFlown()
  {
    RexPricingTrx* trx = createRexTrx();
    ExcItin* excItin = createExcItinWithTwoFareMarets();
    Itin* itin = createItinWithTwoFareMarets();
    itin->fareMarket().front()->changeStatus() = FL;
    trx->exchangeItin().push_back(excItin);
    trx->itin().push_back(itin);

    _collector->invalidateSomeFareMarkets(*trx);
    CPPUNIT_ASSERT(!itin->fareMarket().front()->breakIndicator());
  }
  void testInvalidateSomeFareMarketsDontInvalidateWhenNotApplyForUnflown()
  {
    RexPricingTrx* trx = createRexTrx();
    ExcItin* excItin = createExcItinWithTwoFareMarets();
    Itin* itin = createItinWithTwoFareMarets();
    itin->fareMarket().front()->changeStatus() = UU;
    trx->exchangeItin().push_back(excItin);
    trx->itin().push_back(itin);

    _collector->invalidateSomeFareMarkets(*trx);
    CPPUNIT_ASSERT(!itin->fareMarket().front()->breakIndicator());
  }
  void testInvalidateSomeFareMarketsInvalidateForUnflownWhenRequireCurrentButNone()
  {
    RexPricingTrx* trx = createRexTrx(false, false, true, false);
    ExcItin* excItin = createExcItinWithTwoFareMarets();
    Itin* itin = createItinWithTwoFareMarets();
    itin->fareMarket().front()->changeStatus() = UU;
    trx->exchangeItin().push_back(excItin);
    trx->itin().push_back(itin);

    _collector->invalidateSomeFareMarkets(*trx);
    CPPUNIT_ASSERT(itin->fareMarket().front()->breakIndicator());
  }

  void testRemoveDuplicateFares()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(100, "F1", "F2");
    MockPaxTypeFare fare3(100, "F6", "F2");
    MockPaxTypeFare fare4(100, "F1", "F7");
    MockPaxTypeFare fare5(100, "F1", "F2");
    MockPaxTypeFare fare6(100, "F6", "F2");
    MockPaxTypeFare fare7(101, "F1", "F2");
    MockPaxTypeFare fare8(101, "F1", "F2");
    MockPaxTypeFare fare9(102, "F1", "F2");

    ptFares += &fare1, &fare2, &fare3, &fare4, &fare5, &fare6, &fare7, &fare8, &fare9;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare4, &fare3, &fare7, &fare9;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_empty()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_notSorted()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(101, "F1", "F2");
    MockPaxTypeFare fare3(100, "F1", "F2");

    ptFares += &fare1, &fare2, &fare3;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare2, &fare3;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_firstAmtDifferent()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(101, "F1", "F2");
    MockPaxTypeFare fare3(101, "F1", "F2");

    ptFares += &fare1, &fare2, &fare3;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare2;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_lastAmtDifferent()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(100, "F1", "F2");
    MockPaxTypeFare fare3(101, "F1", "F2");

    ptFares += &fare1, &fare2, &fare3;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare3;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_firstAmtDifferent_someNoteDiff()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(101, "F1", "F2");
    MockPaxTypeFare fare3(101, "F6", "F7");
    MockPaxTypeFare fare4(101, "F1", "F2");
    MockPaxTypeFare fare5(101, "F6", "F7");

    ptFares += &fare1, &fare2, &fare3, &fare4, &fare5;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare2, &fare3;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_lastAmtDifferent_someNoteDiff()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(100, "F1", "F2");
    MockPaxTypeFare fare3(100, "F6", "F7");
    MockPaxTypeFare fare4(100, "F1", "F2");
    MockPaxTypeFare fare5(101, "F6", "F7");

    ptFares += &fare1, &fare2, &fare3, &fare4, &fare5;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare3, &fare5;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_sameAmt_note1Diff()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(100, "F1", "F2");
    MockPaxTypeFare fare3(100, "F6", "F2");
    MockPaxTypeFare fare4(100, "F1", "F2");
    MockPaxTypeFare fare5(100, "F6", "F2");

    ptFares += &fare1, &fare2, &fare3, &fare4, &fare5;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare3;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_sameAmt_note2Diff()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(100, "F1", "F2");
    MockPaxTypeFare fare3(100, "F1", "F2");
    MockPaxTypeFare fare4(100, "F1", "F2");
    MockPaxTypeFare fare5(100, "F1", "F7");

    ptFares += &fare1, &fare2, &fare3, &fare4, &fare5;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare5;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_sameAmt_note1and2Diff()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(100, "F1", "F2");
    MockPaxTypeFare fare3(100, "F6", "F2");
    MockPaxTypeFare fare4(100, "F1", "F2");
    MockPaxTypeFare fare5(100, "F1", "F7");

    ptFares += &fare1, &fare2, &fare3, &fare4, &fare5;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare3, &fare5;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_sameAmt_firstNoteDiff()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(100, "F6", "F7");
    MockPaxTypeFare fare3(100, "F6", "F7");
    MockPaxTypeFare fare4(100, "F6", "F7");
    MockPaxTypeFare fare5(100, "F6", "F7");

    ptFares += &fare1, &fare2, &fare3, &fare4, &fare5;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare2;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_sameAmt_lastNoteDiff()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(100, "F1", "F2");
    MockPaxTypeFare fare3(100, "F1", "F2");
    MockPaxTypeFare fare4(100, "F1", "F2");
    MockPaxTypeFare fare5(100, "F6", "F7");

    ptFares += &fare1, &fare2, &fare3, &fare4, &fare5;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare5;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_sameAmt_beforelastNoteDiff()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(100, "F1", "F2");
    MockPaxTypeFare fare3(100, "F3", "F4");
    MockPaxTypeFare fare4(100, "F1", "F2");

    ptFares += &fare1, &fare2, &fare3, &fare4;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare3;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_sameAmt_threeKindOfDiffNotes()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(100, "F1", "F2");
    MockPaxTypeFare fare3(100, "F6", "F7");
    MockPaxTypeFare fare4(100, "F1", "F7");
    MockPaxTypeFare fare5(100, "F6", "F7");

    ptFares += &fare1, &fare2, &fare3, &fare4, &fare5;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare3, &fare4;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveDuplicateFares_changedOriginalSortedPTFPositions()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    Itin* itin = _memHandle.create<Itin>();
    std::vector<PaxTypeFare*> ptFares;

    MockPaxTypeFare fare1(100, "F1", "F2");
    MockPaxTypeFare fare2(100, "F3", "F4");
    MockPaxTypeFare fare3(100, "F6", "F7");
    MockPaxTypeFare fare4(100, "F1", "F2");
    MockPaxTypeFare fare5(100, "F6", "F7");

    ptFares += &fare1, &fare2, &fare3, &fare4, &fare5;
    setupDataStructures(ptFares, *trx, *fm);

    _collector->removeDuplicateFares(*trx, *itin, *fm);

    std::vector<PaxTypeFare*> expectedPTFares;
    expectedPTFares += &fare1, &fare3, &fare2;
    assertFares(fm->paxTypeCortege()[0].paxTypeFare(), expectedPTFares);
  }

  void testRemoveFaresInvalidForHipIfFareIsCat25()
  {
    RexPricingTrx* trx = createRexTrx();
    Itin* itin = createItinWithTwoFareMarets();
    trx->itin().push_back(itin);
    PaxTypeFare ptf1, ptf2;
    Fare fare;
    fare.nucFareAmount() = 100.0;
    ptf1.initialize(&fare, 0, 0, *trx);
    ptf2.nucFareAmount() = 200.0;
    ptf2.status().set(PaxTypeFare::PTF_FareByRule);
    itin->addFareMarketJustForRexPlusUps(itin->fareMarket()[0]);
    itin->fareMarket()[0]->allPaxTypeFare().push_back(&ptf2);

    _collector->removeFaresInvalidForHip(*trx);
    CPPUNIT_ASSERT(itin->fareMarket()[0]->allPaxTypeFare().empty());
  }

  void testRemoveFaresInvalidForHipIfFareIsNegotiated()
  {
    RexPricingTrx* trx = createRexTrx();
    Itin* itin = createItinWithTwoFareMarets();
    trx->itin().push_back(itin);
    PaxTypeFare ptf1, ptf2;
    Fare fare;
    fare.nucFareAmount() = 100.0;
    ptf1.initialize(&fare, 0, 0, *trx);
    ptf2.nucFareAmount() = 200.0;
    ptf2.status().set(PaxTypeFare::PTF_Negotiated);
    itin->addFareMarketJustForRexPlusUps(itin->fareMarket()[0]);
    itin->fareMarket()[0]->allPaxTypeFare().push_back(&ptf2);

    _collector->removeFaresInvalidForHip(*trx);
    CPPUNIT_ASSERT(itin->fareMarket()[0]->allPaxTypeFare().empty());
  }

  void testRemoveFaresInvalidForHipDontRemove()
  {
    RexPricingTrx* trx = createRexTrx();
    Itin* itin = createItinWithTwoFareMarets();
    trx->itin().push_back(itin);
    PaxTypeFare ptf1, ptf2;
    Fare fare;
    fare.nucFareAmount() = 100.0;
    ptf1.initialize(&fare, 0, 0, *trx);
    ptf2.nucFareAmount() = 200.0;
    itin->addFareMarketJustForRexPlusUps(itin->fareMarket()[0]);
    itin->fareMarket()[0]->allPaxTypeFare().push_back(&ptf2);

    _collector->removeFaresInvalidForHip(*trx);
    CPPUNIT_ASSERT_EQUAL(size_t(1), itin->fareMarket()[0]->allPaxTypeFare().size());
  }
  ExcItin* createExcItinWithTwoFareMarets()
  {
    TravelSeg* ts1 = _memHandle.create<AirSeg>();
    TravelSeg* ts2 = _memHandle.create<AirSeg>();
    ts1->segmentOrder() = 1;
    ts2->segmentOrder() = 2;
    ExcItin* excItin = _memHandle.create<ExcItin>();
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    FareMarket* fm2 = _memHandle.create<FareMarket>();
    fm1->travelSeg().push_back(ts1);
    fm2->travelSeg().push_back(ts2);
    excItin->fareMarket().push_back(fm1);
    excItin->fareMarket().push_back(fm2);
    return excItin;
  }
  Itin* createItinWithTwoFareMarets()
  {
    TravelSeg* ts1 = _memHandle.create<AirSeg>();
    TravelSeg* ts2 = _memHandle.create<AirSeg>();
    ts1->segmentOrder() = 1;
    ts2->segmentOrder() = 2;
    Itin* itin = _memHandle.create<Itin>();
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    FareMarket* fm2 = _memHandle.create<FareMarket>();
    fm1->travelSeg().push_back(ts1);
    fm2->travelSeg().push_back(ts2);
    itin->fareMarket().push_back(fm1);
    itin->fareMarket().push_back(fm2);
    return itin;
  }
  RexPricingTrx* createRexTrx(bool requireCurrentForFlown = false,
                              bool requireNotCurrentForFlown = false,
                              bool requireCurrentForUnflown = false,
                              bool requireNotCurrentForUnflown = false)
  {
    RexPricingTrx* trx = _memHandle.create<RexPricingTrx>();
    RexPricingRequest* rexRequest = _memHandle.create<RexPricingRequest>();
    trx->setRequest(rexRequest);
    rexRequest->ticketingAgent() = _memHandle.create<Agent>();
    trx->allPermutationsRequireCurrentForFlown() = requireCurrentForFlown;
    trx->allPermutationsRequireNotCurrentForFlown() = requireNotCurrentForFlown;
    trx->allPermutationsRequireCurrentForUnflown() = requireCurrentForUnflown;
    trx->allPermutationsRequireNotCurrentForUnflown() = requireNotCurrentForUnflown;
    return trx;
  }
  void setupDataStructures(std::vector<PaxTypeFare*>& ptFares, PricingTrx& trx, FareMarket& fm)
  {
    PaxType paxType;
    trx.paxType().push_back(&paxType);
    trx.paxType().push_back(&paxType);

    PaxTypeBucket ptCortage;
    ptCortage.paxTypeFare().swap(ptFares);
    fm.paxTypeCortege().push_back(ptCortage);
  }
  void assertFares(std::vector<PaxTypeFare*>& ptFares, std::vector<PaxTypeFare*>& expectedPTFares)
  {
    CPPUNIT_ASSERT_EQUAL(expectedPTFares.size(), ptFares.size());
    std::vector<PaxTypeFare*>::const_iterator expectedPTF = expectedPTFares.begin();
    std::vector<PaxTypeFare*>::const_iterator actualPTF = ptFares.begin();
    for (; expectedPTF != expectedPTFares.end() && actualPTF != ptFares.end();
         ++expectedPTF, ++actualPTF)
    {
      CPPUNIT_ASSERT(*expectedPTF == *actualPTF);
    }
  }

  RexPricingTrx* setUpCloneNewFMsWithFareApplication()
  {
    _fmToPtf = _memHandle.create<std::map<const FareMarket*, const PaxTypeFare*> >();
    RexPricingTrx* trx = _memHandle.create<RexPricingTrx>();
    ExcItin* excItin = _memHandle.create<ExcItin>();
    Itin* itin = _memHandle.create<Itin>();
    FareMarket* fm1 = _memHandle.create<FareMarket>();
    TravelSeg* ts1 = _memHandle.create<AirSeg>();

    trx->exchangeItin().push_back(excItin);
    fm1->travelSeg().push_back(ts1);
    itin->fareMarket().push_back(fm1);
    itin->travelSeg().push_back(ts1);
    trx->newItin().push_back(itin);

    return trx;
  }
  void c31cloneAssert(const RexPricingTrx& trx,
                      std::vector<FareMarket*>::size_type count,
                      const std::vector<FareMarket::FareRetrievalFlags>& frfReverseVec)
  {
    CPPUNIT_ASSERT_EQUAL(count, trx.newItin().front()->fareMarket().size());

    for (const FareMarket::FareRetrievalFlags& frf : frfReverseVec)
      CPPUNIT_ASSERT_EQUAL(frf, trx.newItin().front()->fareMarket()[--count]->retrievalFlag());
  }
  void testCloneNewFMsWithFareApplication0()
  {
    RexPricingTrx* trx = setUpCloneNewFMsWithFareApplication();
    _collector->cloneNewFMsWithFareApplication(*trx, *trx->curNewItin());
    c31cloneAssert(*trx, 0, std::vector<FareMarket::FareRetrievalFlags>());
  }
  void testCloneNewFMsWithFareApplicationH()
  {
    RexPricingTrx* trx = setUpCloneNewFMsWithFareApplication();
    trx->fareRetrievalFlags().set(FareMarket::RetrievHistorical);
    _collector->cloneNewFMsWithFareApplication(*trx, *trx->curNewItin());
    c31cloneAssert(*trx, 1, list_of(FareMarket::RetrievHistorical));
  }
  void testCloneNewFMsWithFareApplicationL()
  {
    RexPricingTrx* trx = setUpCloneNewFMsWithFareApplication();
    trx->fareRetrievalFlags().set(FareMarket::RetrievLastReissue);
    _collector->cloneNewFMsWithFareApplication(*trx, *trx->curNewItin());
    c31cloneAssert(*trx, 1, list_of(FareMarket::RetrievLastReissue));
  }
  void testCloneNewFMsWithFareApplicationC()
  {
    RexPricingTrx* trx = setUpCloneNewFMsWithFareApplication();
    trx->fareRetrievalFlags().set(FareMarket::RetrievCurrent);
    _collector->cloneNewFMsWithFareApplication(*trx, *trx->curNewItin());
    c31cloneAssert(*trx, 1, list_of(FareMarket::RetrievCurrent));
  }
  void testCloneNewFMsWithFareApplicationHLC()
  {
    RexPricingTrx* trx = setUpCloneNewFMsWithFareApplication();
    trx->fareRetrievalFlags().set(FareMarket::RetrievHistorical);
    trx->fareRetrievalFlags().set(FareMarket::RetrievLastReissue);
    trx->fareRetrievalFlags().set(FareMarket::RetrievCurrent);
    _collector->cloneNewFMsWithFareApplication(*trx, *trx->curNewItin());
    c31cloneAssert(*trx,
                   3,
                   list_of(FareMarket::RetrievCurrent)(FareMarket::RetrievLastReissue)(
                       FareMarket::RetrievHistorical));
  }

  void testCollectValidNegFares()
  {
    using boost::assign::operator+=;

    FakeBuckets buckets1(_memHandle), buckets2(_memHandle);
    std::vector<FakeBuckets*> buckets;
    buckets += &buckets1, &buckets2;
    NegFareValidatorStub validator;
    std::set<PaxTypeFare*> result;
    std::set<PaxTypeFare*> fares1, fares2;

    for (size_t i = 0; i < 4; ++i)
      fares1.insert(_memHandle.create<PaxTypeFare>());
    for (size_t i = 0; i < 2; ++i)
      fares2.insert(_memHandle.create<PaxTypeFare>());

    EXPECT_CALL(*buckets1._faresGetter, getFares()).WillOnce(Return(ByRef(fares1)));
    EXPECT_CALL(*buckets2._faresGetter, getFares()).WillOnce(Return(ByRef(fares2)));

    _collector->collectValidNegFares(buckets, validator, result);

    ASSERT_EQ(fares1.size() + fares2.size(), result.size());
    ASSERT_TRUE(std::includes(result.begin(), result.end(), fares1.begin(), fares1.end()));
    ASSERT_TRUE(std::includes(result.begin(), result.end(), fares2.begin(), fares2.end()));
  }

  void testMergeValidatingCarriers()
  {
    std::vector<CarrierCode> vcs1, vcs2;

    vcs2.push_back("AA");
    vcs2.push_back("DD");

    FareCollectorOrchestrator::mergeValidatingCarriers(vcs1, vcs2);

    CPPUNIT_ASSERT(vcs1.size() == 2);
    CPPUNIT_ASSERT(vcs1[0] == "AA");
    CPPUNIT_ASSERT(vcs1[1] == "DD");

    FareCollectorOrchestrator::mergeValidatingCarriers(vcs1, vcs2);
    CPPUNIT_ASSERT(vcs1.size() == 2);
    CPPUNIT_ASSERT(vcs1[0] == "AA");
    CPPUNIT_ASSERT(vcs1[1] == "DD");

    vcs1.clear();
    vcs1.push_back("AA");
    vcs1.push_back("BB");

    FareCollectorOrchestrator::mergeValidatingCarriers(vcs1, vcs2);
    CPPUNIT_ASSERT(vcs1.size() == 3);
    CPPUNIT_ASSERT(vcs1[0] == "AA");
    CPPUNIT_ASSERT(vcs1[1] == "BB");
    CPPUNIT_ASSERT(vcs1[2] == "DD");

    vcs1.clear();
    vcs1.push_back("AA");
    vcs1.push_back("BB");

    vcs2.clear();
    FareCollectorOrchestrator::mergeValidatingCarriers(vcs1, vcs2);
    CPPUNIT_ASSERT(vcs1.size() == 2);
    CPPUNIT_ASSERT(vcs1[0] == "AA");
    CPPUNIT_ASSERT(vcs1[1] == "BB");
  }

  void testRestoreOrigValidatingCarriers()
  {
    std::map<std::string, std::vector<FareMarket*> > fmMap;
    FareMarket fm1, fm2, fm3, fm4;

    fmMap["AUSDFW"].push_back(&fm1);
    fmMap["AUSDFW"].push_back(&fm2);
    fmMap["AUSDFW"].push_back(&fm3);
    fmMap["AUSDFW"].push_back(&fm4);

    fm1.validatingCarriers().push_back("V1");
    fm1.validatingCarriers().push_back("V2");
    fm1.validatingCarriers().push_back("V3");
    fm2.validatingCarriers().push_back("V2");
    fm3.validatingCarriers().push_back("V3");
    fm4.validatingCarriers().push_back("V4");

    std::map<FareMarket*, std::vector<CarrierCode> > fmOrigVCs;
    fmOrigVCs[&fm1].push_back("V1");

    _collector->restoreOrigValidatingCarriers(fmMap, &fmOrigVCs);

    CPPUNIT_ASSERT(fm1.validatingCarriers().size() == 1);
    CPPUNIT_ASSERT(fm1.validatingCarriers()[0] == "V1");

    CPPUNIT_ASSERT(fm2.validatingCarriers().size() == 1);
    CPPUNIT_ASSERT(fm2.validatingCarriers()[0] == "V2");

    CPPUNIT_ASSERT(fm3.validatingCarriers().size() == 1);
    CPPUNIT_ASSERT(fm3.validatingCarriers()[0] == "V3");

    CPPUNIT_ASSERT(fm4.validatingCarriers().size() == 1);
    CPPUNIT_ASSERT(fm4.validatingCarriers()[0] == "V4");
  }

  void testRestorePtfVCs()
  {
    FareMarket fm;
    PaxTypeFare ptf1, ptf2, ptf3, ptf4;

    ptf1.validatingCarriers().push_back("V11");
    ptf1.validatingCarriers().push_back("V12");
    ptf1.validatingCarriers().push_back("V13");

    ptf2.validatingCarriers().push_back("V21");
    ptf2.validatingCarriers().push_back("V22");
    ptf2.validatingCarriers().push_back("V23");

    ptf3.validatingCarriers().push_back("V31");
    ptf3.validatingCarriers().push_back("V32");
    ptf3.validatingCarriers().push_back("V33");

    ptf4.validatingCarriers().push_back("V44");

    fm.allPaxTypeFare().push_back(&ptf1);
    fm.allPaxTypeFare().push_back(&ptf2);
    fm.allPaxTypeFare().push_back(&ptf3);
    fm.allPaxTypeFare().push_back(&ptf4);

    std::vector<CarrierCode> mergedVCs, origVCs;
    mergedVCs.push_back("V11");
    mergedVCs.push_back("V12");
    mergedVCs.push_back("V13");
    mergedVCs.push_back("V21");
    mergedVCs.push_back("V22");
    mergedVCs.push_back("V23");
    mergedVCs.push_back("V31");
    mergedVCs.push_back("V32");
    mergedVCs.push_back("V33");
    mergedVCs.push_back("V44");

    origVCs.push_back("V11");
    origVCs.push_back("V12");
    origVCs.push_back("V13");
    origVCs.push_back("V21");
    origVCs.push_back("V22");
    origVCs.push_back("V31");

    _collector->restorePtfVCs(&fm, mergedVCs, origVCs);

    CPPUNIT_ASSERT(ptf1.validatingCarriers().size() == 3);
    CPPUNIT_ASSERT(ptf1.validatingCarriers()[0] == "V11");
    CPPUNIT_ASSERT(ptf1.validatingCarriers()[1] == "V12");
    CPPUNIT_ASSERT(ptf1.validatingCarriers()[2] == "V13");

    CPPUNIT_ASSERT(ptf2.validatingCarriers().size() == 2);
    CPPUNIT_ASSERT(ptf2.validatingCarriers()[0] == "V21");
    CPPUNIT_ASSERT(ptf2.validatingCarriers()[1] == "V22");

    CPPUNIT_ASSERT(ptf3.validatingCarriers().size() == 1);
    CPPUNIT_ASSERT(ptf3.validatingCarriers()[0] == "V31");

    CPPUNIT_ASSERT(ptf4.validatingCarriers().empty());

    mergedVCs.clear();
    origVCs.clear();

    mergedVCs.push_back("V11");
    origVCs.push_back("V11");

    _collector->restorePtfVCs(&fm, mergedVCs, origVCs);
    // Nothing should change when mergedVCs == origVCs

    CPPUNIT_ASSERT(ptf1.validatingCarriers().size() == 3);
    CPPUNIT_ASSERT(ptf1.validatingCarriers()[0] == "V11");
    CPPUNIT_ASSERT(ptf1.validatingCarriers()[1] == "V12");
    CPPUNIT_ASSERT(ptf1.validatingCarriers()[2] == "V13");

    CPPUNIT_ASSERT(ptf2.validatingCarriers().size() == 2);
    CPPUNIT_ASSERT(ptf2.validatingCarriers()[0] == "V21");
    CPPUNIT_ASSERT(ptf2.validatingCarriers()[1] == "V22");

    CPPUNIT_ASSERT(ptf3.validatingCarriers().size() == 1);
    CPPUNIT_ASSERT(ptf3.validatingCarriers()[0] == "V31");

    CPPUNIT_ASSERT(ptf4.validatingCarriers().empty());
  }

  PaxTypeFare* createPaxTypeFare(const Indicator& owrt)
  {
    FareInfo* fi = _memHandle.create<FareInfo>();
    fi->owrt() = owrt;
    Fare* fare = _memHandle.create<Fare>();
    fare->setFareInfo(fi);
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    ptf->setFare(fare);
    ptf->setIsShoppingFare();
    return ptf;
  }
  void setupFareMarket(FareMarket& fm,
                       std::vector<std::vector<std::string> >& rfbTSVec)
  {
    for (std::vector<std::string> rfbRFVec : rfbTSVec)
    {
      std::vector<TravelSeg::RequestedFareBasis> reqFareBasisVec;
      TravelSeg* ts = _memHandle.create<AirSeg>();
      for (std::string rfbRFB : rfbRFVec)
      {
        if (!rfbRFB.empty())
        {
          TravelSeg::RequestedFareBasis reqFareBasis;
          reqFareBasis.fareBasisCode = FareBasisCode(rfbRFB);
          reqFareBasis.amount = 0.0;
          reqFareBasisVec.push_back(reqFareBasis);
        }
      }
      ts->requestedFareBasis() = reqFareBasisVec;
      fm.travelSeg().push_back(ts);
    }
  }

  void setupFareMarket(FareMarket& fm,
                       std::vector<std::vector<TravelSeg::RequestedFareBasis> >& rfbTSVec)
  {
    for (std::vector<TravelSeg::RequestedFareBasis> rfbRFVec : rfbTSVec)
    {
      TravelSeg* ts = _memHandle.create<AirSeg>();
      for (TravelSeg::RequestedFareBasis& rfbRFB : rfbRFVec)
        ts->requestedFareBasis().push_back(rfbRFB);

      fm.travelSeg().push_back(ts);
    }
  }

  void testVerifySpecificFareBasis()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareCalcConfig fCalcConfig;
    trx->fareCalcConfig() = &fCalcConfig;

    FareMarket fm;

    std::vector<std::string> fareClasses {"FBC1", "FBC2", "FBC3", "FBC4"};
    for (std::string fc : fareClasses)
    {
      PaxTypeFare* ptf =  _memHandle.insert(new MockPaxTypeFare("ATP", "LO", 12, "RN12", fc.c_str(),
                                                                100.0, "PLN", 'O', FROM, "ADT"));
      fm.allPaxTypeFare().push_back(ptf);
    }

    // Each line is a TravelSeg with a list of RequestedFareBasis.
    // Empty string indicates that the TravelSeg has an empty list of RequestedFareBasis
    std::vector<std::vector<std::string> > rfbCodes
    {
      {""},
      {"FBC1", "FBC2", "FBC3", "FBC4"},
      {"FBC4", "FBC1", "FBC2", "FBC3"},
      {"FBC3", "FBC4", "FBC1", "FBC2"},
      {"FBC2", "FBC3", "FBC4", "FBC1"},
      {"FBC1", "FBC2", "FBC3"},
      {"FBC3", "FBC1", "FBC2"},
      {"FBC2", "FBC3", "FBC1"},
      {"FBC1", "FBC2"},
      {"FBC2", "FBC1"},
      {"FBC1"},
      {""}
    };
    setupFareMarket(fm, rfbCodes);

    _collector->verifySpecificFareBasis(*trx, fm);

    int validFares = 0;
    for (PaxTypeFare* ptf : fm.allPaxTypeFare())
    {
      if (!ptf->isRequestedFareBasisInvalid())
        ++validFares;
    }

    CPPUNIT_ASSERT_EQUAL(1, validFares);
  }

  void testVerifySpecificFareBasisYY()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareCalcConfig fCalcConfig;
    trx->fareCalcConfig() = &fCalcConfig;

    FareMarket fm;

    std::vector<std::string> fareClasses {"FBC1", "FBC2", "FBC3", "FBC4"};
    for (std::string fc : fareClasses)
    {
      PaxTypeFare* ptf =  _memHandle.insert(new MockPaxTypeFare("ATP", "YY", 12, "RN12", fc.c_str(),
                                                                100.0, "PLN", 'O', FROM, "ADT"));
      fm.allPaxTypeFare().push_back(ptf);
    }

    // Each line is a TravelSeg with a list of RequestedFareBasis.
    // Empty string indicates that the TravelSeg has an empty list of RequestedFareBasis
    std::vector<std::vector<std::string> > rfbCodes
    {
      {""},
      {"YBC1", "YBC2", "YBC3", "YBC4"},
      {"YBC4", "YBC1", "YBC2", "YBC3"},
      {"YBC3", "YBC4", "YBC1", "YBC2"},
      {"YBC2", "YBC3", "YBC4", "YBC1"},
      {"YBC1", "YBC2", "YBC3"},
      {"YBC3", "YBC1", "YBC2"},
      {"YBC2", "YBC3", "YBC1"},
      {"YBC1", "YBC2"},
      {"YBC2", "YBC1"},
      {"YBC1"},
      {""}
    };
    setupFareMarket(fm, rfbCodes);

    _collector->verifySpecificFareBasis(*trx, fm);

    int validFares = 0;
    for (PaxTypeFare* ptf : fm.allPaxTypeFare())
    {
      if (!ptf->isRequestedFareBasisInvalid())
        ++validFares;
    }

    CPPUNIT_ASSERT_EQUAL(1, validFares);
    CPPUNIT_ASSERT_EQUAL('Y', fm.allPaxTypeFare().front()->getAllowedChangeFareBasisBkgCode());
  }

  void testVerifySpecificFareBasisYY_FailBookingCode()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareCalcConfig fCalcConfig;
    trx->fareCalcConfig() = &fCalcConfig;

    FareMarket fm;

    std::vector<std::string> fareClasses {"FBC1", "FBC2", "FBC3", "FBC4"};
    for (std::string fc : fareClasses)
    {
      PaxTypeFare* ptf =  _memHandle.insert(new MockPaxTypeFare("ATP", "YY", 12, "RN12", fc.c_str(),
                                                                100.0, "PLN", 'O', FROM, "ADT"));
      fm.allPaxTypeFare().push_back(ptf);
    }

    // Each line is a TravelSeg with a list of RequestedFareBasis.
    // Empty string indicates that the TravelSeg has an empty list of RequestedFareBasis
    std::vector<std::vector<std::string> > rfbCodes
    {
      {""},
      {"YBC1", "YBC2", "YBC3", "YBC4"},
      {"FBC4", "FBC1", "FBC2", "FBC3"},
      {"FBC3", "FBC4", "FBC1", "FBC2"},
      {"FBC2", "FBC3", "FBC4", "FBC1"},
      {"FBC1", "FBC2", "FBC3"},
      {"FBC3", "FBC1", "FBC2"},
      {"FBC2", "FBC3", "FBC1"},
      {"FBC1", "FBC2"},
      {"FBC2", "FBC1"},
      {"FBC3"},
      {""}
    };
    setupFareMarket(fm, rfbCodes);

    _collector->verifySpecificFareBasis(*trx, fm);

    int validFares = 0;
    for (PaxTypeFare* ptf : fm.allPaxTypeFare())
    {
      if (!ptf->isRequestedFareBasisInvalid())
        ++validFares;
    }

    CPPUNIT_ASSERT_EQUAL(0, validFares);
  }

  void testVerifySpecificFareBasisAllInvalid()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareCalcConfig fCalcConfig;
    trx->fareCalcConfig() = &fCalcConfig;

    FareMarket fm;

    std::vector<std::string> fareClasses {"FBC1", "FBC2", "FBC3", "FBC4"};
    for (std::string fc : fareClasses)
    {
      PaxTypeFare* ptf =  _memHandle.insert(new MockPaxTypeFare("ATP", "LO", 12, "RN12", fc.c_str(),
                                                                100.0, "PLN", 'O', FROM, "ADT"));
      fm.allPaxTypeFare().push_back(ptf);
    }

    std::vector<std::vector<std::string> > rfbCodes
    {
      {""},
      {"FBC1", "FBC2", "FBC3", "FBC4"},
      {"FBC4", "FBC1", "FBC2", "FBC3"},
      {"FBC3", "FBC4", "FBC1", "FBC2"},
      {"FBC2", "FBC3", "FBC4", "FBC1"},
      {"FBC1", "FBC2", "FBC3"},
      {"FBC3", "FBC1", "FBC2"},
      {"FBC2", "FBC3", "FBC1"},
      {"FBC1", "FBC2"},
      {"FBC2", "FBC1"},
      {"FBC3"},
      {""}
    };
    setupFareMarket(fm, rfbCodes);

    _collector->verifySpecificFareBasis(*trx, fm);

    int validFares = 0;
    for (PaxTypeFare* ptf : fm.allPaxTypeFare())
    {
      if (!ptf->isRequestedFareBasisInvalid())
        ++validFares;
    }

    CPPUNIT_ASSERT_EQUAL(0, validFares);
  }

  void testVerifySpecificFareBasisNoRFB()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();

    FareMarket fm;

    std::vector<std::string> fareClasses {"FBC1", "FBC2", "FBC3", "FBC4"};
    for (std::string fc : fareClasses)
    {
      PaxTypeFare* ptf =  _memHandle.insert(new MockPaxTypeFare("ATP", "LO", 12, "RN12", fc.c_str(),
                                                                100.0, "PLN", 'O', FROM, "ADT"));
      fm.allPaxTypeFare().push_back(ptf);
    }

    std::vector<std::vector<std::string> > rfbCodes
    {
      {""},
      {""},
      {""},
      {""},
      {""},
      {""},
      {""},
      {""},
      {""},
      {""},
      {""},
      {""}
    };
    setupFareMarket(fm, rfbCodes);

    _collector->verifySpecificFareBasis(*trx, fm);

    int validFares = 0;
    for (PaxTypeFare* ptf : fm.allPaxTypeFare())
    {
      if (!ptf->isRequestedFareBasisInvalid())
        ++validFares;
    }

    CPPUNIT_ASSERT_EQUAL(4, validFares);

  }

  void testSpecificFareBasisAllArgumentsPositive()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareCalcConfig fCalcConfig;
    trx->fareCalcConfig() = &fCalcConfig;

    PaxTypeFare* ptf = _memHandle.insert(
                         new MockPaxTypeFare("VEND", "CAR", 12, "RULE", "FARECLASSCODE", 999.99,
                                             "PLN", 'O', FROM, "ADT"));
    FareMarket fm;
    fm.allPaxTypeFare().push_back(ptf);

    std::vector<std::vector<TravelSeg::RequestedFareBasis> > rfbVec
    {
      { // TravelSeg #1
        {"",              "",    "",     "",    "",   "",       0.00},
        {"FARECLASSCODE", "ADT", "VEND", "CAR", "12", "RULE", 999.99},
        {"",              "",    "",     "",    "",   "",       0.00}
      },
      { // TravelSeg #2
        {"",              "",    "",     "",    "",   "",       0.00},
        {"-------------", "ADT", "VEND", "CAR", "12", "RULE", 999.99},
        {"FARECLASSCODE", "---", "VEND", "CAR", "12", "RULE", 999.99},
        {"FARECLASSCODE", "ADT", "----", "CAR", "12", "RULE", 999.99},
        {"FARECLASSCODE", "ADT", "VEND", "---", "12", "RULE", 999.99},
        {"FARECLASSCODE", "ADT", "VEND", "CAR", "99", "RULE", 999.99},
        {"FARECLASSCODE", "ADT", "VEND", "CAR", "12", "----", 999.99},
        {"FARECLASSCODE", "ADT", "VEND", "CAR", "12", "RULE", 999.98},
        {"FARECLASSCODE", "ADT", "VEND", "CAR", "12", "RULE",1000.00},
        {"",              "",    "",     "",    "",   "",       0.00},
        {"FARECLASSCODE", "ADT", "VEND", "CAR", "12", "RULE", 999.99},
        {"",              "",    "",     "",    "",   "",       0.00}
      }
    };
    setupFareMarket(fm, rfbVec);

    _collector->verifySpecificFareBasis(*trx, fm);

    CPPUNIT_ASSERT_EQUAL(false, ptf->isRequestedFareBasisInvalid());
  }

  void testSpecificFareBasisAllArgumentsNegative()
  {
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    FareCalcConfig fCalcConfig;
    trx->fareCalcConfig() = &fCalcConfig;

    PaxTypeFare* ptf = _memHandle.insert(
                         new MockPaxTypeFare("VEND", "CAR", 12, "RULE", "FARECLASSCODE", 999.99,
                                             "PLN", 'O', FROM, "ADT"));
    FareMarket fm;
    fm.allPaxTypeFare().push_back(ptf);

    std::vector<std::vector<TravelSeg::RequestedFareBasis> > rfbVec
    {
      { // TravelSeg #1
        {"",              "",    "",     "",    "",   "",       0.00},
        {"FARECLASSCODE", "ADT", "VEND", "CAR", "12", "RULE", 999.99},
        {"",              "",    "",     "",    "",   "",       0.00}
      },
      { // TravelSeg #2
        {"",              "",    "",     "",    "",   "",       0.00},
        {"-------------", "ADT", "VEND", "CAR", "12", "RULE", 999.99},
        {"FARECLASSCODE", "---", "VEND", "CAR", "12", "RULE", 999.99},
        {"FARECLASSCODE", "ADT", "----", "CAR", "12", "RULE", 999.99},
        {"FARECLASSCODE", "ADT", "VEND", "---", "12", "RULE", 999.99},
        {"FARECLASSCODE", "ADT", "VEND", "CAR", "99", "RULE", 999.99},
        {"FARECLASSCODE", "ADT", "VEND", "CAR", "12", "----", 999.99},
        {"FARECLASSCODE", "ADT", "VEND", "CAR", "12", "RULE", 999.98},
        {"FARECLASSCODE", "ADT", "VEND", "CAR", "12", "RULE",1000.00},
        {"",              "",    "",     "",    "",   "",       0.00}
      }
    };
    setupFareMarket(fm, rfbVec);

    _collector->verifySpecificFareBasis(*trx, fm);

    CPPUNIT_ASSERT_EQUAL(true, ptf->isRequestedFareBasisInvalid());
  }

  void testProcessFRRAdjustedSellingLevel()
  {
    FareDisplayTrx fdTrx;
    FareDisplayRequest request;
    fdTrx.setRequest(&request);
    FareDisplayOptions options;
    fdTrx.setOptions(&options);
    Agent agent;
    request.ticketingAgent() = &agent;

    request.requestType() = FARE_DISPLAY_REQUEST;
    CPPUNIT_ASSERT(!_collector->processFRRAdjustedSellingLevel(fdTrx));

    request.requestType() = ENHANCED_RD_REQUEST;
    options.setXRSForFRRule(false);
    CPPUNIT_ASSERT(_collector->processFRRAdjustedSellingLevel(fdTrx));

    options.setXRSForFRRule(true);
    CPPUNIT_ASSERT(!_collector->processFRRAdjustedSellingLevel(fdTrx));
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(FareCollectorOrchestratorTest);
}
