#include "Common/SpecifyMaximumPenaltyCommon.h"

#include "Common/TsePrimitiveTypes.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Rules/VoluntaryChanges.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>

#include <vector>

namespace tse
{
namespace smp
{
class SpecifyMaximumPenaltyCommonTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _pricingTrx = _memHandle.create<PricingTrx>();
    _paxTypeWithSMP = _memHandle.create<PaxType>();
    _paxTypeWithoutSMP = _memHandle.create<PaxType>();
    _paxTypeWithSMP->maxPenaltyInfo() = _memHandle.create<MaxPenaltyInfo>();
    _cat33Record = _memHandle.create<VoluntaryRefundsInfo>();
    _cat31Record = _memHandle.create<VoluntaryChangesInfo>();
    _cat31RecordW = _memHandle.create<VoluntaryChangesInfoW>();
    _cat31RecordW->orig() = _cat31Record;
  }

  void TearDown() { _memHandle.clear(); }

  RecordApplication indicatorToRecordApplication(Indicator value)
  {
    switch(value)
    {
    case 'B':
      return smp::BEFORE;
    case 'A':
      return smp::AFTER;
    case ' ':
      return smp::BOTH;
    default:
      return smp::INVALID;
    }
  }

  void isDepartureMatchingPass(const RecordApplication departureAppl,
                               const RecordApplication application)
  {
    _departureAppl = departureAppl;
    _application = application;
    ASSERT_TRUE(isDepartureMatching(_application, _departureAppl));
  }

  void isDepartureMatchingFail(const RecordApplication departureAppl,
                               const RecordApplication application)
  {
    _departureAppl = departureAppl;
    _application = application;
    ASSERT_FALSE(isDepartureMatching(_application, _departureAppl));
  }

  void setUpRecordRefund(VoluntaryRefundsInfo* r3,
                         Indicator journeyInd,
                         Indicator puInd,
                         Indicator fareComponentInd)
  {
    r3->depOfJourneyInd() = journeyInd;
    r3->puInd() = puInd;
    r3->fareComponentInd() = fareComponentInd;
  }

  void getRecordApplicationChangePass(Indicator& indicator)
  {
    indicator = DepartureAppl::BEFORE;
    ASSERT_TRUE(getRecordApplication(*_cat31RecordW, BEFORE, true, true) == BEFORE);
    indicator = DepartureAppl::AFTER;
    ASSERT_TRUE(getRecordApplication(*_cat31RecordW, AFTER, true, true) == AFTER);
    indicator = DepartureAppl::BOTH;
    ASSERT_TRUE(getRecordApplication(*_cat31RecordW, BOTH, true, true) == BOTH);
  }

  void getRecordApplicationRefundFail(const Indicator indicator)
  {
    _cat33Record->depOfJourneyInd() = indicator;
    _cat33Record->puInd() = indicator;
    _cat33Record->fareComponentInd() = indicator;
    ASSERT_FALSE(toDepartureAppl(getRecordApplication(*_cat33Record, false, false)) != indicator);
  }

  void getRecordApplicationChangeFail(const Indicator indicator)
  {
    _cat31Record->departureInd() = indicator;
    _cat31Record->priceableUnitInd() = indicator;
    _cat31Record->fareComponentInd() = indicator;
    ASSERT_FALSE(toDepartureAppl(getRecordApplication(*_cat31RecordW, indicatorToRecordApplication(indicator), false, false)) != indicator);
  }

  void voluntaryChangesNotPermittedPass(Indicator& indicator, const Indicator setIndicator)
  {
    indicator = setIndicator;
    ASSERT_TRUE(voluntaryChangesNotPermitted(*_cat31RecordW));
  }

protected:
  TestMemHandle _memHandle;
  PricingTrx* _pricingTrx;
  PaxType* _paxTypeWithSMP;
  PaxType* _paxTypeWithoutSMP;
  VoluntaryRefundsInfo* _cat33Record;
  VoluntaryChangesInfo* _cat31Record;
  VoluntaryChangesInfoW* _cat31RecordW;
  RecordApplication _departureAppl;
  RecordApplication _application;
};

