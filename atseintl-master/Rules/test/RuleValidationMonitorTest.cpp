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

#include <boost/range.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include <gmock/gmock.h>

#include "Common/TseEnums.h"
#include "DataModel/FlexFares/ValidationStatus.h"
#include "Rules/RuleValidationContext.h"
#include "Rules/RuleValidationMonitor.h"
#include "Rules/RuleValidationObserver.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
// ==================================
// TEST
// ==================================
using ::testing::_;
using ::testing::StrictMock;

class ObserverMock : public RuleValidationObserver
{
public:
  MOCK_METHOD4(update,
               void(const RuleValidationContext&,
                    flexFares::ValidationStatusPtr,
                    const uint16_t&,
                    const Record3ReturnTypes&));
};

class MonitorTest : public CppUnit::TestFixture
{
  typedef RuleValidationMonitor Monitor;

  CPPUNIT_TEST_SUITE(MonitorTest);

  CPPUNIT_TEST(testMonitorSubscribe);
  CPPUNIT_TEST(testMonitorUnsubscribe);
  CPPUNIT_TEST(testNotifyTwoObserversOneEvent);
  CPPUNIT_TEST(testNotifyTwoObserversTwoEvents);

  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  Monitor* _monitor;
  RuleValidationContext* _context;
  flexFares::ValidationStatusPtr _validationStatus;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _monitor = _memHandle.create<Monitor>();
    _context = _memHandle.create<RuleValidationContext>();
    _validationStatus = std::make_shared<flexFares::ValidationStatus>();
  }

  void tearDown() { _memHandle.clear(); }

  void testMonitorSubscribe()
  {
    StrictMock<ObserverMock> observer;
    EXPECT_CALL(observer, update(_, _, _, _));

    _monitor->subscribe(&observer, Monitor::VALIDATION_RESULT);

    _monitor->notify(
        Monitor::VALIDATION_RESULT, *_context, _validationStatus, PENALTIES_RULE, PASS);
  }

  void testMonitorUnsubscribe()
  {
    StrictMock<ObserverMock> observer;
    EXPECT_CALL(observer, update(_, _, _, _)).Times(0);

    _monitor->subscribe(&observer, Monitor::VALIDATION_RESULT);

    _monitor->unsubscribe(&observer, Monitor::VALIDATION_RESULT);

    _monitor->notify(
        Monitor::VALIDATION_RESULT, *_context, _validationStatus, PENALTIES_RULE, PASS);
  }

  void testNotifyTwoObserversOneEvent()
  {
    StrictMock<ObserverMock> obs1, obs2;
    EXPECT_CALL(obs1, update(_, _, PENALTIES_RULE, PASS));
    EXPECT_CALL(obs2, update(_, _, PENALTIES_RULE, PASS));

    _monitor->subscribe(&obs1, Monitor::VALIDATION_RESULT);
    _monitor->subscribe(&obs2, Monitor::VALIDATION_RESULT);

    _monitor->notify(
        Monitor::VALIDATION_RESULT, *_context, _validationStatus, PENALTIES_RULE, PASS);
  }

  void testNotifyTwoObserversTwoEvents()
  {
    StrictMock<ObserverMock> obs1, obs2;
    EXPECT_CALL(obs2, update(_, _, PENALTIES_RULE, PASS));

    _monitor->subscribe(&obs1, Monitor::VALIDATION_START);
    _monitor->subscribe(&obs2, Monitor::VALIDATION_RESULT);

    _monitor->notify(
        Monitor::VALIDATION_RESULT, *_context, _validationStatus, PENALTIES_RULE, PASS);
  }

}; // MonitorTest

CPPUNIT_TEST_SUITE_REGISTRATION(MonitorTest);
} // tse
