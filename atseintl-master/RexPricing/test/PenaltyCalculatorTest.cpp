#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/RefundPermutation.h"
#include "RexPricing/PenaltyCalculator.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "DataModel/Fare.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareCompInfo.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "DataModel/RefundProcessInfo.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPricingTrx.h"
#include "Common/CurrencyConversionFacade.h"
#include "DataModel/RefundPenalty.h"
#include "DataModel/ExcItin.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "RexPricing/RefundDiscountApplier.h"
#include "test/include/TestLogger.h"
#include "test/include/PrintCollection.h"

namespace tse
{
using namespace boost::assign;

class MockTrx : public RefundPricingTrx
{
public:
  virtual Money convertCurrency(const Money& source, const CurrencyCode& targetCurr) const
  {
    if (source.code() == "PLN" && targetCurr == NUC)
      return Money(source.value() * 0.5, targetCurr);

    if (source.code() == NUC && targetCurr == "PLN")
      return Money(source.value() * 2, targetCurr);

    return source;
  }
};

class MockCalculator : public PenaltyCalculator
{
public:
  MockCalculator(RefundPricingTrx& trx, RefundDiscountApplier& discountApplier)
    : PenaltyCalculator(trx, discountApplier)
  {
  }

protected:
  virtual CurrencyCode getOriginCurrency(const FareUsage& fu, const PuItems& items)
  {
    return "PLN";
  }
};

bool
orderedFee(const RefundPenalty::Fee& l, const RefundPenalty::Fee& r);

class PenaltyCalculatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PenaltyCalculatorTest);

  CPPUNIT_TEST(testDetermineScope_PuOnly);
  CPPUNIT_TEST(testDetermineScope_FcOnly);
  CPPUNIT_TEST(testDetermineScope_Mixed_OptionAOnly);
  CPPUNIT_TEST(testDetermineScope_Mixed_OptionBOnly);
  CPPUNIT_TEST(testDetermineScope_Mixed_Mixed);

  CPPUNIT_TEST(testCreateVoluntaryRefundsArray);
  CPPUNIT_TEST(testUpdateVoluntaryRefundsArray_Pass);
  CPPUNIT_TEST(testUpdateVoluntaryRefundsArray_Fail);

  CPPUNIT_TEST(testPercentagePenalty_singleFC);
  CPPUNIT_TEST(testPercentagePenalty_singleFCzero);
  CPPUNIT_TEST(testPercentagePenalty_multipleFC);
  CPPUNIT_TEST(testPercentagePenalty_multipleFCwithZero);
  CPPUNIT_TEST(testHundredPercentPenalty_singleFC);
  CPPUNIT_TEST(testHundredPercentPenalty_multipleFC);
  CPPUNIT_TEST(testHundredPercentPenalty_multipleFCwithZero);

  CPPUNIT_TEST(testPercentagePenalty_singlePU);
  CPPUNIT_TEST(testPercentagePenalty_singlePUzero);
  CPPUNIT_TEST(testPercentagePenalty_multiplePU);
  CPPUNIT_TEST(testPercentagePenalty_multiplePUwithZero);
  CPPUNIT_TEST(testHundredPercentPenalty_singlePU);
  CPPUNIT_TEST(testHundredPercentPenalty_multiplePU);
  CPPUNIT_TEST(testHundredPercentPenalty_multiplePUwithZero);

  CPPUNIT_TEST(testPercentagePenalty_singlePU_MIXED);
  CPPUNIT_TEST(testPercentagePenalty_singlePU_MIXEDzero);
  CPPUNIT_TEST(testPercentagePenalty_singleFC_MIXED);
  CPPUNIT_TEST(testPercentagePenalty_singleFC_MIXEDzero);
  CPPUNIT_TEST(testPercentagePenalty_multiplePU_MIXED);
  CPPUNIT_TEST(testPercentagePenalty_multipleFC_MIXED);
  CPPUNIT_TEST(testPercentagePenalty_multiplePUandFC_MIXED_PUwins);
  CPPUNIT_TEST(testPercentagePenalty_multiplePUandFC_MIXED_FCwins);
  CPPUNIT_TEST(testHundredPercentPenalty_singleFC_MIXED);
  CPPUNIT_TEST(testHundredPercentPenalty_singlePU_MIXED);
  CPPUNIT_TEST(testHundredPercentPenalty_multiplePU_MIXED);
  CPPUNIT_TEST(testHundredPercentPenalty_multipleFC_MIXED);
  CPPUNIT_TEST(testHundredPercentPenalty_multipleFCandPU_MIXED_FCwins);
  CPPUNIT_TEST(testHundredPercentPenalty_multipleFCandPU_MIXED_PUwins);

  CPPUNIT_TEST(testSpecifiedPenalty_FCsecondCurrencyMatched);
  CPPUNIT_TEST(testSpecifiedPenalty_FCfirstCurrencyMatched);
  CPPUNIT_TEST(testSpecifiedPenalty_FCnoneCurrencyMatched);
  CPPUNIT_TEST(testSpecifiedPenalty_FCzeroCurrencyMatched);
  CPPUNIT_TEST(testSpecifiedPenalty_FCfirstZeroCurrencyMatched);
  CPPUNIT_SKIP_TEST(testSpecifiedPenalty_FCmultiple);

  CPPUNIT_TEST(testSpecifiedPenalty_PUzeroCurrencyMatched);
  CPPUNIT_SKIP_TEST(testSpecifiedPenalty_PUfirstCurrencyMatched);
  CPPUNIT_SKIP_TEST(testSpecifiedPenalty_PUPercentSpecified_winsPercent);
  CPPUNIT_SKIP_TEST(testSpecifiedPenalty_PUPercentSpecified_winsSpecified);
  CPPUNIT_SKIP_TEST(testSpecifiedPenalty_PUPercentSpecifiedMultiple_winsPercent);
  CPPUNIT_SKIP_TEST(testSpecifiedPenalty_PUPercentSpecifiedMultiple_winsSpecified);

  CPPUNIT_SKIP_TEST(testSpecifiedPenalty_MIXEDfc);
  CPPUNIT_SKIP_TEST(testSpecifiedPenalty_MIXEDpu);
  CPPUNIT_SKIP_TEST(testSpecifiedPenalty_MIXEDmultiple);
  CPPUNIT_SKIP_TEST(testSpecifiedPenalty_MIXEDmultipleWithPercent_specWins);
  CPPUNIT_SKIP_TEST(testSpecifiedPenalty_MIXEDmultipleWithPercent_percentWins);

  CPPUNIT_TEST(testHiLoByte_FCscope);
  CPPUNIT_TEST(testHiLoByte_PUscope_specWins);
  CPPUNIT_TEST(testHiLoByte_PUscope_percWins);
  CPPUNIT_TEST(testHiLoByte_MXscope_specWins);
  CPPUNIT_TEST(testHiLoByte_MXscope_percWins);

  CPPUNIT_TEST(testDetermineMethod_OneHundredPenalty);
  CPPUNIT_TEST(testDetermineMethod_SepecificAmount);
  CPPUNIT_TEST(testDetermineMethod_PercentagePenalty);
  CPPUNIT_TEST(testDetermineMethod_HighLowPenalty);
  CPPUNIT_TEST(testDetermineMethod_NoPenalty);

  CPPUNIT_TEST(testCalculationFeeOperatorLess);
  CPPUNIT_TEST(testCalculationFeeOperatorLess_withNoRefundable);
  CPPUNIT_TEST(testCalculationFeeOperatorLess_equal);
  CPPUNIT_TEST(testCalculationFeeOperatorLess_equal_withNoRefundable);

  CPPUNIT_TEST(testSetTotalPenalty_Zero);
  CPPUNIT_TEST(testSetTotalPenalty);

  CPPUNIT_TEST(testSetHighestPenalty);

  CPPUNIT_TEST(testSetMinimumPenalty);
  CPPUNIT_TEST(testSetMinimumPenalty_Zero);
  CPPUNIT_TEST(testSetMinimumPenalty_noRec3);
  CPPUNIT_TEST(testSetMinimumPenalty_noCurrency);

  CPPUNIT_TEST(testScopedFeeComparator);

  CPPUNIT_TEST_SUITE_END();

