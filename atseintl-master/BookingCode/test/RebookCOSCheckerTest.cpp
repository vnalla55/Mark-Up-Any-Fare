//------------------------------------------------------------------
//
//  File: RebookedCOSCheckerTest.cpp
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
#include <gmock/gmock.h>

#include "test/include/GtestHelperMacros.h"
#include "BookingCode/RebookedCOSChecker.h"
#include "Common/ClassOfService.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/TravelSeg.h"

namespace tse
{
using namespace ::testing;

class AvailGetterMock : public FBCVAvailabilityGetter
{
public:
  MOCK_METHOD4(getAvailability,
               const std::vector<ClassOfService*>*(
                   TravelSeg*, PaxTypeFare::SegmentStatus&, PaxTypeFare&, const uint16_t));
  ~AvailGetterMock() {}
};

class RebookedCOSCheckerMock : public RebookedCOSChecker
{
public:
  MOCK_METHOD0(getNumSeats, uint16_t());
  MOCK_METHOD1(getSegmentStatus, PaxTypeFare::SegmentStatus&(size_t));
  MOCK_METHOD3(getCosVec,
               const std::vector<ClassOfService*>*(AirSeg*, PaxTypeFare::SegmentStatus&, size_t));
  MOCK_METHOD2(getBookingCode, BookingCode(AirSeg*, PaxTypeFare::SegmentStatus&));

  RebookedCOSCheckerMock(PricingTrx& trx,
                         PaxTypeFare& paxTfare,
                         FareMarket& mkt,
                         FareUsage* fu,
                         FBCVAvailabilityGetter& fbcValidator,
                         bool statusTypeNotFlowForLocalJrnyCxr)
    : RebookedCOSChecker(trx, paxTfare, mkt, fu, fbcValidator, statusTypeNotFlowForLocalJrnyCxr)
  {
  }

  ~RebookedCOSCheckerMock() {}
};

class RebookedCOSCheckerTest : public Test
{
public:
  void SetUp()
  {
    _trx.setOptions(&_options);
    _numSeats = 1;
    _bkc = BookingCode("M");
    _availGetterMock.reset(new AvailGetterMock);
    _checkerMock.reset(new RebookedCOSCheckerMock(
        _trx, _paxTypeFare, _fareMarket, &_fareUsage, *_availGetterMock, true));
    _checker.reset(new RebookedCOSChecker(
        _trx, _paxTypeFare, _fareMarket, &_fareUsage, *_availGetterMock, true));
  }

  void TearDown()
  {
    // No need for cleanup - see googletest Primer
  }

  // Helper methods

  void fillMarket(size_t segCount)
  {
    for (size_t i = 0; i < segCount; ++i)
    {
      AirSeg* airSeg = 0;
      _trx.dataHandle().get(airSeg);
      _fareMarket.travelSeg().push_back(airSeg);

      std::vector<ClassOfService*>* cosVec = 0;
      _trx.dataHandle().get(cosVec);
      _fareMarket.classOfServiceVec().push_back(cosVec);
    }
  }

  std::vector<ClassOfService*> fillCosVec(const std::string& bkcSeats)
  {
    assert(bkcSeats.size() % 2 == 0);

    std::vector<ClassOfService*> result;
    for (size_t i = 0; i < bkcSeats.size(); i += 2)
    {
      ClassOfService* cos = 0;
      _trx.dataHandle().get(cos);
      cos->cabin().setEconomyClass();
      cos->bookingCode() = BookingCode(bkcSeats[i]);
      cos->numSeats() = static_cast<uint16_t>(bkcSeats[i + 1] - '0');
      result.push_back(cos);
    }
    return result;
  }

  void addSegmentStatus(std::vector<PaxTypeFare::SegmentStatus>& segmentStatus, char bkgCode)
  {
    PaxTypeFare::SegmentStatus segStatus;
    segStatus._bkgCodeReBook = bkgCode;
    segmentStatus.push_back(segStatus);
  }

