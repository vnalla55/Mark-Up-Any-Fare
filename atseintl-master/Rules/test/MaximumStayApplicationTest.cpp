//------------------------------------------------------------------
//
//  File: MaximumStayApplicationTest.cpp
//
//  Copyright Sabre 2014
//
//    The copyright to the computer program(s) herein
//    is the property of Sabre.
//    The program(s) may be used and/or copied only with
//    the written permission of Sabre or in accordance
//    with the terms and conditions stipAulated in the
//    agreement/contract under which the program(s)
//    have been supplied.
//
//-------------------------------------------------------------------
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/include/GtestHelperMacros.h"
#include "Rules/MaximumStayApplication.h"
#include "Rules/MinMaxRulesUtils.h"
#include "Rules/RuleValidationContext.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/MaxStayRestriction.h"
#include "Diagnostic/DiagManager.h"

#include "Rules/test/MockRuleValidationChancelor.h"
#include "Rules/test/MockRuleValidationPolicy.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using namespace ::testing;

class MaximumStayApplicationTest : public Test
{
public:
  void SetUp()
  {
    _options.noMinMaxStayRestr() = 'Y';
    _trx.setOptions(&_options);
    _rule.maxStay() = "12";
    _rule.maxStayUnit() = "MON";
    _category = RuleConst::MAXIMUM_STAY_RULE;
    _chancelorMock.reset(new MockRuleValidationChancelor);
    _ruleValidationPolicyMock.reset(new MockRuleValidationPolicy);
  }

  void setCalls()
  {
    _trx.setFlexFare(true);
    _maximumStayApplication.setChancelor(_chancelorMock);
    EXPECT_CALL(*_chancelorMock, hasPolicy(_category)).WillOnce(Return(true));
    EXPECT_CALL(Const(*_chancelorMock), getPolicy(_category)).Times(2).WillRepeatedly(
        ReturnRef(*_ruleValidationPolicyMock));
    EXPECT_CALL(Const(*_chancelorMock), getContext()).Times(2).WillRepeatedly(ReturnRef(_context));
    EXPECT_CALL(*_ruleValidationPolicyMock, shouldPerform(_)).WillOnce(Return(true));
    EXPECT_CALL(*_ruleValidationPolicyMock, shouldReturn()).WillOnce(Return(true));
  }

  DateTime proxy_determineMaxStayDate(const DateTime& maxStayPeriodReturnDate)
  {
    return _maximumStayApplication.determineLatestReturnDateWithTime(
        maxStayPeriodReturnDate, _rule.tod(), _rule.maxStayUnit());
  }
  DateTime proxy_determineReturnDate(const DateTime& minStayPeriodReturnDate)
  {
    return MinMaxRulesUtils::determineReturnDate(
        minStayPeriodReturnDate, _rule.maxStayDate(), _rule.earlierLaterInd());
  }

  PricingTrx _trx;
  Itin _itin;
  PaxTypeFare _paxTypeFare;
  MaxStayRestriction _rule;
  FareMarket _fareMarket;
  RuleValidationContext _context;
  PricingOptions _options;
  uint16_t _category;
  FarePath _farePath;
  PricingUnit _pricingUnit;
  FareUsage _fareUsage;
  std::shared_ptr<MockRuleValidationChancelor> _chancelorMock;
  std::shared_ptr<MockRuleValidationPolicy> _ruleValidationPolicyMock;
  MaximumStayApplication _maximumStayApplication;
};

TEST_F(MaximumStayApplicationTest, testValidateOldImplementationWithFlexFareChecking1)
{
  _trx.setFlexFare(false);
  ASSERT_EQ(PASS, _maximumStayApplication.validate(_trx, _itin, _paxTypeFare, &_rule, _fareMarket));
}

TEST_F(MaximumStayApplicationTest, testValidateNewImplementationPossitiveCase1)
{
  setCalls();
  ASSERT_EQ(PASS, _maximumStayApplication.validate(_trx, _itin, _paxTypeFare, &_rule, _fareMarket));
}

TEST_F(MaximumStayApplicationTest, testValidateOldImplementationWithFlexFareChecking2)
{
  _trx.setFlexFare(false);
  ASSERT_EQ(PASS,
            _maximumStayApplication.validate(_trx, &_rule, _farePath, _pricingUnit, _fareUsage));
}

TEST_F(MaximumStayApplicationTest, testValidateNewImplementationPossitiveCase2)
{
  setCalls();
  ASSERT_EQ(PASS,
            _maximumStayApplication.validate(_trx, &_rule, _farePath, _pricingUnit, _fareUsage));
}

TEST_F(MaximumStayApplicationTest, testValidateOldImplementationWithFlexFareChecking3)
{
  _trx.setFlexFare(false);
  ASSERT_EQ(PASS, _maximumStayApplication.validate(_trx, &_rule, _itin, _pricingUnit, _fareUsage));
}

TEST_F(MaximumStayApplicationTest, testValidateNewImplementationPossitiveCase3)
{
  setCalls();
  ASSERT_EQ(PASS, _maximumStayApplication.validate(_trx, &_rule, _itin, _pricingUnit, _fareUsage));
}

TEST_F(MaximumStayApplicationTest, testDetermineMaxStayDateDays)
{
  DateTime earliestDate{2015, 10, 25, 2, 25};
  DateTime resultDate{2015, 10, 25, 10, 10};
  _rule.maxStayUnit() = "D";
  _rule.tod() = 610;
  DateTime determinedDate = proxy_determineReturnDate(earliestDate);
  determinedDate = proxy_determineMaxStayDate(determinedDate);
  ASSERT_EQ(resultDate, determinedDate);

  _rule.tod() = 0;
  determinedDate = proxy_determineReturnDate(earliestDate);
  determinedDate = proxy_determineMaxStayDate(determinedDate);
  ASSERT_EQ(DateTime(2015, 10, 25, 23, 59), determinedDate);
}

TEST_F(MaximumStayApplicationTest, testDetermineMaxStayDateHoursAndMinutes)
{
  DateTime earliestDate{2015, 10, 25, 2, 25};
  _rule.maxStayUnit() = "H";
  _rule.tod() = 610;
  DateTime determinedDate = proxy_determineReturnDate(earliestDate);
  determinedDate = proxy_determineMaxStayDate(determinedDate);
  ASSERT_EQ(earliestDate, determinedDate);

  DateTime resultDate{2015, 10, 25, 1, 25};
  _rule.maxStayUnit() = "M";
  _rule.tod() = 85;
  determinedDate = proxy_determineReturnDate(earliestDate);
  determinedDate = proxy_determineMaxStayDate(determinedDate);
  ASSERT_EQ(resultDate, determinedDate);
}

} // namespace tse
