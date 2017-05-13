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

#include <string>
#include <stdexcept>

#include "Rules/CopyableStream.h"

namespace tax
{
class RequestLogicError : public std::exception
{
public:
  RequestLogicError() {};
  virtual ~RequestLogicError() throw() {}

  template <typename T>
  RequestLogicError& operator<<(const T& w)
  {
    _out << w;
    return *this;
  }

  const char* what() const throw() override
  {
    _result = _out.str();
    return _result.c_str();
  }

private:
  CopyableStream _out;
  mutable std::string _result;
};
}
