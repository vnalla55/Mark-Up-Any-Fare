// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "DataModel/Common/Types.h"
#include "Rules/MathUtils.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <iosfwd>
#include <type_traits>

namespace tax
{

namespace FromString
{
template <typename EnumT>
typename std::enable_if<
  std::is_enum<EnumT>::value && sizeof(EnumT) == sizeof(char),
  EnumT // <-- this is the function's return type
>::type
fromStringImpl(const std::string& string, EnumT*)
{
  if (BOOST_UNLIKELY(string.empty()))
    throw std::runtime_error("Empty string where tag expected.");

  char ch = string[0];
  if (BOOST_UNLIKELY(int(ch) >= 127))
    throw std::runtime_error("BAd character for tag.");

  unsigned char uch = static_cast<unsigned char>(ch);
  return static_cast<EnumT>(uch);
}

template <typename T>
typename std::enable_if<
  !std::is_enum<T>::value || !(sizeof(T) == sizeof(char)),
  T // <-- this is the function's return type
>::type
fromStringImpl(const std::string& string, T*)
{
  return boost::lexical_cast<T>(string);
}

inline
bool fromStringImpl(const std::string& string, bool*)
{
  bool result = false;
  try
  {
    result = boost::lexical_cast<bool>(string);
  }
  catch (std::exception const&)
  {
    if (!(string.length() == 1 && string[0] == ' '))
      throw;
  }
  return result;
}

inline
std::string fromStringImpl(const std::string& string, std::string*)
{
  if (string == " ")
    return "";

  return string;
}

inline
type::Date fromStringImpl(const std::string& string, type::Date*)
{
  if (string.empty() || string == " ")
  {
    return type::Date::blank_date();
  }

  if (string == "999999")
  {
    return type::Date::pos_infinity();
  }

  type::Date result;
  std::istringstream str(string);
  str >> result;

  return result;
}

inline
type::Time fromStringImpl(const std::string& string, type::Time*)
{
  type::Time result;
  std::istringstream str(string);
  str >> result;

  return result;
}

inline
boost::gregorian::date fromStringImpl(const std::string& string, boost::gregorian::date*)
{
  return boost::gregorian::from_string(string);
}

inline
boost::posix_time::time_duration fromStringImpl(const std::string& string,
                                                boost::posix_time::time_duration*)
{
  boost::posix_time::time_duration inputDuration(boost::posix_time::duration_from_string(string));
  boost::posix_time::time_duration::hour_type hours = inputDuration.hours();
  boost::posix_time::time_duration::min_type minutes = inputDuration.minutes();
  if (hours < 0 || hours >= 24)
  {
    throw std::out_of_range("Hour out of 0..23 range!");
  }
  if (minutes < 0 || minutes >= 60)
  {
    throw std::out_of_range("Minute out of 0..59 range!");
  }
  return boost::posix_time::time_duration(hours, minutes, 0, 0);
}

inline
int16_t fromStringImpl(const std::string& string, int16_t*)
{
  int16_t result = -1;
  std::istringstream str(string);
  str >> result;

  return result;
}

inline
type::MoneyAmount fromStringImpl(const std::string& string, type::MoneyAmount*)
{
  std::string str(string);
  int scale = 0;
  std::size_t dotPos = str.find('.');
  if (dotPos != std::string::npos)
  {
    scale = int(str.size() - dotPos) - 1;
    str.erase(dotPos, std::size_t(1));
    if (str.find('.') != std::string::npos)
      throw XmlParsingError("Invalid money amount format!");
  }
  type::MoneyAmount result(boost::lexical_cast<int64_t>(str));
  return MathUtils::adjustDecimal(result, scale);
}

template <typename T>
T fromString(std::string const& string)
{
  return fromStringImpl(string, static_cast<T*>(0));
}

}
}
