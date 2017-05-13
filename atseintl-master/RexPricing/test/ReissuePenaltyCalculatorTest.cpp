#include "Common/Money.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "Diagnostic/DiagManager.h"
#include "RexPricing/FarePathChangeDetermination.h"
#include "RexPricing/ReissuePenaltyCalculator.h"
#include "Rules/OriginallyScheduledFlightValidator.h"
#include "Rules/RuleConst.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <boost/assign/std/set.hpp>

namespace tse
{
using namespace boost::assign;

namespace
{
class NoShowDataHandle : public DataHandleMock
{
private:
  std::vector<ReissueSequence*> reissueSequences;

public:
  NoShowDataHandle(std::vector<ReissueSequence*> reissueSequences)
    : DataHandleMock(), reissueSequences(reissueSequences)
  {
  }

  const std::vector<ReissueSequence*>& getReissue(const VendorCode&,
                                                  int,
                                                  const DateTime&,
                                                  const DateTime&)
  {
    return reissueSequences;
  }
};
} // namespace

class ReissuePenaltyCalculatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ReissuePenaltyCalculatorTest);

  CPPUNIT_TEST(testDetermineApplicationScenarioOldJ1);
  CPPUNIT_TEST(testDetermineApplicationScenarioOldJ2);
  CPPUNIT_TEST(testDetermineApplicationScenarioOldP1);
  CPPUNIT_TEST(testDetermineApplicationScenarioOldP2);
  CPPUNIT_TEST(testDetermineApplicationScenarioOldBlank1);
  CPPUNIT_TEST(testDetermineApplicationScenarioOldBlank2);
  CPPUNIT_TEST(testDetermineApplicationScenarioOldBlank3);
  CPPUNIT_TEST(testDetermineApplicationScenarioOldBlankBlank);
  CPPUNIT_TEST(testDetermineApplicationScenarioNew);

  CPPUNIT_TEST(testDetermineApplicationScenarioMixed2);
  CPPUNIT_TEST(testDetermineApplicationScenarioMixed1);
  CPPUNIT_TEST(testDetermineApplicationScenarioMixedCxr3);
  CPPUNIT_TEST(testDetermineApplicationScenarioMixedCxr2);

  CPPUNIT_TEST(testValidatingCarrierOwnFareComponentTrue);
  CPPUNIT_TEST(testValidatingCarrierOwnFareComponentFalse);

  CPPUNIT_TEST(testChildrenDiscount0ws);
  CPPUNIT_TEST(testChildrenDiscount0wos);
  CPPUNIT_TEST(testChildrenDiscount1);
  CPPUNIT_TEST(testChildrenDiscount7);
  CPPUNIT_TEST(testChildrenDiscount8);
  CPPUNIT_TEST(testChildrenDiscount9);
  CPPUNIT_TEST(testCalculateDiscount59);
  CPPUNIT_TEST(testCalculateDiscountNoDiscount);

  CPPUNIT_TEST(testUsePenaltyAmt1InDiffCurrencyWhenOnlyPenaltyAmt1Exists);
  CPPUNIT_TEST(testNotUsePenaltyAmt2WhenOnlyPenaltyAmt2Exists);
  CPPUNIT_TEST(testUsePenaltyAmt2InSameCurrencyWhenPenaltyAmt1InDiffCurrency);
  CPPUNIT_TEST(testUsePenaltyPercentWhenNoPenaltyAmtExists);
  CPPUNIT_TEST(testUseHigherBetweenPenaltyPercentAndPenaltyAmt2);
  CPPUNIT_TEST(testAccumulateCharges);

  CPPUNIT_TEST(testChangeFeeAdjustedByMinimumAmount);

  CPPUNIT_TEST(testCalculateDiscount_Infant_CAT19_Value1);
  CPPUNIT_TEST(testCalculateDiscount_Infant_CAT22_Value1);
  CPPUNIT_TEST(testCalculateDiscount_Infant_INF_CAT19_Value1);
  CPPUNIT_TEST(testCalculateDiscount_Infant_CAT19_Value2346);
  CPPUNIT_TEST(testCalculateDiscount_Child_CAT19_Value2);
  CPPUNIT_TEST(testCalculateDiscount_Child_CAT22_Value2);
  CPPUNIT_TEST(testCalculateDiscount_Child_CAT19_Value1346);
  CPPUNIT_TEST(testCalculateDiscount_Youth_CAT22_Value3);
  CPPUNIT_TEST(testCalculateDiscount_Youth_CAT19_Value3);
  CPPUNIT_TEST(testCalculateDiscount_Youth_CAT22_Value1245);
  CPPUNIT_TEST(testCalculateDiscount_Senior_CAT22_Value4);
  CPPUNIT_TEST(testCalculateDiscount_Senior_CAT19_Value4);
  CPPUNIT_TEST(testCalculateDiscount_Senior_CAT22_Value1235);
  CPPUNIT_TEST(testCalculateDiscount_Senior_CAT22_Value6);
  CPPUNIT_TEST(testCalculateDiscount_Youth_CAT22_Value6);
  CPPUNIT_TEST(testCalculateDiscount_Senior_CAT19_Value6);
  CPPUNIT_TEST(testCalculateDiscount_Youth_CAT19_Value5);
  CPPUNIT_TEST(testCalculateDiscount_Senior_CAT19_Value5);
  CPPUNIT_TEST(testCalculateDiscount_Youth_CAT22_Value5);
  CPPUNIT_TEST(testSMPnonChangeable_no_before_records);
  CPPUNIT_TEST(testSMPnonChangeable_no_after_records);
  CPPUNIT_TEST(testSMPchangeable_no_departure_specified);
  CPPUNIT_TEST(testSMPchangeable_after_departure_specified);

  CPPUNIT_TEST(testSkipNoShowRecordsAnytimeAfter);
  CPPUNIT_TEST(testSkipNoShowRecordsDayAfter);
  CPPUNIT_TEST(testPassTwoReissueSequences);
  CPPUNIT_TEST(testPassValidRecord);

  CPPUNIT_TEST_SUITE_END();

  enum DiscountPaxType
  { NONE_DTYPE,
    INFANT_DTYPE,
    CHILD_DTYPE };

  static const PaxTypeCode INFANT_CODE;
  static const Indicator BEFORE_DEP;
  static const Indicator AFTER_DEP;
  static const Indicator NO_DEP_RES;

  typedef ReissuePenaltyCalculator::FcFees FcFees;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle(new RexPricingTrx);
    _request = _memHandle(new RexPricingRequest);
    _request->newValidatingCarrier() = "UA";
    _trx->setRequest(_request);
    _calculationCurrency = _memHandle(new CurrencyCode);
    *_calculationCurrency = "USD";
    _options = _memHandle(new RexPricingOptions);
    _options->currencyOverride() = *_calculationCurrency;
    _trx->setOptions(_options);
    _permutation = _memHandle(new ProcessTagPermutation);
    _pti = _memHandle(new PaxTypeInfo);
    _calculator = _memHandle(new ReissuePenaltyCalculator());
    RexPricingRequest* request = static_cast<RexPricingRequest*>(_trx->getRequest());

    _calculator->initialize(*_trx,
                            *_calculationCurrency,
                            *_memHandle(new FarePathChangeDeterminationMock),
                            request->newValidatingCarrier(),
                            *_pti,
                            _permutation,
                            nullptr);

    _paxCode = _memHandle(new PaxTypeCode);
    _di = _memHandle(new DiscountInfo);
    _di->discPercent() = 40.0;

    fareInfo.fareAmount() = 100.00;
    fareInfo.currency() = "USD";
    fare.setFareInfo(&fareInfo);
    paxTypeFare.setFare(&fare);
    paxTypeFare.fareMarket() = &fm;
    travelSeg.changeStatus() = TravelSeg::UNCHANGED;
    fm.travelSeg().push_back(&travelSeg);
    fm.governingCarrier() = "AA";

    fareInfo2.fareAmount() = 200.00;
    fareInfo2.currency() = "USD";
    fare2.setFareInfo(&fareInfo2);
    paxTypeFare2.setFare(&fare2);
    paxTypeFare2.fareMarket() = &fm2;
    travelSeg2.changeStatus() = TravelSeg::CHANGED;
    fm2.travelSeg().push_back(&travelSeg2);
    fm2.governingCarrier() = "BA";

    fu.paxTypeFare() = &paxTypeFare;
    fu2.paxTypeFare() = &paxTypeFare2;

    pu.fareUsage().push_back(&fu);
    pu.fareUsage().push_back(&fu2);

    fp.pricingUnit().push_back(&pu);

    paxTypeFare.paxType() = &paxType;
    permutation.reissueCharges() = _calculator->_reissueCharges;
    _emptyDiag = _memHandle.create<DiagManager>(*_trx);
  }

  ReissuePenaltyCalculator* setUpDiscountCalculator()
  {
    trx = _memHandle(new RexPricingTrx);
    trx->setOptions(_options);
    trx->setRequest(_request);
    static_cast<RexPricingTrx*>(trx)->exchangePaxType() = &exchangePaxType;
    exchangePaxType.paxTypeInfo() = &pti;
    tag.record3()->orig() = &voluntaryChangeFD;

    RexPricingRequest* request = static_cast<RexPricingRequest*>(trx->getRequest());

    ReissuePenaltyCalculator* discountCalc = _memHandle.insert(new ReissuePenaltyCalculator());
    discountCalc->initialize(*trx,
                             *_calculationCurrency,
                             *_memHandle(new FarePathChangeDetermination),
                             request->newValidatingCarrier(),
                             pti,
                             &permutation,
                             nullptr);

    permutation.reissueCharges() = discountCalc->_reissueCharges;

    return discountCalc;
  }

  PaxTypeFare* getFareWithDiscount(int category,  MoneyAmount discPercent, PaxTypeCode paxType = "")
  {
    DataHandle* dh = _memHandle(new DataHandle);
    PaxTypeFare* ptf = _memHandle(new PaxTypeFare);

    PaxTypeFareRuleData* discountRuleData =
                           createDiscount(category, discPercent, paxType);
    discountRuleData->baseFare() = ptf;
    ptf->setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, *dh, discountRuleData, true);
    ptf->status().set(PaxTypeFare::PTF_Discounted);
    return ptf;
  }

  PaxTypeFareRuleData* createDiscount(int category, MoneyAmount discPercent, PaxTypeCode paxType = "")
  {
    DiscountInfo* discount = _memHandle(new DiscountInfo);

    discount->category() = category;
    discount->discPercent() = discPercent;
    discount->paxType() = paxType;

    PaxTypeFareRuleData* ptfRuleData = _memHandle(new PaxTypeFareRuleData);
    ptfRuleData->ruleItemInfo() = discount;

    return ptfRuleData;
  }

  void tearDown() { _memHandle.clear(); }

  void rec3ForApplScenario(Indicator journeyInd, Indicator feeAppl, CarrierCode cxr = "UA")
  {
    ProcessTagInfo* pti = _memHandle(new ProcessTagInfo);
    _permutation->processTags().push_back(pti);

    VoluntaryChangesInfo* rec3 = _memHandle(new VoluntaryChangesInfo);
    PaxTypeFare* ptf = _memHandle(new PaxTypeFare);
    FareInfo* fi = _memHandle(new FareInfo);
    fi->carrier() = cxr;
    Fare* fare = new Fare;
    fare->setFareInfo(fi);
    ptf->setFare(fare);
    pti->record3()->orig() = rec3;
    pti->paxTypeFare() = ptf;
    rec3->journeyInd() = journeyInd;
    rec3->feeAppl() = feeAppl;
  }

  VoluntaryChangesInfoW* createR3(Indicator departureInd, const Money& changeFee)
  {
    VoluntaryChangesInfoW* r3w = _memHandle(new VoluntaryChangesInfoW);
    VoluntaryChangesInfo* r3 = _memHandle(new VoluntaryChangesInfo);
    r3->departureInd() = departureInd;
    r3->penaltyAmt1() = changeFee.value();
    r3->cur1() = changeFee.code();

    r3w->orig() = r3;

    return r3w;
  }

  void testDetermineApplicationScenarioOldJ1()
  {
    rec3ForApplScenario(ReissuePenaltyCalculator::JOURNEY_APPL,
                        ReissuePenaltyCalculator::HIGHEST_OF_CHANGED_FC);
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::HIGHEST_OF_CHANGED_FC,
                         _calculator->_applScenario);
  }

  void testDetermineApplicationScenarioOldJ2()
  {
    rec3ForApplScenario(ReissuePenaltyCalculator::JOURNEY_APPL,
                        ReissuePenaltyCalculator::HIGHEST_OF_ALL_FC);
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::HIGHEST_OF_ALL_FC, _calculator->_applScenario);
  }

  void testDetermineApplicationScenarioOldP1()
  {
    rec3ForApplScenario(ReissuePenaltyCalculator::PU_APPL,
                        ReissuePenaltyCalculator::HIGHEST_OF_CHANGED_FC);
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::HIGHEST_FROM_CHANGED_PU,
                         _calculator->_applScenario);
  }

  void testDetermineApplicationScenarioOldP2()
  {
    rec3ForApplScenario(ReissuePenaltyCalculator::PU_APPL,
                        ReissuePenaltyCalculator::HIGHEST_OF_ALL_FC);
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::HIGHEST_FROM_CHANGED_PU,
                         _calculator->_applScenario);
  }

  void testDetermineApplicationScenarioOldBlank1()
  {
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_OF_CHANGED_FC);
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::HIGHEST_OF_CHANGED_FC,
                         _calculator->_applScenario);
  }

  void testDetermineApplicationScenarioOldBlank2()
  {
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_OF_ALL_FC);
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::HIGHEST_OF_ALL_FC, _calculator->_applScenario);
  }

  void testDetermineApplicationScenarioOldBlank3()
  {
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::EACH_OF_CHANGED_FC);
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::EACH_OF_CHANGED_FC, _calculator->_applScenario);
  }

  void testDetermineApplicationScenarioOldBlankBlank()
  {
    rec3ForApplScenario(RuleConst::BLANK, RuleConst::BLANK);
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::EACH_OF_CHANGED_FC, _calculator->_applScenario);
  }

  void testDetermineApplicationScenarioNew()
  {
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::EACH_OF_CHANGED_FC);
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::EACH_OF_CHANGED_FC, _calculator->_applScenario);
  }

  void testDetermineApplicationScenarioMixed2()
  {
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_OF_CHANGED_FC, "AA");
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_OF_ALL_FC, "AA");
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::EACH_OF_CHANGED_FC, "AA");
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_FROM_CHANGED_PU, "AA");
    rec3ForApplScenario(
        RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_FROM_CHANGED_PU_ADDS, "AA");
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::HIGHEST_OF_ALL_FC, _calculator->_applScenario);
  }

  void testDetermineApplicationScenarioMixed1()
  {
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::EACH_OF_CHANGED_FC, "AA");
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_OF_CHANGED_FC, "AA");
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::HIGHEST_OF_CHANGED_FC,
                         _calculator->_applScenario);
  }

  void testDetermineApplicationScenarioMixedCxr3()
  {
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_OF_CHANGED_FC, "UA");
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_OF_ALL_FC, "UA");
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::EACH_OF_CHANGED_FC, "UA");
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_FROM_CHANGED_PU, "UA");
    rec3ForApplScenario(
        RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_FROM_CHANGED_PU_ADDS, "UA");
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::EACH_OF_CHANGED_FC, _calculator->_applScenario);
  }

  void testDetermineApplicationScenarioMixedCxr2()
  {
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_OF_CHANGED_FC, "UA");
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_OF_ALL_FC, "UA");
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::EACH_OF_CHANGED_FC, "AA");
    rec3ForApplScenario(RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_FROM_CHANGED_PU, "UA");
    rec3ForApplScenario(
        RuleConst::BLANK, ReissuePenaltyCalculator::HIGHEST_FROM_CHANGED_PU_ADDS, "UA");
    _calculator->determineApplicationScenario();
    CPPUNIT_ASSERT_EQUAL(ReissuePenaltyCalculator::HIGHEST_OF_ALL_FC, _calculator->_applScenario);
  }

  void testValidatingCarrierOwnFareComponentTrue()
  {
    rec3ForApplScenario(RuleConst::BLANK, RuleConst::BLANK, "AA");
    rec3ForApplScenario(RuleConst::BLANK, RuleConst::BLANK, "UA");
    rec3ForApplScenario(RuleConst::BLANK, RuleConst::BLANK, "AA");
    CPPUNIT_ASSERT(_calculator->validatingCarrierOwnFareComponent("UA"));
  }

  void testValidatingCarrierOwnFareComponentFalse()
  {
    rec3ForApplScenario(RuleConst::BLANK, RuleConst::BLANK, "AA");
    rec3ForApplScenario(RuleConst::BLANK, RuleConst::BLANK, "AA");
    rec3ForApplScenario(RuleConst::BLANK, RuleConst::BLANK, "AA");
    CPPUNIT_ASSERT(!_calculator->validatingCarrierOwnFareComponent("UA"));
  }

  VoluntaryChangesInfoW& createRecord3(Indicator tag1 = ' ',
                                       Indicator tag2 = ' ',
                                       Indicator tag3 = ' ',
                                       Indicator tag4 = ' ')
  {
    VoluntaryChangesInfo* info = _memHandle(new VoluntaryChangesInfo);
    info->discountTag1() = tag1;
    info->discountTag2() = tag2;
    info->discountTag3() = tag3;
    info->discountTag4() = tag4;
    VoluntaryChangesInfoW& rec3 = *_memHandle(new VoluntaryChangesInfoW);
    rec3.orig() = info;
    return rec3;
  }

  void testChildrenDiscount0ws()
  {
    _calculator->_infantWithSeat = true;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        60.0, _calculator->getChildrenDiscount('0', *_pti, *_paxCode, *_di), EPSILON);
  }
  void testChildrenDiscount0wos()
  {
    _calculator->_infantWithoutSeat = true;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100.0, _calculator->getChildrenDiscount('0', *_pti, *_paxCode, *_di), EPSILON);
  }
  void testChildrenDiscount1()
  {
    _pti->infantInd() = 'Y';
    _pti->initPsgType();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        60.0, _calculator->getChildrenDiscount('1', *_pti, *_paxCode, *_di), EPSILON);
  }
  void testChildrenDiscount7()
  {
    _calculator->_infantWithSeat = true;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        60.0, _calculator->getChildrenDiscount('7', *_pti, *_paxCode, *_di), EPSILON);
  }
  void testChildrenDiscount8()
  {
    _calculator->_infantWithoutSeat = true;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        60.0, _calculator->getChildrenDiscount('8', *_pti, *_paxCode, *_di), EPSILON);
  }
  void testChildrenDiscount9()
  {
    _calculator->_infantWithoutSeat = true;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        100.0, _calculator->getChildrenDiscount('9', *_pti, *_paxCode, *_di), EPSILON);
  }

  void testCalculateDiscount59()
  {
    ReissuePenaltyCalculator* calculator = setUpDiscountCalculator();
    calculator->_infantWithoutSeat = true;
    setDiscountTagsData('9', ' ', '5', ' ');
    setDiscountPaxTypeData(INFANT_CODE);

    paxTypeFareFD = getFareWithDiscount(RuleConst::CHILDREN_DISCOUNT_RULE, 50, INFANT_CODE);
    tag.paxTypeFare() = paxTypeFareFD;
    double percent = calculator->calculateDiscount(tag);

    CPPUNIT_ASSERT_EQUAL(100.0, percent);
  }

  void testCalculateDiscountNoDiscount()
  {
    ReissuePenaltyCalculator* calculator = setUpDiscountCalculator();
    calculator->_infantWithoutSeat = true;
    setDiscountTagsData(' ', ' ', ' ', ' ');
    setDiscountPaxTypeData(INFANT_CODE);

    paxTypeFareFD = getFareWithDiscount(RuleConst::CHILDREN_DISCOUNT_RULE, 50, INFANT_CODE);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(0.0, calculator->calculateDiscount(tag));
  }

  //--- from old ChargeCalculator ---

  void testUsePenaltyAmt1InDiffCurrencyWhenOnlyPenaltyAmt1Exists()
  {
    VoluntaryChangesInfo voluntaryChanges;
    voluntaryChangesW.orig() = &voluntaryChanges;
    voluntaryChanges.penaltyAmt1() = 100.00;
    voluntaryChanges.cur1() = "USD";
    PenaltyFee penaltyFee;
    _calculator->calculatePenaltyFee(paxTypeFare, voluntaryChangesW, penaltyFee);

    CPPUNIT_ASSERT(penaltyFee.penaltyAmount == 100.00);
    CPPUNIT_ASSERT(penaltyFee.penaltyCurrency == "USD");
  }

  void testNotUsePenaltyAmt2WhenOnlyPenaltyAmt2Exists()
  {
    VoluntaryChangesInfo voluntaryChanges;
    voluntaryChangesW.orig() = &voluntaryChanges;
    voluntaryChanges.penaltyAmt2() = 100.00;
    voluntaryChanges.cur2() = "EUR";
    PenaltyFee penaltyFee;
    _calculator->calculatePenaltyFee(paxTypeFare, voluntaryChangesW, penaltyFee);

    CPPUNIT_ASSERT(penaltyFee.penaltyAmount == 0);
    CPPUNIT_ASSERT(penaltyFee.penaltyCurrency == "");
  }

  void testUsePenaltyAmt2InSameCurrencyWhenPenaltyAmt1InDiffCurrency()
  {
    VoluntaryChangesInfo voluntaryChanges;
    voluntaryChangesW.orig() = &voluntaryChanges;
    voluntaryChanges.penaltyAmt1() = 100.00;
    voluntaryChanges.cur1() = "EUR";
    voluntaryChanges.penaltyAmt2() = 110.00;
    voluntaryChanges.cur2() = "USD";

    PenaltyFee penaltyFee;
    _calculator->calculatePenaltyFee(paxTypeFare, voluntaryChangesW, penaltyFee);

    CPPUNIT_ASSERT(penaltyFee.penaltyAmount == 110.00);
    CPPUNIT_ASSERT(penaltyFee.penaltyCurrency == "USD");
  }

  void testUsePenaltyPercentWhenNoPenaltyAmtExists()
  {
    VoluntaryChangesInfo voluntaryChanges;
    voluntaryChangesW.orig() = &voluntaryChanges;
    voluntaryChanges.percent() = 30.00;

    PenaltyFee penaltyFee;
    _calculator->calculatePenaltyFee(paxTypeFare, voluntaryChangesW, penaltyFee);

    CPPUNIT_ASSERT(penaltyFee.penaltyAmount == 30.00);
    CPPUNIT_ASSERT(penaltyFee.penaltyCurrency == "USD");
  }

  void testUseHigherBetweenPenaltyPercentAndPenaltyAmt2()
  {
    VoluntaryChangesInfo voluntaryChanges;
    voluntaryChangesW.orig() = &voluntaryChanges;
    voluntaryChanges.percent() = 30.00;
    voluntaryChanges.penaltyAmt1() = 100.00;
    voluntaryChanges.cur1() = "EUR";
    voluntaryChanges.penaltyAmt2() = 40.00;
    voluntaryChanges.cur2() = "USD";
    voluntaryChanges.highLowInd() = 'H';

    PenaltyFee penaltyFee;
    //        _calculator->setEquivalentCurrency();
    _calculator->calculatePenaltyFee(paxTypeFare, voluntaryChangesW, penaltyFee);

    CPPUNIT_ASSERT(penaltyFee.penaltyAmount == 40.00);
    CPPUNIT_ASSERT(penaltyFee.penaltyCurrency == "USD");
    CPPUNIT_ASSERT(penaltyFee.penaltyFromPercent > 0);
    CPPUNIT_ASSERT(penaltyFee.penaltyFromPercentCurrency == "USD");
    CPPUNIT_ASSERT(penaltyFee.penaltyAmt1InEquivCurrency == 0);
    CPPUNIT_ASSERT(penaltyFee.penaltyAmt2InEquivCurrency == 0);
    CPPUNIT_ASSERT(penaltyFee.penaltyFromPercentInEquivCurrency == 0);
    CPPUNIT_ASSERT(penaltyFee.penaltyAmountInEquivCurrency == 0);
  }

  void testAccumulateCharges()
  {
    PenaltyFee penalty;
    penalty.penaltyAmount = 20.00;
    penalty.penaltyCurrency = "USD";
    PenaltyFee penalty2;
    penalty2.penaltyAmount = 30.00;
    penalty2.penaltyCurrency = "USD";
    _calculator->_reissueCharges->penaltyFees.insert(std::make_pair(&paxTypeFare, &penalty));
    _calculator->_reissueCharges->penaltyFees.insert(std::make_pair(&paxTypeFare2, &penalty2));

    _calculator->_equivalentCurrency = "USD";
    _calculator->_chargeCurrency.insert("USD");
    _calculator->accumulateCharges();
    CPPUNIT_ASSERT_EQUAL(50.00, _calculator->_reissueCharges->changeFee);
    CPPUNIT_ASSERT_EQUAL(50.00, permutation.reissueCharges()->changeFee);
    CPPUNIT_ASSERT(permutation.reissueCharges()->changeFeeCurrency == "USD");
  }

  void testChangeFeeAdjustedByMinimumAmount()
  {
    VoluntaryChangesInfo voluntaryChange;
    voluntaryChangesW.orig() = &voluntaryChange;
    voluntaryChange.minAmt() = 100.00;
    voluntaryChange.minCur() = "USD";
    _calculator->_reissueCharges->changeFee = 30.00;
    _calculator->_reissueCharges->changeFeeCurrency = "USD";
    PenaltyFee penaltyFee;
    penaltyFee.penaltyAmount = 30.00;
    penaltyFee.penaltyCurrency = "USD";
    _calculator->_reissueCharges->penaltyFees.insert(std::make_pair(&paxTypeFare, &penaltyFee));

    _calculator->_fareVoluntaryChangeMap.insert(
        std::make_pair(&paxTypeFare, &voluntaryChangesW));
    _calculator->_equivalentCurrency = "USD";
    _calculator->adjustChangeFeeByMinAmount();
    CPPUNIT_ASSERT_EQUAL(100.00, _calculator->_reissueCharges->changeFee);
  }

  void testCalculateDiscount_Infant_CAT19_Value1()
  {
    setDiscountPaxTypeData(INFANT_CODE, INFANT_DTYPE);
    setDiscountTagsData('1', ' ', '1', ' ');
    paxTypeFareFD = getFareWithDiscount(RuleConst::CHILDREN_DISCOUNT_RULE, 50);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(50.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Infant_CAT22_Value1()
  {
    setDiscountPaxTypeData(INFANT_CODE, INFANT_DTYPE);
    setDiscountTagsData('1', ' ', '1', ' ');
    paxTypeFareFD = getFareWithDiscount(RuleConst::CHILDREN_DISCOUNT_RULE, 20);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(80.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Infant_INF_CAT19_Value1()
  {
    setDiscountPaxTypeData(INFANT_CODE);
    setDiscountTagsData('1', ' ', '1', ' ');
    paxTypeFareFD = getFareWithDiscount(RuleConst::CHILDREN_DISCOUNT_RULE, 20);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(0.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Infant_CAT19_Value2346()
  {
    setDiscountPaxTypeData(INFANT_CODE, INFANT_DTYPE);
    setDiscountTagsData('2', '3', '4', '6');
    paxTypeFareFD = getFareWithDiscount(RuleConst::CHILDREN_DISCOUNT_RULE, 20);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(0.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Child_CAT19_Value2()
  {
    setDiscountPaxTypeData("CHI", CHILD_DTYPE);
    setDiscountTagsData('2', '3', '4', '6');
    paxTypeFareFD = getFareWithDiscount(RuleConst::CHILDREN_DISCOUNT_RULE, 50);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(50.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Child_CAT22_Value2()
  {
    setDiscountPaxTypeData("CHI", CHILD_DTYPE);
    setDiscountTagsData('2', '3', '4', '6');
    paxTypeFareFD = getFareWithDiscount(RuleConst::CHILDREN_DISCOUNT_RULE, 20);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(80.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Child_CAT19_Value1346()
  {
    setDiscountPaxTypeData("CHI", CHILD_DTYPE);
    setDiscountTagsData('1', '3', '4', '6');
    paxTypeFareFD = getFareWithDiscount(RuleConst::CHILDREN_DISCOUNT_RULE, 20);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(0.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Youth_CAT22_Value3()
  {
    setDiscountPaxTypeData(ReissuePenaltyCalculator::YOUTH_CODES);
    setDiscountTagsData('1', '3', '4', '6');
    paxTypeFareFD = getFareWithDiscount(RuleConst::OTHER_DISCOUNT_RULE, 50);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(50.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Youth_CAT19_Value3()
  {
    setDiscountPaxTypeData(ReissuePenaltyCalculator::YOUTH_CODES);
    setDiscountTagsData('1', '3', '4', '6');
    paxTypeFareFD = getFareWithDiscount(RuleConst::OTHER_DISCOUNT_RULE, 20);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(80.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Youth_CAT22_Value1245()
  {
    setDiscountPaxTypeData(ReissuePenaltyCalculator::YOUTH_CODES);
    setDiscountTagsData('1', '2', '4', '6');
    paxTypeFareFD = getFareWithDiscount(RuleConst::OTHER_DISCOUNT_RULE, 20);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(0.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Senior_CAT22_Value4()
  {
    setDiscountPaxTypeData(ReissuePenaltyCalculator::SENIOR_CODES);
    setDiscountTagsData(' ', ' ', ' ', '4');
    paxTypeFareFD = getFareWithDiscount(RuleConst::OTHER_DISCOUNT_RULE, 50);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(50.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Senior_CAT19_Value4()
  {
    setDiscountPaxTypeData(ReissuePenaltyCalculator::SENIOR_CODES);
    setDiscountTagsData(' ', ' ', ' ', '4');
    paxTypeFareFD = getFareWithDiscount(RuleConst::OTHER_DISCOUNT_RULE, 20);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(80.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Senior_CAT22_Value1235()
  {
    setDiscountPaxTypeData(ReissuePenaltyCalculator::SENIOR_CODES);
    setDiscountTagsData('1', '2', '3', '5');
    paxTypeFareFD = getFareWithDiscount(RuleConst::OTHER_DISCOUNT_RULE,  20);
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(0.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Senior_CAT22_Value6()
  {
    setDiscountPaxTypeData(ReissuePenaltyCalculator::SENIOR_CODES);
    setDiscountTagsData('6', ' ', ' ', ' ');
    paxTypeFareFD = getFareWithDiscount(RuleConst::OTHER_DISCOUNT_RULE, 50,
                                        ReissuePenaltyCalculator::SENIOR_CODES.front());
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(50.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Youth_CAT22_Value6()
  {
    setDiscountPaxTypeData(ReissuePenaltyCalculator::YOUTH_CODES);
    setDiscountTagsData('6', ' ', ' ', ' ');
    paxTypeFareFD = getFareWithDiscount(RuleConst::OTHER_DISCOUNT_RULE, 20,
                                        ReissuePenaltyCalculator::SENIOR_CODES.front());
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(0.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Senior_CAT19_Value6()
  {
    setDiscountPaxTypeData(ReissuePenaltyCalculator::SENIOR_CODES);
    setDiscountTagsData('6', ' ', ' ', ' ');
    paxTypeFareFD = getFareWithDiscount(RuleConst::OTHER_DISCOUNT_RULE, 20,
                                        ReissuePenaltyCalculator::SENIOR_CODES.front());
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(80.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Youth_CAT19_Value5()
  {
    setDiscountPaxTypeData(ReissuePenaltyCalculator::YOUTH_CODES);
    setDiscountTagsData('5', ' ', ' ', ' ');
    paxTypeFareFD = getFareWithDiscount(RuleConst::OTHER_DISCOUNT_RULE, 50,
                                        ReissuePenaltyCalculator::YOUTH_CODES.front());
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(0.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Senior_CAT19_Value5()
  {
    setDiscountPaxTypeData(ReissuePenaltyCalculator::SENIOR_CODES);
    setDiscountTagsData('5', ' ', ' ', ' ');
    paxTypeFareFD = getFareWithDiscount(RuleConst::OTHER_DISCOUNT_RULE, 20,
                                        ReissuePenaltyCalculator::YOUTH_CODES.front());
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(0.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testCalculateDiscount_Youth_CAT22_Value5()
  {
    setDiscountPaxTypeData(ReissuePenaltyCalculator::YOUTH_CODES);
    setDiscountTagsData('5', ' ', ' ', ' ');
    paxTypeFareFD = getFareWithDiscount(RuleConst::OTHER_DISCOUNT_RULE, 20,
                                        ReissuePenaltyCalculator::YOUTH_CODES.front());
    tag.paxTypeFare() = paxTypeFareFD;

    CPPUNIT_ASSERT_EQUAL(0.0, setUpDiscountCalculator()->calculateDiscount(tag));
  }

  void testSMPnonChangeable_no_before_records()
  {
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    std::unordered_set<const VoluntaryChangesInfoW*> records;
    records.insert(createR3(AFTER_DEP, Money(300, "USD")));
    records.insert(createR3(AFTER_DEP, Money(150, "USD")));
    FcFees result = _calculator->getPenalties(records, paxTypeFare, smp::BEFORE, *_emptyDiag);

    CPPUNIT_ASSERT_EQUAL(size_t(0), result.size());
  }

  void testSMPnonChangeable_no_after_records()
  {
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    std::unordered_set<const VoluntaryChangesInfoW*> records;
    records.insert(createR3(BEFORE_DEP, Money(300, "USD")));
    records.insert(createR3(BEFORE_DEP, Money(150, "USD")));
    FcFees result = _calculator->getPenalties(records, paxTypeFare, smp::AFTER, *_emptyDiag);

    CPPUNIT_ASSERT_EQUAL(size_t(0), result.size());
  }

  void testSMPchangeable_no_departure_specified()
  {
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    std::unordered_set<const VoluntaryChangesInfoW*> records;
    records.insert(createR3(BEFORE_DEP, Money(300, "USD")));
    records.insert(createR3(AFTER_DEP, Money(150, "USD")));
    records.insert(createR3(NO_DEP_RES, Money(200, "USD")));
    FcFees result = _calculator->getPenalties(records, paxTypeFare, smp::BOTH, *_emptyDiag);

    CPPUNIT_ASSERT_EQUAL(size_t(3), result.size());
  }

  void testSMPchangeable_after_departure_specified()
  {
    _trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    std::unordered_set<const VoluntaryChangesInfoW*> records;
    records.insert(createR3(BEFORE_DEP, Money(300, "USD")));
    records.insert(createR3(AFTER_DEP, Money(150, "USD")));
    records.insert(createR3(NO_DEP_RES, Money(200, "USD")));
    records.insert(createR3(NO_DEP_RES, Money(0, "USD")));
    FcFees result = _calculator->getPenalties(records, paxTypeFare, smp::AFTER, *_emptyDiag);

    CPPUNIT_ASSERT_EQUAL(size_t(3), result.size());
  }

  void testSkipNoShowRecordsAnytimeAfter()
  {
    ReissueSequence seq;
    seq.unusedFlightInd() = OriginallyScheduledFlightValidator::ANYTIME_AFTER;

    _memHandle.create<NoShowDataHandle>(std::vector<ReissueSequence*>{&seq});
    std::unordered_set<const VoluntaryChangesInfoW*> records{
        createR3(smp::BEFORE, Money(0., "USD"))};
    FcFees result = _calculator->getPenalties(records, paxTypeFare, smp::BEFORE, *_emptyDiag);

    CPPUNIT_ASSERT(result.empty());
  }

  void testSkipNoShowRecordsDayAfter()
  {
    ReissueSequence seq;
    seq.unusedFlightInd() = OriginallyScheduledFlightValidator::DAY_AFTER;

    _memHandle.create<NoShowDataHandle>(std::vector<ReissueSequence*>{&seq});
    std::unordered_set<const VoluntaryChangesInfoW*> records{
        createR3(smp::BEFORE, Money(0., "USD"))};
    FcFees result = _calculator->getPenalties(records, paxTypeFare, smp::BEFORE, *_emptyDiag);

    CPPUNIT_ASSERT(result.empty());
  }

  void testPassTwoReissueSequences()
  {
    ReissueSequence seq1;
    seq1.origSchedFltUnit() = OriginallyScheduledFlightValidator::ANYTIME_AFTER;
    ReissueSequence seq2;
    seq2.origSchedFltUnit() = OriginallyScheduledFlightValidator::ANYTIME_BEFORE;

    _memHandle.create<NoShowDataHandle>(std::vector<ReissueSequence*>{&seq1, &seq2});
    std::unordered_set<const VoluntaryChangesInfoW*> records{
        createR3(smp::BEFORE, Money(0., "USD"))};
    FcFees result = _calculator->getPenalties(records, paxTypeFare, smp::BEFORE, *_emptyDiag);

    CPPUNIT_ASSERT_EQUAL(size_t(1), result.size());
  }

  void testPassValidRecord()
  {
    std::unordered_set<const VoluntaryChangesInfoW*> records{
        createR3(smp::BEFORE, Money(0., "USD"))};
    FcFees result = _calculator->getPenalties(records, paxTypeFare, smp::BEFORE, *_emptyDiag);

    CPPUNIT_ASSERT_EQUAL(size_t(1), result.size());
  }

  //----------- helper methods --------------------------------------------------------------------
  void createFareCalcConfig()
  {
    trx->fareCalcConfig() = _memHandle.insert(new FareCalcConfig);
    trx->fareCalcConfig()->fareBasisTktDesLng() = ' ';
  }

  void
  setDiscountPaxTypeData(const std::vector<PaxTypeCode> paxTypes, DiscountPaxType type = NONE_DTYPE)
  {
    setDiscountPaxTypeData(paxTypes.front(), type);
  }

  void
  setDiscountPaxTypeData(PaxTypeCode paxType, DiscountPaxType type = NONE_DTYPE)
  {
    pti.paxType() = paxType;

    if (type == INFANT_DTYPE)
    {
      pti.infantInd() = 'Y';
    }
    else if (type == CHILD_DTYPE)
    {
      pti.childInd() = 'Y';
      pti.infantInd() = 'N';
    }

    if (type != NONE_DTYPE)
      pti.initPsgType();
  }

  void setDiscountTagsData(Indicator discountTag1,
                           Indicator discountTag2,
                           Indicator discountTag3,
                           Indicator discountTag4)
  {
    voluntaryChangeFD.discountTag1() = discountTag1;
    voluntaryChangeFD.discountTag2() = discountTag2;
    voluntaryChangeFD.discountTag3() = discountTag3;
    voluntaryChangeFD.discountTag4() = discountTag4;
  }

protected:
  PricingTrx* _trx;
  RexPricingRequest* _request;
  RexPricingOptions* _options;
  ProcessTagPermutation* _permutation;
  CurrencyCode* _calculationCurrency;
  VoluntaryChangesInfo* _rec3;

  ReissuePenaltyCalculator* _calculator;

  DiagManager* _emptyDiag;

  PaxTypeInfo* _pti;
  PaxTypeCode* _paxCode;
  DiscountInfo* _di;

  TestMemHandle _memHandle;

  PricingTrx* trx;
  FareInfo fareInfo;
  Fare fare;
  PaxTypeFare paxTypeFare;
  PaxType paxType;
  FareInfo fareInfo2;
  Fare fare2;
  PaxTypeFare paxTypeFare2;
  FareMarket fm;
  FareMarket fm2;
  AirSeg travelSeg;
  AirSeg travelSeg2;
  FareUsage fu;
  FareUsage fu2;
  PricingUnit pu;
  FarePath fp;
  ProcessTagPermutation permutation;
  PaxType exchangePaxType;
  PaxTypeInfo pti;
  ProcessTagInfo tag;
  VoluntaryChangesInfo voluntaryChangeFD;
  VoluntaryChangesInfoW voluntaryChangesW;
  PaxTypeFare* paxTypeFareFD;
  CurrencyCode _baseFareCurrCode;
};

const PaxTypeCode ReissuePenaltyCalculatorTest::INFANT_CODE = "IFE";
const Indicator ReissuePenaltyCalculatorTest::BEFORE_DEP = 'B';
const Indicator ReissuePenaltyCalculatorTest::AFTER_DEP = 'A';
const Indicator ReissuePenaltyCalculatorTest::NO_DEP_RES = ' ';

CPPUNIT_TEST_SUITE_REGISTRATION(ReissuePenaltyCalculatorTest);
}
