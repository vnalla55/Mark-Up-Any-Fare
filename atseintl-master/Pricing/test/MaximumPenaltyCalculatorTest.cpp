//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ProcessTagInfo.h"
#include "Diagnostic/DiagManager.h"
#include "DBAccess/ReissueSequence.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Pricing/MaximumPenaltyCalculator.h"
#include "Pricing/Cat31PenaltyEstimator.h"
#include "RexPricing/ReissuePenaltyCalculator.h"
#include "Rules/VoluntaryRefunds.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

#include <vector>

namespace tse
{
class MaximumPenaltyCalculatorTest : public CppUnit::TestFixture
{
  class MyDataHandle : public DataHandleMock
  {
  public:
    const std::vector<ReissueSequence*>&
    getReissue(const VendorCode& vendor,
               int itemNo,
               const DateTime& date,
               const DateTime& applDate = DateTime::emptyDate())
    {
      return _reissueSequence;
    }

    std::vector<ReissueSequence*> _reissueSequence;
  };

  CPPUNIT_TEST_SUITE(MaximumPenaltyCalculatorTest);
  CPPUNIT_TEST(testAnalyzeMaxFromAllFCs);
  CPPUNIT_TEST(testSumMaxFromFirstComponentWhenAppl3);
  CPPUNIT_TEST(testSumMaxFromSecondComponentWhenAppl3);
  CPPUNIT_TEST(testSumMaxWhenAppl3Before);
  CPPUNIT_TEST(testSumMaxWhenAppl3After);
  CPPUNIT_TEST(testSumMaxWhenAppl3NoMaximumFee);
  CPPUNIT_TEST(testPartitionBefore);
  CPPUNIT_TEST(testPartitionAfter);
  CPPUNIT_TEST(testPartitionBeforeEmpty);
  CPPUNIT_TEST(testPartitionAfterEmpty);
  CPPUNIT_TEST(testNoR3Both);
  CPPUNIT_TEST(testNoR3Before);
  CPPUNIT_TEST(testNoR3After);
  CPPUNIT_TEST(testNonJCxr);
  CPPUNIT_TEST(testNonJ);
  CPPUNIT_TEST(testNonP);
  CPPUNIT_TEST(testNonNSingle);
  CPPUNIT_TEST(testNonNMulti);
  CPPUNIT_TEST(testNonNWithChangeable);
  CPPUNIT_TEST(testNonTag8);
  CPPUNIT_TEST(testRefundPermutationBeforeFirstPUApplication);
  CPPUNIT_TEST(testRefundPermutationBeforeSecondPUApplication);
  CPPUNIT_TEST(testRefundPermutationBeforeFirstFCApplication);
  CPPUNIT_TEST(testRefundPermutationBeforeSecondFCApplication);
  CPPUNIT_TEST(testAllRecordsPartiallyFlownProcessing_firstFC);
  CPPUNIT_TEST(testAllRecordsPartiallyFlownProcessing_lastFC);
  CPPUNIT_TEST(testAllRecordsPartiallyFlownprocessing_noPartiallyFlownRecords_fail);
  CPPUNIT_TEST(testAllRecordsPartiallyFlownProcessing_fail);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  MyDataHandle* _myDataHandle;
  VoluntaryChangesInfoW* _r3w;
  VoluntaryChangesInfo* _r3;
  FarePath* _fp;
  FareUsage* _fu;
  PricingTrx* _trx;
  DiagManager* _diag;
  MaximumPenaltyCalculator* _mpCalc;
  std::vector<ReissuePenaltyCalculator::FcFees>* _fees;

public:
  void setUpFarePath()
  {
    _fp = _memHandle(new FarePath);
    _fp->calculationCurrency() = NUC;
  }

