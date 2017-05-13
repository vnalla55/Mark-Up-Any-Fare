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
#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include <gmock/gmock.h>

#include "Rules/RuleControllerWithChancelor.h"

#include "DataModel/PaxTypeFare.h"
#include "Rules/FareMarketRuleController.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleProcessingData.h"
#include "Rules/RuleValidationChancelor.h"

#include "DataModel/test/MockPricingTrx.h"
#include "Rules/test/MockFareMarketRuleController.h"
#include "Rules/test/MockPricingUnitRuleController.h"
#include "Rules/test/MockRuleControllerDataAccess.h"
#include "Rules/test/MockRuleValidationChancelor.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

using ::testing::_;
using ::testing::DefaultValue;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Const;

// ==================================
// TEST
// ==================================
class RuleControllerWithChancelorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RuleControllerWithChancelorTest);

  CPPUNIT_TEST(testNoPolicy);
  CPPUNIT_TEST(testSetPolicy);

  CPPUNIT_TEST_SUITE_END();

  typedef RuleControllerWithChancelor<FareMarketRuleController,
                                      NiceMock<MockRuleValidationChancelor> > FMRuleControllerWC;

  typedef RuleControllerWithChancelor<PricingUnitRuleController,
                                      NiceMock<MockRuleValidationChancelor> > PURuleControllerWC;

  class FriendlyFMRuleControllerWC : public FMRuleControllerWC
  {
    friend class RuleControllerWithChancelorTest;

  public:
    FriendlyFMRuleControllerWC(const PricingTrx* trx = 0) : FMRuleControllerWC(trx) {}
  };

  class FriendlyPURuleControllerWC : public PURuleControllerWC
  {
    friend class RuleControllerWithChancelorTest;

  public:
    FriendlyPURuleControllerWC(const PricingTrx* trx = 0) : PURuleControllerWC(trx) {}
  };

private:
  RuleValidationContext _context;
  TestMemHandle _memHandle;
  MockRuleValidationChancelor* _chancelor;
  FriendlyFMRuleControllerWC* _fmRuleController;
  FriendlyPURuleControllerWC* _puRuleController;

  MockPricingTrx* _trx;
  NiceMock<MockRuleControllerDataAccess>* _da;
  RuleProcessingData* _rpData;
  RuleValidationMonitor* _monitor;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _trx = _memHandle.create<MockPricingTrx>();
    _trx->setTrxType(PricingTrx::MIP_TRX);
    _trx->setFlexFare(true);

    _monitor = _memHandle.create<RuleValidationMonitor>();
    DefaultValue<RuleValidationMonitor&>::Set(*_monitor);

    _fmRuleController = _memHandle.create<FriendlyFMRuleControllerWC>(_trx);
    _puRuleController = _memHandle.create<FriendlyPURuleControllerWC>(_trx);

    _da = _memHandle.create<NiceMock<MockRuleControllerDataAccess> >();
    _rpData = _memHandle.create<RuleProcessingData>();
  }

  void tearDown() { _memHandle.clear(); }

  void testNoPolicy()
  {
    _chancelor = &_fmRuleController->getMutableChancelor();

    _context._paxTypeFare = 0;
    EXPECT_CALL(Const(*_chancelor), getContext()).Times(0);
    EXPECT_CALL(*_chancelor, hasPolicy(_)).Times(0);
    EXPECT_CALL(*_chancelor, getPolicy(_)).Times(0);
    EXPECT_CALL(*_chancelor, getMutableMonitor()).Times(0);

    Record3ReturnTypes result = FAIL;
    ASSERT_EQ(result,
              _fmRuleController->doCategoryPostProcessing(
                  *_trx, *_da, RuleConst::PENALTIES_RULE, *_rpData, result));
  }

  void testSetPolicy()
  {
    using namespace boost::assign;
    RuleControllerWithChancelor<FareMarketRuleController> controller;
    std::vector<uint16_t> categories;

    categories += ELIGIBILITY_RULE, ADVANCE_RESERVATION_RULE, MINIMUM_STAY_RULE, MAXIMUM_STAY_RULE,
        PENALTIES_RULE; // FlexFares categories
    for (unsigned i = 0; i < categories.size(); i++)
      ASSERT_TRUE(controller.getMutableChancelor().hasPolicy(categories[i]));

    categories.clear();
    categories += DAY_TIME_RULE, SEASONAL_RULE, FLIGHT_APPLICATION_RULE, STOPOVER_RULE,
        TRANSFER_RULE, COMBINABILITY_RULE, SALE_RESTRICTIONS_RULE, HIP_RULE; // random categories
    for (unsigned i = 0; i < categories.size(); i++)
      ASSERT_FALSE(controller.getMutableChancelor().hasPolicy(categories[i]));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RuleControllerWithChancelorTest);
} // tse