public:
  TestMemHandle _memH;
  PenaltyCalculator* _calculator;
  FarePath* _fp;
  RefundPricingTrx* _trx;
  std::vector<VoluntaryRefundsInfo*>* _vri;
  ExcItin* _excItin;
  PaxType* _paxType;
  RootLoggerGetOff _loggerOff;

  static const CurrencyCode PLN;
  static const Indicator BLANK;

  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _excItin = _memH(new ExcItin);
    _excItin->calculationCurrency() = PLN;

    _trx = _memH(new MockTrx);
    _trx->exchangeItin().push_back(_excItin);
    _trx->setRequest(_memH(new PricingRequest));
    _trx->getRequest()->ticketingAgent() = _memH(new Agent);

    _fp = _memH(new FarePath);
    _excItin->farePath().push_back(_fp);
    _fp->paxType() = _memH(new PaxType);

    _paxType = _memH(new PaxType);
    _trx->exchangePaxType() = _paxType;
    _paxType->paxType() = "ADT";
    _paxType->paxTypeInfo() = _memH(new PaxTypeInfo);

    _calculator = _memH(new MockCalculator(*_trx, *_memH(new RefundDiscountApplier(*_paxType))));
  }

  void tearDown() { _memH.clear(); }

  PricingUnit* setUpPercentagePenalties(const std::vector<Percent>& percents)
  {
    _vri = _memH(new std::vector<VoluntaryRefundsInfo*>);
    for (unsigned i = 0; i < percents.size(); ++i)
    {
      *_vri += createRecord3();
      (*_vri)[i]->penaltyPercent() = percents[i];
    }

    return populateVrArray(*_vri);
  }

  typedef RefundPenalty::Fee Fee;
  typedef std::vector<Fee> FeeVec;

  static bool orderedFee(const Fee& l, const Fee& r)
  {
    return (
        (l.amount().code() != r.amount().code()
             ? l.amount().code() < r.amount().code()
             : (l.nonRefundable() != r.nonRefundable() ? l.nonRefundable() < r.nonRefundable()
                                                       : l.amount().value() < r.amount().value())));
  }

  void assertPenalty(FeeVec expect, RefundPenalty::Scope scope, const RefundPenalty& penalty)
  {
    FeeVec fees = penalty.fee();

    std::sort(fees.begin(), fees.end(), orderedFee);
    std::sort(expect.begin(), expect.end(), orderedFee);

    CPPUNIT_ASSERT(expect == fees);
    CPPUNIT_ASSERT_EQUAL(scope, penalty.scope());
  }

  Fee getFee(const Money& mnt, bool refundable = REFUNDABLE)
  {
    return Fee(mnt, NODISCOUNT, refundable);
  }

  Fee getFee(MoneyAmount amt, bool refundable = REFUNDABLE)
  {
    return getFee(Money(amt, PLN), refundable);
  }

  void testPercentagePenalty_singleFC()
  {
    PricingUnit& pu = *setUpPercentagePenalties(std::vector<Percent>(1, 25.0));

    assertPenalty(FeeVec(1, getFee(50.0)), RefundPenalty::FC, *_calculator->calculateInFcScope(pu));
  }

  void testPercentagePenalty_singleFCzero()
  {
    PricingUnit& pu = *setUpPercentagePenalties(std::vector<Percent>(1, 0.0));

    assertPenalty(FeeVec(1, getFee(0.0)), RefundPenalty::FC, *_calculator->calculateInFcScope(pu));
  }

  void testPercentagePenalty_multipleFC()
  {
    std::vector<Percent> percents;
    percents += 15.0, 30.0, 50.0;

    PricingUnit& pu = *setUpPercentagePenalties(percents);

    FeeVec expect;
    expect += getFee(30.0), getFee(60.0), getFee(100.0);
    assertPenalty(expect, RefundPenalty::FC, *_calculator->calculateInFcScope(pu));
  }

  void testPercentagePenalty_multipleFCwithZero()
  {
    std::vector<Percent> percents;
    percents += 1.0, 0.0, 99.0;

    PricingUnit& pu = *setUpPercentagePenalties(percents);

    FeeVec expect;
    expect += getFee(2.0), getFee(0.0), getFee(198.0);
    assertPenalty(expect, RefundPenalty::FC, *_calculator->calculateInFcScope(pu));
  }

  void testHundredPercentPenalty_singleFC()
  {
    PricingUnit& pu = *setUpPercentagePenalties(std::vector<Percent>(1, 25.0));
    populateRecords3(*_vri,
                     &VoluntaryRefundsInfo::cancellationInd,
                     std::vector<Indicator>(1, PenaltyCalculator::HUNDRED_PERCENT_PENALTY));

    RefundPenalty* result = _calculator->calculateInFcScope(pu);

    assertPenalty(FeeVec(1, getFee(200.0, NOREFUNDABLE)), RefundPenalty::FC, *result);
  }

  void testHundredPercentPenalty_multipleFC()
  {
    std::vector<Percent> percents;
    percents += 15.0, 30.0, 50.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    std::vector<Indicator> cancellation;
    cancellation += BLANK, PenaltyCalculator::HUNDRED_PERCENT_PENALTY, BLANK;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::cancellationInd, cancellation);

    RefundPenalty* result = _calculator->calculateInFcScope(pu);

    FeeVec expect;
    expect += getFee(30.0), getFee(200.0, NOREFUNDABLE), getFee(100.0);
    assertPenalty(expect, RefundPenalty::FC, *result);
  }

  void testHundredPercentPenalty_multipleFCwithZero()
  {
    std::vector<Percent> percents;
    percents += 1.0, 0.0, 0.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    std::vector<Indicator> cancellation;
    cancellation += BLANK, PenaltyCalculator::HUNDRED_PERCENT_PENALTY, BLANK;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::cancellationInd, cancellation);

    RefundPenalty* result = _calculator->calculateInFcScope(pu);

    FeeVec expect;
    expect += getFee(2.0), getFee(200.0, NOREFUNDABLE), getFee(0.0);
    assertPenalty(expect, RefundPenalty::FC, *result);
  }

  void testPercentagePenalty_singlePU()
  {
    PricingUnit& pu = *setUpPercentagePenalties(std::vector<Percent>(1, 25.0));

    assertPenalty(FeeVec(1, getFee(50.0)), RefundPenalty::PU, *_calculator->calculateInPuScope(pu));
  }

  void testPercentagePenalty_singlePUzero()
  {
    PricingUnit& pu = *setUpPercentagePenalties(std::vector<Percent>(1, 0.0));

    assertPenalty(FeeVec(1, getFee(0.0)), RefundPenalty::PU, *_calculator->calculateInPuScope(pu));
  }

  void testPercentagePenalty_multiplePU()
  {
    std::vector<Percent> percents;
    percents += 15.0, 30.0, 50.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    assertPenalty(
        FeeVec(1, getFee(300.0)), RefundPenalty::PU, *_calculator->calculateInPuScope(pu));
  }

  void testPercentagePenalty_multiplePUwithZero()
  {
    std::vector<Percent> percents;
    percents += 0.0, 70.0, 99.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    assertPenalty(
        FeeVec(1, getFee(594.0)), RefundPenalty::PU, *_calculator->calculateInPuScope(pu));
  }

  void testHundredPercentPenalty_singlePU()
  {
    PricingUnit& pu = *setUpPercentagePenalties(std::vector<Percent>(1, 25.0));
    populateRecords3(*_vri,
                     &VoluntaryRefundsInfo::cancellationInd,
                     std::vector<Indicator>(1, PenaltyCalculator::HUNDRED_PERCENT_PENALTY));

    assertPenalty(FeeVec(1, getFee(200.0, NOREFUNDABLE)),
                  RefundPenalty::PU,
                  *_calculator->calculateInPuScope(pu));
  }

  void testHundredPercentPenalty_multiplePU()
  {
    std::vector<Percent> percents;
    percents += 15.0, 30.0, 50.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    std::vector<Indicator> cancellation;
    cancellation += BLANK, PenaltyCalculator::HUNDRED_PERCENT_PENALTY, BLANK;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::cancellationInd, cancellation);

    assertPenalty(FeeVec(1, getFee(600.0, NOREFUNDABLE)),
                  RefundPenalty::PU,
                  *_calculator->calculateInPuScope(pu));
  }

  void testHundredPercentPenalty_multiplePUwithZero()
  {
    std::vector<Percent> percents;
    percents += 1.0, 0.0, 0.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    std::vector<Indicator> cancellation;
    cancellation += BLANK, PenaltyCalculator::HUNDRED_PERCENT_PENALTY, BLANK;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::cancellationInd, cancellation);

    assertPenalty(FeeVec(1, getFee(600.0, NOREFUNDABLE)),
                  RefundPenalty::PU,
                  *_calculator->calculateInPuScope(pu));
  }

  void testPercentagePenalty_singlePU_MIXED()
  {
    PricingUnit& pu = *setUpPercentagePenalties(std::vector<Percent>(1, 25.0));

    populateRecords3(*_vri,
                     &VoluntaryRefundsInfo::reissueFeeInd,
                     std::vector<Indicator>(1, PenaltyCalculator::REISSUEFEE_PU));

    assertPenalty(FeeVec(1, getFee(50.0, REFUNDABLE)),
                  RefundPenalty::PU,
                  *_calculator->calculateInMixedScope(pu));
  }

  void testPercentagePenalty_singlePU_MIXEDzero()
  {
    PricingUnit& pu = *setUpPercentagePenalties(std::vector<Percent>(1, 0.0));

    populateRecords3(*_vri,
                     &VoluntaryRefundsInfo::reissueFeeInd,
                     std::vector<Indicator>(1, PenaltyCalculator::REISSUEFEE_PU));

    assertPenalty(FeeVec(1, getFee(0.0, REFUNDABLE)),
                  RefundPenalty::PU,
                  *_calculator->calculateInMixedScope(pu));
  }

  void testPercentagePenalty_singleFC_MIXED()
  {
    PricingUnit& pu = *setUpPercentagePenalties(std::vector<Percent>(1, 25.0));

    populateRecords3(*_vri,
                     &VoluntaryRefundsInfo::reissueFeeInd,
                     std::vector<Indicator>(1, PenaltyCalculator::REISSUEFEE_FC));

    assertPenalty(
        FeeVec(1, getFee(50.0)), RefundPenalty::FC, *_calculator->calculateInMixedScope(pu));
  }

  void testPercentagePenalty_singleFC_MIXEDzero()
  {
    PricingUnit& pu = *setUpPercentagePenalties(std::vector<Percent>(1, 0.0));

    populateRecords3(*_vri,
                     &VoluntaryRefundsInfo::reissueFeeInd,
                     std::vector<Indicator>(1, PenaltyCalculator::REISSUEFEE_FC));

    assertPenalty(
        FeeVec(1, getFee(0.0)), RefundPenalty::FC, *_calculator->calculateInMixedScope(pu));
  }

  void testPercentagePenalty_multiplePU_MIXED()
  {
    std::vector<Percent> percents;
    percents += 15.0, 30.0, 50.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    std::vector<Indicator> reissue;
    reissue += PenaltyCalculator::REISSUEFEE_PU, PenaltyCalculator::REISSUEFEE_PU,
        PenaltyCalculator::REISSUEFEE_PU;

    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    assertPenalty(
        FeeVec(1, getFee(300.0)), RefundPenalty::PU, *_calculator->calculateInMixedScope(pu));
  }

  void testPercentagePenalty_multipleFC_MIXED()
  {
    std::vector<Percent> percents;
    percents += 15.0, 30.0, 50.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    std::vector<Indicator> reissue;
    reissue += PenaltyCalculator::REISSUEFEE_FC, PenaltyCalculator::REISSUEFEE_FC,
        PenaltyCalculator::REISSUEFEE_FC;

    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    assertPenalty(
        FeeVec(1, getFee(100.0)), RefundPenalty::FC, *_calculator->calculateInMixedScope(pu));
  }

  void testPercentagePenalty_multiplePUandFC_MIXED_PUwins()
  {
    std::vector<Percent> percents;
    percents += 15.0, 30.0, 50.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    std::vector<Indicator> reissue;
    reissue += PenaltyCalculator::REISSUEFEE_FC, PenaltyCalculator::REISSUEFEE_PU,
        PenaltyCalculator::REISSUEFEE_FC;

    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    assertPenalty(
        FeeVec(1, getFee(180.0)), RefundPenalty::PU, *_calculator->calculateInMixedScope(pu));
  }

  void testPercentagePenalty_multiplePUandFC_MIXED_FCwins()
  {
    std::vector<Percent> percents;
    percents += 15.0, 30.0, 50.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    std::vector<Indicator> reissue;
    reissue += PenaltyCalculator::REISSUEFEE_PU, PenaltyCalculator::REISSUEFEE_FC,
        PenaltyCalculator::REISSUEFEE_FC;

    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    assertPenalty(
        FeeVec(1, getFee(100.0)), RefundPenalty::FC, *_calculator->calculateInMixedScope(pu));
  }

  void testHundredPercentPenalty_singleFC_MIXED()
  {
    PricingUnit& pu = *setUpPercentagePenalties(std::vector<Percent>(1, 25.0));
    populateRecords3(*_vri,
                     &VoluntaryRefundsInfo::cancellationInd,
                     std::vector<Indicator>(1, PenaltyCalculator::HUNDRED_PERCENT_PENALTY));
    populateRecords3(*_vri,
                     &VoluntaryRefundsInfo::reissueFeeInd,
                     std::vector<Indicator>(1, PenaltyCalculator::REISSUEFEE_FC));

    assertPenalty(FeeVec(1, getFee(200.0, NOREFUNDABLE)),
                  RefundPenalty::FC,
                  *_calculator->calculateInMixedScope(pu));
  }

  void testHundredPercentPenalty_singlePU_MIXED()
  {
    PricingUnit& pu = *setUpPercentagePenalties(std::vector<Percent>(1, 25.0));
    populateRecords3(*_vri,
                     &VoluntaryRefundsInfo::cancellationInd,
                     std::vector<Indicator>(1, PenaltyCalculator::HUNDRED_PERCENT_PENALTY));
    populateRecords3(*_vri,
                     &VoluntaryRefundsInfo::reissueFeeInd,
                     std::vector<Indicator>(1, PenaltyCalculator::REISSUEFEE_PU));

    assertPenalty(FeeVec(1, getFee(200.0, NOREFUNDABLE)),
                  RefundPenalty::PU,
                  *_calculator->calculateInMixedScope(pu));
  }

  void testHundredPercentPenalty_multiplePU_MIXED()
  {
    std::vector<Percent> percents;
    percents += 15.0, 30.0, 50.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    std::vector<Indicator> cancellation;
    cancellation += BLANK, PenaltyCalculator::HUNDRED_PERCENT_PENALTY, BLANK;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::cancellationInd, cancellation);

    std::vector<Indicator> reissue;
    reissue += PenaltyCalculator::REISSUEFEE_PU, PenaltyCalculator::REISSUEFEE_PU,
        PenaltyCalculator::REISSUEFEE_PU;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    assertPenalty(FeeVec(1, getFee(600.0, NOREFUNDABLE)),
                  RefundPenalty::PU,
                  *_calculator->calculateInMixedScope(pu));
  }

  void testHundredPercentPenalty_multipleFC_MIXED()
  {
    std::vector<Percent> percents;
    percents += 15.0, 30.0, 50.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    std::vector<Indicator> cancellation;
    cancellation += BLANK, PenaltyCalculator::HUNDRED_PERCENT_PENALTY, BLANK;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::cancellationInd, cancellation);

    std::vector<Indicator> reissue;
    reissue += PenaltyCalculator::REISSUEFEE_FC, PenaltyCalculator::REISSUEFEE_FC,
        PenaltyCalculator::REISSUEFEE_FC;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    assertPenalty(FeeVec(1, getFee(200.0, NOREFUNDABLE)),
                  RefundPenalty::FC,
                  *_calculator->calculateInMixedScope(pu));
  }

  void testHundredPercentPenalty_multipleFCandPU_MIXED_FCwins()
  {
    std::vector<Percent> percents;
    percents += 15.0, 30.0, 5.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    std::vector<Indicator> cancellation;
    cancellation += BLANK, PenaltyCalculator::HUNDRED_PERCENT_PENALTY, BLANK;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::cancellationInd, cancellation);

    std::vector<Indicator> reissue;
    reissue += PenaltyCalculator::REISSUEFEE_PU, PenaltyCalculator::REISSUEFEE_FC,
        PenaltyCalculator::REISSUEFEE_PU;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    assertPenalty(FeeVec(1, getFee(200.0, NOREFUNDABLE)),
                  RefundPenalty::FC,
                  *_calculator->calculateInMixedScope(pu));
  }

  void testHundredPercentPenalty_multipleFCandPU_MIXED_PUwins()
  {
    std::vector<Percent> percents;
    percents += 15.0, 30.0, 50.0;
    PricingUnit& pu = *setUpPercentagePenalties(percents);

    std::vector<Indicator> cancellation;
    cancellation += BLANK, PenaltyCalculator::HUNDRED_PERCENT_PENALTY, BLANK;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::cancellationInd, cancellation);

    std::vector<Indicator> reissue;
    reissue += PenaltyCalculator::REISSUEFEE_FC, PenaltyCalculator::REISSUEFEE_PU,
        PenaltyCalculator::REISSUEFEE_FC;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    assertPenalty(FeeVec(1, getFee(600.0, NOREFUNDABLE)),
                  RefundPenalty::PU,
                  *_calculator->calculateInMixedScope(pu));
  }

  struct Specified
  {
    Specified(const CurrencyCode& curr1,
              const MoneyAmount& amt1,
              const CurrencyCode& curr2,
              const MoneyAmount& amt2)
      : _curr1(curr1), _amt1(amt1), _curr2(curr2), _amt2(amt2)
    {
    }

    CurrencyCode _curr1;
    MoneyAmount _amt1;
    CurrencyCode _curr2;
    MoneyAmount _amt2;
  };

  PricingUnit* setUpSpecifiedPenalties(const std::vector<Specified>& specified)
  {
    _vri = _memH(new std::vector<VoluntaryRefundsInfo*>);
    for (unsigned i = 0; i < specified.size(); ++i)
    {
      *_vri += createRecord3();
      (*_vri)[i]->penalty1Cur() = specified[i]._curr1;
      (*_vri)[i]->penalty1Amt() = specified[i]._amt1;
      (*_vri)[i]->penalty2Cur() = specified[i]._curr2;
      (*_vri)[i]->penalty2Amt() = specified[i]._amt2;
      (*_vri)[i]->highLowInd() = PenaltyCalculator::HIGH;
    }

    return populateVrArray(*_vri);
  }

  void testSpecifiedPenalty_FCsecondCurrencyMatched()
  {
    std::vector<Specified> specified;
    specified.push_back(Specified(NUC, 30.0, PLN, 20.0));
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    assertPenalty(FeeVec(1, getFee(20.0)), RefundPenalty::FC, *_calculator->calculateInFcScope(pu));
  }

  void testSpecifiedPenalty_FCfirstCurrencyMatched()
  {
    std::vector<Specified> specified;
    specified.push_back(Specified(PLN, 10.0, NUC, 50.0));
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    assertPenalty(FeeVec(1, getFee(10.0)), RefundPenalty::FC, *_calculator->calculateInFcScope(pu));
  }

  void testSpecifiedPenalty_FCnoneCurrencyMatched()
  {
    std::vector<Specified> specified;
    specified.push_back(Specified(PLN, 10.0, "JPY", 20.0));
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    assertPenalty(FeeVec(1, getFee(10.0)), RefundPenalty::FC, *_calculator->calculateInFcScope(pu));
  }

  void testSpecifiedPenalty_FCzeroCurrencyMatched()
  {
    std::vector<Specified> specified;
    specified.push_back(Specified(PLN, 10.0, NUC, 0.0));
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    assertPenalty(FeeVec(1, getFee(10.0)), RefundPenalty::FC, *_calculator->calculateInFcScope(pu));
  }

  void testSpecifiedPenalty_FCfirstZeroCurrencyMatched()
  {
    std::vector<Specified> specified;
    specified.push_back(Specified(NUC, 0.0, NUC, 10.0));
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    assertPenalty(FeeVec(1, getFee(Money(0.0, NUC))),
                  RefundPenalty::FC,
                  *_calculator->calculateInMixedScope(pu));
  }

  void testSpecifiedPenalty_FCmultiple()
  {
    std::vector<Specified> specified;
    specified += Specified(NUC, 10.0, PLN, 20.0), Specified(PLN, 0.0, "JPY", 0.0),
        Specified(PLN, 10.0, "JPY", 20.0), Specified(NUC, 0.0, NUC, 10.0),
        Specified("JPY", 10.0, NUC, 20.0);
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    FeeVec expect;
    expect += getFee(20.0), getFee(0.0), getFee(10.0), getFee(Money(0.0, NUC)),
        getFee(Money(10.0, "JPY"));
    assertPenalty(expect, RefundPenalty::FC, *_calculator->calculateInFcScope(pu));
  }

  void testSpecifiedPenalty_PUzeroCurrencyMatched()
  {
    std::vector<Specified> specified;
    specified.push_back(Specified(NUC, 0.0, NUC, 0.0));
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    assertPenalty(FeeVec(1, getFee(Money(0.0, NUC))),
                  RefundPenalty::PU,
                  *_calculator->calculateInPuScope(pu));
  }

  void testSpecifiedPenalty_PUfirstCurrencyMatched()
  {
    std::vector<Specified> specified;
    specified.push_back(Specified(NUC, 20.0, NUC, 0.0));
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    assertPenalty(FeeVec(1, getFee(Money(20.0, NUC))),
                  RefundPenalty::PU,
                  *_calculator->calculateInPuScope(pu));
  }

  void setUpPercentForSpecified(const std::vector<Percent>& percents)
  {
    for (unsigned i = 0; i < percents.size(); ++i)
      (*_vri)[i]->penaltyPercent() = percents[i];
  }

  void testSpecifiedPenalty_PUPercentSpecified_winsPercent()
  {
    std::vector<Specified> specified;
    specified.push_back(Specified(NUC, 20.0, NUC, 0.0));
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    std::vector<Percent> percents(1, 35.0);
    setUpPercentForSpecified(percents);

    assertPenalty(FeeVec(1, getFee(70.0)), RefundPenalty::PU, *_calculator->calculateInPuScope(pu));
  }

  void testSpecifiedPenalty_PUPercentSpecified_winsSpecified()
  {
    std::vector<Specified> specified;
    specified.push_back(Specified(NUC, 20.0, NUC, 0.0));
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    std::vector<Percent> percents(1, 5.0);
    setUpPercentForSpecified(percents);

    assertPenalty(FeeVec(1, getFee(Money(20.0, NUC))),
                  RefundPenalty::PU,
                  *_calculator->calculateInPuScope(pu));
  }

  void testSpecifiedPenalty_PUPercentSpecifiedMultiple_winsPercent()
  {
    std::vector<Specified> specified;
    specified += Specified(NUC, 10.0, NUC, 0.0), Specified(NUC, 25.0, NUC, 0.0),
        Specified(NUC, 20.0, NUC, 0.0);
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    std::vector<Percent> percents;
    percents += 5.0, 40.0, 15.0;
    setUpPercentForSpecified(percents);

    assertPenalty(
        FeeVec(1, getFee(240.0)), RefundPenalty::PU, *_calculator->calculateInPuScope(pu));
  }

  void testSpecifiedPenalty_PUPercentSpecifiedMultiple_winsSpecified()
  {
    std::vector<Specified> specified;
    specified += Specified(NUC, 10.0, NUC, 0.0), Specified(NUC, 70.0, NUC, 0.0),
        Specified(NUC, 20.0, NUC, 0.0);
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    std::vector<Percent> percents;
    percents += 5.0, 10.0, 15.0;
    setUpPercentForSpecified(percents);

    assertPenalty(FeeVec(1, getFee(Money(70.0, NUC))),
                  RefundPenalty::PU,
                  *_calculator->calculateInPuScope(pu));
  }

  void testSpecifiedPenalty_MIXEDfc()
  {
    std::vector<Specified> specified;
    specified += Specified(NUC, 20.0, NUC, 0.0);
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    std::vector<Indicator> reissue(1, PenaltyCalculator::REISSUEFEE_FC);
    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    assertPenalty(FeeVec(1, getFee(Money(20.0, NUC))),
                  RefundPenalty::PU,
                  *_calculator->calculateInPuScope(pu));
  }

  void testSpecifiedPenalty_MIXEDpu()
  {
    std::vector<Specified> specified;
    specified += Specified(NUC, 20.0, NUC, 10.0);
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    std::vector<Indicator> reissue(1, PenaltyCalculator::REISSUEFEE_PU);
    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    assertPenalty(FeeVec(1, getFee(Money(20.0, NUC))),
                  RefundPenalty::PU,
                  *_calculator->calculateInPuScope(pu));
  }

  void testSpecifiedPenalty_MIXEDmultiple()
  {
    std::vector<Specified> specified;
    specified += Specified(NUC, 15.0, NUC, 0.0), Specified(NUC, 20.0, NUC, 0.0),
        Specified(NUC, 25.0, NUC, 0.0);
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    std::vector<Indicator> reissue;
    reissue += PenaltyCalculator::REISSUEFEE_FC, PenaltyCalculator::REISSUEFEE_PU,
        PenaltyCalculator::REISSUEFEE_FC;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    assertPenalty(FeeVec(1, getFee(Money(25.0, NUC))),
                  RefundPenalty::FC,
                  *_calculator->calculateInMixedScope(pu));
  }

  void testSpecifiedPenalty_MIXEDmultipleWithPercent_specWins()
  {
    std::vector<Specified> specified;
    specified += Specified(NUC, 10.0, NUC, 0.0), Specified(NUC, 25.0, NUC, 0.0),
        Specified(NUC, 20.0, NUC, 0.0);
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    std::vector<Indicator> reissue;
    reissue += PenaltyCalculator::REISSUEFEE_FC, PenaltyCalculator::REISSUEFEE_PU,
        PenaltyCalculator::REISSUEFEE_FC;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    std::vector<Percent> percents;
    percents += 5.0, 1.0, 1.0;
    setUpPercentForSpecified(percents);

    assertPenalty(FeeVec(1, getFee(Money(25.0, NUC))),
                  RefundPenalty::PU,
                  *_calculator->calculateInMixedScope(pu));
  }

  void testSpecifiedPenalty_MIXEDmultipleWithPercent_percentWins()
  {
    std::vector<Specified> specified;
    specified += Specified(NUC, 10.0, NUC, 0.0), Specified(NUC, 25.0, NUC, 0.0),
        Specified(NUC, 20.0, NUC, 0.0);
    PricingUnit& pu = *setUpSpecifiedPenalties(specified);

    std::vector<Indicator> reissue;
    reissue += PenaltyCalculator::REISSUEFEE_FC, PenaltyCalculator::REISSUEFEE_PU,
        PenaltyCalculator::REISSUEFEE_FC;
    populateRecords3(*_vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);

    std::vector<Percent> percents;
    percents += 5.0, 50.0, 15.0;
    setUpPercentForSpecified(percents);

    assertPenalty(
        FeeVec(1, getFee(300.0)), RefundPenalty::PU, *_calculator->calculateInMixedScope(pu));
  }

  FareUsage* createFareUsage(PaxTypeFare* ptf)
  {
    FareUsage* fu = _memH(new FareUsage);
    fu->paxTypeFare() = ptf;
    ptf->nucFareAmount() = 100.0;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, fu->totalFareAmount(), EPSILON);
    return fu;
  }

  PricingUnit* createPricingUnit(const std::vector<PaxTypeFare*>& ptf)
  {
    PricingUnit* pu = _memH(new PricingUnit);
    pu->setTotalPuNucAmount(0.0);
    for (unsigned i = 0; i < ptf.size(); ++i)
    {
      FareUsage* fu = createFareUsage(ptf[i]);
      pu->fareUsage() += fu;
      pu->setTotalPuNucAmount(pu->getTotalPuNucAmount() + fu->totalFareAmount());
    }
    return pu;
  }

  void populateFarePath(PricingUnit* pu) { _fp->pricingUnit() += pu; }

  void populateFarePath(const std::vector<PricingUnit*>& pu)
  {
    for (unsigned i = 0; i < pu.size(); ++i)
      populateFarePath(pu[i]);
  }

  PaxTypeFare* createPaxTypeFare()
  {
    PaxTypeFare* ptf = _memH(new PaxTypeFare);
    ptf->setFare(_memH(new Fare));
    return ptf;
  }

  void assertPricingUnit(const PricingUnit* pu, const std::vector<PaxTypeFare*>& ptf)
  {
    PenaltyCalculator::VoluntaryRefundsArray::const_iterator it = _calculator->_vrArray.find(pu);
    CPPUNIT_ASSERT(it != _calculator->_vrArray.end());

    typedef PenaltyCalculator::PuItems::const_iterator It;
    for (It i = it->second.begin(); i != it->second.end(); ++i)
      CPPUNIT_ASSERT(std::find(ptf.begin(), ptf.end(), i->first->paxTypeFare()) != ptf.end());
  }

  void testCreateVoluntaryRefundsArray()
  {
    std::vector<PaxTypeFare*> ptf_pu1, ptf_pu2;
    ptf_pu1 += createPaxTypeFare(), createPaxTypeFare();
    ptf_pu2 += createPaxTypeFare();

    std::vector<PricingUnit*> pu;
    pu += createPricingUnit(ptf_pu1), createPricingUnit(ptf_pu2);

    populateFarePath(pu);

    _calculator->_vrArray.create(*_fp);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _calculator->_vrArray.size());
    assertPricingUnit(pu[0], ptf_pu1);
    assertPricingUnit(pu[1], ptf_pu2);
  }

  VoluntaryRefundsInfo* createRecord3() { return _memH(new VoluntaryRefundsInfo); }

  RefundPermutation* createPermutation(const std::vector<PaxTypeFare*>& ptf,
                                       const std::vector<VoluntaryRefundsInfo*>& vri)
  {
    CPPUNIT_ASSERT_EQUAL(ptf.size(), vri.size());

    RefundPermutation* perm = _memH(new RefundPermutation);
    for (unsigned i = 0; i < ptf.size(); ++i)
    {
      RefundProcessInfo* info = _memH(new RefundProcessInfo);
      info->assign(vri[i], ptf[i], 0);
      perm->processInfos() += info;
    }

    return perm;
  }

  struct HasPtf
  {
    HasPtf(const PaxTypeFare* ptf) : _ptf(ptf) {}

    bool operator()(const PenaltyCalculator::PuItems::value_type& pair) const
    {
      return pair.first->paxTypeFare() == _ptf;
    }
    const PaxTypeFare* _ptf;
  };

  void assertVoluntaryRefunds(const PricingUnit* pu,
                              const PaxTypeFare* ptf,
                              const VoluntaryRefundsInfo* vri)
  {
    PenaltyCalculator::VoluntaryRefundsArray::const_iterator it = _calculator->_vrArray.find(pu);
    CPPUNIT_ASSERT(it != _calculator->_vrArray.end());

    typedef PenaltyCalculator::PuItems::const_iterator It;
    It fu = std::find_if(it->second.begin(), it->second.end(), HasPtf(ptf));
    CPPUNIT_ASSERT(fu != it->second.end());
    CPPUNIT_ASSERT_EQUAL(vri, fu->second);
  }

  void testUpdateVoluntaryRefundsArray_Pass()
  {
    std::vector<PaxTypeFare*> ptf, ptf_pu1, ptf_pu2;
    ptf += createPaxTypeFare(), createPaxTypeFare(), createPaxTypeFare();
    ptf_pu1 += ptf[0], ptf[1];
    ptf_pu2 += ptf[2];

    std::vector<PricingUnit*> pu;
    pu += createPricingUnit(ptf_pu1), createPricingUnit(ptf_pu2);

    populateFarePath(pu);

    _calculator->_vrArray.create(*_fp);

    std::vector<VoluntaryRefundsInfo*> vri;
    vri += createRecord3(), createRecord3(), createRecord3();

    RefundPermutation* per = createPermutation(ptf, vri);

    CPPUNIT_ASSERT(_calculator->_vrArray.update(*per));

    assertVoluntaryRefunds(pu[0], ptf[0], vri[0]);
    assertVoluntaryRefunds(pu[0], ptf[1], vri[1]);
    assertVoluntaryRefunds(pu[1], ptf[2], vri[2]);
  }

  void testUpdateVoluntaryRefundsArray_Fail()
  {
    std::vector<PaxTypeFare*> ptf, other_ptf;
    ptf += createPaxTypeFare();
    other_ptf += createPaxTypeFare();

    std::vector<PricingUnit*> pu;
    pu += createPricingUnit(ptf);

    populateFarePath(pu);

    _calculator->_vrArray.create(*_fp);

    std::vector<VoluntaryRefundsInfo*> vri;
    vri += createRecord3();

    RefundPermutation* per = createPermutation(other_ptf, vri);

    CPPUNIT_ASSERT(!_calculator->_vrArray.update(*per));
  }

  PricingUnit* populateVrArray(const std::vector<VoluntaryRefundsInfo*>& vri)
  {
    std::vector<PaxTypeFare*> ptf;
    for (unsigned i = 0; i < vri.size(); ++i)
      ptf += createPaxTypeFare();

    PricingUnit* pu = createPricingUnit(ptf);

    std::vector<PricingUnit*> puv;
    puv += pu;

    populateFarePath(puv);

    _calculator->_vrArray.create(*_fp);

    CPPUNIT_ASSERT(_calculator->_vrArray.update(*createPermutation(ptf, vri)));

    return pu;
  }

  typedef Indicator& (VoluntaryRefundsInfo::*Setter)();

  template <typename T>
  void populateRecords3(std::vector<VoluntaryRefundsInfo*>& vec,
                        T& (VoluntaryRefundsInfo::*set_method)(),
                        const std::vector<T>& bytes)
  {
    CPPUNIT_ASSERT_EQUAL(vec.size(), bytes.size());
    typedef typename std::vector<T>::const_iterator It;
    std::vector<VoluntaryRefundsInfo*>::iterator vri = vec.begin();
    for (It i = bytes.begin(); i != bytes.end(); ++i, ++vri)
      ((*vri)->*set_method)() = *i;
  }

  std::vector<VoluntaryRefundsInfo*> createVoulunrayRefundsVector(unsigned size)
  {
    std::vector<VoluntaryRefundsInfo*> vri;
    for (unsigned i = 0; i < size; ++i)
      vri += createRecord3();
    return vri;
  }

  PricingUnit*
  setupScope(const std::vector<Indicator>& reissue, const std::vector<Indicator>& options)
  {
    CPPUNIT_ASSERT_EQUAL(reissue.size(), options.size());

    std::vector<VoluntaryRefundsInfo*> vri = createVoulunrayRefundsVector(reissue.size());

    populateRecords3(vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);
    populateRecords3(vri, &VoluntaryRefundsInfo::calcOption, options);

    return populateVrArray(vri);
  }

  void testDetermineScope_PuOnly()
  {
    std::vector<Indicator> reissue, options;
    reissue += PenaltyCalculator::REISSUEFEE_PU, PenaltyCalculator::REISSUEFEE_PU;
    options += PenaltyCalculator::CALCOPTION_A, PenaltyCalculator::CALCOPTION_A;

    PricingUnit& pu = *setupScope(reissue, options);

    CPPUNIT_ASSERT_EQUAL(PenaltyCalculator::PU_SCOPE, _calculator->determineScope(pu));
  }

  void testDetermineScope_FcOnly()
  {
    std::vector<Indicator> reissue, options;
    reissue += PenaltyCalculator::REISSUEFEE_FC, PenaltyCalculator::REISSUEFEE_FC;
    options += PenaltyCalculator::CALCOPTION_A, PenaltyCalculator::CALCOPTION_A;

    PricingUnit& pu = *setupScope(reissue, options);

    CPPUNIT_ASSERT_EQUAL(PenaltyCalculator::FC_SCOPE, _calculator->determineScope(pu));
  }

  void testDetermineScope_Mixed_OptionAOnly()
  {
    std::vector<Indicator> reissue, options;
    reissue += PenaltyCalculator::REISSUEFEE_PU, PenaltyCalculator::REISSUEFEE_FC;
    options += PenaltyCalculator::CALCOPTION_A, PenaltyCalculator::CALCOPTION_A;

    PricingUnit& pu = *setupScope(reissue, options);

    CPPUNIT_ASSERT_EQUAL(PenaltyCalculator::PU_SCOPE, _calculator->determineScope(pu));
  }

  void testDetermineScope_Mixed_OptionBOnly()
  {
    std::vector<Indicator> reissue, options;
    reissue += PenaltyCalculator::REISSUEFEE_PU, PenaltyCalculator::REISSUEFEE_FC;
    options += PenaltyCalculator::CALCOPTION_B, PenaltyCalculator::CALCOPTION_B;

    PricingUnit& pu = *setupScope(reissue, options);

    CPPUNIT_ASSERT_EQUAL(PenaltyCalculator::MX_SCOPE, _calculator->determineScope(pu));
  }

  void testDetermineScope_Mixed_Mixed()
  {
    std::vector<Indicator> reissue, options;
    reissue += PenaltyCalculator::REISSUEFEE_PU, PenaltyCalculator::REISSUEFEE_FC;
    options += PenaltyCalculator::CALCOPTION_A, PenaltyCalculator::CALCOPTION_B;

    PricingUnit& pu = *setupScope(reissue, options);

    CPPUNIT_ASSERT_EQUAL(PenaltyCalculator::PU_SCOPE, _calculator->determineScope(pu));
  }

  PenaltyCalculator::PuItems::value_type* createPuItem()
  {
    return _memH(new PenaltyCalculator::PuItems::value_type(createFareUsage(createPaxTypeFare()),
                                                            createRecord3()));
  }

  PricingUnit* setupHiLo(const std::vector<Indicator>& hilo, PenaltyCalculator::Scope scope)
  {
    std::vector<MoneyAmount> amount(2, 40.00);
    std::vector<CurrencyCode> curr(2, NUC);
    std::vector<Percent> percent(2, 25.00);
    std::vector<Indicator> reissue(2, (scope == PenaltyCalculator::PU_SCOPE ? 'P' : 'F'));

    std::vector<VoluntaryRefundsInfo*> vri = createVoulunrayRefundsVector(2);

    populateRecords3(vri, &VoluntaryRefundsInfo::penalty1Amt, amount);
    populateRecords3(vri, &VoluntaryRefundsInfo::penalty1Cur, curr);
    populateRecords3(vri, &VoluntaryRefundsInfo::penaltyPercent, percent);
    populateRecords3(vri, &VoluntaryRefundsInfo::reissueFeeInd, reissue);
    populateRecords3(vri, &VoluntaryRefundsInfo::highLowInd, hilo);

    return populateVrArray(vri);
  }

  void testHiLoByte_FCscope()
  {
    std::vector<Indicator> hilo;
    hilo += PenaltyCalculator::HIGH, PenaltyCalculator::LOW;

    PricingUnit& pu = *setupHiLo(hilo, PenaltyCalculator::FC_SCOPE);

    FeeVec expect;
    expect += getFee(Money(40.0, NUC), REFUNDABLE), getFee(50.0, REFUNDABLE);
    assertPenalty(expect, RefundPenalty::FC, *_calculator->calculateInFcScope(pu));
  }

  void testHiLoByte_PUscope_specWins()
  {
    std::vector<Indicator> hilo;
    hilo += PenaltyCalculator::LOW, BLANK;

    PricingUnit& pu = *setupHiLo(hilo, PenaltyCalculator::PU_SCOPE);

    assertPenalty(FeeVec(1, getFee(Money(40.0, NUC))),
                  RefundPenalty::PU,
                  *_calculator->calculateInPuScope(pu));
  }

  void testHiLoByte_PUscope_percWins()
  {
    std::vector<Indicator> hilo;
    hilo += PenaltyCalculator::HIGH, BLANK;

    PricingUnit& pu = *setupHiLo(hilo, PenaltyCalculator::PU_SCOPE);

    assertPenalty(
        FeeVec(1, getFee(100.0)), RefundPenalty::PU, *_calculator->calculateInPuScope(pu));
  }

  void testHiLoByte_MXscope_specWins()
  {
    std::vector<Indicator> hilo;
    hilo += PenaltyCalculator::HIGH, BLANK;

    PricingUnit& pu = *setupHiLo(hilo, PenaltyCalculator::MX_SCOPE);

    assertPenalty(FeeVec(1, getFee(Money(40.0, NUC))),
                  RefundPenalty::FC,
                  *_calculator->calculateInMixedScope(pu));
  }

  void testHiLoByte_MXscope_percWins()
  {
    std::vector<Indicator> hilo;
    hilo += PenaltyCalculator::LOW, BLANK;

    PricingUnit& pu = *setupHiLo(hilo, PenaltyCalculator::MX_SCOPE);

    assertPenalty(
        FeeVec(1, getFee(50.0)), RefundPenalty::FC, *_calculator->calculateInMixedScope(pu));
  }

  void testDetermineMethod_OneHundredPenalty()
  {
    VoluntaryRefundsInfo info;
    info.cancellationInd() = PenaltyCalculator::HUNDRED_PERCENT_PENALTY;

    CPPUNIT_ASSERT_EQUAL(PenaltyCalculator::HNDR_MTH, _calculator->determineMethod(info));
  }

  void testDetermineMethod_SepecificAmount()
  {
    VoluntaryRefundsInfo info;
    info.cancellationInd() = BLANK;
    info.penalty1Cur() = NUC;
    info.penaltyPercent() = 0.0;
    info.highLowInd() = BLANK;

    CPPUNIT_ASSERT_EQUAL(PenaltyCalculator::SPEC_MTH, _calculator->determineMethod(info));
  }

  void testDetermineMethod_PercentagePenalty()
  {
    VoluntaryRefundsInfo info;
    info.cancellationInd() = BLANK;
    info.penalty1Cur() = "";
    info.penaltyPercent() = 10.0;
    info.highLowInd() = BLANK;

    CPPUNIT_ASSERT_EQUAL(PenaltyCalculator::PERC_MTH, _calculator->determineMethod(info));
  }

  void testDetermineMethod_HighLowPenalty()
  {
    VoluntaryRefundsInfo info;
    info.cancellationInd() = BLANK;
    info.penalty1Cur() = NUC;
    info.penaltyPercent() = 10.0;
    info.highLowInd() = PenaltyCalculator::HIGH;

    CPPUNIT_ASSERT_EQUAL(PenaltyCalculator::HILO_MTH, _calculator->determineMethod(info));
  }

  void testDetermineMethod_NoPenalty()
  {
    VoluntaryRefundsInfo info;
    info.cancellationInd() = BLANK;

    CPPUNIT_ASSERT_EQUAL(PenaltyCalculator::ZERO_MTH, _calculator->determineMethod(info));
  }

  enum
  {
    NODISCOUNT = 0,
    DISCOUNT = 1
  };
  enum
  {
    NOREFUNDABLE = 1,
    REFUNDABLE = 0
  };

  void testCalculationFeeOperatorLess()
  {
    Money ignored(100.0, NUC);
    PenaltyCalculator::CalculationFee small(ignored, 100.0, NODISCOUNT, REFUNDABLE),
        large(ignored, 120.0, NODISCOUNT, REFUNDABLE);

    CPPUNIT_ASSERT(small < large);
    CPPUNIT_ASSERT(!(large < small));
  }

  void testCalculationFeeOperatorLess_withNoRefundable()
  {
    Money ignored(100.0, NUC);

    PenaltyCalculator::CalculationFee small(ignored, 100.0, NODISCOUNT, REFUNDABLE),
        large(ignored, 100.0, NODISCOUNT, NOREFUNDABLE);

    CPPUNIT_ASSERT(small < large);
    CPPUNIT_ASSERT(!(large < small));
  }

  void testCalculationFeeOperatorLess_equal()
  {
    Money ignored(100.0, NUC);
    PenaltyCalculator::CalculationFee small(ignored, 100.0, NODISCOUNT, REFUNDABLE),
        large(ignored, 100.0, NODISCOUNT, REFUNDABLE);

    CPPUNIT_ASSERT(!(small < large));
    CPPUNIT_ASSERT(!(large < small));
  }

  void testCalculationFeeOperatorLess_equal_withNoRefundable()
  {
    Money ignored(100.0, NUC);
    PenaltyCalculator::CalculationFee small(ignored, 100.0, NODISCOUNT, NOREFUNDABLE),
        large(ignored, 100.0, NODISCOUNT, NOREFUNDABLE);

    CPPUNIT_ASSERT(!(small < large));
    CPPUNIT_ASSERT(!(large < small));
  }

  RefundPenalty* createPenalty(const FeeVec& feeVec)
  {
    RefundPenalty* pen = _memH.insert(new RefundPenalty);
    pen->assign(feeVec, RefundPenalty::FC);
    return pen;
  }

  void testSetTotalPenalty_Zero()
  {
    RefundPermutation perm;

    _calculator->setTotalPenalty(perm);

    CPPUNIT_ASSERT_EQUAL(Money(0.0, PLN), perm.totalPenalty());
  }

  void testSetTotalPenalty()
  {
    FeeVec fees1 = {Money(100.0, PLN), Money(100.0, PLN)};
    FeeVec fees2 = {Money(100.0, PLN)};

    PricingUnit pu1, pu2;

    RefundPermutation perm;
    perm.penaltyFees()[&pu1] = createPenalty(fees1);
    perm.penaltyFees()[&pu2] = createPenalty(fees2);

    _calculator->setTotalPenalty(perm);

    CPPUNIT_ASSERT_EQUAL(Money(300.0, PLN), perm.totalPenalty());
  }

  void testSetHighestPenalty()
  {
    FeeVec fees1 = {Money(110.0, PLN), Money(150.0, PLN), Money(90.0, PLN)};
    FeeVec fees2 = {Money(80.0, PLN), Money(170.0, PLN), Money(40.0, PLN)};

    PricingUnit pu1, pu2;

    RefundPermutation perm;
    perm.penaltyFees()[&pu1] = createPenalty(fees1);
    perm.penaltyFees()[&pu2] = createPenalty(fees2);

    _calculator->setHighestPenalty(perm);

    CPPUNIT_ASSERT_EQUAL(Money(110.0, PLN), perm.penaltyFees()[&pu1]->fee()[0].amount());
    CPPUNIT_ASSERT_EQUAL(Money(150.0, PLN), perm.penaltyFees()[&pu1]->fee()[1].amount());
    CPPUNIT_ASSERT_EQUAL(Money(90.0, PLN), perm.penaltyFees()[&pu1]->fee()[2].amount());
    CPPUNIT_ASSERT_EQUAL(Money(80.0, PLN), perm.penaltyFees()[&pu2]->fee()[0].amount());
    CPPUNIT_ASSERT_EQUAL(Money(170.0, PLN), perm.penaltyFees()[&pu2]->fee()[1].amount());
    CPPUNIT_ASSERT_EQUAL(Money(40.0, PLN), perm.penaltyFees()[&pu2]->fee()[2].amount());

    CPPUNIT_ASSERT(!perm.penaltyFees()[&pu1]->fee()[0].highest());
    CPPUNIT_ASSERT(!perm.penaltyFees()[&pu1]->fee()[1].highest());
    CPPUNIT_ASSERT(!perm.penaltyFees()[&pu1]->fee()[2].highest());
    CPPUNIT_ASSERT(!perm.penaltyFees()[&pu2]->fee()[0].highest());
    CPPUNIT_ASSERT(perm.penaltyFees()[&pu2]->fee()[1].highest());
    CPPUNIT_ASSERT(!perm.penaltyFees()[&pu2]->fee()[2].highest());
  }

  template <typename T, typename R>
  RefundPermutation& populateByteInRecords3(T& (VoluntaryRefundsInfo::*set_method1)(),
                                            const std::vector<T>& bytes1,
                                            R& (VoluntaryRefundsInfo::*set_method2)(),
                                            const std::vector<R>& bytes2)
  {
    CPPUNIT_ASSERT_EQUAL(bytes1.size(), bytes2.size());
    RefundPermutation* perm = _memH(new RefundPermutation);

    typedef typename std::vector<T>::const_iterator It;
    typename std::vector<R>::const_iterator j = bytes2.begin();
    for (It i = bytes1.begin(); i != bytes1.end(); ++i, ++j)
    {
      VoluntaryRefundsInfo* vri = _memH(new VoluntaryRefundsInfo);
      (vri->*set_method1)() = *i;
      (vri->*set_method2)() = *j;
      RefundProcessInfo* info = _memH(new RefundProcessInfo);
      info->assign(vri, _memH(new PaxTypeFare), 0);
      perm->processInfos().push_back(info);
    }
    return *perm;
  }

  void testSetMinimumPenalty_Zero()
  {
    std::vector<MoneyAmount> amt = {20, 30, 10};
    std::vector<CurrencyCode> curr = {PLN, PLN, PLN};

    RefundPermutation& perm = populateByteInRecords3(
        &VoluntaryRefundsInfo::minimumAmt, amt, &VoluntaryRefundsInfo::minimumAmtCur, curr);

    perm.totalPenalty() = Money(100, NUC);
    _calculator->setMinimumPenalty(perm);

    CPPUNIT_ASSERT_EQUAL(Money(0.0, NUC), perm.minimumPenalty());
  }

  void testSetMinimumPenalty()
  {
    std::vector<MoneyAmount> amt = {20, 30, 10};
    std::vector<CurrencyCode> curr = {PLN, PLN, PLN};

    RefundPermutation& perm = populateByteInRecords3(
        &VoluntaryRefundsInfo::minimumAmt, amt, &VoluntaryRefundsInfo::minimumAmtCur, curr);

    perm.totalPenalty() = Money(10, NUC);
    _calculator->setMinimumPenalty(perm);

    CPPUNIT_ASSERT_EQUAL(Money(30.0, PLN), perm.minimumPenalty());
  }

  void testSetMinimumPenalty_noCurrency()
  {
    std::vector<MoneyAmount> amt = {20, 30, 10};
    std::vector<CurrencyCode> curr = {"", "", ""};

    RefundPermutation& perm = populateByteInRecords3(
        &VoluntaryRefundsInfo::minimumAmt, amt, &VoluntaryRefundsInfo::minimumAmtCur, curr);

    perm.totalPenalty() = Money(100, NUC);
    _calculator->setMinimumPenalty(perm);

    CPPUNIT_ASSERT_EQUAL(Money(0.0, NUC), perm.minimumPenalty());
  }

  void testSetMinimumPenalty_noRec3()
  {
    RefundPermutation perm;
    perm.totalPenalty() = Money(100, NUC);
    _calculator->setMinimumPenalty(perm);

    CPPUNIT_ASSERT_EQUAL(Money(0.0, NUC), perm.minimumPenalty());
  }

  void testScopedFeeComparator()
  {
    Money ignoredMoney(100.0, NUC);
    RefundPenalty::Scope ignoredScope = RefundPenalty::FC;

    PenaltyCalculator::ScopedFee smallScopedFee = std::make_pair(
        PenaltyCalculator::CalculationFee(ignoredMoney, 100.0, NODISCOUNT, REFUNDABLE),
        ignoredScope);

    PenaltyCalculator::ScopedFee largeScopedFee = std::make_pair(
        PenaltyCalculator::CalculationFee(ignoredMoney, 120.0, NODISCOUNT, REFUNDABLE),
        ignoredScope);

    CPPUNIT_ASSERT(smallScopedFee < largeScopedFee);
    CPPUNIT_ASSERT(!(smallScopedFee > largeScopedFee));
  }
};

const CurrencyCode PenaltyCalculatorTest::PLN = "PLN";
const Indicator PenaltyCalculatorTest::BLANK = ' ';

CPPUNIT_TEST_SUITE_REGISTRATION(PenaltyCalculatorTest);

std::ostream& operator<<(std::ostream& os, const RefundPenalty::Fee& fee)
{
  return os << "(" << fee.amount() << "," << fee.nonRefundable() << ")";
}

bool operator==(const RefundPenalty::Fee& l, const RefundPenalty::Fee& r)
{
  return ((l.amount().value() == r.amount().value()) && (l.amount().code() == r.amount().code()) &&
          (l.nonRefundable() == r.nonRefundable()));
}
}
