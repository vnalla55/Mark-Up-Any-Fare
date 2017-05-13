//----------------------------------------------------------------------------
//
//     File:           ORACLEBoundParameterTypes.cpp
//     Description:
//     Created:        11/17/2009
//     Authors:        Andrew Ahmad
//
//     Updates:
//
//     Copyright @2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited.
//
//----------------------------------------------------------------------------

#include "DBAccess/ORACLEBoundParameterTypes.h"

#include "DBAccess/ParameterBinder.h"

namespace DBAccess
{
void
ORACLEBoundInteger::bind(const ParameterBinder& binder)
{
  binder.bindParameter(this);
}

void
ORACLEBoundLong::bind(const ParameterBinder& binder)
{
  binder.bindParameter(this);
}

void
ORACLEBoundString::trim(const char* value)
{
  if (!value || !*value)
  {
    _size = 2;
    _value = new char[_size];
    _value[0] = ' ';
    _value[_size - 1] = 0;
  }
  else
  {
    /*
    ** If in the future the application wants to ignore leading spaces, then...
        // shift past leading.
        while ( *value && *value == ' ' )
        {
          value++  ;
        }
    */
    size_t length(strlen(value));
    // will only copy from first non blank to last non-blank character.
    // reduce by trailing
    while (length && value[length - 1] == ' ')
      length--;
    // length is pointing at the last space or null.
    if (length == 0)
    {
      _size = 2;
      _value = new char[_size];
      // oracle spaces for nulls requires a single space for empty strings
      _value[0] = ' ';
      _value[1] = 0;
    }
    else
    {
      _size = length + 1;
      _value = new char[_size];
      memcpy(_value, value, length);
      _value[length] = 0;
    }
  }
}

ORACLEBoundString::ORACLEBoundString(int32_t index, size_t position, const std::string& value)
  : BoundParameter(index, position)
{
  trim(value.c_str());
}

ORACLEBoundString::ORACLEBoundString(int32_t index, size_t position, const char* value)
  : BoundParameter(index, position)
{
  trim(value);
}

void
ORACLEBoundString::bind(const ParameterBinder& binder)
{
  binder.bindParameter(this);
}

ORACLEBoundDate::ORACLEBoundDate(int32_t index, size_t position, const tse::DateTime& value)
  : BoundParameter(index, position)
{
  uint8_t century(0), year(0), month(0), day(0), hour(0), min(0), sec(0);

  if (UNLIKELY(value.isInfinity()))
  {
    century = 199;
    year = 199;
    month = 12;
    day = 31;

    // hour, min, sec are in excess-one notation
    //
    hour = 24; // 1 + 23
    min = 60; // 1 + 59
    sec = 60; // 1 + 59
  }
  else if (UNLIKELY(value.isNegInfinity()))
  {
    century = 53;
    year = 88;
    month = 1;
    day = 1;
  }
  else
  {
    century = (value.year() / 100) + 100;
    year = (value.year() % 100) + 100;
    month = value.month();
    day = value.day();

    // hour, min, sec are in excess-one notation
    //
    /*
    hour    = value.hours() + 1;
    min     = value.minutes() + 1;
    sec     = value.seconds() + 1;
    */

    // When binding date-only values, set the hour, min and sec to zero.
    // In excess-one notation, 0 is actually 1 (0 + 1 = 1)
    //
    hour = 1;
    min = 1;
    sec = 1;
  }
  _value[0] = century;
  _value[1] = year;
  _value[2] = month;
  _value[3] = day;
  _value[4] = hour;
  _value[5] = min;
  _value[6] = sec;
}

void
ORACLEBoundDate::bind(const ParameterBinder& binder)
{
  binder.bindParameter(this);
}

void
ORACLEBoundDateTime::bind(const ParameterBinder& binder)
{
  binder.bindParameter(this);
}
}
