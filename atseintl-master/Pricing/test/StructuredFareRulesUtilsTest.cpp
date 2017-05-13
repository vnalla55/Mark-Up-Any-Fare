// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
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

#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PricingUnit.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <gtest/gtest.h>

namespace tse
{
using namespace ::testing;

class StructuredFareRulesUtilsTest : public Test
{
public:
  void SetUp() override
  {
    _farePath = _memHandle.create<FarePath>();
    _pricingUnit = _memHandle.create<PricingUnit>();
    _farePath->pricingUnit().push_back(_pricingUnit);
  }

  void TearDown() override { _memHandle.clear(); }

  class FareUsageBuilder
  {
  public:
    FareUsageBuilder(TestMemHandle& memHandle) : _memHandle(memHandle)
    {
      _fu = _memHandle.create<FareUsage>();
      _fu->createStructuredRuleDataIfNonexistent();
      PaxTypeFare*& paxTypeFare = _fu->paxTypeFare();
      paxTypeFare = _memHandle.create<PaxTypeFare>();
      _trx = _memHandle.create<PricingTrx>();
    }

    FareUsageBuilder& addBaseFare()
    {
      PaxTypeFare* baseFare = _memHandle.create<PaxTypeFare>();
      FBRPaxTypeFareRuleData* rd = _memHandle.create<FBRPaxTypeFareRuleData>();
      FareByRuleItemInfo* ruleInfo = _memHandle.create<FareByRuleItemInfo>();
      rd->ruleItemInfo() = ruleInfo;
      rd->baseFare() = baseFare;
      auto paxTypeFare = _fu->paxTypeFare();
      paxTypeFare->setRuleData(RuleConst::FARE_BY_RULE, _trx->dataHandle(), rd, true);
      return *this;
    }

    FareUsageBuilder& addMinimumStayData(LocCode locCode, DateTime dateTime, uint16_t segOrder)
    {
      auto& SFRData = _fu->getStructuredRuleData();
      SFRData._minStayLocation = locCode;
      SFRData._minStayDate = dateTime;
      SFRData._minStaySegmentOrder = segOrder;
      return *this;
    }

    FareUsageBuilder& addMaximumStayData(MaxStayMapElem elem)
    {
      MaxStayMap& map = _fu->getStructuredRuleData()._maxStayMostRestrictiveFCData;
      structuredFareRulesUtils::updateMaxStayTrvData(map, elem);
      return *this;
    }

    FareUsageBuilder& addAdvResTktPTFBaseFareData(DateTime advRes, DateTime advTkt)
    {
      PaxTypeFare& paxTypeFare = *_fu->paxTypeFare();
      PaxTypeFare& basePTF = *paxTypeFare.getBaseFare();

      basePTF.createStructuredRuleDataIfNonexistent();
      PaxTypeFareStructuredRuleData sfrData;
      sfrData.setAdvanceReservationWithEarlier(advRes);
      sfrData.setAdvanceTicketingWithEarlier(advTkt);
      basePTF.setStructuredRuleData(sfrData);

      return *this;
    }

    FareUsageBuilder& addMaximumStayPTFData(MaxStayMapElem elem)
    {
      PaxTypeFare& paxTypeFare = *_fu->paxTypeFare();

      paxTypeFare.createStructuredRuleDataIfNonexistent();

      structuredFareRulesUtils::updateMaxStayTrvData(
          paxTypeFare.getStructuredRuleData()._maxStayMap, elem);
      return *this;
    }
    using PaxTypeFareRuleDataByCatNo =
        std::array<std::atomic<PaxTypeFare::PaxTypeFareAllRuleData*>, 36>;

