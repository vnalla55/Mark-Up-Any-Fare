#include "Rules/PeriodOfStay.h"

#include "Common/DateTime.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TseEnums.h"
#include "Common/TseUtil.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TravelSeg.h"

#include <boost/lexical_cast.hpp>

namespace tse
{
static Logger
logger("atseintl.Rules.PeriodOfStay");

const std::string
PeriodOfStay::FIRST_WEEK("01");
const std::string
PeriodOfStay::LAST_WEEK("52");

PeriodOfStay::PeriodOfStay(const std::string& period, const std::string& unitName)
  : _periodOfStay(period), _strUnit(unitName)
{
  if (LIKELY(!(period.empty()) && !(unitName.empty())))
  {
    convertPeriodOfStay();
    convertUnit();
    _isValid = true;
  }
} // lint !e1541

//-------------------------------------------------------------------
//   @method convertPeriodOfStay
//
//   Description: Converts the minimum/maximum stay from a 3 character
//                number in the range of 000 - 999 to an integer
//                or leaves it as a 3 character day of week:
//                SUN - SAT.
//
//   @return void
//-------------------------------------------------------------------
void
PeriodOfStay::convertPeriodOfStay()
{
  LOG4CXX_INFO(logger, " Entered PeriodOfStay::convertPeriodOfStay()");

  int dayOfWeekNo = getDayOfWeek(_periodOfStay);

  if (!((dayOfWeekNo >= 0) && (dayOfWeekNo <= 6)))
  {
    // Must be a number in range of 1 - 999

    LOG4CXX_DEBUG(logger, "min/max stay is: " << _periodOfStay.c_str());
    _iPeriodOfStay = atoi(_periodOfStay.c_str());
    LOG4CXX_DEBUG(logger, "Converted min/max stay to a number: " << _iPeriodOfStay);
  }
  else
  {
    _dayOfWeek = Weekdays(dayOfWeekNo);
    _isDayOfWeek = true;
  }

  LOG4CXX_INFO(logger, " Leaving PeriodOfStay::convertPeriodOfStay()");
}

//------------------------------------------------------------------------
//   @method convertUnit
//
//   Description: Determines whether the minimum/maximum stay unit is a string
//                representation of a number between 01 - 52 or a character
//                representing one of the following: N - Minutes, H - Hours,
//                D - Days, or M - Months.
//
//   @return void
//------------------------------------------------------------------------
void
PeriodOfStay::convertUnit()
{

  LOG4CXX_INFO(logger, " Entered PeriodOfStay::convertUnit()");

  if (_strUnit >= FIRST_WEEK && _strUnit <= LAST_WEEK)
  {
    _unit = atoi(_strUnit.c_str());
    LOG4CXX_DEBUG(logger, "Converted min/max stay unit to a number: " << _unit);
  }
  else
  {
    _unit = _strUnit[0];
    LOG4CXX_DEBUG(logger, "min/max stay unit is : " << _unit);
  }

  LOG4CXX_INFO(logger, " Leaving PeriodOfStay::convertUnit()");
}

//---------------------------------------------------------
//   @method getDayOfWeek
//
//   Description: Returns the day of week as an integer
//
//   @return int - day of week
//--------------------------------------------------------
int
PeriodOfStay::getDayOfWeek(const std::string& dayOfWeekName)
{
  int weekDay = -1;

  for (int i = 0; i < 7; i++)
  {
    if (dayOfWeekName == WEEKDAYS_UPPER_CASE[i])
    {
      weekDay = i;
      break;
    }
  }

  return weekDay;
}

//---------------------------------------------------------
//   @method isOneYear
//
//   Description: Returns true if this Minimum/Maximum Stay
//                represents one year.
//
//   @return bool - true, minimum/maximum stay is one year,
//                  else false.
//--------------------------------------------------------
const bool
PeriodOfStay::isOneYear()
{
  if ((_unit == DAYS && _iPeriodOfStay == 365) || (_unit == MONTHS && _iPeriodOfStay == 12))
  {
    return true;
  }

  return false;
}

const std::string
PeriodOfStay::getPeriodOfStayAsString() const
{
  if (_isValid && !_isDayOfWeek)
  {
    std::string stringPeriod(boost::lexical_cast<std::string>(_iPeriodOfStay));

    switch (_unit)
    {
    case MINUTES:
      stringPeriod += " MINUTE";
      break;
    case HOURS:
      stringPeriod += " HOUR";
      break;
    case DAYS:
      stringPeriod += " DAY";
      break;
    case MONTHS:
      stringPeriod += " MONTH";
    }

    stringPeriod += _iPeriodOfStay > 1 ? "S" : "";
    return stringPeriod;
  }
  return "NOT VALID PERIOD OF STAY";
}
}