TEST_F(SpecifyMaximumPenaltyCommonTest, isPenaltyCalculationRequiredPass)
{
  _pricingTrx->paxType().push_back(_paxTypeWithSMP);
  _pricingTrx->paxType().push_back(_paxTypeWithoutSMP);
  ASSERT_TRUE(isPenaltyCalculationRequired(*_pricingTrx));
}

TEST_F(SpecifyMaximumPenaltyCommonTest, isPenaltyCalculationRequiredFail)
{
  _pricingTrx->paxType().push_back(_paxTypeWithoutSMP);
  ASSERT_FALSE(isPenaltyCalculationRequired(*_pricingTrx));
}

TEST_F(SpecifyMaximumPenaltyCommonTest, testJourneyIndicatorMatch)
{
  setUpRecordRefund(_cat33Record, DepartureAppl::BEFORE, DepartureAppl::BOTH, DepartureAppl::BOTH);
  ASSERT_EQ(BEFORE, getRecordApplication(*_cat33Record, false, false));

  setUpRecordRefund(_cat33Record, DepartureAppl::AFTER, DepartureAppl::BOTH, DepartureAppl::BOTH);
  ASSERT_EQ(AFTER, getRecordApplication(*_cat33Record, false, false));
}

TEST_F(SpecifyMaximumPenaltyCommonTest, testPricingUnitIndicatorMatch)
{
  setUpRecordRefund(_cat33Record, DepartureAppl::BOTH, DepartureAppl::BEFORE, DepartureAppl::BOTH);
  ASSERT_EQ(BOTH, getRecordApplication(*_cat33Record, false, false));

  // record with puInd set to B can be before on first pricing unit
  ASSERT_EQ(BEFORE, getRecordApplication(*_cat33Record, false, true));

  setUpRecordRefund(_cat33Record, DepartureAppl::BOTH, DepartureAppl::AFTER, DepartureAppl::BOTH);
  ASSERT_EQ(AFTER, getRecordApplication(*_cat33Record, false, false));
}

TEST_F(SpecifyMaximumPenaltyCommonTest, testFareComponentIndicatorMatch)
{
  setUpRecordRefund(_cat33Record, DepartureAppl::BOTH, DepartureAppl::BOTH, DepartureAppl::BEFORE);
  ASSERT_EQ(BOTH, getRecordApplication(*_cat33Record, false, false));

  // record with fareComponentInd set to B can be before on first fare component
  ASSERT_EQ(BEFORE, getRecordApplication(*_cat33Record, true, false));

  setUpRecordRefund(_cat33Record, DepartureAppl::BOTH, DepartureAppl::BOTH, DepartureAppl::AFTER);
  ASSERT_EQ(AFTER, getRecordApplication(*_cat33Record, false, false));
}

TEST_F(SpecifyMaximumPenaltyCommonTest, getRecordApplicationRefundFail)
{
  getRecordApplicationRefundFail(DepartureAppl::BEFORE);
  getRecordApplicationRefundFail(DepartureAppl::AFTER);
  getRecordApplicationRefundFail(DepartureAppl::BOTH);
}

TEST_F(SpecifyMaximumPenaltyCommonTest, getRecordApplicationChangePass)
{
  getRecordApplicationChangePass(_cat31Record->departureInd());
  getRecordApplicationChangePass(_cat31Record->priceableUnitInd());
  getRecordApplicationChangePass(_cat31Record->fareComponentInd());
}

TEST_F(SpecifyMaximumPenaltyCommonTest, getRecordApplicationChangeFail)
{
  getRecordApplicationChangeFail(DepartureAppl::BEFORE);
  getRecordApplicationChangeFail(DepartureAppl::AFTER);
  getRecordApplicationChangeFail(DepartureAppl::BOTH);
}

TEST_F(SpecifyMaximumPenaltyCommonTest, isDepartureMatchingPass)
{
  isDepartureMatchingPass(BEFORE, BEFORE);
  isDepartureMatchingPass(AFTER, AFTER);
  isDepartureMatchingPass(BOTH, BOTH);
  isDepartureMatchingPass(BOTH, BEFORE);
  isDepartureMatchingPass(BOTH, AFTER);
  isDepartureMatchingPass(BEFORE, BOTH);
  isDepartureMatchingPass(AFTER, BOTH);
}

