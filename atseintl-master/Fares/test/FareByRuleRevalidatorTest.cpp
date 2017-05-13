//----------------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "Fares/FareByRuleRevalidator.h"

#include "Common/ClassOfService.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/SimilarItinData.h"
#include "DataModel/TravelSeg.h"

#include "test/include/TestMemHandle.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

namespace tse
{
using testing::Return;

class FareByRuleRevalidatorTest : public testing::Test
{
public:
  void SetUp() override
  {
    _fareMarket = _memHandle.create<FareMarket>();
    _segments = &_fareMarket->travelSeg();
    _classes = &_fareMarket->classOfServiceVec();

    _dataHandle = _memHandle.create<DataHandle>();
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _fbrData = _memHandle.create<FBRPaxTypeFareRuleData>();
    _fbrData->isBaseFareAvailBkcMatched() = true;
    _fbrData->baseFareInfoBookingCodes().insert(BookingCode("A"));
    _fareByRuleItemInfo = _memHandle.create<FareByRuleItemInfo>();
    _fbrData->ruleItemInfo() = _fareByRuleItemInfo;
    _paxTypeFare->setRuleData(RuleConst::FARE_BY_RULE, *_dataHandle, _fbrData, true);
  }

  void TearDown() override
  {
    _itins.clear();
    _memHandle.clear();
    _segments = nullptr;
    _classes = nullptr;
  }

protected:
  using CosVec = std::vector<ClassOfService*>;

  class AvailabilityCheckerMock
  {
  public:
    MOCK_CONST_METHOD4(checkAvailability,
                       bool(const uint16_t /*numSeatsRequired*/,
                            const BookingCode& /*bookingCode*/,
                            const std::vector<CosVec*>& /*cosVecs*/,
                            const std::vector<TravelSeg*>& /*travelSegments*/));

    bool checkAvailability(const uint16_t numSeatsRequired,
                            const BookingCode& bookingCode,
                            const std::vector<std::vector<ClassOfService>>& cosVecs,
                            const std::vector<TravelSeg*>& travelSegments) const
    {
      return checkAvailCosVersion(numSeatsRequired, bookingCode, cosVecs, travelSegments);
    }

    MOCK_CONST_METHOD4(checkAvailCosVersion,
                       bool(const uint16_t /*numSeatsRequired*/,
                            const BookingCode& /*bookingCode*/,
                            const std::vector<std::vector<ClassOfService>>& /*cosVecs*/,
                            const std::vector<TravelSeg*>& /*travelSegments*/));
  };

  FareByRuleRevalidator _revalidator;
  AvailabilityCheckerMock _availabilityMock;
  FareMarket* _fareMarket;
  std::vector<TravelSeg*>* _segments;
  std::vector<CosVec*>* _classes;
  std::vector<Itin*> _itins;

  PaxTypeFare* _paxTypeFare;
  FBRPaxTypeFareRuleData* _fbrData;
  FareByRuleItemInfo* _fareByRuleItemInfo;

  DataHandle* _dataHandle;
  TestMemHandle _memHandle;
};

TEST_F(FareByRuleRevalidatorTest, testOneItinNotMother_fail_tooManySegments)
{
  Itin itin;
  _itins.push_back(&itin);
  AirSeg seg1, seg2;
  _segments->push_back(&seg1);
  _segments->push_back(&seg2);
  _fareByRuleItemInfo->fltSegCnt() = 1;
  EXPECT_CALL(_availabilityMock, checkAvailability(1, BookingCode("A"), *_classes, *_segments)).Times(0);
  ASSERT_FALSE(_revalidator.checkFBR(*_paxTypeFare, 1, *_fareMarket, _itins, _availabilityMock));
}

TEST_F(FareByRuleRevalidatorTest, testOneItinNotMother_pass)
{
  Itin itin;
  _itins.push_back(&itin);
  AirSeg seg;
  _segments->push_back(&seg);
  _fareByRuleItemInfo->fltSegCnt() = 1;
  EXPECT_CALL(_availabilityMock, checkAvailability(1, BookingCode("A"), *_classes, *_segments))
      .WillOnce(Return(true));
  ASSERT_TRUE(_revalidator.checkFBR(*_paxTypeFare, 1, *_fareMarket, _itins, _availabilityMock));
}

TEST_F(FareByRuleRevalidatorTest, testOneItinMother_pass_forSimilarItin)
{
  Itin itin, similarItin;
  _itins.push_back(&itin);
  std::vector<SimilarItinData> similarItins;
  similarItins.emplace_back(&similarItin);
  FareMarketData fmData;
  AirSeg seg1, seg2;
  _segments->push_back(&seg1);
  fmData.travelSegments.resize(2, &seg2);
  fmData.classOfService.resize(2);
  ClassOfService cos;
  fmData.classOfService[0].resize(1, cos);
  fmData.classOfService[1].resize(1, cos);
  similarItins.front().fareMarketData[_fareMarket] = fmData;
  itin.swapSimilarItins(similarItins);
  EXPECT_CALL(_availabilityMock, checkAvailability(1, BookingCode("A"), *_classes, *_segments))
      .WillOnce(Return(false));
  EXPECT_CALL(
      _availabilityMock,
      checkAvailCosVersion(1, BookingCode("A"), fmData.classOfService, fmData.travelSegments))
      .WillOnce(Return(true));
  ASSERT_TRUE(_revalidator.checkFBR(*_paxTypeFare, 1, *_fareMarket, _itins, _availabilityMock));
}
}
