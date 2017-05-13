//----------------------------------------------------------------------------
//  Copyright Sabre 2016
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

#include "Shopping/Cat31RestrictionMerger.h"

#include "Common/ExchShopCalendarUtils.h"
#include "DataModel/RexShoppingTrx.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>

#include <array>

namespace tse
{

namespace
{
using DateRange = ExchShopCalendar::DateRange;

bool operator==(const DateRange& rangeOne, const DateRange& rangeTwo)
{
  return rangeOne.firstDate == rangeTwo.firstDate && rangeOne.lastDate == rangeTwo.lastDate;
}

class Cat31RestrictionMergerMock : public Cat31RestrictionMerger
{
public:
  Cat31RestrictionMergerMock(OADResponseDataMap& oadMergedR3Data,
                             bool oneCarrierTicket,
                             const Itin& itin,
                             RexShoppingTrx& trx)
    : Cat31RestrictionMerger(oadMergedR3Data, oneCarrierTicket, itin, trx)
  {
  }
  auto divideConstraintsToCorrectRanges(const RexShoppingTrx::R3SeqsConstraintMap& oadData)
  {
    return Cat31RestrictionMerger::divideConstraintsToCorrectRanges(oadData);
  }
};
}

class Cat31RestrictionMergerTest : public ::testing::Test
{
protected:
  const CarrierCode robb = "ROB";
  const CarrierCode knows = "KNW";
  const CarrierCode pozalSieBoze = "PSB";

  using OADResponseDataMap = RexShoppingTrx::OADResponseDataMap;
  using OADResponseData = RexShoppingTrx::OADResponseData;
  using R3SeqsConstraint = RexShoppingTrx::R3SeqsConstraint;
  using R3SeqsConstraintVec = std::vector<const R3SeqsConstraint*>;
  Cat31RestrictionMergerMock* merger;
  std::vector<R3SeqsConstraintVec> rec3Constraints;
  std::vector<OADResponseData> oadResponseVec;
  RexShoppingTrx::R3SeqsConstraintMap r3SeqConstraintMap;

  OADResponseData* createOAD()
  {
    return _memHandle.create<OADResponseData>();
  }

  void addOnd(PricingTrx::OriginDestination& ond)
  {
    _trx->orgDest.push_back(ond);
  }

  void addR3SeqConstraint(const DateRange& dateRange)
  {
    auto* newR3Seq = _memHandle.create<R3SeqsConstraint>();
    newR3Seq->calendarRange = dateRange;
    const size_t currentSize = rec3Constraints.size();
    rec3Constraints.resize(currentSize + 1);
    rec3Constraints[currentSize].push_back(newR3Seq);
  }

  void addCarriers(OADResponseData& oad, const CarrierCode& code)
  {
    oad.fareByteCxrAppl.cxrList.insert(code);
  }

  template<typename... Args>
  void addCarriers(OADResponseData& oad, const CarrierCode& code, Args... args)
  {
    addCarriers(oad, code);
    addCarriers(oad, args...);
  }

  void turnOffCalendar()
  {
    _trx->orgDest.clear();
  }

  void insertCalendarRestrToOADData(uint32_t seqNo,
                                    ExchShopCalendar::DateRange calendarRange,
                                    ExchShopCalendar::DateApplication calendarAppl)
  {
    RexShoppingTrx::R3SeqsConstraint constr;
    constr.calendarRange = calendarRange;
    constr.calendarAppl = calendarAppl;
    r3SeqConstraintMap.emplace(std::make_pair(seqNo, std::vector<decltype(constr)>{constr}));
  }

private:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<RexShoppingTrx>();
    _oadMap = _memHandle.create<OADResponseDataMap>();
    _itin = _memHandle.create<Itin>();
    _trx->orgDest.push_back(PricingTrx::OriginDestination());
    _trx->orgDest.front().calDaysBefore = 3;
    _trx->orgDest.front().calDaysAfter = 3;
    merger = _memHandle.create<Cat31RestrictionMergerMock>(*_oadMap, false, *_itin, *_trx);
  }

  void TearDown()
  {
    _memHandle.clear();
  }

  TestMemHandle _memHandle;
  RexShoppingTrx* _trx;
  OADResponseDataMap* _oadMap;
  Itin* _itin;
};

