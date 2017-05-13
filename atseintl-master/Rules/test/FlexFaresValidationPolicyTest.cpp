//------------------------------------------------------------------
//
//  File: FlexFaresValidationPolicyParametrizedTest.cpp
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

#include "test/include/GtestHelperMacros.h"
#include "Rules/FlexFaresValidationPolicy.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"

namespace tse
{
using namespace ::testing;

class FlexFaresValidationPolicyTest : public Test
{
public:
  void SetUp()
  {
    _context._trx = &_pricingTrx;
    _context._groupId = 1u;
  }

  void TearDown() {}

  PricingTrx _pricingTrx;
  RuleValidationContext _context;
  PricingRequest _pricingReq;
  flexFares::GroupsData _groupsData;
};

TEST_F(FlexFaresValidationPolicyTest, testShouldPerformNegativeCaseNoPenalties)
{
  FlexFaresValidationPolicyNoPenalties validationPolicy;

  _pricingTrx.setTrxType(PricingTrx::MIP_TRX);
  _groupsData.requireNoPenalties(_context._groupId + 1);
  _pricingTrx.setRequest(&_pricingReq);
  _pricingReq.getMutableFlexFaresGroupsData() = _groupsData;

  ASSERT_FALSE(validationPolicy.shouldPerform(_context));
}

TEST_F(FlexFaresValidationPolicyTest, testShouldPerformNegativeCaseNoMinMax)
{
  FlexFaresValidationPolicyNoMinMax validationPolicy;
  _context._contextType = RuleValidationContext::FARE_MARKET;
  _pricingTrx.setTrxType(PricingTrx::MIP_TRX);
  _groupsData.requireNoMinMaxStay(_context._groupId + 1);
  _pricingTrx.setRequest(&_pricingReq);
  _pricingReq.getMutableFlexFaresGroupsData() = _groupsData;

  ASSERT_FALSE(validationPolicy.shouldPerform(this->_context));
}

TEST_F(FlexFaresValidationPolicyTest, testShouldPerformNegativeCaseNoAdvancePurchase)
{
  FlexFaresValidationPolicyNoAdvancePurchase validationPolicy;
  _context._contextType = RuleValidationContext::FARE_MARKET;
  _pricingTrx.setTrxType(PricingTrx::MIP_TRX);
  _groupsData.requireNoAdvancePurchase(_context._groupId + 1);
  _pricingTrx.setRequest(&_pricingReq);
  _pricingReq.getMutableFlexFaresGroupsData() = _groupsData;

  ASSERT_FALSE(validationPolicy.shouldPerform(_context));
}

TEST_F(FlexFaresValidationPolicyTest, testShouldPerformNegativeCaseNoEligibility)
{
  FlexFaresValidationPolicyNoEligibility validationPolicy;
  _context._contextType = RuleValidationContext::FARE_MARKET;
  _pricingTrx.setTrxType(PricingTrx::MIP_TRX);
  _groupsData.addCorpId("123", _context._groupId);
  _pricingTrx.setRequest(&_pricingReq);
  _pricingReq.getMutableFlexFaresGroupsData() = _groupsData;

  ASSERT_FALSE(validationPolicy.shouldPerform(_context));
}

template <typename T>
class FlexFaresValidationPolicyParametrizedTest : public FlexFaresValidationPolicyTest
{
public:
  void SetUp()
  {
    FlexFaresValidationPolicyTest::SetUp();
    _groupsData.requireNoAdvancePurchase(_context._groupId);
    _groupsData.requireNoMinMaxStay(_context._groupId);
    _groupsData.requireNoPenalties(_context._groupId);
    _groupsData.addCorpId("123", _context._groupId);
    _groupsData.addAccCode("123", _context._groupId);
  }

  void TearDown() {}
};
TYPED_TEST_CASE_P(FlexFaresValidationPolicyParametrizedTest);

TYPED_TEST_P(FlexFaresValidationPolicyParametrizedTest, testShouldPerformNegativeCase1)
{
  TypeParam validationPolicy;
  ASSERT_FALSE(validationPolicy.shouldPerform(this->_context));
}

TYPED_TEST_P(FlexFaresValidationPolicyParametrizedTest, testShouldPerformNegativeCase2)
{
  TypeParam validationPolicy;
  this->_pricingTrx.setTrxType(PricingTrx::REPRICING_TRX);
  ASSERT_FALSE(validationPolicy.shouldPerform(this->_context));
}

TYPED_TEST_P(FlexFaresValidationPolicyParametrizedTest, testShouldPerformPossitiveCase)
{
  TypeParam validationPolicy;
  this->_context._contextType = RuleValidationContext::PU_FP;
  this->_pricingTrx.setTrxType(PricingTrx::MIP_TRX);
  this->_pricingTrx.setRequest(&(this->_pricingReq));
  this->_pricingReq.getMutableFlexFaresGroupsData() = this->_groupsData;

  ASSERT_TRUE(validationPolicy.shouldPerform(this->_context));
}

REGISTER_TYPED_TEST_CASE_P(FlexFaresValidationPolicyParametrizedTest,
                           testShouldPerformNegativeCase1,
                           testShouldPerformNegativeCase2,
                           testShouldPerformPossitiveCase);

typedef ::testing::Types<FlexFaresValidationPolicyNoEligibility,
                         FlexFaresValidationPolicyNoPenalties,
                         FlexFaresValidationPolicyNoMinMax,
                         FlexFaresValidationPolicyNoAdvancePurchase> FlexFaresValidationPolicyTypes;

INSTANTIATE_TYPED_TEST_CASE_P(TestInstance,
                              FlexFaresValidationPolicyParametrizedTest,
                              FlexFaresValidationPolicyTypes);

} // namespace tse