  void setUpComponent()
  {
    _fu = _memHandle(new FareUsage);
    _fu->paxTypeFare() = _memHandle(new PaxTypeFare);
    FareMarket* fm = _memHandle(new FareMarket);
    fm->fareBasisCode() = "COCKEYED";
    fm->fareCalcFareAmt() = "CHICKEN";
    _fu->paxTypeFare()->fareMarket() = fm;
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _myDataHandle = _memHandle(new MyDataHandle);
    _myDataHandle->_reissueSequence.push_back(_memHandle(new ReissueSequence));

    _r3w = _memHandle(new VoluntaryChangesInfoW);
    _r3 = _memHandle(new VoluntaryChangesInfo);
    _r3w->orig() = _r3;

    _fees = _memHandle(new std::vector<ReissuePenaltyCalculator::FcFees>);
    _trx = _memHandle(new PricingTrx);
    _diag = _memHandle.create<DiagManager>(*_trx);
    setUpFarePath();
    _mpCalc = _memHandle(new MaximumPenaltyCalculator(*_trx, *_fp));
    _fp->pricingUnit().push_back(_memHandle(new PricingUnit));
    _fp->itin() = _memHandle(new Itin);
    setUpComponent();
  }

  void tearDown() { _memHandle.clear(); }

  void constructComponent(std::vector<MoneyAmount> values, std::vector<Indicator> applications)
  {
    _fees->push_back(ReissuePenaltyCalculator::FcFees());
    std::vector<ReissuePenaltyCalculator::FcFees>::size_type fareComponentIndex = 0;
    for (const MoneyAmount& value : values)
    {
      _fees->back().push_back(std::make_tuple(value, applications[fareComponentIndex], _r3w));
      ++fareComponentIndex;
    }

    _fp->pricingUnit().front()->fareUsage().push_back(_fu);
    _fp->itin()->travelSeg().push_back(_memHandle(new AirSeg()));
  }

  void callAndAssert(bool nonBef,
                     bool nonAft,
                     const boost::optional<MoneyAmount>& feeBef,
                     const boost::optional<MoneyAmount>& feeAft,
                     smp::RecordApplication departureInd = smp::BOTH)
  {
    MaxPenaltyResponse::Fees result =
        Cat31PenaltyEstimator(NUC, *_fp, *_trx, DiagManager(*_trx)).estimate(*_fees, departureInd);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("NON BEF", nonBef, result._before._non);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("NON AFT", nonAft, result._after._non);

    if (feeBef.is_initialized())
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("FEE BEF", *feeBef, result._before._fee.get().value(), EPSILON);
    else
      CPPUNIT_ASSERT_MESSAGE("FEE BEF", !result._before._fee);

    if (feeAft.is_initialized())
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("FEE AFT", *feeAft, result._after._fee.get().value(), EPSILON);
    else
      CPPUNIT_ASSERT_MESSAGE("FEE AFT", !result._after._fee);
  }

  void testNoR3Both()
  {
    constructComponent({}, {});
    callAndAssert(true, true, {}, {});
  }

  void testNoR3Before()
  {
    constructComponent({}, {});
    callAndAssert(true, true, {}, {}, smp::BEFORE);
  }

  void testNoR3After()
  {
    constructComponent({}, {});
    callAndAssert(true, true, {}, {}, smp::AFTER);
  }

  void testAnalyzeMaxFromAllFCs()
  {
    constructComponent({190.0, 150.0, 200.0}, {'1', '1', '1'});
    constructComponent({150.0, 100.0, 0.0}, {'1', '1', '1'});
    callAndAssert(false, false, 200.0, 200.0);
  }

  void testSumMaxFromFirstComponentWhenAppl3()
  {
    constructComponent({190.0, 150.0, 200.0}, {'1', '3', '2'});
    constructComponent({150.0, 100.0, 0.0}, {'1', '2', '2'});

    callAndAssert(false, false, 300.0, 300.0);
  }

  void testSumMaxFromSecondComponentWhenAppl3()
  {
    constructComponent({190.0, 150.0, 200.0}, {'1', '2', '2'});
    constructComponent({150.0, 100.0, 0.0}, {'1', '2', '3'});

    callAndAssert(false, false, 200.0, 200.0);
  }

  void testSumMaxWhenAppl3Before()
  {
    _r3->departureInd() = 'B';
    constructComponent({50.0}, {'3'});
    constructComponent({100.0}, {'1'});
    callAndAssert(false, true, 150.0, {}, smp::BEFORE);
  }

  void testSumMaxWhenAppl3After()
  {
    _r3->departureInd() = 'A';
    constructComponent({50.0, 100.0}, {'3', '2'});
    constructComponent({100.0, 50.0, 100.0}, {'1', '1', '3'});
    callAndAssert(true, false, {}, 200.0, smp::AFTER);
  }

