// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Common/DateTime.h"
#include "Common/TypeConvert.h"

#include <limits>
#include <string>
#include <type_traits>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

namespace tse
{
namespace
{
template <typename O>
bool
unsignedIntegralStringToValue(const std::string& stringValue, O& output)
{
  static_assert(std::is_unsigned<O>::value && std::is_integral<O>::value, "Invalid type");

  if (stringValue.empty())
    return false;

  char* endptr = nullptr;

  errno = 0;
  const unsigned long long value = strtoull(stringValue.c_str(), &endptr, 10);
  if (stringValue.find('-') != std::string::npos || errno ||
      value > std::numeric_limits<O>::max() || endptr != stringValue.c_str() + stringValue.size())
    return false;

  output = value;
  return true;
}

template <typename O>
bool
signedIntegralStringToValue(const std::string& stringValue, O& output)
{
  static_assert(std::is_signed<O>::value && std::is_integral<O>::value, "Invalid type");

  if (stringValue.empty())
    return false;

  char* endptr = nullptr;

  errno = 0;
  const long long value = strtoll(stringValue.c_str(), &endptr, 10);
  if (errno || value > std::numeric_limits<O>::max() || value < std::numeric_limits<O>::min() ||
      endptr != stringValue.c_str() + stringValue.size())
    return false;

  output = value;
  return true;
}

template <typename I>
std::string
unsignedIntegralValueToString(const I value)
{
  static_assert(std::is_unsigned<I>::value && std::is_integral<I>::value, "Invalid type");

  char buf[21];
  snprintf(buf, sizeof buf, "%lu", static_cast<unsigned long>(value));
  return std::string(buf);
}

template <typename I>
std::string
signedIntegralValueToString(const I value)
{
  static_assert(std::is_signed<I>::value && std::is_integral<I>::value, "Invalid type");

  char buf[21];
  snprintf(buf, sizeof buf, "%li", static_cast<long>(value));
  return std::string(buf);
}
}

namespace TypeConvert
{
std::string
valueToString(const DateTime& value)
{
  return value.dateToString(YYYYMMDD, "-");
}

bool
stringToValue(const std::string& stringValue, DateTime& output) try
{
  output = DateTime(stringValue, 0);
  return true;
}
catch (...)
{
  return false;
}

std::string
valueToString(const uint8_t value)
{
  return unsignedIntegralValueToString<uint8_t>(value);
}

bool
stringToValue(const std::string& stringValue, uint8_t& output)
{
  return unsignedIntegralStringToValue<uint8_t>(stringValue, output);
}

std::string
valueToString(const uint16_t value)
{
  return unsignedIntegralValueToString<uint16_t>(value);
}

bool
stringToValue(const std::string& stringValue, uint16_t& output)
{
  return unsignedIntegralStringToValue<uint16_t>(stringValue, output);
}

std::string
valueToString(const uint32_t value)
{
  return unsignedIntegralValueToString<uint32_t>(value);
}

bool
stringToValue(const std::string& stringValue, uint32_t& output)
{
  return unsignedIntegralStringToValue<uint32_t>(stringValue, output);
}

std::string
valueToString(const uint64_t value)
{
  return unsignedIntegralValueToString<uint64_t>(value);
}

bool
stringToValue(const std::string& stringValue, uint64_t& output)
{
  return unsignedIntegralStringToValue<uint64_t>(stringValue, output);
}

std::string
valueToString(const int8_t value)
{
  return signedIntegralValueToString<int8_t>(value);
}

bool
stringToValue(const std::string& stringValue, int8_t& output)
{
  return signedIntegralStringToValue<int8_t>(stringValue, output);
}

std::string
valueToString(const int16_t value)
{
  return signedIntegralValueToString<int16_t>(value);
}

bool
stringToValue(const std::string& stringValue, int16_t& output)
{
  return signedIntegralStringToValue<int16_t>(stringValue, output);
}

std::string
valueToString(const int32_t value)
{
  return signedIntegralValueToString<int32_t>(value);
}

bool
stringToValue(const std::string& stringValue, int32_t& output)
{
  return signedIntegralStringToValue<int32_t>(stringValue, output);
}

std::string
valueToString(const int64_t value)
{
  return signedIntegralValueToString<int64_t>(value);
}

bool
stringToValue(const std::string& stringValue, int64_t& output)
{
  return signedIntegralStringToValue<int64_t>(stringValue, output);
}

bool
stringToValue(const std::string& stringValue, bool& output)
{
  if (!isBool(stringValue))
    return false;

  output = pssCharToBool(stringValue[0]);
  return true;
}

std::string
valueToString(const bool value)
{
  return value ? "Y" : "N";
}

bool
stringToValue(const std::string& stringValue, char& output)
{
  if (stringValue.size() != 1)
    return false;
  output = stringValue[0];
  return true;
}

std::string
valueToString(const char value)
{
  return std::string(1, value);
}

std::string
valueToString(const float value)
{
  return std::to_string(value);
}

bool
stringToValue(const std::string& stringValue, float& output) try
{
  size_t end = -1;
  const float value = std::stof(stringValue.c_str(), &end);
  if (end != stringValue.size())
    return false;

  output = value;
  return true;
}
catch (...)
{
  return false;
}

std::string
valueToString(const double value)
{
  return std::to_string(value);
}

bool
stringToValue(const std::string& stringValue, double& output) try
{
  size_t end = -1;
  const double value = std::stod(stringValue.c_str(), &end);
  if (end != stringValue.size())
    return false;

  output = value;
  return true;
}
catch (...)
{
  return false;
}
}
}
