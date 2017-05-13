
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

#include "Pricing/Shopping/Utils/BaseLogger.h"

namespace tse
{

namespace utils
{

// Does nothing
class DummyLogger : public BaseLogger
{
public:
  void message(LOGGER_LEVEL level, const std::string& msg) override
  {
  }

  bool enabled(LOGGER_LEVEL level) const override
  {
    return false;
  }
};

} // namespace utils

} // namespace tse

