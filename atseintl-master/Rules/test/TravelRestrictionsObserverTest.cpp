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
#include "Rules/TravelRestrictions.h"

#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <gtest/gtest.h>

namespace tse
{
using namespace ::testing;

using PTFObserver = TravelRestrictionsObserverType<PaxTypeFare>;
using FUObserver = TravelRestrictionsObserverType<FareUsage>;

class TravelRestrictionsObserverTest : public Test
{
public:
  void SetUp() override
  {
    _fareUsage = _memHandle.create<FareUsage>();
    _trx = _memHandle.create<StructuredRuleTrx>();
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
  }

  void TearDown() override { _memHandle.clear(); }

  TestMemHandle _memHandle;
  FareUsage* _fareUsage;
  PaxTypeFare* _paxTypeFare;
  StructuredRuleTrx* _trx;

  const SegmentOrder segOrder = 2;
  const LocCode location = LocCode("LON");
  const DateTime date = DateTime(2015, 10, 10);
};

static const int NO_TIME = -1;

TEST_F(TravelRestrictionsObserverTest, TestDifferentNotificationTypeDoNotNotify)
{
  TravelRestrictions tvlRestrictions;
  auto observer =
      FUObserver::create(ObserverType::TRV_REST_COMMENCE_SFR, _trx->dataHandle(), &tvlRestrictions);
  observer->notify(
      NotificationType::PAX_TYPE_FARE_TRAVEL_MUST_COMPLETE, segOrder, date, location, NO_TIME);

  observer->updateIfNotified(*_fareUsage);

  ASSERT_TRUE(!_fareUsage->getNVAData());
  ASSERT_FALSE(_fareUsage->hasStructuredRuleData());
}

TEST_F(TravelRestrictionsObserverTest, TestSFRTvlRestCompleteFareUsageAddLastMinuteOfDay)
{
  MaxStayData maxStayData;
  maxStayData._mustComplete = date;
  maxStayData._location = location;

  maxStayData._mustComplete.addMinutes(23 * 60 + 59);

  TravelRestrictions tvlRestrictions;
  auto observer =
      FUObserver::create(ObserverType::TRV_REST_COMPLETE_SFR, _trx->dataHandle(), &tvlRestrictions);

  observer->notify(NotificationType::TRAVEL_MUST_COMPLETE, segOrder, date, location, NO_TIME);

  observer->updateIfNotified(*_fareUsage);
  ASSERT_TRUE(_fareUsage->hasStructuredRuleData());
  auto nvaData = _fareUsage->getNVAData();
  ASSERT_EQ((*nvaData)[segOrder], &date);
  auto sfrMaxMap = _fareUsage->getStructuredRuleData()._maxStayMostRestrictiveFCData;
  ASSERT_EQ(maxStayData, sfrMaxMap[segOrder]);
}

TEST_F(TravelRestrictionsObserverTest, TestTvlRestCompleteFareUsageNoSFRData)
{
  TravelRestrictions tvlRestrictions;
  auto observer =
      FUObserver::create(ObserverType::TRV_REST_COMPLETE, _trx->dataHandle(), &tvlRestrictions);

  observer->notify(NotificationType::TRAVEL_MUST_COMPLETE, segOrder, date, location, NO_TIME);

  observer->updateIfNotified(*_fareUsage);

  auto nvaData = _fareUsage->getNVAData();
  ASSERT_EQ((*nvaData)[segOrder], &date);
  ASSERT_FALSE(_fareUsage->hasStructuredRuleData());
}

TEST_F(TravelRestrictionsObserverTest, TestSFRTvlRestCommenceFareUsageAddLastMinuteOfDay)
{
  MaxStayData maxStayData;
  maxStayData._mustCommence = date;
  maxStayData._location = location;

  maxStayData._mustCommence.addMinutes(23 * 60 + 59);

  TravelRestrictions tvlRestrictions;
  auto observer =
      FUObserver::create(ObserverType::TRV_REST_COMMENCE_SFR, _trx->dataHandle(), &tvlRestrictions);

  observer->notify(NotificationType::TRAVEL_MUST_COMMENCE, segOrder, date, location, NO_TIME);

  observer->updateIfNotified(*_fareUsage);

  ASSERT_TRUE(_fareUsage->hasStructuredRuleData());
  auto nvaData = _fareUsage->getNVAData();
  ASSERT_EQ((*nvaData)[segOrder], &date);
  auto sfrMaxMap = _fareUsage->getStructuredRuleData()._maxStayMostRestrictiveFCData;
  ASSERT_EQ(maxStayData, sfrMaxMap[segOrder]);
}

TEST_F(TravelRestrictionsObserverTest, TestSFRTvlRestCommenceFareUsageDoNotAddLastMinuteOfDay)
{
  DateTime dateWithTime(2015, 11, 10, 11, 12);
  MaxStayData maxStayData;
  maxStayData._mustCommence = dateWithTime;
  maxStayData._location = location;

  TravelRestrictions tvlRestrictions;
  auto observer =
      FUObserver::create(ObserverType::TRV_REST_COMMENCE_SFR, _trx->dataHandle(), &tvlRestrictions);

  observer->notify(
      NotificationType::TRAVEL_MUST_COMMENCE, segOrder, dateWithTime, location, NO_TIME);

  observer->updateIfNotified(*_fareUsage);

  ASSERT_TRUE(_fareUsage->hasStructuredRuleData());
  auto nvaData = _fareUsage->getNVAData();
  ASSERT_EQ((*nvaData)[segOrder], &dateWithTime);
  auto sfrMaxMap = _fareUsage->getStructuredRuleData()._maxStayMostRestrictiveFCData;
  ASSERT_EQ(maxStayData, sfrMaxMap[segOrder]);
}

TEST_F(TravelRestrictionsObserverTest, TestSFRTvlRestCompletePaxTypeFareAddLastMinuteOfDay)
{
  MaxStayData maxStayData;
  maxStayData._mustComplete = date;
  maxStayData._location = location;

  maxStayData._mustComplete.addMinutes(23 * 60 + 59);

  TravelRestrictions tvlRestrictions;
  auto observer = PTFObserver::create(
      ObserverType::TRV_REST_COMPLETE_SFR, _trx->dataHandle(), &tvlRestrictions);

  observer->notify(
      NotificationType::PAX_TYPE_FARE_TRAVEL_MUST_COMPLETE, segOrder, date, location, NO_TIME);

  observer->updateIfNotified(*_paxTypeFare);
  ASSERT_TRUE(_paxTypeFare->hasStructuredRuleData());
  auto nvaData = _paxTypeFare->getNVAData();
  ASSERT_EQ((*nvaData)[segOrder], &date);
  auto sfrMaxMap = _paxTypeFare->getStructuredRuleData()._maxStayMap;
  ASSERT_EQ(maxStayData, sfrMaxMap[segOrder]);
}

TEST_F(TravelRestrictionsObserverTest, TestTvlRestCompletePaxTypeFareNoSFR)
{
  MaxStayData maxStayData;
  maxStayData._mustComplete = date;
  maxStayData._location = location;

  maxStayData._mustComplete.addMinutes(23 * 60 + 59);

  TravelRestrictions tvlRestrictions;
  auto observer =
      PTFObserver::create(ObserverType::TRV_REST_COMPLETE, _trx->dataHandle(), &tvlRestrictions);

  observer->notify(
      NotificationType::PAX_TYPE_FARE_TRAVEL_MUST_COMPLETE, segOrder, date, location, NO_TIME);

  observer->updateIfNotified(*_paxTypeFare);
  ASSERT_FALSE(_paxTypeFare->hasStructuredRuleData());
  auto nvaData = _paxTypeFare->getNVAData();
  ASSERT_EQ((*nvaData)[segOrder], &date);
}

TEST_F(TravelRestrictionsObserverTest, TestTvlRestWithTimeSFR)
{
  const int time = 127;

  MaxStayData maxStayData;
  maxStayData._mustComplete = date;
  maxStayData._location = location;
  maxStayData._mustComplete.addMinutes(time);

  TravelRestrictions tvlRestrictions;
  auto observer = PTFObserver::create(
      ObserverType::TRV_REST_COMPLETE_SFR, _trx->dataHandle(), &tvlRestrictions);

  observer->notify(
      NotificationType::PAX_TYPE_FARE_TRAVEL_MUST_COMPLETE, segOrder, date, location, time);

  observer->updateIfNotified(*_paxTypeFare);
  ASSERT_TRUE(_paxTypeFare->hasStructuredRuleData());
  auto nvaData = _paxTypeFare->getNVAData();
  ASSERT_EQ((*nvaData)[segOrder], &date);
  auto sfrMaxMap = _paxTypeFare->getStructuredRuleData()._maxStayMap;
  ASSERT_EQ(maxStayData, sfrMaxMap[segOrder]);
}

TEST_F(TravelRestrictionsObserverTest, TestTvlRestWithTimeMidnightSaveMinuteLess)
{
  const int time = 1440;

  MaxStayData maxStayData;
  maxStayData._mustComplete = date;
  maxStayData._location = location;
  maxStayData._mustComplete.addMinutes(time - 1);

  TravelRestrictions tvlRestrictions;
  auto observer = PTFObserver::create(
      ObserverType::TRV_REST_COMPLETE_SFR, _trx->dataHandle(), &tvlRestrictions);

  observer->notify(
      NotificationType::PAX_TYPE_FARE_TRAVEL_MUST_COMPLETE, segOrder, date, location, time);

  observer->updateIfNotified(*_paxTypeFare);
  ASSERT_TRUE(_paxTypeFare->hasStructuredRuleData());
  auto nvaData = _paxTypeFare->getNVAData();
  ASSERT_EQ((*nvaData)[segOrder], &date);
  auto sfrMaxMap = _paxTypeFare->getStructuredRuleData()._maxStayMap;
  ASSERT_EQ(maxStayData, sfrMaxMap[segOrder]);
}
}
