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

#include "Pricing/Cat16MaxPenaltyCalculator.h"

#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/PenaltyInfo.h"
#include "Diagnostic/Diagnostic.h"
#include "Rules/Penalties.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>

namespace tse
{
namespace
{
inline void
setRecordRefundApplies(PenaltyInfo* record)
{
  record->cancelRefundAppl() = Penalties::APPLIES;
  record->penaltyRefund() = Penalties::APPLIES;
  record->penaltyCancel() = Penalties::APPLIES;
}

inline void
setRecordChangeApplies(PenaltyInfo* record)
{
  record->volAppl() = Penalties::APPLIES;
  record->penaltyReissue() = Penalties::APPLIES;
  record->penaltyNoReissue() = Penalties::APPLIES;
}
}

class Cat16MaxPenaltyCalculatorTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _records = _memHandle.create<Cat16MaxPenaltyCalculator::PenaltiesCollection>();
    _record = _memHandle.create<PenaltyInfo>();
    _record->penaltyRefund() = Penalties::APPLIES;
    _record->penaltyReissue() = Penalties::APPLIES;
    _goodRecord = _memHandle.create<PenaltyInfo>();
    _goodRecord->noRefundInd() = Penalties::TICKETNRF_AND_RESNOCHANGE;
    _goodRecord->penaltyAppl() = smp::PenaltyAppl::ANYTIME_BLANK;
    _goodRecord->penaltyRefund() = Penalties::APPLIES;
    _goodRecord->penaltyReissue() = Penalties::APPLIES;
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _ptf = _memHandle.create<PaxTypeFare>();
    _fareInfo = _memHandle.create<FareInfo>();
    _ptf->fare()->setFareInfo(_fareInfo);
    _calculator = _memHandle.create<Cat16MaxPenaltyCalculator>(*_trx, true);
  }

  void TearDown() { _memHandle.clear(); }

  bool isNonRefundable(const PenaltyInfo* penaltyInfo)
  {
    return Cat16MaxPenaltyCalculator::isNonRefundable(penaltyInfo);
  }

  bool isNonChangeable(const PenaltyInfo* penaltyInfo)
  {
    return Cat16MaxPenaltyCalculator::isNonChangeable(penaltyInfo);
  }

  bool changeApplies(const PenaltyInfo* penaltyInfo)
  {
    return Cat16MaxPenaltyCalculator::changeApplies(penaltyInfo);
  }

  bool refundApplies(const PenaltyInfo* penaltyInfo)
  {
    return Cat16MaxPenaltyCalculator::refundApplies(penaltyInfo);
  }

  bool isDepartureMatching(const PenaltyInfo* penaltyInfo, smp::RecordApplication app)
  {
    return smp::isDepartureMatching(smp::getRecordApplication(*penaltyInfo), app);
  }

  Money calcPenalty(const PenaltyInfo& record, CurrencyCode code) const
  {
    return Penalties::getPenaltyAmount(*_trx, *_ptf, record, code);
  }

protected:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PaxTypeFare* _ptf;
  FareInfo* _fareInfo;
  Cat16MaxPenaltyCalculator::PenaltiesCollection* _records;
  PenaltyInfo* _record;
  PenaltyInfo* _goodRecord;

  Cat16MaxPenaltyCalculator* _calculator;
};

