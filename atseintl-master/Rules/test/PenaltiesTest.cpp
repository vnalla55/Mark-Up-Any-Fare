//------------------------------------------------------------------
//
//  File: PenaltiesTest.cpp
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
#include "test/include/CppUnitHelperMacros.h"
#include <gmock/gmock.h>

#include "Rules/Penalties.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingOptions.h"

#include "Rules/test/MockRuleValidationChancelor.h"
#include "Rules/test/MockPenalties.h"
#include "Rules/test/MockRuleValidationPolicy.h"
#include "DataModel/test/MockPricingTrx.h"
#include "test/include/TestConfigInitializer.h"

#include <memory>

namespace tse
{
using namespace ::testing;

class PenaltiesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PenaltiesTest);
  CPPUNIT_TEST(testFlexFaresWithoutValidationFU1);
  CPPUNIT_TEST(testFlexFaresWithoutValidationFU2);
  CPPUNIT_TEST(testFlexFaresWithoutValidationFU3);
  CPPUNIT_TEST(testFlexFaresWithValidationFU1);
  CPPUNIT_TEST(testFlexFaresWithValidationFU2);
  CPPUNIT_TEST(testFlexFaresWithoutValidationFM1);
  CPPUNIT_TEST(testFlexFaresWithoutValidationFM2);
  CPPUNIT_TEST(testFlexFaresWithoutValidationFM3);
  CPPUNIT_TEST(testFlexFaresWithValidationFM1);
  CPPUNIT_TEST(testFlexFaresWithValidationFM2);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _cfg.reset(new TestConfigInitializer);
    _category = RuleConst::PENALTIES_RULE;
    _penaltiesMock.reset(new MockPenalties());
    _chancelorMock.reset(new MockRuleValidationChancelor());
    _ruleValidationPolicyMock.reset(new MockRuleValidationPolicy());
    _pricingTrxMock.reset(new MockPricingTrx());
    _pricingOptions.noPenalties() = 'A';
    _pricingTrxMock->setOptions(&_pricingOptions);
    _result1 = PASS;
    _result2 = SOFTPASS;
  }

  void tearDown() { _cfg.reset(); }

  void testFlexFaresWithoutValidationFU1()
  {
    _pricingTrxMock->setFlexFare(false);
    _penaltiesMock.reset(new MockPenalties());

    EXPECT_CALL(*_penaltiesMock, validateOptions(_, _)).Times(0);
    ASSERT_EQ(_result1,
              _penaltiesMock->validate(*_pricingTrxMock, &_info, _pricingUnit, _fareUsage));
  }

  void testFlexFaresWithoutValidationFU2()
  {
    _pricingTrxMock->setFlexFare(true);
    _penaltiesMock->setChancelor(_chancelorMock);
    EXPECT_CALL(*_chancelorMock, hasPolicy(_category)).WillOnce(Return(false));

    EXPECT_CALL(*_penaltiesMock, validateOptions(_, _)).Times(0);
    ASSERT_EQ(_result1,
              _penaltiesMock->validate(*_pricingTrxMock, &_info, _pricingUnit, _fareUsage));
  }

  void testFlexFaresWithoutValidationFU3()
  {
    _pricingTrxMock->setFlexFare(true);
    _penaltiesMock->setChancelor(_chancelorMock);
    EXPECT_CALL(*_chancelorMock, hasPolicy(_category)).WillOnce(Return(true));
    EXPECT_CALL(Const(*_chancelorMock), getPolicy(_category))
        .WillOnce(ReturnRef(*_ruleValidationPolicyMock));
    EXPECT_CALL(Const(*_chancelorMock), getContext()).WillOnce(ReturnRef(_context));
    EXPECT_CALL(*_ruleValidationPolicyMock, shouldPerform(_)).WillOnce(Return(false));

    EXPECT_CALL(*_penaltiesMock, validateOptions(_, _)).Times(0);
    ASSERT_EQ(_result1,
              _penaltiesMock->validate(*_pricingTrxMock, &_info, _pricingUnit, _fareUsage));
  }

  void testFlexFaresWithValidationFU1()
  {
    _pricingTrxMock->setFlexFare(false);
    _pricingOptions.noPenalties() = 'T';
    _pricingTrxMock->setOptions(&_pricingOptions);

    EXPECT_CALL(*_penaltiesMock, validateOptions(_, _)).WillOnce(Return(_result1));
    ASSERT_EQ(_result1,
              _penaltiesMock->validate(*_pricingTrxMock, &_info, _pricingUnit, _fareUsage));
  }

  void testFlexFaresWithValidationFU2()
  {
    _pricingTrxMock->setFlexFare(true);
    _penaltiesMock->setChancelor(_chancelorMock);
    EXPECT_CALL(*_chancelorMock, hasPolicy(_category)).WillOnce(Return(true));
    EXPECT_CALL(Const(*_chancelorMock), getPolicy(_category)).Times(2).WillRepeatedly(
        ReturnRef(*_ruleValidationPolicyMock));
    EXPECT_CALL(Const(*_chancelorMock), getContext()).Times(2).WillRepeatedly(ReturnRef(_context));
    EXPECT_CALL(*_ruleValidationPolicyMock, shouldPerform(_)).WillOnce(Return(true));
    EXPECT_CALL(*_ruleValidationPolicyMock, shouldReturn()).WillOnce(Return(true));

    EXPECT_CALL(*_penaltiesMock, validateOptions(_, _)).WillOnce(Return(_result1));
    ASSERT_EQ(_result1,
              _penaltiesMock->validate(*_pricingTrxMock, &_info, _pricingUnit, _fareUsage));
  }

  void testFlexFaresWithoutValidationFM1()
  {
    _pricingTrxMock->setFlexFare(false);
    _penaltiesMock.reset(new MockPenalties());

    EXPECT_CALL(*_penaltiesMock, validateOptions(_, _)).Times(0);
    ASSERT_EQ(_result2,
              _penaltiesMock->validate(*_pricingTrxMock, _itin, _paxTypeFare, &_info, _fareMarket));
  }

  void testFlexFaresWithoutValidationFM2()
  {
    _pricingTrxMock->setFlexFare(true);
    _penaltiesMock->setChancelor(_chancelorMock);
    EXPECT_CALL(*_chancelorMock, hasPolicy(_category)).WillOnce(Return(false));

    EXPECT_CALL(*_penaltiesMock, validateOptions(_, _)).Times(0);
    ASSERT_EQ(_result2,
              _penaltiesMock->validate(*_pricingTrxMock, _itin, _paxTypeFare, &_info, _fareMarket));
  }

  void testFlexFaresWithoutValidationFM3()
  {
    _pricingTrxMock->setFlexFare(true);
    _penaltiesMock->setChancelor(_chancelorMock);
    EXPECT_CALL(*_chancelorMock, hasPolicy(_category)).WillOnce(Return(true));
    EXPECT_CALL(Const(*_chancelorMock), getPolicy(_category))
        .WillOnce(ReturnRef(*_ruleValidationPolicyMock));
    EXPECT_CALL(Const(*_chancelorMock), getContext()).WillOnce(ReturnRef(_context));
    EXPECT_CALL(*_ruleValidationPolicyMock, shouldPerform(_)).WillOnce(Return(false));

    EXPECT_CALL(*_penaltiesMock, validateOptions(_, _)).Times(0);
    ASSERT_EQ(_result2,
              _penaltiesMock->validate(*_pricingTrxMock, _itin, _paxTypeFare, &_info, _fareMarket));
  }

  void testFlexFaresWithValidationFM1()
  {
    _pricingTrxMock->setFlexFare(false);
    _pricingOptions.noPenalties() = 'T';
    _pricingTrxMock->setOptions(&_pricingOptions);

    EXPECT_CALL(*_penaltiesMock, validateOptions(_, _)).WillOnce(Return(_result2));
    ASSERT_EQ(_result2,
              _penaltiesMock->validate(*_pricingTrxMock, _itin, _paxTypeFare, &_info, _fareMarket));
  }

  void testFlexFaresWithValidationFM2()
  {
    _pricingTrxMock->setFlexFare(true);
    _penaltiesMock->setChancelor(_chancelorMock);
    EXPECT_CALL(*_chancelorMock, hasPolicy(_category)).WillOnce(Return(true));
    EXPECT_CALL(Const(*_chancelorMock), getPolicy(_category)).Times(2).WillRepeatedly(
        ReturnRef(*_ruleValidationPolicyMock));
    EXPECT_CALL(Const(*_chancelorMock), getContext()).Times(2).WillRepeatedly(ReturnRef(_context));
    EXPECT_CALL(*_ruleValidationPolicyMock, shouldPerform(_)).WillOnce(Return(true));
    EXPECT_CALL(*_ruleValidationPolicyMock, shouldReturn()).WillOnce(Return(true));

    EXPECT_CALL(*_penaltiesMock, validateOptions(_, _)).WillOnce(Return(_result2));
    ASSERT_EQ(_result2,
              _penaltiesMock->validate(*_pricingTrxMock, _itin, _paxTypeFare, &_info, _fareMarket));
  }

  std::unique_ptr<TestConfigInitializer> _cfg;
  Record3ReturnTypes _result1, _result2;
  Itin _itin;
  PaxTypeFare _paxTypeFare;
  FareMarket _fareMarket;
  uint16_t _category;
  PenaltyInfo _info;
  PricingUnit _pricingUnit;
  FareUsage _fareUsage;
  RuleValidationContext _context;
  std::shared_ptr<MockRuleValidationChancelor> _chancelorMock;
  std::shared_ptr<MockPenalties> _penaltiesMock;
  std::shared_ptr<MockRuleValidationPolicy> _ruleValidationPolicyMock;
  std::shared_ptr<MockPricingTrx> _pricingTrxMock;
  PricingOptions _pricingOptions;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PenaltiesTest);

} // namespace tse
