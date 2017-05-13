
//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2014
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

#include "Pricing/Shopping/FiltersAndPipes/IFilterObserver.h"
#include "Pricing/Shopping/Utils/ILogger.h"

#include <sstream>
#include <string>

namespace tse
{
namespace utils
{
// This class grabs elements failed by a filter
// that it observes and logs info about them
// using the given logger.
template <typename T>
class BadElementLogger : public IFilterObserver<T>
{
public:
  BadElementLogger(ILogger& log,
                   LOGGER_LEVEL loggingLevel,
                   const std::string& elementMetaname = "Element")
    : _log(log), _loggingLevel(loggingLevel), _elementMetaname(elementMetaname)
  {
  }

  // Here comes info about a bad element.
  void elementInvalid(const T& t, const INamedPredicate<T>& failedPredicate) override
  {
    if (_log.enabled(_loggingLevel))
    {
      std::ostringstream out;
      out << _elementMetaname << " " << t << " failed. Reason: " << failedPredicate.getName();
      _log.message(_loggingLevel, out.str());
    }
  }

private:
  ILogger& _log;
  LOGGER_LEVEL _loggingLevel;
  std::string _elementMetaname;
};

} // namespace utils

} // namespace tse