    FareUsageBuilder& addCat15RuleData(MaxStayMapElem elem)
    {
      PaxTypeFare::PaxTypeFareAllRuleData* allRuleData =
          _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
      allRuleData->fareRuleData = _memHandle.create<PaxTypeFareRuleData>();
      PaxTypeFare& paxTypeFare = *_fu->paxTypeFare();

      for (uint16_t i = 1; i < RuleConst::FARE_BY_RULE; ++i)
        (*paxTypeFare.paxTypeFareRuleDataMap())[i] = nullptr;

      (*paxTypeFare.paxTypeFareRuleDataMap())[RuleConst::FARE_BY_RULE] = allRuleData;

      PaxTypeFare* basePTF = _memHandle.create<PaxTypeFare>();
      basePTF->createStructuredRuleDataIfNonexistent();
      allRuleData->fareRuleData->baseFare() = basePTF;

      structuredFareRulesUtils::updateMaxStayTrvData(basePTF->getStructuredRuleData()._maxStayMap,
                                                     elem);
      return *this;
    }

    FareUsageBuilder& setAdvanceReservationDate(const DateTime& date)
    {
      _fu->getStructuredRuleData()._advanceReservation = date;
      return *this;
    }

    FareUsageBuilder& setAdvanceTicketingDate(const DateTime& date)
    {
      _fu->getStructuredRuleData()._advanceTicketing = date;
      return *this;
    }

    FareUsageBuilder& setAdvanceReservationDatePTF(const DateTime& date)
    {
      PaxTypeFare& paxTypeFare = *_fu->paxTypeFare();
      paxTypeFare.createStructuredRuleDataIfNonexistent();

      paxTypeFare.getStructuredRuleData()._advanceReservation = date;
      return *this;
    }

    FareUsage* build() { return _fu; }

  private:
    TestMemHandle& _memHandle;
    FareUsage* _fu = nullptr;
    PricingTrx* _trx;
  };

  void addFareUsage(PricingUnit& pu, FareUsage* fu) { pu.fareUsage().push_back(fu); }
  void addTravelSeg(TravelSeg* travelSeg) { _pricingUnit->travelSeg().push_back(travelSeg); }

