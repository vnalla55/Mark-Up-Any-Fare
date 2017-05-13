// -------------------------------------------------------------------
//
//
//  Copyright (C) Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include "Rules/RuleValidationChancelor.h"

#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FareUsage.h"
#include "Rules/test/MockRuleControllerDataAccess.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include "test/include/CppUnitHelperMacros.h"
#include <gmock/gmock.h>

namespace tse
{

using ::testing::Return;
using ::testing::ReturnPointee;
using ::testing::ReturnRef;

// ==================================
// TEST
// ==================================
class RuleValidationChancelorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RuleValidationChancelorTest);

  CPPUNIT_TEST(testContextDefaultValues);
  CPPUNIT_TEST(testUpdateContextType);
  CPPUNIT_TEST(testUpdateContext);

  CPPUNIT_TEST_SUITE_END();

  typedef RuleValidationContext Context;

private:
  TestMemHandle _memHandle;
  RuleValidationChancelor* _chancelor;
  MockRuleControllerDataAccess* _da;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _chancelor = _memHandle.create<RuleValidationChancelor>();
    _da = _memHandle.create<MockRuleControllerDataAccess>();
  }

  void tearDown() { _memHandle.clear(); }

  void checkContextValues(const Context& context,
                          PricingTrx* const trx,
                          PaxTypeFare* const ptf,
                          FareUsage* const fu,
                          const flexFares::GroupId& groupId,
                          const Context::ContextType& ct)
  {
    ASSERT_EQ(trx, context._trx);
    ASSERT_EQ(ptf, context._paxTypeFare);
    ASSERT_EQ(fu, context._fareUsage);
    ASSERT_EQ(groupId, context._groupId);
    ASSERT_EQ(ct, context._contextType);
  }

  void testContextDefaultValues()
  {
    checkContextValues(_chancelor->getContext(), 0, 0, 0, 0, Context::FARE_MARKET);
  }

  void testUpdateContextType()
  {
    _chancelor->updateContextType(RuleValidationContext::PU_FP);
    checkContextValues(_chancelor->getContext(), 0, 0, 0, 0, Context::PU_FP);
  }

  void testUpdateContext()
  {
    PricingTrx trx;
    PaxTypeFare ptf;
    FareUsage fu;

    EXPECT_CALL(*_da, trx()).Times(2).WillRepeatedly(ReturnRef(trx));
    EXPECT_CALL(*_da, paxTypeFare()).WillOnce(ReturnRef(ptf));
    EXPECT_CALL(*_da, getFareUsage()).WillOnce(Return(&fu));

    _chancelor->updateContext(*_da);

    checkContextValues(_chancelor->getContext(), &trx, &ptf, &fu, 0, Context::FARE_MARKET);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RuleValidationChancelorTest);
} // tse