  void fillSegmentStatus()
  {
    addSegmentStatus(_fareUsage.segmentStatus(), 'M');
    addSegmentStatus(_paxTypeFare.segmentStatus(), 'Q');
    addSegmentStatus(_paxTypeFare.segmentStatusRule2(), 'Y');
  }

  // Delegate to protected methods

  PaxTypeFare::SegmentStatus& delegate_getSegmentStatus(size_t segIdx)
  {
    return _checker->getSegmentStatus(segIdx);
  }

  const std::vector<ClassOfService*>*
  delegate_getCosVec(AirSeg* airSeg, PaxTypeFare::SegmentStatus& segStat, size_t segIdx)
  {
    return _checker->getCosVec(airSeg, segStat, segIdx);
  }

  BookingCode delegate_getBookingCode(AirSeg* airSeg, PaxTypeFare::SegmentStatus& segStat)
  {
    return _checker->getBookingCode(airSeg, segStat);
  }

  // Manipulate state for protected method tests

  void resetFareUsage(FareUsage* fu) { _checker->_fu = fu; }

  void resetStatusTypeNotFlowForLocalJrnyCxr(bool status)
  {
    _checker->_statusTypeNotFlowForLocalJrnyCxr = status;
  }

  PricingTrx _trx;
  PaxTypeFare _paxTypeFare;
  FareMarket _fareMarket;
  PricingOptions _options;
  FareUsage _fareUsage;
  uint16_t _numSeats;
  PaxTypeFare::SegmentStatus _segStatus;
  BookingCode _bkc;
  std::shared_ptr<AvailGetterMock> _availGetterMock;
  std::shared_ptr<RebookedCOSCheckerMock> _checkerMock;
  std::shared_ptr<RebookedCOSChecker> _checker;
};

TEST_F(RebookedCOSCheckerTest, testSingleSegValid)
{
  fillMarket(1);
  std::vector<ClassOfService*> cosVec = fillCosVec("M4Y4");

  EXPECT_CALL(*_checkerMock, getNumSeats()).WillOnce(Return(_numSeats));
  EXPECT_CALL(*_checkerMock, getSegmentStatus(_)).WillOnce(ReturnRef(_segStatus));
  EXPECT_CALL(*_checkerMock, getCosVec(_, _, _)).WillOnce(Return(&cosVec));
  EXPECT_CALL(*_checkerMock, getBookingCode(_, _)).WillOnce(Return(_bkc));

  ASSERT_TRUE(_checkerMock->checkRebookedCOS());
}

TEST_F(RebookedCOSCheckerTest, testSingleSegInvalid)
{
  fillMarket(1);
  std::vector<ClassOfService*> cosVec = fillCosVec("M0Y4");

  EXPECT_CALL(*_checkerMock, getNumSeats()).WillOnce(Return(_numSeats));
  EXPECT_CALL(*_checkerMock, getSegmentStatus(_)).WillOnce(ReturnRef(_segStatus));
  EXPECT_CALL(*_checkerMock, getCosVec(_, _, _)).WillOnce(Return(&cosVec));
  EXPECT_CALL(*_checkerMock, getBookingCode(_, _)).WillOnce(Return(_bkc));

  ASSERT_FALSE(_checkerMock->checkRebookedCOS());
}

TEST_F(RebookedCOSCheckerTest, testTwoSegValid)
{
  fillMarket(2);
  std::vector<ClassOfService*> cosVec = fillCosVec("M4Y4");
  std::vector<ClassOfService*> cosVec2 = fillCosVec("M0Y4");
  BookingCode bkc2("Y");

  EXPECT_CALL(*_checkerMock, getNumSeats()).WillRepeatedly(Return(_numSeats));
  EXPECT_CALL(*_checkerMock, getSegmentStatus(_)).WillRepeatedly(ReturnRef(_segStatus));
  EXPECT_CALL(*_checkerMock, getCosVec(_, _, _)).Times(2).WillOnce(Return(&cosVec)).WillOnce(
      Return(&cosVec2));
  EXPECT_CALL(*_checkerMock, getBookingCode(_, _)).Times(2).WillOnce(Return(_bkc)).WillOnce(
      Return(bkc2));

  ASSERT_TRUE(_checkerMock->checkRebookedCOS());
}

TEST_F(RebookedCOSCheckerTest, testTwoSegInvalid)
{
  fillMarket(2);
  std::vector<ClassOfService*> cosVec = fillCosVec("M4Y4");
  std::vector<ClassOfService*> cosVec2 = fillCosVec("M0Y4");

  EXPECT_CALL(*_checkerMock, getNumSeats()).WillRepeatedly(Return(_numSeats));
  EXPECT_CALL(*_checkerMock, getSegmentStatus(_)).WillRepeatedly(ReturnRef(_segStatus));
  EXPECT_CALL(*_checkerMock, getCosVec(_, _, _)).Times(2).WillOnce(Return(&cosVec)).WillOnce(
      Return(&cosVec2));
  EXPECT_CALL(*_checkerMock, getBookingCode(_, _)).WillRepeatedly(Return(_bkc));

  ASSERT_FALSE(_checkerMock->checkRebookedCOS());
}

TEST_F(RebookedCOSCheckerTest, testGetSegmentStatusFromFareUsage)
{
  fillSegmentStatus();

  ASSERT_EQ(BookingCode('M'), delegate_getSegmentStatus(0)._bkgCodeReBook);
}

TEST_F(RebookedCOSCheckerTest, testGetSegmentStatusFromNotFlow)
{
  fillSegmentStatus();

  resetFareUsage(0);

  ASSERT_EQ(BookingCode('Q'), delegate_getSegmentStatus(0)._bkgCodeReBook);
}

TEST_F(RebookedCOSCheckerTest, testGetSegmentStatusFromFlow)
{
  fillSegmentStatus();

  resetFareUsage(0);
  resetStatusTypeNotFlowForLocalJrnyCxr(false);

  ASSERT_EQ(BookingCode('Y'), delegate_getSegmentStatus(0)._bkgCodeReBook);
}

TEST_F(RebookedCOSCheckerTest, testGetCosVecFromDependency)
{
  fillMarket(1);
  AirSeg* airSeg = _fareMarket.travelSeg().front()->toAirSeg();
  std::vector<ClassOfService*> cosVec = fillCosVec("M4Y4");

  EXPECT_CALL(*_availGetterMock, getAvailability(_, _, _, _)).WillOnce(Return(&cosVec));

  ASSERT_EQ(&cosVec, delegate_getCosVec(airSeg, _segStatus, 0));
}

TEST_F(RebookedCOSCheckerTest, testGetCosVecFromFareMarket)
{
  fillMarket(1);
  AirSeg* airSeg = _fareMarket.travelSeg().front()->toAirSeg();
  std::vector<ClassOfService*>* cosVec = _fareMarket.classOfServiceVec()[0];
  std::vector<ClassOfService*>* cosVecNull = 0;

  EXPECT_CALL(*_availGetterMock, getAvailability(_, _, _, _)).WillOnce(Return(cosVecNull));

  ASSERT_EQ(cosVec, delegate_getCosVec(airSeg, _segStatus, 0));
}

TEST_F(RebookedCOSCheckerTest, testGetBookingCodeFromRebook)
{
  fillMarket(1);
  AirSeg* airSeg = _fareMarket.travelSeg().front()->toAirSeg();

  _segStatus._bkgCodeReBook = 'M';
  _segStatus._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);

  airSeg->setBookingCode(BookingCode('Y'));

  ASSERT_EQ(BookingCode('M'), delegate_getBookingCode(airSeg, _segStatus));
}

TEST_F(RebookedCOSCheckerTest, testGetBookingCodeFromSegment)
{
  fillMarket(1);
  AirSeg* airSeg = _fareMarket.travelSeg().front()->toAirSeg();

  airSeg->setBookingCode(BookingCode('Y'));

  ASSERT_EQ(BookingCode('Y'), delegate_getBookingCode(airSeg, _segStatus));
}

} // namespace tse