  void testSumMaxWhenAppl3NoMaximumFee()
  {
    constructComponent({100.0, 100.0}, {'3', '1'});
    constructComponent({100.0, 100.0}, {'1', '1'});
    callAndAssert(false, false, 200.0, 200.0);
  }

  void testSumMaxWhenAppl3SmallestFee()
  {
    constructComponent({50.0, 100.0}, {'3', '1'});
    constructComponent({100.0, 50.0}, {'1', '3'});
    callAndAssert(false, false, 200.0, 200.0);
  }

  void testPartitionBefore()
  {
    _r3->departureInd() = 'B';
    constructComponent({170.0}, {'1'});
    callAndAssert(false, true, 170.0, {}, smp::BEFORE);
  }

  void testPartitionAfter()
  {
    _r3->departureInd() = 'A';
    constructComponent({170.0}, {'1'});
    callAndAssert(true, true, {}, {}, smp::AFTER);
  }

  void testPartitionAfterEmpty()
  {
    _r3->departureInd() = 'B';
    constructComponent({170.0}, {'1'});
    callAndAssert(true, true, {}, {}, smp::AFTER);
  }

  void testPartitionBeforeEmpty()
  {
    _r3->departureInd() = 'A';
    constructComponent({170.0}, {'1'});
    callAndAssert(true, true, {}, {}, smp::BEFORE);
  }

  void testNonJCxr()
  {
    _r3->changeInd() = 'J';
    constructComponent({170.0}, {'1'});
    callAndAssert(true, true, {}, {});
  }

  void testNonJ()
  {
    _r3->changeInd() = 'J';
    _fp->pricingUnit().front()->setTotalPuNucAmount(650.0);
    _fu->paxTypeFare()->fareMarket()->governingCarrier() = CARRIER_9B;
    constructComponent({170.0}, {'1'});
    setUpComponent();
    _fu->paxTypeFare()->fareMarket()->governingCarrier() = CARRIER_JJ;
    constructComponent({250.0}, {'1'});
    callAndAssert(false, false, 650.0, 650.0);
  }

  void testNonP()
  {
    _r3->changeInd() = 'P';
    _fp->pricingUnit().front()->setTotalPuNucAmount(75.0);
    constructComponent({50.0}, {'1'});
    callAndAssert(true, true, 75.0, {});
  }

  void testNonNSingle()
  {
    _r3->changeInd() = 'N';
    constructComponent({45.0}, {'1'});
    _fp->pricingUnit().front()->fareUsage().front()->surchargeAmt() = 65.0;
    callAndAssert(true, true, {}, {});
  }

  void testNonNMulti()
  {
    _r3->changeInd() = 'N';
    constructComponent({45.0}, {'1'});
    _fp->pricingUnit().front()->fareUsage().front()->surchargeAmt() = 65.0;

    callAndAssert(true, true, {}, {});

    _r3w = _memHandle(new VoluntaryChangesInfoW);
    _r3 = _memHandle(new VoluntaryChangesInfo);
    _r3w->orig() = _r3;
    _r3->changeInd() = 'N';
    constructComponent({45.0}, {'1'});
    _fp->pricingUnit().front()->fareUsage().front()->surchargeAmt() = 65.0;

    callAndAssert(true, true, {}, {});
  }

  void testNonNWithChangeable()
  {
    _r3->changeInd() = 'N';
    constructComponent({100.0}, {'1'});

    callAndAssert(true, true, {}, {});

    _r3w = _memHandle(new VoluntaryChangesInfoW);
    _r3 = _memHandle(new VoluntaryChangesInfo);
    _r3w->orig() = _r3;

    constructComponent({10.0}, {'1'});

    callAndAssert(true, true, 10.0, 10.0);
  }

  void testNonTag8()
  {
    _myDataHandle->_reissueSequence.back()->processingInd() = CANCEL_AND_START_OVER;
    constructComponent({170.0}, {'1'});
    callAndAssert(true, true, {}, {});
  }

  void testWaivedR3()
  {
    _r3->waiverTblItemNo() = 1;
    constructComponent({100}, {'1'});
    callAndAssert(true, true, {}, {});
  }