  FarePath* _farePath;
  PricingUnit* _pricingUnit;
  TestMemHandle _memHandle;
};

TEST_F(StructuredFareRulesUtilsTest, testEmpty)
{
  FareUsage* fu = FareUsageBuilder(_memHandle).build();
  _pricingUnit->createMostRestrictivePricingUnitSFRData();

  structuredFareRulesUtils::updateMostRestrictivePricingUnitData(
      _pricingUnit->getMostRestrictivePricingUnitSFRData(), fu->getStructuredRuleData());
  ASSERT_TRUE(_pricingUnit->getMostRestrictivePricingUnitSFRData()._minStayMap.empty());
}

TEST_F(StructuredFareRulesUtilsTest, testMultipleDataDifferentTravelSeg)
{
  _pricingUnit->createMostRestrictivePricingUnitSFRData();
  DateTime firstDate(2015, 10, 1, 2, 30);
  DateTime secondDate(2015, 12, 1, 2, 30);

  FareUsage* fu1 =
      FareUsageBuilder(_memHandle).addMinimumStayData(LocCode("LON"), firstDate, 0).build();
  FareUsage* fu2 =
      FareUsageBuilder(_memHandle).addMinimumStayData(LocCode("LON"), secondDate, 3).build();

  structuredFareRulesUtils::updateMostRestrictivePricingUnitData(
      _pricingUnit->getMostRestrictivePricingUnitSFRData(), fu1->getStructuredRuleData());
  structuredFareRulesUtils::updateMostRestrictivePricingUnitData(
      _pricingUnit->getMostRestrictivePricingUnitSFRData(), fu2->getStructuredRuleData());
  auto& minStayMap = _pricingUnit->getMostRestrictivePricingUnitSFRData()._minStayMap;
  ASSERT_TRUE(minStayMap.size() == 2);
  ASSERT_TRUE(minStayMap[0].first == firstDate);
  ASSERT_TRUE(minStayMap[3].first == secondDate);
}

TEST_F(StructuredFareRulesUtilsTest, testMultipleDataSameTravelSeg)
{
  _pricingUnit->createMostRestrictivePricingUnitSFRData();
  DateTime expectedDate(2015, 12, 1, 2, 30);
  FareUsage* fu1 =
      FareUsageBuilder(_memHandle).addMinimumStayData(LocCode("LON"), expectedDate, 0).build();
  FareUsage* fu2 = FareUsageBuilder(_memHandle)
                       .addMinimumStayData(LocCode("LON"), DateTime(2015, 10, 1, 2, 30), 0)
                       .build();

  structuredFareRulesUtils::updateMostRestrictivePricingUnitData(
      _pricingUnit->getMostRestrictivePricingUnitSFRData(), fu1->getStructuredRuleData());
  structuredFareRulesUtils::updateMostRestrictivePricingUnitData(
      _pricingUnit->getMostRestrictivePricingUnitSFRData(), fu2->getStructuredRuleData());
  auto& minStayMap = _pricingUnit->getMostRestrictivePricingUnitSFRData()._minStayMap;
  ASSERT_TRUE(minStayMap.size() == 1);
  ASSERT_TRUE(minStayMap[0].first == expectedDate);
}

TEST_F(StructuredFareRulesUtilsTest, testMostRestrictiveMaxStayDataOnFareUsage)
{
  FareUsage* fu1 =
      FareUsageBuilder(_memHandle)
          .addMaximumStayData(
               {1, {LocCode("LON"), DateTime(2015, 8, 1, 2, 30), DateTime(2000, 8, 1, 2, 30)}})
          .addMaximumStayData(
               {1, {LocCode("LON"), DateTime(2015, 10, 1, 2, 30), DateTime(2000, 2, 1, 2, 30)}})
          .addMaximumStayPTFData(
               {1, {LocCode("LON"), DateTime(2015, 11, 1, 2, 30), DateTime(1999, 1, 1, 2, 30)}})
          .build();

  structuredFareRulesUtils::updateFUFromPaxTypeFare(*fu1);
  MaxStayMap& maxStayMostRestrictiveFCData =
      fu1->getStructuredRuleData()._maxStayMostRestrictiveFCData;
  ASSERT_EQ(maxStayMostRestrictiveFCData[1],
            MaxStayData(LocCode("LON"), DateTime(2015, 8, 1, 2, 30), DateTime(1999, 1, 1, 2, 30)));
}

TEST_F(StructuredFareRulesUtilsTest, testUpdateSFRDataIfPaxTypeBaseFare)
{
  FareUsage* fu1 =
      FareUsageBuilder(_memHandle)
          .addCat15RuleData(
               {1, {LocCode("LON"), DateTime(2015, 8, 1, 2, 30), DateTime(2000, 8, 1, 2, 30)}})
          .build();

  structuredFareRulesUtils::updateFUFromPaxTypeFare(*fu1);
  MaxStayMap& maxStayMostRestrictiveFCData =
      fu1->getStructuredRuleData()._maxStayMostRestrictiveFCData;
  ASSERT_EQ(maxStayMostRestrictiveFCData[1],
            MaxStayData(LocCode("LON"), DateTime(2015, 8, 1, 2, 30), DateTime(2000, 8, 1, 2, 30)));
}

TEST_F(StructuredFareRulesUtilsTest, testAdvanceReservationDataOnFareUsage)
{
  DateTime fuDate(2016, 3, 1, 0, 1, 0), ptfDate(2016, 2, 1, 0, 1, 0);
  FareUsage* fu = FareUsageBuilder(_memHandle)
                      .setAdvanceReservationDate(fuDate)
                      .setAdvanceReservationDatePTF(ptfDate)
                      .build();
  structuredFareRulesUtils::updateFUFromPaxTypeFare(*fu);
  ASSERT_EQ(ptfDate, fu->getStructuredRuleData()._advanceReservation);
}

TEST_F(StructuredFareRulesUtilsTest, testMustCommenceOnBegOfPU)
{
  FareUsage* fu1 =
      FareUsageBuilder(_memHandle)
          .addMaximumStayData(
               {1, {LocCode("LON"), DateTime(2000, 8, 1, 2, 30), DateTime(2003, 8, 1, 2, 30)}})
          .build();

  addFareUsage(*_pricingUnit, fu1);

  structuredFareRulesUtils::finalizeDataCollection(*_farePath);
  MaxStayMap& _maxStayMostRestrictiveFCData =
      fu1->getStructuredRuleData()._maxStayMostRestrictiveFCData;
  ASSERT_EQ(MaxStayData(LocCode("LON"), DateTime(2000, 8, 1, 2, 30), DateTime(2003, 8, 1, 2, 30)),
            _maxStayMostRestrictiveFCData[1]);
}

// ________________________________________________________________________________________________
// Test description:
//
//  We check calculation of Most Restrictive restriction for each
//  segment in Pricing.
//
//  1. fu1 and fu2 refers to the same segment of journey so we expect most restrictive calculation
//  from both
//
//  2. fu3 refers to only one segment of journey so we expect most restrictive calculation from fu3
//  only
//
//  3. fu4 and fu5 refers to the same segment of journey so we expect most restrictive form both
// ________________________________________________________________________________________________

TEST_F(StructuredFareRulesUtilsTest, testMostRestrictiveMaxStayOnPricingUnitLevel)
{
  FareUsage* fu1 =
      FareUsageBuilder(_memHandle)
          .addMaximumStayData(
               {1, {LocCode("LON"), DateTime(2015, 8, 1, 2, 30), DateTime(2000, 8, 1, 2, 30)}})
          .addMaximumStayData(
               {1, {LocCode("LON"), DateTime(2015, 10, 1, 2, 30), DateTime(2000, 2, 1, 2, 30)}})
          .addMaximumStayPTFData(
               {1, {LocCode("LON"), DateTime(2015, 11, 1, 2, 30), DateTime(1999, 1, 1, 2, 30)}})
          .build();

  FareUsage* fu2 =
      FareUsageBuilder(_memHandle)
          .addMaximumStayData(
               {1, {LocCode("LON"), DateTime(2045, 8, 1, 2, 30), DateTime(1800, 8, 1, 2, 30)}})
          .addMaximumStayData(
               {1, {LocCode("LON"), DateTime(1500, 10, 1, 2, 30), DateTime(1800, 2, 1, 2, 30)}})
          .addMaximumStayPTFData(
               {1, {LocCode("LON"), DateTime(2045, 11, 1, 2, 30), DateTime(1799, 1, 1, 2, 30)}})
          .build();

  FareUsage* fu3 =
      FareUsageBuilder(_memHandle)
          .addMaximumStayData(
               {2, {LocCode("NYC"), DateTime(1545, 10, 6, 2, 30), DateTime(1800, 8, 1, 2, 30)}})
          .addMaximumStayData(
               {2, {LocCode("NYC"), DateTime(1545, 10, 1, 2, 30), DateTime(1800, 2, 1, 2, 30)}})
          .addMaximumStayPTFData(
               {2, {LocCode("NYC"), DateTime(1545, 11, 1, 2, 30), DateTime(1699, 1, 1, 2, 30)}})
          .build();

  FareUsage* fu4 =
      FareUsageBuilder(_memHandle)
          .addMaximumStayPTFData(
               {3, {LocCode("BRU"), DateTime(3045, 10, 6, 2, 30), DateTime::openDate()}})
          .build();

  FareUsage* fu5 =
      FareUsageBuilder(_memHandle)
          .addMaximumStayPTFData(
               {3, {LocCode("BRU"), DateTime::openDate(), DateTime(1670, 10, 6, 2, 30)}})
          .build();

  addFareUsage(*_pricingUnit, fu1);
  addFareUsage(*_pricingUnit, fu2);
  addFareUsage(*_pricingUnit, fu3);
  addFareUsage(*_pricingUnit, fu4);
  addFareUsage(*_pricingUnit, fu5);

  structuredFareRulesUtils::finalizeDataCollection(*_farePath);
  MaxStayMap& _maxStayMostRestrictivePUData =
      _pricingUnit->getMostRestrictivePricingUnitSFRData()._maxStayMap;

  // 2
  ASSERT_EQ(_maxStayMostRestrictivePUData[1],
            MaxStayData(LocCode("LON"), DateTime(1500, 10, 1, 2, 30), DateTime(1799, 1, 1, 2, 30)));
  // 2
  ASSERT_EQ(_maxStayMostRestrictivePUData[2],
            MaxStayData(LocCode("NYC"), DateTime(1545, 10, 1, 2, 30), DateTime(1699, 1, 1, 2, 30)));
  // 3
  ASSERT_EQ(
      _maxStayMostRestrictivePUData[3],
      MaxStayData(LocCode("BRU"), DateTime(3045, 10, 6, 2, 30), DateTime(1670, 10, 6, 2, 30)));
}

TEST_F(StructuredFareRulesUtilsTest, testUpdateMaxStayCommenceDataOneLocation)
{
  StructuredRuleData structuredData;
  MaxStayMap& maxStayMap = structuredData._maxStayMostRestrictiveFCData;

  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap, 1, DateTime(2018, 2, 14, 12, 0, 1), LocCode("BRU"));

