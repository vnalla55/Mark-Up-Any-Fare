
//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include <boost/format.hpp>

#include <string>

namespace tse
{

namespace utils
{
enum class LOGGER_LEVEL : uint8_t
{ DEBUG = 0,
  INFO,
  ERROR };

// Bridge DP
class ILogger
{
public:
  virtual void message(LOGGER_LEVEL level, const std::string& msg) = 0;

  virtual void debug(const std::string& msg) = 0;
  virtual void debug(const boost::format& msg) = 0;

  virtual void info(const std::string& msg) = 0;
  virtual void info(const boost::format& msg) = 0;

  virtual void error(const std::string& msg) = 0;
  virtual void error(const boost::format& msg) = 0;

  virtual bool enabled(LOGGER_LEVEL level) const = 0;
  virtual ~ILogger() = default;
};

} // namespace utils

} // namespace tse