TEST_F(Cat31RestrictionMergerTest, mergeCalendarRangeEmptyResponseVec)
{
  ASSERT_NO_THROW(merger->mergeCalendarRange(rec3Constraints, oadResponseVec));
  ASSERT_TRUE(oadResponseVec.empty());
}

TEST_F(Cat31RestrictionMergerTest, mergeCalendarRangeNotEXSCalendar)
{
  turnOffCalendar();
  OADResponseData oadRsp;
  addCarriers(oadRsp, robb, knows);
  oadResponseVec.push_back(oadRsp);
  ASSERT_FALSE(oadResponseVec.empty());
  addR3SeqConstraint(DateRange{DateTime(), DateTime()});
  ASSERT_NO_THROW(merger->mergeCalendarRange(rec3Constraints, oadResponseVec));
  ASSERT_FALSE(oadResponseVec.empty());
}

TEST_F(Cat31RestrictionMergerTest, mergeCalendarRangeOnlySameCarriersMerged)
{
  OADResponseData* oadRsp = createOAD();
  addCarriers(*oadRsp, robb, knows);
  oadResponseVec.push_back(*oadRsp);
  addR3SeqConstraint(DateRange{DateTime(2016,1,10), DateTime(2016,1,13)});

  oadRsp = createOAD();
  addCarriers(*oadRsp, robb, knows);
  oadResponseVec.push_back(*oadRsp);
  addR3SeqConstraint(DateRange{DateTime(2016,1,12), DateTime(2016,1,15)});

  oadRsp = createOAD();
  addCarriers(*oadRsp, robb, knows);
  oadResponseVec.push_back(*oadRsp);
  addR3SeqConstraint(DateRange{DateTime(2016,1,12), DateTime(2016,1,18)});

  ASSERT_NO_THROW(merger->mergeCalendarRange(rec3Constraints, oadResponseVec));
  ASSERT_FALSE(oadResponseVec.empty());
  ASSERT_TRUE(oadResponseVec.size() == 1);
  ASSERT_TRUE(oadResponseVec.front().calendarRange == DateRange({DateTime(2016,1,10), DateTime(2016,1,18)}));
}

TEST_F(Cat31RestrictionMergerTest, mergeCalendarRangeOnlySameCarriersNoneMerged)
{
  std::array<DateRange, 3> resultsArr;

  OADResponseData* oadRsp = createOAD();
  addCarriers(*oadRsp, robb, knows);
  oadResponseVec.push_back(*oadRsp);
  resultsArr[0] = {DateRange{DateTime(2016,1,10), DateTime(2016,1,10)}};
  addR3SeqConstraint(resultsArr[0]);

  oadRsp = createOAD();
  addCarriers(*oadRsp, robb, knows);
  oadResponseVec.push_back(*oadRsp);
  resultsArr[1] = {DateRange{DateTime(2016,1,12), DateTime(2016,1,15)}};
  addR3SeqConstraint(resultsArr[1]);

  oadRsp = createOAD();
  addCarriers(*oadRsp, robb, knows);
  oadResponseVec.push_back(*oadRsp);
  resultsArr[2] = {DateRange{DateTime(2016,1,17), DateTime(2016,1,18)}};
  addR3SeqConstraint(resultsArr[2]);

  ASSERT_NO_THROW(merger->mergeCalendarRange(rec3Constraints, oadResponseVec));
  ASSERT_FALSE(oadResponseVec.empty());
  ASSERT_TRUE(oadResponseVec.size() == resultsArr.size());

  size_t idx = 0;
  for (const DateRange& expectedResult : resultsArr)
    ASSERT_TRUE(oadResponseVec[idx++].calendarRange == expectedResult);
}