  EXPECT_EQ(maxStayMap[1],
            MaxStayData(LocCode("BRU"), DateTime(2018, 2, 14, 12, 0, 1), DateTime::openDate()));

  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap, 1, DateTime(2018, 2, 13, 12, 0, 1), LocCode("BRU"));
  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap, 1, DateTime(2018, 2, 13, 12, 0, 0), LocCode("BRU"));

  // Earlier date should be applied
  EXPECT_EQ(maxStayMap[1],
            MaxStayData(LocCode("BRU"), DateTime(2018, 2, 13, 12, 0, 0), DateTime::openDate()));

  EXPECT_EQ(maxStayMap[2], MaxStayData(LocCode(), DateTime::openDate(), DateTime::openDate()));
}

TEST_F(StructuredFareRulesUtilsTest, testUpdateMaxStayCommenceDataMultipleLocation)
{
  StructuredRuleData structuredData;
  MaxStayMap& maxStayMap = structuredData._maxStayMostRestrictiveFCData;

  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap, 1, DateTime(2018, 2, 14, 12, 0, 1), LocCode("BRU"));

  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap, 1, DateTime(2018, 2, 13, 12, 0, 1), LocCode("BRU"));

  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap, 2, DateTime(2000, 2, 14, 12, 0, 1), LocCode("LON"));

  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap, 2, DateTime(2000, 2, 13, 12, 0, 1), LocCode("LON"));

  // Earlier date should be applied for each location
  EXPECT_EQ(maxStayMap[1],
            MaxStayData(LocCode("BRU"), DateTime(2018, 2, 13, 12, 0, 1), DateTime::openDate()));

  EXPECT_EQ(maxStayMap[2],
            MaxStayData(LocCode("LON"), DateTime(2000, 2, 13, 12, 0, 1), DateTime::openDate()));

  EXPECT_EQ(maxStayMap[3], MaxStayData(LocCode(), DateTime::openDate(), DateTime::openDate()));
}

