// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Pricing/StructuredFareRulesUtils.h"
#include "Rules/SubjectObserved.h"
#include "Rules/UpdaterObserver.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/StructuredRuleTrx.h"
#include "Rules/SalesRestrictionRule.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <gtest/gtest.h>

namespace tse
{
using namespace ::testing;

using PaxTypeFareObserver = AdvanceResAndTktObserverType<PaxTypeFare>;
using FareUsageObserver = AdvanceResAndTktObserverType<FareUsage>;

class AdvanceResAndTktObserversTest : public Test
{
public:
  void SetUp() override
  {
    _fareUsage = _memHandle.create<FareUsage>();
    _trx = _memHandle.create<StructuredRuleTrx>();
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _salesRestrictionRule = _memHandle.create<SalesRestrictionRule>();
  }

  void TearDown() override { _memHandle.clear(); }

  TestMemHandle _memHandle;
  FareUsage* _fareUsage;
  PaxTypeFare* _paxTypeFare;
  StructuredRuleTrx* _trx;
  SalesRestrictionRule* _salesRestrictionRule;

  const DateTime date1 = DateTime(2015, 10, 10);
  const DateTime date2 = DateTime(2015, 9, 10, 22, 10, 0);
};

TEST_F(AdvanceResAndTktObserversTest, TestLastDateToBookLastMinuteOfDay)
{
  auto fareUsageobserver = FareUsageObserver::create(
      ObserverType::ADVANCE_RESERVATION_SFR, _trx->dataHandle(), _salesRestrictionRule);
  auto paxTypeFareObserver = PaxTypeFareObserver::create(
      ObserverType::ADVANCE_RESERVATION_SFR, _trx->dataHandle(), _salesRestrictionRule);

  fareUsageobserver->notify(NotificationType::LAST_DATE_TO_BOOK, date1);
  paxTypeFareObserver->notify(NotificationType::LAST_DATE_TO_BOOK, date2);
  fareUsageobserver->updateIfNotified(*_fareUsage);
  paxTypeFareObserver->updateIfNotified(*_paxTypeFare);

  ASSERT_TRUE(_fareUsage->hasStructuredRuleData());
  ASSERT_TRUE(_paxTypeFare->hasStructuredRuleData());

  ASSERT_EQ(DateTime(2015, 10, 10, 23, 59, 0),
            _fareUsage->getStructuredRuleData()._advanceReservation);
  ASSERT_EQ(date2, _paxTypeFare->getStructuredRuleData()._advanceReservation);
  ASSERT_FALSE(_fareUsage->getStructuredRuleData()._advanceTicketing.isValid());
}

TEST_F(AdvanceResAndTktObserversTest, TestEarlierAdvanceReservation)
{
  auto fareUsageLastDayToBookobserver = FareUsageObserver::create(
      ObserverType::ADVANCE_RESERVATION_SFR, _trx->dataHandle(), _salesRestrictionRule);

  fareUsageLastDayToBookobserver->notify(NotificationType::LAST_DATE_TO_BOOK, date1);
  fareUsageLastDayToBookobserver->updateIfNotified(*_fareUsage);
  fareUsageLastDayToBookobserver->notify(NotificationType::LAST_DATE_TO_BOOK, date2);
  fareUsageLastDayToBookobserver->updateIfNotified(*_fareUsage);
  ASSERT_TRUE(_fareUsage->hasStructuredRuleData());
  ASSERT_EQ(date2, _fareUsage->getStructuredRuleData()._advanceReservation);
  ASSERT_FALSE(_fareUsage->getStructuredRuleData()._advanceTicketing.isValid());
}

TEST_F(AdvanceResAndTktObserversTest, TestLatestTktDayLastMinuteOfDay)
{
  auto fareUsageobserver = FareUsageObserver::create(
      ObserverType::ADVANCE_TICKETING_SFR, _trx->dataHandle(), _salesRestrictionRule);
  auto paxTypeFareObserver = PaxTypeFareObserver::create(
      ObserverType::ADVANCE_TICKETING_SFR, _trx->dataHandle(), _salesRestrictionRule);

  fareUsageobserver->notify(NotificationType::LATEST_TKT_DAY, date1);
  paxTypeFareObserver->notify(NotificationType::LATEST_TKT_DAY, date1);

  fareUsageobserver->updateIfNotified(*_fareUsage);
  paxTypeFareObserver->updateIfNotified(*_paxTypeFare);

  ASSERT_TRUE(_fareUsage->hasStructuredRuleData());
  ASSERT_TRUE(_paxTypeFare->hasStructuredRuleData());

  ASSERT_EQ(DateTime(2015, 10, 10, 23, 59, 0),
            _fareUsage->getStructuredRuleData()._advanceTicketing);
  ASSERT_EQ(DateTime(2015, 10, 10, 23, 59, 0),
            _paxTypeFare->getStructuredRuleData()._advanceTicketing);
  ASSERT_FALSE(_paxTypeFare->getStructuredRuleData()._advanceReservation.isValid());
}

TEST_F(AdvanceResAndTktObserversTest, TestEarlierLatestTicketing)
{
  auto fareUsageLastDayToBookobserver = FareUsageObserver::create(
      ObserverType::ADVANCE_TICKETING_SFR, _trx->dataHandle(), _salesRestrictionRule);

  fareUsageLastDayToBookobserver->notify(NotificationType::LATEST_TKT_DAY, date1);
  fareUsageLastDayToBookobserver->updateIfNotified(*_fareUsage);
  fareUsageLastDayToBookobserver->notify(NotificationType::LATEST_TKT_DAY, date2);
  fareUsageLastDayToBookobserver->updateIfNotified(*_fareUsage);
  ASSERT_TRUE(_fareUsage->hasStructuredRuleData());
  ASSERT_EQ(date2, _fareUsage->getStructuredRuleData()._advanceTicketing);
  ASSERT_FALSE(_fareUsage->getStructuredRuleData()._advanceReservation.isValid());
}
}
