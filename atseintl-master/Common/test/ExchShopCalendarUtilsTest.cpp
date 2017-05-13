#include "Common/ExchShopCalendarUtils.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/Itin.h"
#include "test/include/TestMemHandle.h"
#include <gtest/gtest.h>

namespace tse
{
namespace ExchShopCalendar
{

class ExchShopCalendarUtilsTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _outbound.travelDate = DateTime(2016, 6, 7);
    _outbound.calDaysBefore = 3;
    _outbound.calDaysAfter = 3;

    _inbound.travelDate = DateTime(2016, 6, 21);
    _inbound.calDaysBefore = 3;
    _inbound.calDaysAfter = 3;

    std::vector<PricingTrx::OriginDestination> onds = {_outbound, _inbound};
    _placeholderItin = _memHandle.create<Itin>();
    _valResult = _memHandle.create<R3ValidationResult>(
        onds, *_placeholderItin, *_placeholderItin, *_memHandle.create<DataHandle>());
  }
  void TearDown() { _memHandle.clear(); }

protected:
  TestMemHandle _memHandle;
  PricingTrx::OriginDestination _outbound, _inbound;
  Itin* _placeholderItin;
  R3ValidationResult* _valResult;
};

TEST_F(ExchShopCalendarUtilsTest, testGetDateRange)
{
  DateRange dateRange = _valResult->getDateRange();
  ASSERT_STREQ(dateRange.firstDate.dateToString(DateFormat::DDMMYYYY,"-").c_str(), "04-06-2016");
  ASSERT_STREQ(dateRange.lastDate.dateToString(DateFormat::DDMMYYYY,"-").c_str(), "24-06-2016");
}

TEST_F(ExchShopCalendarUtilsTest, testAddDateRangeLimitBegin)
{
  DateRange inputDateRange = {DateTime(2016, 6, 5), DateTime(2016, 6, 13)};
  _valResult->addDateRange(inputDateRange, 0);

  ASSERT_EQ(_valResult->isValid(), true);

  DateRange outputDateRange = _valResult->getDateRange();
  ASSERT_STREQ(outputDateRange.firstDate.dateToString(DateFormat::DDMMYYYY,"-").c_str(), "05-06-2016");
  ASSERT_STREQ(outputDateRange.lastDate.dateToString(DateFormat::DDMMYYYY,"-").c_str(), "24-06-2016");
}

TEST_F(ExchShopCalendarUtilsTest, testAddDateRangeLimitEnd)
{
  DateRange inputDateRange = {DateTime(2016, 6, 1), DateTime(2016, 6, 20)};
  _valResult->addDateRange(inputDateRange, 1);

  ASSERT_EQ(_valResult->isValid(), true);

  DateRange outputDateRange = _valResult->getDateRange();
  ASSERT_STREQ(outputDateRange.firstDate.dateToString(DateFormat::DDMMYYYY,"-").c_str(), "04-06-2016");
  ASSERT_STREQ(outputDateRange.lastDate.dateToString(DateFormat::DDMMYYYY,"-").c_str(), "20-06-2016");
}

TEST_F(ExchShopCalendarUtilsTest, testAddDateRangeOutboundFailed)
{
  DateRange inputDateRange = {DateTime(2016, 5, 1), DateTime(2016, 5, 20)};
  _valResult->addDateRange(inputDateRange, 0);

  ASSERT_EQ(_valResult->isValid(), false);
}

TEST_F(ExchShopCalendarUtilsTest, testAddDateRangeInboundFailed)
{
  DateRange inputDateRange = {DateTime(2016, 7, 1), DateTime(2016, 7, 20)};
  _valResult->addDateRange(inputDateRange, 1);

  ASSERT_EQ(_valResult->isValid(), false);
}


} //ExchShopCalendar
} //tse
