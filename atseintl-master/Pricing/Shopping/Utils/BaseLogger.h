
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

#include "Pricing/Shopping/Utils/ILogger.h"

#include <iostream>

namespace tse
{

namespace utils
{

// Convenience class implementing
// some of interface methods in a common way
class BaseLogger : public ILogger
{
public:
  void message(LOGGER_LEVEL level, const std::string& msg) override = 0;
  bool enabled(LOGGER_LEVEL level) const override = 0;

  void debug(const std::string& msg) override
  {
    message(LOGGER_LEVEL::DEBUG, msg);
  }

  void info(const std::string& msg) override
  {
    message(LOGGER_LEVEL::INFO, msg);
  }

  void error(const std::string& msg) override
  {
    message(LOGGER_LEVEL::ERROR, msg);
  }

  void debug(const boost::format& msg) override
  {
    debug(msg.str());
  }

  void info(const boost::format& msg) override
  {
    info(msg.str());
  }

  void error(const boost::format& msg) override
  {
    error(msg.str());
  }
};

inline std::ostream& operator<<(std::ostream& out, LOGGER_LEVEL level)
{
  switch (level)
  {
  case LOGGER_LEVEL::DEBUG:
    out << "DEBUG";
    break;
  case LOGGER_LEVEL::INFO:
    out << "INFO";
    break;
  case LOGGER_LEVEL::ERROR:
    out << "ERROR";
    break;
  }
  return out;
}

} // namespace utils

} // namespace tse

