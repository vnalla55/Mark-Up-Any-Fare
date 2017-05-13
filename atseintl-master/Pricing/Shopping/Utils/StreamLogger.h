
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

#include <iomanip>
#include <iostream>

namespace tse
{

namespace utils
{

class StreamLogger : public BaseLogger
{
public:
  StreamLogger(std::ostream* out = 0)
  {
    if (out)
    {
      _stream = out;
    }
    else
    {
      _stream = &std::cout;
    }
  }

  void message(LOGGER_LEVEL level, const std::string& msg) override
  {
    (*_stream) << std::setw(5) << level << ": " << msg << std::endl;
  }

  bool enabled(LOGGER_LEVEL level) const override
  {
    return true;
  }

private:
  std::ostream* _stream;
};

} // namespace utils

} // namespace tse

