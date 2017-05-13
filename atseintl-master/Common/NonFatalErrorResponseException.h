#pragma once

#include "Common/ErrorResponseException.h"

namespace tse
{

class NonFatalErrorResponseException : public ErrorResponseException
{
public:
  NonFatalErrorResponseException(ErrorResponseCode code, const char* msg = nullptr)
    : ErrorResponseException(code, msg)
  {
  }

  virtual ~NonFatalErrorResponseException() throw() {}
};
}

