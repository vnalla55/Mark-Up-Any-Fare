#include "Common/XMLChString.h"

#include <boost/lexical_cast.hpp>

#include <algorithm>

namespace
{
const XMLCh emptyStr[] = { 0 };
inline const XMLCh*
makeNullEmpty(const XMLCh* str)
{
  if (str == nullptr)
  {
    return emptyStr;
  }
  else
  {
    return str;
  }
}
}

namespace tse
{

XMLChString::XMLChString(const XMLCh* str)
  : _str(makeNullEmpty(str)), _end(_str + xercesc::XMLString::stringLen(_str)), _cstr(nullptr)
{
}

XMLChString::XMLChString(const XMLChString& rhs) : _str(rhs._str), _end(rhs._end), _cstr(nullptr) {}

XMLChString&
XMLChString::
operator=(const XMLChString& rhs)
{
  if (&rhs == this)
    return *this;

  if (_cstr != _buf)
  {
    xercesc::XMLString::release(&_cstr);
  }

  _str = rhs._str;
  _end = rhs._end;
  _cstr = nullptr;

  return *this;
}

XMLChString::~XMLChString()
{
  if (_cstr != _buf)
  {
    xercesc::XMLString::release(&_cstr);
  }
}

const char*
XMLChString::c_str() const
{
  if (_cstr == nullptr)
  {
    // lint -e{574}
    const bool res =
        size() < sizeof(_buf) && xercesc::XMLString::transcode(x_str(), _buf, static_cast<uint32_t>(sizeof(_buf) - 1));
    if (res)
    {
      _cstr = _buf;
    }
    else
    {
      _cstr = xercesc::XMLString::transcode(x_str());
      if (_cstr == nullptr)
      {
        _buf[0] = 0;
        _cstr = _buf;
      }
    }
  }

  return _cstr;
}

bool
XMLChString::parseLongLong(int64_t& n) const
{
  if (empty())
  {
    return false;
  }

  try
  {
    n = boost::lexical_cast<int64_t>(c_str());
    return true;
  }
  catch (const boost::bad_lexical_cast&) {}

  return false;
}

bool
XMLChString::parseULongLong(uint64_t& n) const
{
  if (empty())
  {
    return false;
  }

  try
  {
    n = boost::lexical_cast<uint64_t>(c_str());
    return true;
  }
  catch (const boost::bad_lexical_cast&) {}

  return false;
}

XMLChString
XMLAttributes::
operator[](const char* str) const
{

  // we need to transcode 'str' to being an XMLCh*. The most
  // efficient way to do this is by allocating a buffer on the stack.
  // We choose a 'bufsize' for this stack-based buffer. If the string
  // is shorter than 'bufsize', we can use the stack-based buffer,
  // otherwise we resort to the slower method of allocating dynamically
  // via a vector
  const size_t bufsize = 64;
  const size_t len = strlen(str);
  if (len < bufsize)
  {
    XMLCh buf[bufsize];
    if (xercesc::XMLString::transcode(str, buf, bufsize - 1))
    {
      return XMLChString(_attr.getValue(buf));
    }
    else
    {
      return XMLChString(nullptr);
    }
  }
  else
  {
    std::vector<XMLCh> buf(len + 1);
    if (xercesc::XMLString::transcode(str, &buf[0], unsigned(len)))
    {
      return XMLChString(_attr.getValue(&buf[0]));
    }
    else
    {
      return XMLChString(nullptr);
    }
  }
}
}