TEST_F(Cat16MaxPenaltyCalculatorTest, areNonRefundableEmpty)
{
  ASSERT_TRUE(_records->empty());
  EXPECT_FALSE(_calculator->areNonRefundable(*_records, smp::BOTH));
  EXPECT_FALSE(_calculator->areNonRefundable(*_records, smp::AFTER));
  EXPECT_FALSE(_calculator->areNonRefundable(*_records, smp::BEFORE));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, areNonRefundablePass)
{
  _record->noRefundInd() = Penalties::TICKET_NON_REFUNDABLE;
  _record->penaltyAppl() = smp::PenaltyAppl::BEFORE_DEP;
  ASSERT_TRUE(_records->empty());
  _records->insert(_goodRecord);
  ASSERT_TRUE(_calculator->areNonRefundable(*_records, smp::AFTER));
  _records->insert(_record);
  EXPECT_TRUE(_calculator->areNonRefundable(*_records, smp::BEFORE));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, areNonChangeableEmpty)
{
  ASSERT_TRUE(_records->empty());
  EXPECT_FALSE(_calculator->areNonChangeable(*_records, smp::BOTH));
  EXPECT_FALSE(_calculator->areNonChangeable(*_records, smp::AFTER));
  EXPECT_FALSE(_calculator->areNonChangeable(*_records, smp::BEFORE));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, areNonChangeablePass)
{
  _record->noRefundInd() = Penalties::RESERVATIONS_CANNOT_BE_CHANGED;
  _record->penaltyAppl() = smp::PenaltyAppl::AFTER_DEP;
  ASSERT_TRUE(_records->empty());
  _records->insert(_goodRecord);
  ASSERT_TRUE(_calculator->areNonChangeable(*_records, smp::BEFORE));
  _records->insert(_record);
  EXPECT_TRUE(_calculator->areNonChangeable(*_records, smp::AFTER));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, isNonRefundablePass)
{
  ASSERT_TRUE(isNonRefundable(_goodRecord));
  _record->noRefundInd() = Penalties::TICKET_NON_REFUNDABLE;
  EXPECT_TRUE(isNonRefundable(_record));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, isNonRefundableFail)
{
  ASSERT_TRUE(isNonRefundable(_goodRecord));
  _record->noRefundInd() = Penalties::BLANK;
  EXPECT_FALSE(isNonRefundable(_record));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, isNonChangeablePass)
{
  ASSERT_TRUE(isNonChangeable(_goodRecord));
  _record->noRefundInd() = Penalties::RESERVATIONS_CANNOT_BE_CHANGED;
  EXPECT_TRUE(isNonChangeable(_record));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, isNonChangeableFail)
{
  ASSERT_TRUE(isNonChangeable(_goodRecord));
  _record->noRefundInd() = Penalties::BLANK;
  EXPECT_FALSE(isNonChangeable(_record));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, isDepartureMatchingBeforePass)
{
  ASSERT_TRUE(isDepartureMatching(_goodRecord, smp::BEFORE));
  _record->penaltyAppl() = smp::PenaltyAppl::BEFORE_DEP;
  EXPECT_TRUE(isDepartureMatching(_record, smp::BEFORE));
  _record->penaltyAppl() = smp::PenaltyAppl::BEFORE_DEP_CHILD_DSC;
  EXPECT_TRUE(isDepartureMatching(_record, smp::BEFORE));
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME_CHILD_DSC;
  EXPECT_TRUE(isDepartureMatching(_record, smp::BEFORE));
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME_BLANK;
  EXPECT_TRUE(isDepartureMatching(_record, smp::BEFORE));
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME;
  EXPECT_TRUE(isDepartureMatching(_record, smp::BEFORE));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, isDepartureMatchingBeforeFail)
{
  ASSERT_TRUE(isDepartureMatching(_goodRecord, smp::BEFORE));
  _record->penaltyAppl() = smp::PenaltyAppl::AFTER_DEP;
  EXPECT_FALSE(isDepartureMatching(_record, smp::BEFORE));
  _record->penaltyAppl() = smp::PenaltyAppl::AFTER_DEP_CHILD_DSC;
  EXPECT_FALSE(isDepartureMatching(_record, smp::BEFORE));
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME_CHILD_DSC;
  EXPECT_TRUE(isDepartureMatching(_record, smp::BEFORE));
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME_BLANK;
  EXPECT_TRUE(isDepartureMatching(_record, smp::BEFORE));
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME;
  EXPECT_TRUE(isDepartureMatching(_record, smp::BEFORE));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, isDepartureMatchingAfterPass)
{
  ASSERT_TRUE(isDepartureMatching(_goodRecord, smp::AFTER));
  _record->penaltyAppl() = smp::PenaltyAppl::AFTER_DEP;
  EXPECT_TRUE(isDepartureMatching(_record, smp::AFTER));
  _record->penaltyAppl() = smp::PenaltyAppl::AFTER_DEP_CHILD_DSC;
  EXPECT_TRUE(isDepartureMatching(_record, smp::AFTER));
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME_CHILD_DSC;
  EXPECT_TRUE(isDepartureMatching(_record, smp::AFTER));
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME_BLANK;
  EXPECT_TRUE(isDepartureMatching(_record, smp::AFTER));
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME;
  EXPECT_TRUE(isDepartureMatching(_record, smp::AFTER));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, isDepartureMatchingAfterFail)
{
  ASSERT_TRUE(isDepartureMatching(_goodRecord, smp::AFTER));
  _record->penaltyAppl() = smp::PenaltyAppl::BEFORE_DEP;
  EXPECT_FALSE(isDepartureMatching(_record, smp::AFTER));
  _record->penaltyAppl() = smp::PenaltyAppl::BEFORE_DEP_CHILD_DSC;
  EXPECT_FALSE(isDepartureMatching(_record, smp::AFTER));
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME_CHILD_DSC;
  EXPECT_TRUE(isDepartureMatching(_record, smp::AFTER));
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME_BLANK;
  EXPECT_TRUE(isDepartureMatching(_record, smp::AFTER));
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME;
  EXPECT_TRUE(isDepartureMatching(_record, smp::AFTER));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, isDepartureMatchingBothPass)
{
  ASSERT_TRUE(isDepartureMatching(_goodRecord, smp::BOTH));
  ASSERT_TRUE(isDepartureMatching(_record, smp::BOTH));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxChangePenaltyEmptyRecords)
{
  ASSERT_EQ(0, _records->size());
  _fareInfo->currency() = NUC;

  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::CHANGE_PEN);

  ASSERT_EQ(*fees._before._fee, Money(0, NUC));
  ASSERT_EQ(*fees._after._fee, Money(0, NUC));
  ASSERT_FALSE(fees._before._non);
  ASSERT_FALSE(fees._after._non);
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxPenaltyAllRecordsIsUnavailTagSkip)
{
  PenaltyInfo record;
  record.unavailTag() = smp::UnavailTag::TEXT_ONLY;
  _records->insert(&record);
  _fareInfo->currency() = NUC;

  ASSERT_FALSE(_records->empty());
  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::CHANGE_PEN);

  ASSERT_FALSE(fees._before._fee.is_initialized());
  ASSERT_FALSE(fees._after._fee.is_initialized());
  ASSERT_FALSE(fees._before._non);
  ASSERT_FALSE(fees._after._non);
  ASSERT_TRUE(fees.missingDataInsideCalc() & smp::BOTH);

  fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::REFUND_PEN);

  ASSERT_FALSE(fees._before._fee.is_initialized());
  ASSERT_FALSE(fees._after._fee.is_initialized());
  ASSERT_FALSE(fees._before._non);
  ASSERT_FALSE(fees._after._non);
  ASSERT_TRUE(fees.missingDataInsideCalc() & smp::BOTH);
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxPenaltyAllRecordsIsUnavailTagFail)
{
  PenaltyInfo record;
  record.unavailTag() = smp::UnavailTag::DATA_UNAVAILABLE;
  _records->insert(&record);
  _fareInfo->currency() = NUC;

  ASSERT_FALSE(_records->empty());
  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::CHANGE_PEN);

  ASSERT_TRUE(*fees._before._fee == Money(0., NUC));
  ASSERT_TRUE(*fees._after._fee == Money(0., NUC));
  ASSERT_FALSE(fees._before._non);
  ASSERT_FALSE(fees._after._non);

  fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::REFUND_PEN);

  ASSERT_TRUE(*fees._before._fee == Money(0., NUC));
  ASSERT_TRUE(*fees._after._fee == Money(0., NUC));
  ASSERT_FALSE(fees._before._non);
  ASSERT_FALSE(fees._after._non);
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxChangePenaltyCorrectRecordNonChangeable)
{
  PenaltyInfo record;
  record.noRefundInd() = Penalties::RESERVATIONS_CANNOT_BE_CHANGED;
  record.volAppl() = Penalties::APPLIES;

  ASSERT_TRUE(isNonChangeable(&record));
  ASSERT_TRUE(changeApplies(&record));

  _records->insert(&record);
  _fareInfo->currency() = NUC;

  ASSERT_FALSE(_records->empty());
  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::CHANGE_PEN);

  // fees do not exist and record is nonchangeable
  ASSERT_FALSE(fees._before._fee.is_initialized());
  ASSERT_FALSE(fees._after._fee.is_initialized());
  ASSERT_TRUE(fees._before._non);
  ASSERT_TRUE(fees._after._non);
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxRefundPenaltyCorrectRecordNonRefundable)
{
  PenaltyInfo record;
  record.noRefundInd() = Penalties::TICKET_NON_REFUNDABLE;
  record.cancelRefundAppl() = Penalties::APPLIES;

  ASSERT_TRUE(isNonRefundable(&record));
  ASSERT_TRUE(refundApplies(&record));

  _records->insert(&record);
  _fareInfo->currency() = NUC;

  ASSERT_FALSE(_records->empty());
  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::REFUND_PEN);

  // fees do not exist and record is nonrefundable
  ASSERT_FALSE(fees._before._fee.is_initialized());
  ASSERT_FALSE(fees._after._fee.is_initialized());
  ASSERT_TRUE(fees._before._non);
  ASSERT_TRUE(fees._after._non);
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxChangePenaltyRecordPassMaxFeeFromRecord)
{
  PenaltyInfo record;
  record.penaltyRefund() = Penalties::APPLIES;
  record.penaltyReissue() = Penalties::APPLIES;
  record.penaltyCancel() = Penalties::APPLIES;
  _records->insert(&record);
  _fareInfo->currency() = NUC;

  ASSERT_FALSE(_records->empty());
  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::CHANGE_PEN);

  // fee exists and record is still changeable
  ASSERT_TRUE(fees._before._fee.is_initialized());
  ASSERT_TRUE(fees._after._fee.is_initialized());
  ASSERT_FALSE(fees._before._non);
  ASSERT_FALSE(fees._after._non);
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxPenaltyAllRecordsMissingData)
{
  ASSERT_TRUE(_records->empty());

  _record->unavailTag() = smp::UnavailTag::TEXT_ONLY;
  _record->penaltyAppl() = smp::PenaltyAppl::ANYTIME;
  _records->insert(_record);

  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::CHANGE_PEN);

  ASSERT_FALSE(fees._before._fee.is_initialized());
  ASSERT_FALSE(fees._after._fee.is_initialized());

  ASSERT_TRUE(fees.missingDataInsideCalc() == smp::BOTH);

  ASSERT_FALSE(fees._before._non);
  ASSERT_FALSE(fees._after._non);
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxPenaltyBeforeMissingData)
{
  _record->unavailTag() = smp::UnavailTag::TEXT_ONLY;
  _records->insert(_record);

  _record = _memHandle.create<PenaltyInfo>();
  _record->penaltyAppl() = smp::PenaltyAppl::AFTER_DEP;
  _records->insert(_record);

  _record = _memHandle.create<PenaltyInfo>();
  _record->penaltyAppl() = smp::PenaltyAppl::AFTER_DEP;
  _record->penaltyRefund() = Penalties::APPLIES;
  _records->insert(_record);

  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::REFUND_PEN);

  ASSERT_TRUE(fees.missingDataInsideCalc() == smp::BEFORE);

  EXPECT_FALSE(fees._before._fee.is_initialized());
  EXPECT_TRUE(fees._after._fee.is_initialized());
  EXPECT_TRUE(fees._after._fee.get() == Money(0, NUC));

  EXPECT_FALSE(fees._before._non);
  EXPECT_FALSE(fees._after._non);
}
TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxPenaltyAfterMissingData)
{
  _record->unavailTag() = smp::UnavailTag::TEXT_ONLY;
  _records->insert(_record);

  _record = _memHandle.create<PenaltyInfo>();
  _record->penaltyAppl() = smp::PenaltyAppl::BEFORE_DEP;
  _records->insert(_record);

  _record = _memHandle.create<PenaltyInfo>();
  _record->penaltyAppl() = smp::PenaltyAppl::BEFORE_DEP;
  _record->penaltyRefund() = Penalties::APPLIES;
  _records->insert(_record);

  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::REFUND_PEN);

  ASSERT_TRUE(fees.missingDataInsideCalc() == smp::AFTER);

  EXPECT_TRUE(fees._before._fee.is_initialized());
  EXPECT_TRUE(fees._before._fee.get() == Money(0, NUC));
  EXPECT_FALSE(fees._after._fee.is_initialized());

  EXPECT_FALSE(fees._before._non);
  EXPECT_FALSE(fees._after._non);
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxRefundPenaltyRecordPassMaxFeeFromRecord)
{
  PenaltyInfo record;
  record.penaltyRefund() = Penalties::APPLIES;
  record.penaltyReissue() = Penalties::APPLIES;
  record.penaltyCancel() = Penalties::APPLIES;
  _records->insert(&record);
  _fareInfo->currency() = NUC;

  ASSERT_FALSE(_records->empty());
  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::REFUND_PEN);

  // fee exists and record is still refundable
  ASSERT_TRUE(fees._before._fee.is_initialized());
  ASSERT_TRUE(fees._after._fee.is_initialized());
  ASSERT_FALSE(fees._before._non);
  ASSERT_FALSE(fees._after._non);

  ASSERT_TRUE(fees._before._fee.get() == Money(0, NUC));
  ASSERT_TRUE(fees._after._fee.get() == Money(0, NUC));
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxChangePenaltyNonChangeableSinglePenalty)
{
  auto r0 = _memHandle.create<PenaltyInfo>();
  // nonchangeable
  r0->noRefundInd() = Penalties::RESERVATIONS_CANNOT_BE_CHANGED;
  setRecordChangeApplies(r0);

  auto r1 = _memHandle.create<PenaltyInfo>();
  setRecordChangeApplies(r1);
  r1->penaltyAmt1() = 0.;
  r1->penaltyCur1() = NUC;
  _fareInfo->currency() = NUC;
  Money r1Penalty = calcPenalty(*r1, NUC);
  EXPECT_EQ(0., r1Penalty.value());
  EXPECT_EQ(NUC, r1Penalty.code());

  auto r2 = _memHandle.create<PenaltyInfo>();
  setRecordChangeApplies(r2);
  r2->penaltyAmt1() = 100.;
  r2->penaltyCur1() = NUC;
  _fareInfo->currency() = NUC;
  Money r2Penalty = calcPenalty(*r2, NUC);
  EXPECT_EQ(100., r2Penalty.value());
  EXPECT_EQ(NUC, r2Penalty.code());

  _records->insert({r0, r1, r2});
  ASSERT_EQ(3, _records->size());

  _fareInfo->currency() = NUC;

  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::CHANGE_PEN);

  ASSERT_TRUE(fees._before._non);
  ASSERT_TRUE(fees._after._non);
  ASSERT_EQ(100., fees._before._fee.get().value());
  ASSERT_EQ(100., fees._after._fee.get().value());
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxChangePenaltyNonRefundableSinglePenalty)
{
  auto r0 = _memHandle.create<PenaltyInfo>();
  // nonchangeable
  r0->noRefundInd() = Penalties::TICKET_NON_REFUNDABLE;
  // penalty applies
  setRecordRefundApplies(r0);

  auto r1 = _memHandle.create<PenaltyInfo>();
  setRecordRefundApplies(r1);
  r1->penaltyAmt1() = 0.;
  r1->penaltyCur1() = NUC;
  _fareInfo->currency() = NUC;
  Money r1Penalty = calcPenalty(*r1, NUC);
  EXPECT_EQ(0., r1Penalty.value());
  EXPECT_EQ(NUC, r1Penalty.code());

  auto r2 = _memHandle.create<PenaltyInfo>();
  setRecordRefundApplies(r2);
  r2->penaltyAmt1() = 100.;
  r2->penaltyCur1() = NUC;
  _fareInfo->currency() = NUC;
  Money r2Penalty = calcPenalty(*r2, NUC);
  EXPECT_EQ(100., r2Penalty.value());
  EXPECT_EQ(NUC, r2Penalty.code());

  _records->insert({r0, r1, r2});
  EXPECT_EQ(3, _records->size());

  _fareInfo->currency() = NUC;

  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::REFUND_PEN);

  ASSERT_TRUE(fees._before._non);
  ASSERT_TRUE(fees._after._non);
  ASSERT_EQ(100., fees._before._fee.get().value());
  ASSERT_EQ(NUC, fees._before._fee.get().code());
  ASSERT_EQ(100., fees._after._fee.get().value());
  ASSERT_EQ(NUC, fees._after._fee.get().code());
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxPenaltySinglePenalty)
{
  auto r0 = _memHandle.create<PenaltyInfo>();
  setRecordChangeApplies(r0);
  r0->penaltyAmt1() = 100.;
  r0->penaltyCur1() = USD;
  r0->penaltyAmt2() = 50.;
  r0->penaltyCur2() = NUC;
  _fareInfo->currency() = USD;
  Money r0PenaltyUSD = calcPenalty(*r0, USD);
  _fareInfo->currency() = NUC;
  Money r0PenaltyNUC = calcPenalty(*r0, NUC);
  EXPECT_EQ(100., r0PenaltyUSD.value());
  EXPECT_EQ(USD, r0PenaltyUSD.code());
  EXPECT_EQ(50., r0PenaltyNUC.value());
  EXPECT_EQ(NUC, r0PenaltyNUC.code());

  _records->insert(r0);

  _fareInfo->currency() = USD;

  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, USD, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::CHANGE_PEN);

  ASSERT_FALSE(fees._before._non);
  ASSERT_FALSE(fees._after._non);
  ASSERT_EQ(100., fees._before._fee.get().value());
  ASSERT_EQ(USD, fees._before._fee.get().code());
  ASSERT_EQ(100., fees._after._fee.get().value());
  ASSERT_EQ(USD, fees._after._fee.get().code());

  _fareInfo->currency() = NUC;

  fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::CHANGE_PEN);

  ASSERT_FALSE(fees._before._non);
  ASSERT_FALSE(fees._after._non);
  ASSERT_EQ(50., fees._before._fee.get().value());
  ASSERT_EQ(NUC, fees._before._fee.get().code());
  ASSERT_EQ(50., fees._after._fee.get().value());
  ASSERT_EQ(NUC, fees._after._fee.get().code());
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxPenaltyTwoCurrenciesUSD)
{
  auto r0 = _memHandle.create<PenaltyInfo>();
  setRecordChangeApplies(r0);
  r0->penaltyAmt1() = 100.;
  r0->penaltyCur1() = USD;
  _fareInfo->currency() = USD;
  Money r0Penalty = calcPenalty(*r0, USD);
  EXPECT_EQ(100., r0Penalty.value());
  EXPECT_EQ(USD, r0Penalty.code());

  auto r1 = _memHandle.create<PenaltyInfo>();
  setRecordChangeApplies(r1);
  r1->penaltyAmt1() = 50.;
  r1->penaltyCur1() = USD;
  r1->penaltyAmt2() = 100.;
  r1->penaltyCur2() = NUC;
  _fareInfo->currency() = USD;
  Money r1PenaltyUSD = calcPenalty(*r1, USD);
  EXPECT_EQ(50., r1PenaltyUSD.value());
  EXPECT_EQ(USD, r1PenaltyUSD.code());
  _fareInfo->currency() = NUC;
  Money r1PenaltyNUC = calcPenalty(*r1, NUC);
  EXPECT_EQ(100., r1PenaltyNUC.value());
  EXPECT_EQ(NUC, r1PenaltyNUC.code());

  _records->insert({r0, r1});
  EXPECT_EQ(2, _records->size());

  _fareInfo->currency() = USD;

  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, USD, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::CHANGE_PEN);

  ASSERT_FALSE(fees._before._non);
  ASSERT_FALSE(fees._after._non);
  ASSERT_EQ(100., fees._before._fee.get().value());
  ASSERT_EQ(USD, fees._before._fee.get().code());
  ASSERT_EQ(100., fees._after._fee.get().value());
  ASSERT_EQ(USD, fees._after._fee.get().code());
}

