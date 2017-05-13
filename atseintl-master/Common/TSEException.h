#pragma once

#include "Util/StackUtil.h"

#include <boost/array.hpp>

#include <exception>
#include <string>

namespace tse
{

class TSEException : public std::exception
{
public:
  // public types
  // ====== =====

  enum TSEExceptionCode
  { NO_ERROR = 0000,
    SITA_FIELD_REQUESTED_FROM_ATPCO_FARE = 1001,
    UNKNOWN_TICKETING_CODE_MODIFIER = 1002,
    INVALID_FLIGHT_APPLICATION_DATA = 1003,
    UNKNOWN_DATABASE = 1004 };

  // construction/destruction & assigment
  // ======================== = =========

  TSEException(TSEExceptionCode code, const char* msg = nullptr);
  TSEException(const TSEException& rhs) : std::exception(), _code(rhs._code),
    _message(rhs._message), _stackRawSize(0)
  {
  }

  virtual ~TSEException() throw() {};

  TSEException& operator=(const TSEException& rhs);

  // main interface
  // ==== =========

  static void assertOrThrow(TSEExceptionCode code, const char* msg = nullptr) throw();

  const char* what() const throw() override { return _message.c_str(); }
  const char* where() throw();

private:
  TSEExceptionCode _code;
  std::string _message;
  boost::array<void*, StackUtil::DEFAULT_MAX_DEPTH> _stackRaw;
  int _stackRawSize;
  std::string _stack;
};
}

