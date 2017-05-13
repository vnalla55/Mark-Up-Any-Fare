// Copyright Sabre 2016

#include "Rules/Cat31ChangeFinder.h"

#include "Common/ExchShopCalendarUtils.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/MultiAirportCity.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>
#include <array>

namespace tse
{
using namespace ExchShopCalendar;
using OriginDestination = PricingTrx::OriginDestination;

class Cat31ChangeFinderTest : public testing::Test
{
public:
  class CustomValidator : public ExchShopCalendar::R3ValidationResult
  {
  public:
    CustomValidator(const std::vector<PricingTrx::OriginDestination>& ondVec,
                    const Itin& excItin,
                    const Itin& newItin,
                    DataHandle& dh)
      : ExchShopCalendar::R3ValidationResult(ondVec, excItin, newItin, dh)
    {
    }

  private:
    bool
    isSameCity(const TravelSeg* lhs, const TravelSeg* rhs, DataHandle& dh, bool isBoarding) const
        override
    {
      auto func = [&](const TravelSeg& seg)
      { return (isBoarding ? seg.boardMultiCity() : seg.offMultiCity()); };

      return func(*lhs) == func(*rhs);
    }
  };

  void SetUp() override
  {
    _memHandle.create<TestConfigInitializer>();
    _handle = _memHandle.create<DataHandle>();
    _trx = _memHandle.create<RexPricingTrx>();
    _excItin = _memHandle.create<Itin>();
    _newItin = _memHandle.create<Itin>();
    addSegToVec("KRK", "WAW", DateTime(2016, 6, 16), 1);
    addSegToVec("WAW", "NYC", DateTime(2016, 6, 17), 1);
    addSegToVec("NYC", "KRK", DateTime(2016, 6, 18), 2, TravelSeg::CHANGED);
    addOndToVec("KRK", "NYC", DateTime(2016, 6, 16));
    addOndToVec("NYC", "KRK", DateTime(2016, 6, 18));
    _customValidator = _memHandle.create<CustomValidator>(_ondVec, *_excItin, *_newItin, *_handle);
    _finder = _memHandle.create<Cat31ChangeFinder>(_travelSegVec, _ondVec, _customValidator, true, *_trx);
    _finderSegVecPtr = &_finder->_travelSegVec;
  }

  void TearDown() override
  {
    _memHandle.clear();
  }

protected:
  void addSegToVec(const LocCode& origin,
                   const LocCode& destination,
                   const DateTime& date,
                   const uint32_t legId,
                   const TravelSeg::ChangeStatus& changed = TravelSeg::UNCHANGED)
  {
    AirSeg* newSeg = _memHandle.create<AirSeg>();
    newSeg->boardMultiCity() = origin;
    newSeg->offMultiCity() = destination;
    newSeg->departureDT() = date;
    newSeg->pssDepartureDate() = date.toSimpleString();
    newSeg->itinIndex() = 0;
    newSeg->changeStatus() = changed;
    newSeg->carrier() = "ROBB";
    newSeg->legId() = legId;
    _travelSegVec.push_back(newSeg);

    if (_newItin->fareMarket().size() < legId)
    {
      _newItin->fareMarket().push_back(_memHandle.create<FareMarket>());
    }
    _newItin->fareMarket()[legId - 1]->travelSeg().push_back(newSeg);
    _excItin->travelSeg().push_back(newSeg);
  }

  ArunkSeg* createArunkFromSeg(const TravelSeg& travelSeg)
  {
    ArunkSeg* arunkSeg = _memHandle.create<ArunkSeg>();
    arunkSeg->boardMultiCity() = travelSeg.boardMultiCity();
    arunkSeg->offMultiCity() = travelSeg.offMultiCity();
    arunkSeg->departureDT() = travelSeg.departureDT();
    arunkSeg->pssDepartureDate() = travelSeg.pssDepartureDate();
    arunkSeg->legId() = travelSeg.legId();
    arunkSeg->itinIndex() = 0;
    arunkSeg->changeStatus() = travelSeg.changeStatus();
    return arunkSeg;
  }

