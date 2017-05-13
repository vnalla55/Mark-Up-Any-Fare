//------------------------------------------------------------------
//
//  File: MinimumStayApplicationTest.cpp
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
#include "Rules/MinimumStayApplication.h"
#include "Rules/RuleValidationContext.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/MinStayRestriction.h"
#include "DBAccess/RuleItemInfo.h"
#include "Diagnostic/Diag306Collector.h"
#include "Rules/MinMaxRulesUtils.h"
#include "Rules/PeriodOfStay.h"
#include "test/include/TestMemHandle.h"

#include "test/include/TestConfigInitializer.h"
#include "Rules/test/MockRuleValidationChancelor.h"
#include "Rules/test/MockRuleValidationPolicy.h"

namespace tse
{
using namespace ::testing;

class MinimumStayApplicationTest : public Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _category = RuleConst::MINIMUM_STAY_RULE;
    _chancelorMock.reset(new MockRuleValidationChancelor);
    _ruleValidationPolicyMock.reset(new MockRuleValidationPolicy);
    _options.noMinMaxStayRestr() = 'Y';
    _trx.setOptions(&_options);
    _rule = _memHandle.create<MinStayRestriction>();

    _diag = _memHandle.create<Diag306Collector>();
    _diag->trx() = &_trx;
    _diag->rootDiag() = &_trx.diagnostic();
    _diag->activate();
    _rule->minStayUnit() = "D";
    _rule->tod() = 0;
  }

  void TearDown() { _memHandle.clear(); }

  void setCalls()
  {
    _trx.setFlexFare(true);
    _minimumStayApplication.setChancelor(_chancelorMock);
    EXPECT_CALL(*_chancelorMock, hasPolicy(_category)).WillOnce(Return(true));
    EXPECT_CALL(Const(*_chancelorMock), getPolicy(_category)).Times(2).WillRepeatedly(
        ReturnRef(*_ruleValidationPolicyMock));
    EXPECT_CALL(Const(*_chancelorMock), getContext()).Times(2).WillRepeatedly(ReturnRef(_context));
    EXPECT_CALL(*_ruleValidationPolicyMock, shouldPerform(_)).WillOnce(Return(true));
    EXPECT_CALL(*_ruleValidationPolicyMock, shouldReturn()).WillOnce(Return(true));
  }

  Diag306Collector* _diag;
  TestMemHandle _memHandle;
  MinimumStayApplication _minimumStayApplication;
  PricingTrx _trx;
  Itin _itin;
  PaxTypeFare _paxTypeFare;
  FareDisplayTrx _fareDisplayTrx;
  FarePath _farePath;
  MinStayRestriction* _rule;
  FareMarket _fareMarket;
  PricingUnit _pricingUnit;
  FareUsage _fareUsage;
  uint16_t _category;
  RuleValidationContext _context;
  PricingOptions _options;
  std::shared_ptr<MockRuleValidationChancelor> _chancelorMock;
  std::shared_ptr<MockRuleValidationPolicy> _ruleValidationPolicyMock;

  Record3ReturnTypes proxy_minimumStayApplication_Validate()
  {
    return _minimumStayApplication.validate(
        _trx, &_fareDisplayTrx, _fareMarket, _itin, _paxTypeFare, _rule);
  }
  DateTime proxy_determineEarliestReturnDate(const DateTime& minStayPeriodReturnDate)
  {
    PeriodOfStay periodOfStay(_rule->minStay(), _rule->minStayUnit());
    return _minimumStayApplication.determineEarliestReturnDate(
        minStayPeriodReturnDate, _rule, periodOfStay, _diag);
  }

  DateTime proxy_determineReturnDate(const DateTime& minStayPeriodReturnDate)
  {
    return MinMaxRulesUtils::determineReturnDate(
        minStayPeriodReturnDate, _rule->minStayDate(), _rule->earlierLaterInd());
  }

  DateTime proxy_determineMinStayDate(const DateTime& minStayPeriodReturnDate)
  {
    return _minimumStayApplication.determineEarliestReturnDateWithTime(
        minStayPeriodReturnDate, _rule->tod(), _rule->minStayUnit());
  }
  Record3ReturnTypes
  proxy_validateReturnDate(const DateTime& travelSegDate, const DateTime& earliestReturnDate)
  {
    return _minimumStayApplication.validateReturnDate(travelSegDate, earliestReturnDate);
  }
};

TEST_F(MinimumStayApplicationTest, testValidateOldImplementationWithFlexFareChecking1)
{
  _trx.setFlexFare(false);
  ASSERT_EQ(FAIL, _minimumStayApplication.validate(_trx, _itin, _paxTypeFare, _rule, _fareMarket));
}

TEST_F(MinimumStayApplicationTest, testValidateNewImplementationPossitiveCase1)
{
  setCalls();
  ASSERT_EQ(FAIL, _minimumStayApplication.validate(_trx, _itin, _paxTypeFare, _rule, _fareMarket));
}

TEST_F(MinimumStayApplicationTest, testValidateOldImplementationWithFlexFareChecking2)
{
  _trx.setFlexFare(false);
  ASSERT_EQ(FAIL,
            _minimumStayApplication.validate(_trx, _rule, _farePath, _pricingUnit, _fareUsage));
}

TEST_F(MinimumStayApplicationTest, testValidateEsvNewImplementationPossitiveCase2)
{
  setCalls();
  ASSERT_EQ(FAIL,
            _minimumStayApplication.validate(_trx, _rule, _farePath, _pricingUnit, _fareUsage));
}