TEST_F(StructuredFareRulesUtilsTest, testUpdateMaxStayCompleteDataOneLocation)
{
  StructuredRuleData structuredData;
  MaxStayMap& maxStayMap = structuredData._maxStayMostRestrictiveFCData;

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap, 1, DateTime(2018, 2, 14, 12, 0, 1), LocCode("BRU"));

  EXPECT_EQ(maxStayMap[1],
            MaxStayData(LocCode("BRU"), DateTime::openDate(), DateTime(2018, 2, 14, 12, 0, 1)));

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap, 1, DateTime(2018, 2, 13, 12, 0, 1), LocCode("BRU"));
  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap, 1, DateTime(2018, 2, 13, 12, 0, 0), LocCode("BRU"));

  // Earlier date should be applied
  EXPECT_EQ(maxStayMap[1],
            MaxStayData(LocCode("BRU"), DateTime::openDate(), DateTime(2018, 2, 13, 12, 0, 0)));

  EXPECT_EQ(maxStayMap[2], MaxStayData(LocCode(), DateTime::openDate(), DateTime::openDate()));
}

TEST_F(StructuredFareRulesUtilsTest, testUpdateMaxStayCompleteDataMultipleLocation)
{
  StructuredRuleData structuredData;
  MaxStayMap& maxStayMap = structuredData._maxStayMostRestrictiveFCData;

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap, 1, DateTime(2018, 2, 14, 12, 0, 1), LocCode("BRU"));

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap, 1, DateTime(2018, 2, 13, 12, 0, 1), LocCode("BRU"));

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap, 2, DateTime(2000, 2, 14, 12, 0, 1), LocCode("LON"));

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap, 2, DateTime(2000, 2, 13, 12, 0, 1), LocCode("LON"));

  // Earlier date should be applied for each location
  EXPECT_EQ(maxStayMap[1],
            MaxStayData(LocCode("BRU"), DateTime::openDate(), DateTime(2018, 2, 13, 12, 0, 1)));

  EXPECT_EQ(maxStayMap[2],
            MaxStayData(LocCode("LON"), DateTime::openDate(), DateTime(2000, 2, 13, 12, 0, 1)));

  EXPECT_EQ(maxStayMap[3], MaxStayData(LocCode(), DateTime::openDate(), DateTime::openDate()));
}

