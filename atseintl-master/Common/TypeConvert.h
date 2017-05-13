//----------------------------------------------------------------------------
//
//  File:        TypeConver.h
//  Created:     6/7/2004
//  Authors:     Mike Carroll
//
//  Description: Disparate system type conversion
//
//  Updates:
//
//  Copyright Sabre 2004
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
#pragma once

#include "Common/TseStringTypes.h"

namespace tse
{
class DateTime;

namespace TypeConvert
{
inline bool
pssCharToBool(char pssValue)
{
  return pssValue == 'Y' || pssValue == 'T';
}

inline bool
isBool(const std::string& value)
{
  if (value.size() != 1)
    return false;

  switch (value[0])
  {
  case 'T':
  case 'F':
  case 'Y':
  case 'N':
    return true;
  default:
    return false;
  }
}

std::string
valueToString(const DateTime& value);
std::string
valueToString(const uint8_t value);
std::string
valueToString(const uint16_t value);
std::string
valueToString(const uint32_t value);
std::string
valueToString(const uint64_t value);
std::string
valueToString(const int8_t value);
std::string
valueToString(const int16_t value);
std::string
valueToString(const int32_t value);
std::string
valueToString(const int64_t value);
std::string
valueToString(const bool value);
std::string
valueToString(const char value);
std::string
valueToString(const float value);
std::string
valueToString(const double value);

bool
stringToValue(const std::string& stringValue, DateTime& output);
bool
stringToValue(const std::string& stringValue, uint8_t& output);
bool
stringToValue(const std::string& stringValue, uint16_t& output);
bool
stringToValue(const std::string& stringValue, uint32_t& output);
bool
stringToValue(const std::string& stringValue, uint64_t& output);
bool
stringToValue(const std::string& stringValue, int8_t& output);
bool
stringToValue(const std::string& stringValue, int16_t& output);
bool
stringToValue(const std::string& stringValue, int32_t& output);
bool
stringToValue(const std::string& stringValue, int64_t& output);
bool
stringToValue(const std::string& stringValue, bool& output);
bool
stringToValue(const std::string& stringValue, char& output);
bool
stringToValue(const std::string& stringValue, float& output);
bool
stringToValue(const std::string& stringValue, double& output);

template <class T>
bool
stringToValue(const std::string& stringValue, T& output)
{
  output = stringValue;
  return true;
}

template <class T>
std::string
valueToString(const T& value)
{
  return std::string(value);
}

}
} // end tse namespace