TEST_F(MinimumStayApplicationTest, testValidateOldImplementationWithFlexFareChecking3)
{
  _trx.setFlexFare(false);
  ASSERT_EQ(FAIL, proxy_minimumStayApplication_Validate());
}

TEST_F(MinimumStayApplicationTest, testValidateEsvNewImplementationPossitiveCase3)
{
  setCalls();
  ASSERT_EQ(FAIL, proxy_minimumStayApplication_Validate());
}

TEST_F(MinimumStayApplicationTest, testMinStayDateEarlier)
{
  DateTime minStayDate{2015, 10, 25, 10, 10};
  _rule->minStayDate() = minStayDate;
  _rule->minStayUnit() = "H";
  _rule->earlierLaterInd() = MinMaxRulesUtils::EARLIER;
  DateTime minStayPeriodDate{2015, 10, 28, 10, 00};
  DateTime determinedDate = proxy_determineReturnDate(minStayPeriodDate);
  ASSERT_EQ(determinedDate, minStayDate);
}

TEST_F(MinimumStayApplicationTest, testMinStayDateLater)
{

  DateTime minStayDate{2015, 10, 25};
  _rule->minStayDate() = minStayDate;
  _rule->earlierLaterInd() = MinMaxRulesUtils::LATER;
  DateTime minStayPeriodDate{2015, 10, 28};
  DateTime determinedDate = proxy_determineReturnDate(minStayPeriodDate);

  ASSERT_EQ(determinedDate, minStayPeriodDate);
}

TEST_F(MinimumStayApplicationTest, testValidateReturnDateNoTodDaysUnitPass)
{
  DateTime minStayDate{2015, 10, 25, 2, 25};
  DateTime resultDate{2015, 10, 25};
  _rule->minStayUnit() = "D";
  _rule->tod() = 0;
  DateTime determinedDate = proxy_determineEarliestReturnDate(minStayDate);
  ASSERT_EQ(resultDate, determinedDate);
  ASSERT_TRUE(_diag->str().find("RETURN TIME  -  02:25") != std::string::npos);
  ASSERT_TRUE(_diag->str().find("VALIDATE AGAINST   -  2015-10-25 00:00") != std::string::npos);
  ASSERT_EQ(PASS, proxy_validateReturnDate(DateTime(2015, 10, 25, 10, 25), determinedDate));
  ASSERT_EQ(FAIL, proxy_validateReturnDate(DateTime(2015, 10, 24, 10, 25), determinedDate));
}

TEST_F(MinimumStayApplicationTest, testValidateReturnDateTodDaysUnit)
{
  DateTime earliestDate{2015, 10, 25, 2, 25};
  DateTime travelSegDate{2015, 10, 25, 10, 0};
  DateTime resultDate{2015, 10, 25, 9, 50};
  _rule->minStayUnit() = "D";
  _rule->tod() = 590;
  DateTime determinedDate = proxy_determineEarliestReturnDate(earliestDate);
  ASSERT_TRUE(_diag->str().find("RETURN TIME  -  02:25") != std::string::npos);
  ASSERT_TRUE(_diag->str().find("VALIDATE AGAINST   -  2015-10-25 09:50") != std::string::npos);
  ASSERT_EQ(resultDate, determinedDate);

  ASSERT_EQ(PASS, proxy_validateReturnDate(travelSegDate, determinedDate));
  _rule->tod() = 610;
  determinedDate = proxy_determineReturnDate(earliestDate);
  determinedDate = proxy_determineMinStayDate(determinedDate);
  ASSERT_EQ(FAIL, proxy_validateReturnDate(travelSegDate, determinedDate));
}
TEST_F(MinimumStayApplicationTest, testValidateReturnDateTodHoursUnit)
{
  DateTime earliestDate{2015, 10, 25, 10, 0};
  DateTime travelSegDate{2015, 10, 25, 11, 0};
  _rule->minStayUnit() = MinimumStayApplication::HOURS;
  _rule->tod() = 0;
  DateTime determinedDate = proxy_determineEarliestReturnDate(earliestDate);
  ASSERT_TRUE(_diag->str().find("RETURN TIME  -  10:00") != std::string::npos);
  ASSERT_TRUE(_diag->str().find("VALIDATE AGAINST   -  2015-10-25 10:00") != std::string::npos);
  _diag->clear();
  ASSERT_EQ(PASS, proxy_validateReturnDate(travelSegDate, determinedDate));
  travelSegDate = DateTime{2015, 10, 25, 9, 0};
  ASSERT_EQ(FAIL, proxy_validateReturnDate(travelSegDate, determinedDate));
  travelSegDate = DateTime{2015, 10, 25, 11, 0};
  _rule->minStayUnit() = MinimumStayApplication::MINUTES;
  _rule->tod() = 670;
  determinedDate = proxy_determineReturnDate(earliestDate);
  _diag->displayReturnDates(determinedDate);
  ASSERT_TRUE(_diag->str().find("RETURN TIME  -  10:00") != std::string::npos);
  determinedDate = proxy_determineMinStayDate(earliestDate);
  ASSERT_EQ(FAIL, proxy_validateReturnDate(travelSegDate, determinedDate));
}

TEST_F(MinimumStayApplicationTest, testDetermineReturnDate)
{
  DateTime earliestDate{2015, 10, 25, 10, 0};
  DateTime result{2015, 10, 25, 10, 0};
  _rule->minStayUnit() = MinimumStayApplication::HOURS;
  _rule->tod() = 0;
  DateTime determinedDate = proxy_determineReturnDate(earliestDate);
  determinedDate = proxy_determineMinStayDate(determinedDate);
  ASSERT_EQ(determinedDate, result);
}

} // namespace tse