TEST_F(StructuredFareRulesUtilsTest, testUpdateMaxStayCommenceAndCompleteOnTheSameSeg)
{
  StructuredRuleData structuredData;
  MaxStayMap& maxStayMap = structuredData._maxStayMostRestrictiveFCData;

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap, 1, DateTime(2018, 2, 14, 12, 0, 1), LocCode("BRU"));

  structuredFareRulesUtils::updateMaxStayTrvCompleteData(
      maxStayMap, 1, DateTime(2018, 2, 13, 12, 0, 1), LocCode("BRU"));

  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap, 1, DateTime(2000, 2, 14, 12, 0, 1), LocCode("BRU"));

  structuredFareRulesUtils::updateMaxStayTrvCommenceData(
      maxStayMap, 1, DateTime(2000, 2, 13, 12, 0, 1), LocCode("BRU"));

  // Earlier date should be applied for each location
  EXPECT_EQ(maxStayMap[1],
            MaxStayData(
                LocCode("BRU"), DateTime(2000, 2, 13, 12, 0, 1), DateTime(2018, 2, 13, 12, 0, 1)));

  EXPECT_EQ(maxStayMap[2], MaxStayData(LocCode(), DateTime::openDate(), DateTime::openDate()));
}

TEST_F(StructuredFareRulesUtilsTest, testUpdateMaxStayTrvData)
{
  StructuredRuleData structuredData;
  MaxStayMap& maxStayMap = structuredData._maxStayMostRestrictiveFCData;

  structuredFareRulesUtils::updateMaxStayTrvData(
      maxStayMap,
      std::make_pair(1,
                     MaxStayData(LocCode("BRU"),
                                 DateTime(2000, 2, 14, 12, 0, 1),
                                 DateTime(2018, 2, 14, 12, 0, 1))));

  structuredFareRulesUtils::updateMaxStayTrvData(
      maxStayMap,
      std::make_pair(1,
                     MaxStayData(LocCode("BRU"),
                                 DateTime(2000, 2, 13, 12, 0, 1),
                                 DateTime(2018, 2, 13, 12, 0, 1))));

  // Earlier date should be applied for each location
  EXPECT_EQ(maxStayMap[1],
            MaxStayData(
                LocCode("BRU"), DateTime(2000, 2, 13, 12, 0, 1), DateTime(2018, 2, 13, 12, 0, 1)));

  EXPECT_EQ(maxStayMap[2], MaxStayData(LocCode(), DateTime::openDate(), DateTime::openDate()));
}