TEST_F(Cat31RestrictionMergerTest, mergeCalendarRangeOnlyOtherCarriersNotMerged)
{
  std::array<DateRange, 3> resultsArr;

  OADResponseData* oadRsp = createOAD();
  addCarriers(*oadRsp, robb);
  oadResponseVec.push_back(*oadRsp);
  resultsArr[0] = {DateRange{DateTime(2016,1,10), DateTime(2016,1,13)}};
  addR3SeqConstraint(resultsArr[0]);
  oadRsp = createOAD();
  addCarriers(*oadRsp, robb, knows, pozalSieBoze);
  oadResponseVec.push_back(*oadRsp);
  resultsArr[1] = {DateRange{DateTime(2016,1,12), DateTime(2016,1,15)}};
  addR3SeqConstraint(resultsArr[1]);
  oadRsp = createOAD();
  addCarriers(*oadRsp, knows);
  oadResponseVec.push_back(*oadRsp);
  resultsArr[2] = {DateRange{DateTime(2016,1,12), DateTime(2016,1,18)}};
  addR3SeqConstraint(resultsArr[2]);

  ASSERT_NO_THROW(merger->mergeCalendarRange(rec3Constraints, oadResponseVec));
  ASSERT_FALSE(oadResponseVec.empty());
  ASSERT_TRUE(oadResponseVec.size() == resultsArr.size());

  size_t idx = 0;
  for (const DateRange& expectedResult : resultsArr)
  {
    ASSERT_TRUE(oadResponseVec[idx++].calendarRange == expectedResult);
  }
}

TEST_F(Cat31RestrictionMergerTest, mergeCalendarRangeMixedCarriersOnlySameMerged)
{
  std::array<DateRange, 2> resultsArr;

  OADResponseData* oadRsp = createOAD();
  addCarriers(*oadRsp, pozalSieBoze);
  oadResponseVec.push_back(*oadRsp);
  resultsArr[0] = {DateRange{DateTime(2016,1,12), DateTime(2016,1,15)}};
  addR3SeqConstraint(resultsArr[0]);

  oadRsp = createOAD();
  addCarriers(*oadRsp, robb, knows);
  oadResponseVec.push_back(*oadRsp);
  resultsArr[1] = {DateRange{DateTime(2016,1,10), DateTime(2016,1,13)}};
  addR3SeqConstraint(resultsArr[1]);

  oadRsp = createOAD();
  addCarriers(*oadRsp, robb, knows);
  oadResponseVec.push_back(*oadRsp);
  resultsArr[1] = {DateRange{DateTime(2016,1,12), DateTime(2016,1,18)}};
  addR3SeqConstraint(resultsArr[1]);

  resultsArr[1].firstDate = {DateTime(2016,1,10)};

  ASSERT_NO_THROW(merger->mergeCalendarRange(rec3Constraints, oadResponseVec));
  ASSERT_FALSE(oadResponseVec.empty());
  ASSERT_TRUE(oadResponseVec.size() == resultsArr.size());

  size_t idx = 0;
  for (const DateRange& expectedResult : resultsArr)
  {
    ASSERT_TRUE(oadResponseVec[idx++].calendarRange == expectedResult);
  }
}

TEST_F(Cat31RestrictionMergerTest, test_divideConstraintsToCorrectRanges_empty)
{
  auto result = merger->divideConstraintsToCorrectRanges(r3SeqConstraintMap);
  ASSERT_TRUE(result.empty());
}

TEST_F(Cat31RestrictionMergerTest, test_divideConstraintsToCorrectRanges_wholePeriod)
{
  insertCalendarRestrToOADData(1, {DateTime(2016, 7, 20), DateTime(2016, 7, 26)},
                               ExchShopCalendar::WHOLE_PERIOD);

  insertCalendarRestrToOADData(2, {DateTime(2016, 7, 20), DateTime(2016, 7, 26)},
                               ExchShopCalendar::WHOLE_PERIOD);

  auto result = merger->divideConstraintsToCorrectRanges(r3SeqConstraintMap);
  ASSERT_TRUE(1 == result.size());
  ASSERT_TRUE(2 == result[0].size());
  ASSERT_TRUE(ExchShopCalendar::WHOLE_PERIOD == result[0].front()->calendarAppl);
}

