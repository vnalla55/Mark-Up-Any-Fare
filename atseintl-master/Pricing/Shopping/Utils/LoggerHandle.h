
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

#include "Common/Assert.h"
#include "Pricing/Shopping/Utils/DummyLogger.h"
#include "Pricing/Shopping/Utils/ILogger.h"

#include <boost/utility.hpp>

#include <memory>

namespace tse
{

namespace utils
{

typedef boost::format Fmt;

// By default, points to a dummy logger
// that does nothing. User can install
// any concrete logger in the handle, using
// the method install
class LoggerHandle : boost::noncopyable
{
public:
  LoggerHandle() { clear(); }

  // Installs a user-provided logger
  // Warning: Logger must be created using operator new
  void install(ILogger* logger)
  {
    TSE_ASSERT(logger != nullptr);
    _logger.reset(logger);
  }

  // Removes the current logger and installs
  // a dummy logger
  void clear() { _logger.reset(new DummyLogger()); }

  ILogger& operator*() const { return *_logger; }

  ILogger* operator->() const { return _logger.operator->(); }

  ILogger* get() const { return _logger.get(); }

private:
  std::unique_ptr<ILogger> _logger;
};
} // namespace utils
} // namespace tse