TEST_F(Cat16MaxPenaltyCalculatorTest, testCalculateMaxPenaltyTwoCurrenciesNUC)
{
  auto r0 = _memHandle.create<PenaltyInfo>();
  setRecordChangeApplies(r0);
  r0->penaltyAmt1() = 100.;
  r0->penaltyCur1() = USD;
  r0->penaltyAmt2() = 0.;
  r0->penaltyCur2() = NUC;
  _fareInfo->currency() = USD;
  Money r0Penalty = calcPenalty(*r0, USD);
  EXPECT_EQ(100., r0Penalty.value());
  EXPECT_EQ(USD, r0Penalty.code());

  auto r1 = _memHandle.create<PenaltyInfo>();
  setRecordChangeApplies(r1);
  r1->penaltyAmt1() = 50.;
  r1->penaltyCur1() = USD;
  r1->penaltyAmt2() = 100.;
  r1->penaltyCur2() = NUC;
  _fareInfo->currency() = USD;
  Money r1PenaltyUSD = calcPenalty(*r1, USD);
  EXPECT_EQ(50., r1PenaltyUSD.value());
  EXPECT_EQ(USD, r1PenaltyUSD.code());
  _fareInfo->currency() = NUC;
  Money r1PenaltyNUC = calcPenalty(*r1, NUC);
  EXPECT_EQ(100., r1PenaltyNUC.value());
  EXPECT_EQ(NUC, r1PenaltyNUC.code());

  _records->insert({r0, r1});
  EXPECT_EQ(2, _records->size());

  _fareInfo->currency() = NUC;

  MaxPenaltyResponse::Fees fees = _calculator->calculateMaxPenalty(
      *_records, NUC, *_ptf, nullptr, smp::BOTH, Cat16MaxPenaltyCalculator::CHANGE_PEN);

  ASSERT_FALSE(fees._before._non);
  ASSERT_FALSE(fees._after._non);
  ASSERT_EQ(100., fees._before._fee.get().value());
  ASSERT_EQ(NUC, fees._before._fee.get().code());
  ASSERT_EQ(100., fees._after._fee.get().value());
  ASSERT_EQ(NUC, fees._after._fee.get().code());
}

} // namespace tse