  void addOndToVec(const LocCode& origin, const LocCode& destination, const DateTime& date)
  {
    OriginDestination ond;
    ond.boardMultiCity = origin;
    ond.offMultiCity = destination;
    ond.travelDate = date;
    ond.calDaysBefore = _calendarDaysBefore;
    ond.calDaysAfter = _calendarDaysAfter;
    _ondVec.push_back(ond);
  }

  const uint32_t getOndIndexFromMap(const TravelSeg& tvlSeg)
  {
    return _finder->findOndIndexForSeg(tvlSeg);
  }

  void clearTravelSegVec() { _finder->_travelSegVec.clear(); }

  bool callMatchedChangedSeg(const TravelSeg* travelSeg)
  {
    return _finder->matchedChangedSegOnNewItin(travelSeg);
  }

  void callUpdateValidationResults(const TravelSeg& travelSeg)
  {
    if (_customValidator)
      _finder->updateCalendarValidationResults(travelSeg);
    else
      throw std::exception();
  }

  void setAllSegStatuses(const TravelSeg::ChangeStatus& changed)
  {
    for (auto* seg : _travelSegVec)
      seg->changeStatus() = changed;
  }

  bool matchDatesUpToDays(const DateTime& date, const DateTime& dateDate)
  {
    return date.year() == dateDate.year() && date.month() == dateDate.month() &&
           date.day() == dateDate.day();
  }

  void resetFinder(const bool isCalendar = false)
  {
    _customValidator = _memHandle.create<CustomValidator>(_ondVec, *_excItin, *_newItin, *_handle);
    _finder =
        _memHandle.create<Cat31ChangeFinder>(_travelSegVec, _ondVec, _customValidator, isCalendar, *_trx);
    _finderSegVecPtr = &_finder->_travelSegVec;
  }

