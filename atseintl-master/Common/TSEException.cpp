#include "Common/TSEException.h"

#include <map>

#include <cassert>

namespace tse
{
namespace
{

const std::map<TSEException::TSEExceptionCode, std::string>
createMessageMap()
{
  std::map<TSEException::TSEExceptionCode, std::string> msgMap;

  msgMap[TSEException::SITA_FIELD_REQUESTED_FROM_ATPCO_FARE] =
      "SITA FIELD REQUESTED FROM ATPCO FARE";
  msgMap[TSEException::UNKNOWN_TICKETING_CODE_MODIFIER] = "UNKNOWN TICKETING CODE MODIFIER";
  msgMap[TSEException::INVALID_FLIGHT_APPLICATION_DATA] = "INVALID FLIGHT APPLICATION DATA";

  return msgMap;
}

static const std::map<TSEException::TSEExceptionCode, std::string>
_msgMap(createMessageMap());
static const std::string
_emptyString("");
}

TSEException::TSEException(TSEExceptionCode code, const char* msg) : _code(code), _stackRawSize(0)
{
  if (msg != nullptr)
    _message = msg;
  else
  {
    std::map<TSEExceptionCode, std::string>::const_iterator it = _msgMap.find(code);

    if (it != _msgMap.end())
      _message = it->second;
    else
      _message = _emptyString;
  }

  _stackRawSize = StackUtil::getRawStackTrace(&_stackRaw.front());
}

TSEException&
TSEException::
operator=(const TSEException& rhs)
{
  _code = rhs._code;
  _message = rhs._message;
  _stackRaw = rhs._stackRaw;
  _stackRawSize = rhs._stackRawSize;
  _stack = rhs._stack;

  return *this;
}

void
TSEException::assertOrThrow(TSEExceptionCode code, const char* msg) throw()
{
  assert(false); // lint !e506

  throw TSEException(code, msg); // lint !e1549
}

const char*
TSEException::where() throw()
{
  if (_stack.empty())
    _stack = StackUtil::getStackTrace(&_stackRaw.front(), _stackRawSize);

  return _stack.c_str();
}
}
