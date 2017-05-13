//----------------------------------------------------------------------------
// Copyright Sabre 2015
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//----------------------------------------------------------------------------
#include "Decode/FrequentFlyerStatusGenerator.h"

#include "DataModel/FrequentFlyerTrx.h"
#include "DBAccess/FreqFlyerStatus.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace tse
{
using namespace ::testing;

namespace
{
class FrequentFlyerStatusGeneratorTestDataHandle : public DataHandleMock
{
public:
  MOCK_METHOD3(getFreqFlyerStatuses,
               std::vector<const FreqFlyerStatus*>(const CarrierCode carrier,
                                                   const DateTime& date,
                                                   bool useHistorical));
};
}

class FrequentFlyerStatusGeneratorTest : public testing::Test
{
public:
  void SetUp() override
  {
    _memHandle.create<TestConfigInitializer>();
    _dataHandle = _memHandle.create<FrequentFlyerStatusGeneratorTestDataHandle>();
    _trx = _memHandle.create<FrequentFlyerTrx>();
    _generator = _memHandle.create<FrequentFlyerStatusGenerator>(*_trx);
  }

  void TearDown() override
  {
    _memHandle.clear();
  }

protected:
  FrequentFlyerStatusGeneratorTestDataHandle* _dataHandle = nullptr;
  FrequentFlyerTrx* _trx = nullptr;
  FrequentFlyerStatusGenerator* _generator;
  TestMemHandle _memHandle;
};

TEST_F(FrequentFlyerStatusGeneratorTest, testOneCarrierNotFound)
{
  CarrierCode AA("AA");
  std::set<CarrierCode> carriers{AA};
  _trx->setCxrs(carriers);
  std::vector<const FreqFlyerStatus*> result;
  EXPECT_CALL(*_dataHandle, getFreqFlyerStatuses(AA, _, _)).WillOnce(Return(result));
  _generator->generateStatusList();

  const auto& statuses = _trx->getCxrs();
  ASSERT_EQ(1, statuses.size());
  ASSERT_TRUE(statuses.begin()->second.empty());
}

TEST_F(FrequentFlyerStatusGeneratorTest, testTwoCarriers)
{
  CarrierCode AA("AA");
  std::vector<const FreqFlyerStatus*> resultAA;
  EXPECT_CALL(*_dataHandle, getFreqFlyerStatuses(AA, _, _)).WillOnce(Return(resultAA));
  CarrierCode LH("LH");
  FreqFlyerStatus lhStatus;
  lhStatus._maxPassengersSamePNR = 4;
  lhStatus._maxPassengersDiffPNR = 1;
  std::vector<const FreqFlyerStatus*> resultLH(1, &lhStatus);
  EXPECT_CALL(*_dataHandle, getFreqFlyerStatuses(LH, _, _)).WillOnce(Return(resultLH));
  std::set<CarrierCode> carriers{AA, LH};
  _trx->setCxrs(carriers);
  _generator->generateStatusList();

  const auto& statuses = _trx->getCxrs();
  ASSERT_EQ(2, statuses.size());
  ASSERT_TRUE(statuses.begin()->second.empty());
  auto lhData = *(++statuses.begin());
  ASSERT_FALSE(lhData.second.empty());
  ASSERT_EQ(&lhStatus, lhData.second.front()._dbData);
  ASSERT_EQ(5, lhData.second.front()._maxPassengersTotal);
}
}