  TestMemHandle _memHandle;
  DataHandle* _handle;
  RexPricingTrx* _trx;
  Itin* _excItin, *_newItin;
  Cat31ChangeFinder* _finder;
  std::vector<TravelSeg*> _travelSegVec;
  std::vector<TravelSeg*>* _finderSegVecPtr;
  std::vector<OriginDestination> _ondVec;
  ExchShopCalendar::R3ValidationResult* _customValidator;
  const uint32_t _calendarDaysBefore = 3, _calendarDaysAfter = 3;
};

TEST_F(Cat31ChangeFinderTest, segToOndMappedPass)
{
  std::array<uint32_t, 3> expectedValues{0, 0, 1};
  size_t index = 0;

  for (auto* travelSeg : _travelSegVec)
    ASSERT_EQ(getOndIndexFromMap(*travelSeg), expectedValues[index++]);
}

TEST_F(Cat31ChangeFinderTest, segToOndMappedFail)
{
  std::array<uint32_t, 3> notExpectedValues{2, 1, 0};
  size_t index = 0;

  for (auto* travelSeg : _travelSegVec)
    ASSERT_FALSE(getOndIndexFromMap(*travelSeg) == notExpectedValues[index++]);

  addSegToVec("A", "D", DateTime(2016, 6, 19), 3);
  ASSERT_EQ(getOndIndexFromMap(*_travelSegVec.back()), ExchShopCalendar::INVALID_OND_INDEX);
}

TEST_F(Cat31ChangeFinderTest, updateValidationResults)
{
  const TravelSeg& travelSegFirst = *_travelSegVec.front(), &travelSegLast = *_travelSegVec.back();
  ASSERT_NO_THROW(callUpdateValidationResults(travelSegLast));
  DateRange range = _customValidator->getDateRange();
  ASSERT_TRUE(matchDatesUpToDays(range.firstDate, travelSegFirst.departureDT().subtractDays(3)));
  ASSERT_TRUE(matchDatesUpToDays(range.lastDate, travelSegLast.departureDT()));
}

TEST_F(Cat31ChangeFinderTest, updateValidationResultsEmpty)
{
  resetFinder(true);

  for (auto travelSeg : _travelSegVec)
    ASSERT_NO_THROW(callUpdateValidationResults(*travelSeg));
}

TEST_F(Cat31ChangeFinderTest, notChangedAnyChanged)
{
  resetFinder(true);
  ASSERT_TRUE(_finder->notChanged(_travelSegVec));
}

TEST_F(Cat31ChangeFinderTest, notChangedAllChanged)
{
  resetFinder(true);
  setAllSegStatuses(TravelSeg::CHANGED);
  ASSERT_TRUE(_finder->notChanged(_travelSegVec));
}

TEST_F(Cat31ChangeFinderTest, notChangedAllUnchanged)
{
  resetFinder(true);
  setAllSegStatuses(TravelSeg::UNCHANGED);
  ASSERT_TRUE(_finder->notChanged(_travelSegVec));
}

TEST_F(Cat31ChangeFinderTest, notChangedAllNotChanged)
{
  resetFinder(true);

  _travelSegVec.clear();
  addSegToVec("KRK", "WAW", DateTime(2016, 1, 1), 1, TravelSeg::UNCHANGED);
  addSegToVec("WAW", "NYC", DateTime(2016, 1, 2), 1, TravelSeg::INVENTORYCHANGED);
  addSegToVec("NYC", "DEN", DateTime(2016, 1, 3), 2, TravelSeg::CONFIRMOPENSEGMENT);

  ASSERT_TRUE(_finder->notChanged(_travelSegVec));
}

TEST_F(Cat31ChangeFinderTest, notChangedEmpty)
{
  ASSERT_TRUE(_finder->notChanged(*_memHandle.create<std::vector<TravelSeg*>>()));

  clearTravelSegVec();
  ASSERT_FALSE(_finder->notChanged(_travelSegVec));
}

TEST_F(Cat31ChangeFinderTest, matchedChangedEmpty)
{
  clearTravelSegVec();
  for (auto travelSeg : _travelSegVec)
    ASSERT_FALSE(callMatchedChangedSeg(travelSeg));
}

TEST_F(Cat31ChangeFinderTest, noMatchedChangedArunkToAir)
{
  const TravelSeg* travelSeg = createArunkFromSeg(*_travelSegVec.front());
  ASSERT_FALSE(callMatchedChangedSeg(travelSeg));
}

TEST_F(Cat31ChangeFinderTest, matchedChangedArunkToArunk)
{
  TravelSeg* travelSeg = createArunkFromSeg(*_travelSegVec.front());
  _travelSegVec[0] = travelSeg;
  resetFinder();
  ASSERT_TRUE(callMatchedChangedSeg(travelSeg));
}

TEST_F(Cat31ChangeFinderTest, noMatchedChangedNotMatchingSeg)
{
  const TravelSeg* travelSeg = _memHandle.create<AirSeg>();
  ASSERT_FALSE(callMatchedChangedSeg(travelSeg));
}

TEST_F(Cat31ChangeFinderTest, noMatchedChangedOndOutOfDateRange)
{
  addSegToVec("KRK", "DEN", DateTime(2322, 6, 23), 3);
  addOndToVec("KRK", "DEN", DateTime(2322, 6, 19));
  resetFinder();
  ASSERT_FALSE(callMatchedChangedSeg(_travelSegVec.back()));
}

TEST_F(Cat31ChangeFinderTest, matchedChangedDifferentCarrier)
{
  AirSeg* airSeg = static_cast<AirSeg*>(_travelSegVec.front());
  airSeg->carrier() = "KNOWS";
  ASSERT_TRUE(callMatchedChangedSeg(airSeg))
      << static_cast<AirSeg*>(_finderSegVecPtr->front())->carrier() << " " << airSeg->carrier();
}

} // tse