TEST_F(SpecifyMaximumPenaltyCommonTest, isDepartureMatchingFail)
{
  isDepartureMatchingFail(BEFORE, AFTER);
  isDepartureMatchingFail(AFTER, BEFORE);
}

TEST_F(SpecifyMaximumPenaltyCommonTest, voluntaryChangesNotPermittedPass)
{
  voluntaryChangesNotPermittedPass(_cat31Record->changeInd(), VoluntaryChanges::NOT_PERMITTED);
  voluntaryChangesNotPermittedPass(_cat31Record->changeInd(), VoluntaryChanges::CHG_IND_J);
  voluntaryChangesNotPermittedPass(_cat31Record->changeInd(), VoluntaryChanges::CHG_IND_P);
}

TEST_F(SpecifyMaximumPenaltyCommonTest, voluntaryChangesNotPermittedFail)
{
  _cat31Record->changeInd() = ' ';
  ASSERT_FALSE(voluntaryChangesNotPermitted(*_cat31RecordW));
}

TEST_F(SpecifyMaximumPenaltyCommonTest, changePenaltyAfterDep)
{
  _cat31Record->departureInd() = DepartureAppl::AFTER;
  _cat31Record->fareComponentInd() = DepartureAppl::BOTH;
  _cat31Record->priceableUnitInd() = DepartureAppl::BOTH;

  ASSERT_EQ(smp::AFTER, getRecordApplication(*_cat31RecordW, smp::AFTER, true, true));
}

TEST_F(SpecifyMaximumPenaltyCommonTest, changePenaltyAfterDepFCAfter)
{
  _cat31Record->departureInd() = DepartureAppl::BOTH;
  _cat31Record->fareComponentInd() = DepartureAppl::AFTER;
  _cat31Record->priceableUnitInd() = DepartureAppl::BOTH;

  ASSERT_EQ(smp::AFTER, getRecordApplication(*_cat31RecordW, smp::AFTER, true, true));
}

TEST_F(SpecifyMaximumPenaltyCommonTest, changePenaltyAfterDepPUAfter)
{
  _cat31Record->departureInd() = DepartureAppl::BOTH;
  _cat31Record->fareComponentInd() = DepartureAppl::BOTH;
  _cat31Record->priceableUnitInd() = DepartureAppl::AFTER;

  ASSERT_EQ(smp::AFTER, getRecordApplication(*_cat31RecordW, smp::AFTER, true, true));
}

TEST_F(SpecifyMaximumPenaltyCommonTest, changePenaltyAfterDepFCBeforeNotFirst)
{
  _cat31Record->departureInd() = DepartureAppl::BOTH;
  _cat31Record->fareComponentInd() = DepartureAppl::BEFORE;
  _cat31Record->priceableUnitInd() = DepartureAppl::BOTH;

  ASSERT_EQ(smp::BOTH, getRecordApplication(*_cat31RecordW, smp::AFTER, false, false));
}

TEST_F(SpecifyMaximumPenaltyCommonTest, changePenaltyAfterDepPUBeforeNotFirst)
{
  _cat31Record->departureInd() = DepartureAppl::BOTH;
  _cat31Record->fareComponentInd() = DepartureAppl::BOTH;
  _cat31Record->priceableUnitInd() = DepartureAppl::BEFORE;

  ASSERT_EQ(smp::BOTH, getRecordApplication(*_cat31RecordW, smp::AFTER, false, false));
}

TEST_F(SpecifyMaximumPenaltyCommonTest, changePenaltyAfterDepPUBefore)
{
  _cat31Record->departureInd() = DepartureAppl::BOTH;
  _cat31Record->fareComponentInd() = DepartureAppl::BOTH;
  _cat31Record->priceableUnitInd() = DepartureAppl::BEFORE;

  ASSERT_EQ(smp::BEFORE, getRecordApplication(*_cat31RecordW, smp::AFTER, false, true));
}


} // namespace smp
} // namespace tse