TEST_F(StructuredFareRulesUtilsTest, testCopyAdvanceReservationNoData)
{
  FareUsage from, to;
  structuredFareRulesUtils::copyAdvanceResAndTktData(from, to);
  ASSERT_FALSE(to.hasStructuredRuleData());
}

// Test if StructuredRuleData is not created when fromFU doesn't have valida date
TEST_F(StructuredFareRulesUtilsTest, testCopyAdvanceReservationDataNotValid)
{
  FareUsage* from =
      FareUsageBuilder(_memHandle)
          .addMaximumStayData(
               {1, {LocCode("LON"), DateTime(2015, 8, 1, 2, 30), DateTime(2000, 8, 1, 2, 30)}})
          .build();
  FareUsage to;
  structuredFareRulesUtils::copyAdvanceResAndTktData(*from, to);
  ASSERT_FALSE(to.hasStructuredRuleData());
}

// Test if StructuredRuleData is created and filled with data if does not exist
TEST_F(StructuredFareRulesUtilsTest, testCopyAdvanceReservationCreateStructure)
{
  const DateTime lastDateToBook(2010, 1, 1, 23, 59, 0);
  const DateTime lastTktDate(2015, 1, 1, 23, 59, 0);
  FareUsage* from = FareUsageBuilder(_memHandle)
                        .setAdvanceReservationDate(lastDateToBook)
                        .setAdvanceTicketingDate(lastTktDate)
                        .build();
  ;
  FareUsage to;
  structuredFareRulesUtils::copyAdvanceResAndTktData(*from, to);
  ASSERT_TRUE(to.hasStructuredRuleData());
  ASSERT_EQ(lastDateToBook, to.getStructuredRuleData()._advanceReservation);
  ASSERT_EQ(lastTktDate, to.getStructuredRuleData()._advanceTicketing);
}

TEST_F(StructuredFareRulesUtilsTest, testGetLastMinuteOfDayDate)
{
  ASSERT_EQ(DateTime(2015, 8, 1, 23, 59, 0),
            structuredFareRulesUtils::getDateWithEarliestTime(DateTime(2015, 8, 1)));
}

TEST_F(StructuredFareRulesUtilsTest, testOnlyMaxMapPricingUnitHasSFRData)
{
  FareUsage* fu1 =
      FareUsageBuilder(_memHandle)
          .addMaximumStayData(
               {1, {LocCode("LON"), DateTime(2000, 8, 1, 2, 30), DateTime(2003, 8, 1, 2, 30)}})
          .build();

  addFareUsage(*_pricingUnit, fu1);

  structuredFareRulesUtils::finalizeDataCollection(*_farePath);
  MaxStayMap& _maxStayMostRestrictiveFCData =
      fu1->getStructuredRuleData()._maxStayMostRestrictiveFCData;
  ASSERT_EQ(MaxStayData(LocCode("LON"), DateTime(2000, 8, 1, 2, 30), DateTime(2003, 8, 1, 2, 30)),
            _maxStayMostRestrictiveFCData[1]);
  ASSERT_TRUE(_pricingUnit->hasMostRestrictivePricingUnitSFRData());
  auto sfrData = _pricingUnit->getMostRestrictivePricingUnitSFRData();
  ASSERT_TRUE(!sfrData._maxStayMap.empty());
}

TEST_F(StructuredFareRulesUtilsTest, testUpdatePaxTypeFareFromBaseFare)
{
  DateTime advRes(2011, 8, 1, 2, 30);
  DateTime advTkt(2013, 8, 1, 2, 30);

  FareUsage* fu1 = FareUsageBuilder(_memHandle)
                       .addBaseFare()
                       .addAdvResTktPTFBaseFareData(advRes, advTkt)
                       .build();
  addFareUsage(*_pricingUnit, fu1);
  structuredFareRulesUtils::updateFUFromPaxTypeFare(*fu1);
  ASSERT_EQ(advRes, fu1->getStructuredRuleData()._advanceReservation);
  ASSERT_EQ(advTkt, fu1->getStructuredRuleData()._advanceTicketing);
}
}