  void testRefundPermutationBeforeFirstPUApplication()
  {
    auto first_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    first_r3->puInd() = smp::DepartureAppl::BEFORE;
    auto first_ptf = _memHandle.create<PaxTypeFare>();
    auto second_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    auto second_ptf = _memHandle.create<PaxTypeFare>();

    RefundProcessInfo rpi1{first_r3, first_ptf, nullptr};
    RefundProcessInfo rpi2{second_r3, second_ptf, nullptr};

    MaximumPenaltyCalculator::PtfPuMap ptfPuMap;
    ptfPuMap[first_ptf] = 0;
    ptfPuMap[second_ptf] = 0;

    RefundPermutation perm;
    std::deque<RefundProcessInfo*> rpis{&rpi1, &rpi2};
    perm.assign(0, rpis);

    auto status = _mpCalc->refundPermutationAppl(perm, ptfPuMap);
    CPPUNIT_ASSERT_EQUAL(smp::BEFORE, status);
  }

  void testRefundPermutationBeforeSecondPUApplication()
  {
    auto first_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    auto first_ptf = _memHandle.create<PaxTypeFare>();
    auto second_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    second_r3->puInd() = smp::DepartureAppl::BEFORE;
    auto second_ptf = _memHandle.create<PaxTypeFare>();

    RefundProcessInfo rpi1{first_r3, first_ptf, nullptr};
    RefundProcessInfo rpi2{second_r3, second_ptf, nullptr};

    MaximumPenaltyCalculator::PtfPuMap ptfPuMap;
    ptfPuMap[first_ptf] = 0;
    ptfPuMap[second_ptf] = 1;

    RefundPermutation perm;
    std::deque<RefundProcessInfo*> rpis{&rpi1, &rpi2};
    perm.assign(0, rpis);

    auto status = _mpCalc->refundPermutationAppl(perm, ptfPuMap);
    CPPUNIT_ASSERT_EQUAL(smp::BOTH, status);
  }

  void testRefundPermutationBeforeFirstFCApplication()
  {
    auto first_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    first_r3->fareComponentInd() = smp::DepartureAppl::BEFORE;
    auto first_ptf = _memHandle.create<PaxTypeFare>();
    auto second_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    auto second_ptf = _memHandle.create<PaxTypeFare>();

    RefundProcessInfo rpi1{first_r3, first_ptf, nullptr};
    RefundProcessInfo rpi2{second_r3, second_ptf, nullptr};

    MaximumPenaltyCalculator::PtfPuMap ptfPuMap;
    ptfPuMap[first_ptf] = 0;
    ptfPuMap[second_ptf] = 0;

    RefundPermutation perm;
    std::deque<RefundProcessInfo*> rpis{&rpi1, &rpi2};
    perm.assign(0, rpis);

    auto status = _mpCalc->refundPermutationAppl(perm, ptfPuMap);
    CPPUNIT_ASSERT_EQUAL(smp::BEFORE, status);
  }

  void testRefundPermutationBeforeSecondFCApplication()
  {
    auto first_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    auto first_ptf = _memHandle.create<PaxTypeFare>();
    auto second_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    second_r3->fareComponentInd() = smp::DepartureAppl::BEFORE;
    auto second_ptf = _memHandle.create<PaxTypeFare>();

    RefundProcessInfo rpi1{first_r3, first_ptf, nullptr};
    RefundProcessInfo rpi2{second_r3, second_ptf, nullptr};

    MaximumPenaltyCalculator::PtfPuMap ptfPuMap;
    ptfPuMap[first_ptf] = 0;
    ptfPuMap[second_ptf] = 0;

    RefundPermutation perm;
    std::deque<RefundProcessInfo*> rpis{&rpi1, &rpi2};
    perm.assign(0, rpis);

    auto status = _mpCalc->refundPermutationAppl(perm, ptfPuMap);
    CPPUNIT_ASSERT_EQUAL(smp::BOTH, status);
  }

