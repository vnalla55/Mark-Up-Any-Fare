
//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/DateTime.h"


#include <string>

namespace tse
{

/**
*   @class PeriodOfStay
*
*   Description:
*   The PeriodOfStay class encapsulates the database fields:
*   MinStay or MaxStay and Unit of Time for Category 6 and 7
*   Minimum Stay and Maximum Stay.  These fields are interdependent on each other.
*   The MinStay/MaxStay can either be represented as a 3 character day of
*   week (SUN - SAT) or a number 1 - 999(Max Stay can be 000 - 999). The Unit of Time
*   can be represented  as a number such as: 1 - 52 or a 1 character field :
*   Nb - Minutes, Hb - Hours, Db - Days and Mb - Months. If there is a MinStay/MaxStay
*   and no Unit of Time then this is an invalid combination. The reverse is also true.
*   The user of this class must check: isValid prior to using this object.
*   Possible combinations of MinStay/MaxStay and Unit of Time are:
*
*   MinStay/
*   MaxStay        Unit of Time
*   -------        --------------
*   SUN            1               meaning 1st Sunday from some date
*   FRI            5               meaning 5th Friday from some date
*   999            N               meaning 999 minutes from some date/time
*   000            D               Same Day - only used for Max Stay - Cat 7
*   2              D               meaning 2 days from some date
*   3              H               meaning 3 hours from some date/time
*   3              M               meaning 3 months from some date
*
*   The Unit of Time is converted to an integral value consisting of:
*   1 - 52 or DAYS = 68, HOURS = 72, MONTHS = 77 or MINUTES = 78.
*
*/
class PeriodOfStay
{
public:
  PeriodOfStay(const std::string& period, const std::string& unit);

  // Convert Unit of Time in Database from 2 chars to an enum value
  //
  enum PeriodOfStayUnit
  {
    DAYS = 68,
    HOURS = 72,
    MONTHS = 77,
    MINUTES = 78
  };

  /**
  *   @method minStay
  *
  *   Description: Retrieves the minimum/maximum stay day of week as a string
  *                SUN - SAT.
  *
  *   @return string - day of week
  */
  operator const std::string&() const { return _periodOfStay; }

  /**
  *   @method int&
  *
  *   Description: Retrieves the minimum/maximum stay as a number
  *                001 - 999(max stay includes 000).
  *
  *   @return int - minimum/maximum stay as an elapsed period of time.
  */
  operator const int&() const { return _iPeriodOfStay; }

  /**
  *   @method dayOfWeek
  *
  *   Description: Get/Set Minimum/Maximum stay day of week
  *
  *   @return Weekdays    - DateTime day of week - Sunday(0) - Saturday(6)
  */
  Weekdays& dayOfWeek() { return _dayOfWeek; }
  const Weekdays& dayOfWeek() const { return _dayOfWeek; }

  /**
  *   @method isValid
  *
  *   Description: Returns true if this Minimum/Maximum Stay is valid.
  *
  *   @return bool    - true, minimum stay is valid and applies ,
  *                     else false.
  */
  bool isValid() const { return _isValid; }

  /**
  *   @method isDayOfWeek
  *
  *   Description: Returns true if this Minimum/Maximum Stay
  *                represents a day of the week.
  *
  *   @return bool    - true, minimum/maximum stay is day of week,
  *                     else false.
  */
  const bool& isDayOfWeek() const { return _isDayOfWeek; }

  /**
  *   @method isOneYear
  *
  *   Description: Returns true if this Minimum/Maximum Stay
  *                represents one year.
  *
  *   @return bool    - true, minimum/maximum stay is one year,
  *                     else false.
  */
  const bool isOneYear();

  /**
  *   @method unit
  *
  *   Description: Get/Set Minimum/Maximum Stay unit of time
  *
  *   @return int     - minimum/maximum stay unit of time
  */
  int& unit() { return _unit; }
  const int& unit() const { return _unit; }

  const std::string getPeriodOfStayAsString() const;

private:
  /**
  *   @method convertPeriodOfStay
  *
  *   Description: Converts the minimum/maximum stay from a 3 character
  *                number in the range of 001 - 999 to an integer
  *                otherwise it just returns original string.
  *
  *   @return void
  */
  void convertPeriodOfStay();

  /**
  *   @method convertUnit
  *
  *   Description: Checks the minimum stay unit is a string representation
  *                of a number between 01 - 52 or a character representing
  *                one of the following: N - Minutes, H - Hours, D - Days,
  *                or M - Months. If the return is false then use the char
  *                return parameter, else use the int return parameter.
  *
  *   @return void
  */
  void convertUnit();

  /**
  *   @method getDayOfWeek
  *
  *   Description: Returns the day of week as an integer
  *
  *   @param  string            - day of week
  *
  *   @return int               - day of week
  */
  int getDayOfWeek(const std::string& dayOfWeek);

  static const std::string FIRST_WEEK;
  static const std::string LAST_WEEK;

private:
  std::string _periodOfStay;
  int _iPeriodOfStay = -1;

  std::string _strUnit;
  int _unit = -1;

  bool _isValid = false;

  Weekdays _dayOfWeek = Weekdays::Sunday;
  bool _isDayOfWeek = false;
};

} // namespace tse