TEST_F(Cat31RestrictionMergerTest, test_divideConstraintsToCorrectRanges_wholePeriodWithSameDate)
{
  insertCalendarRestrToOADData(1, {DateTime(2016, 7, 20), DateTime(2016, 7, 26)},
                               ExchShopCalendar::WHOLE_PERIOD);

  insertCalendarRestrToOADData(2, {DateTime(2016, 7, 23), DateTime(2016, 7, 23)},
                               ExchShopCalendar::SAME_DEPARTURE_DATE);

  auto result = merger->divideConstraintsToCorrectRanges(r3SeqConstraintMap);
  ASSERT_TRUE(3 == result.size());
  ASSERT_TRUE(ExchShopCalendar::WHOLE_PERIOD == result[0].front()->calendarAppl);
  ASSERT_TRUE(ExchShopCalendar::SAME_DEPARTURE_DATE == result[1].front()->calendarAppl);
  ASSERT_TRUE(ExchShopCalendar::WHOLE_PERIOD == result[2].front()->calendarAppl);
}

TEST_F(Cat31RestrictionMergerTest, test_divideConstraintsToCorrectRanges_sameDate)
{
  insertCalendarRestrToOADData(1, {DateTime(2016, 7, 20), DateTime(2016, 7, 20)},
                               ExchShopCalendar::SAME_DEPARTURE_DATE);

  auto result = merger->divideConstraintsToCorrectRanges(r3SeqConstraintMap);
  ASSERT_TRUE(1 == result.size());
  ASSERT_TRUE(ExchShopCalendar::SAME_DEPARTURE_DATE == result[0].front()->calendarAppl);
}

TEST_F(Cat31RestrictionMergerTest, test_divideConstraintsToCorrectRanges_sameAndLaterDate)
{
  insertCalendarRestrToOADData(1, {DateTime(2016, 7, 20), DateTime(2016, 7, 20)},
                               ExchShopCalendar::SAME_DEPARTURE_DATE);

  insertCalendarRestrToOADData(2, {DateTime(2016, 7, 21), DateTime(2016, 7, 23)},
                               ExchShopCalendar::LATER_DEPARTURE_DATE);

  auto result = merger->divideConstraintsToCorrectRanges(r3SeqConstraintMap);
  ASSERT_TRUE(2 == result.size());
  ASSERT_TRUE(ExchShopCalendar::SAME_DEPARTURE_DATE == result[0].front()->calendarAppl);
  ASSERT_TRUE(ExchShopCalendar::LATER_DEPARTURE_DATE == result[1].front()->calendarAppl);
}

TEST_F(Cat31RestrictionMergerTest, test_divideConstraintsToCorrectRanges_wholePeriodWithLaterDate)
{
  insertCalendarRestrToOADData(1, {DateTime(2016, 7, 17), DateTime(2016, 7, 23)},
                               ExchShopCalendar::WHOLE_PERIOD);

  insertCalendarRestrToOADData(2, {DateTime(2016, 7, 21), DateTime(2016, 7, 23)},
                               ExchShopCalendar::LATER_DEPARTURE_DATE);

  auto result = merger->divideConstraintsToCorrectRanges(r3SeqConstraintMap);
  ASSERT_TRUE(2 == result.size());
  ASSERT_TRUE(ExchShopCalendar::WHOLE_PERIOD == result[0].front()->calendarAppl);
  ASSERT_TRUE(ExchShopCalendar::LATER_DEPARTURE_DATE == result[1].front()->calendarAppl);
}

TEST_F(Cat31RestrictionMergerTest, test_divideConstraintsToCorrectRanges_allRanges)
{
  insertCalendarRestrToOADData(1, {DateTime(2016, 7, 17), DateTime(2016, 7, 23)},
                               ExchShopCalendar::WHOLE_PERIOD);

  insertCalendarRestrToOADData(2, {DateTime(2016, 7, 20), DateTime(2016, 7, 20)},
                               ExchShopCalendar::SAME_DEPARTURE_DATE);

  insertCalendarRestrToOADData(3, {DateTime(2016, 7, 21), DateTime(2016, 7, 23)},
                               ExchShopCalendar::LATER_DEPARTURE_DATE);

  auto result = merger->divideConstraintsToCorrectRanges(r3SeqConstraintMap);
  ASSERT_TRUE(3 == result.size());
  ASSERT_TRUE(ExchShopCalendar::WHOLE_PERIOD == result[0].front()->calendarAppl);
  ASSERT_TRUE(ExchShopCalendar::SAME_DEPARTURE_DATE == result[1].front()->calendarAppl);
  ASSERT_TRUE(ExchShopCalendar::LATER_DEPARTURE_DATE == result[2].front()->calendarAppl);
}

} //tse