  void testAllRecordsPartiallyFlownProcessing_firstFC()
  {
    auto first_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    first_r3->fullyFlown() = VoluntaryRefunds::PARTIALLY_FLOWN;
    first_r3->depOfJourneyInd() = smp::DepartureAppl::AFTER;
    auto second_r3 = _memHandle.create<VoluntaryRefundsInfo>();

    auto first_ptf = _memHandle.create<PaxTypeFare>();
    auto second_ptf = _memHandle.create<PaxTypeFare>();

    MaximumPenaltyCalculator::PtfPuMap ptfPuMap;
    ptfPuMap[first_ptf] = 0;
    ptfPuMap[second_ptf] = 0;

    RefundProcessInfo rpi1{first_r3, first_ptf, nullptr};
    RefundProcessInfo rpi2{second_r3, second_ptf, nullptr};

    RefundPermutationGenerator::FCtoSequence seqsToFc;
    seqsToFc[0] = {&rpi1};
    seqsToFc[1] = {&rpi2};

    CPPUNIT_ASSERT(_mpCalc->isAnyFCPartiallyFlownAfter(seqsToFc, ptfPuMap));
  }

  void testAllRecordsPartiallyFlownProcessing_lastFC()
  {
    auto first_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    auto second_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    second_r3->fullyFlown() = VoluntaryRefunds::PARTIALLY_FLOWN;
    second_r3->depOfJourneyInd() = smp::DepartureAppl::AFTER;

    auto first_ptf = _memHandle.create<PaxTypeFare>();
    auto second_ptf = _memHandle.create<PaxTypeFare>();

    MaximumPenaltyCalculator::PtfPuMap ptfPuMap;
    ptfPuMap[first_ptf] = 0;
    ptfPuMap[second_ptf] = 0;

    RefundProcessInfo rpi1{first_r3, first_ptf, nullptr};
    RefundProcessInfo rpi2{second_r3, second_ptf, nullptr};

    RefundPermutationGenerator::FCtoSequence seqsToFc;
    seqsToFc[0] = {&rpi1};
    seqsToFc[1] = {&rpi2};

    CPPUNIT_ASSERT(_mpCalc->isAnyFCPartiallyFlownAfter(seqsToFc, ptfPuMap));
  }

  void testAllRecordsPartiallyFlownprocessing_noPartiallyFlownRecords_fail()
  {
    auto first_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    first_r3->depOfJourneyInd() = smp::DepartureAppl::BEFORE;
    auto second_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    second_r3->depOfJourneyInd() = smp::DepartureAppl::BEFORE;

    auto first_ptf = _memHandle.create<PaxTypeFare>();
    auto second_ptf = _memHandle.create<PaxTypeFare>();

    MaximumPenaltyCalculator::PtfPuMap ptfPuMap;
    ptfPuMap[first_ptf] = 0;
    ptfPuMap[second_ptf] = 0;

    RefundProcessInfo rpi1{first_r3, first_ptf, nullptr};
    RefundProcessInfo rpi2{second_r3, second_ptf, nullptr};

    RefundPermutationGenerator::FCtoSequence seqsToFc;
    seqsToFc[0] = {&rpi1};
    seqsToFc[1] = {&rpi2};

    CPPUNIT_ASSERT(!_mpCalc->isAnyFCPartiallyFlownAfter(seqsToFc, ptfPuMap));
  }

  void testAllRecordsPartiallyFlownProcessing_fail()
  {
    auto first_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    auto second_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    second_r3->fullyFlown() = VoluntaryRefunds::BLANK;
    second_r3->depOfJourneyInd() = smp::DepartureAppl::AFTER;
    auto third_r3 = _memHandle.create<VoluntaryRefundsInfo>();
    first_r3->fullyFlown() = VoluntaryRefunds::PARTIALLY_FLOWN;
    first_r3->depOfJourneyInd() = smp::DepartureAppl::AFTER;

    auto first_ptf = _memHandle.create<PaxTypeFare>();
    auto second_ptf = _memHandle.create<PaxTypeFare>();

    MaximumPenaltyCalculator::PtfPuMap ptfPuMap;
    ptfPuMap[first_ptf] = 0;
    ptfPuMap[second_ptf] = 0;

    RefundProcessInfo rpi1{first_r3, first_ptf, nullptr};
    RefundProcessInfo rpi2{second_r3, first_ptf, nullptr};
    RefundProcessInfo rpi3{third_r3, second_ptf, nullptr};

    RefundPermutationGenerator::FCtoSequence seqsToFc;
    seqsToFc[0] = {&rpi1, &rpi2};
    seqsToFc[1] = {&rpi3};

    CPPUNIT_ASSERT(!_mpCalc->isAnyFCPartiallyFlownAfter(seqsToFc, ptfPuMap));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MaximumPenaltyCalculatorTest);

} // tse
