//-------------------------------------------------------------------
//
//  File:        XMLChString.h
//  Created:     August 03, 2004
//  Authors:     David White
//
//  Description: A wrapper for Xerces XMLCh* strings, that can
//               be initialized with either a const XMLCh* or
//               a const char*. Initializing with a const XMLCh*
//               doesn't require dynamic memory allocation, and
//               is thus fast.
//
//               This allows an XML document to be parsed without
//               having to perform a dynamic allocation every time
//               a string needs to be accessed and compared.
//
//  Copyright Sabre 2004
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

#include "Common/Code.h"
#include "Common/FallbackUtil.h"
#include "Common/TseStringTypes.h"

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace tse
{
FIXEDFALLBACK_DECL(xmlEmptyStringComparison)

class XMLChString
{
public:
  //'str' must remain valid for the life of the object
  explicit XMLChString(const XMLCh* str);
  XMLChString(const XMLChString& rhs);
  XMLChString& operator=(const XMLChString& rhs);
  ~XMLChString();

  const XMLCh* x_str() const { return _str; }
  const char* c_str() const;

  std::size_t size() const { return _end - _str; }
  bool empty() const { return size() == 0; }

  static char convertChar(XMLCh c) { return static_cast<char>(c); }

  // functor which can compare unicode characters with ascii characters
  struct charsEqual
  {
    bool operator()(char a, XMLCh b) const { return XMLCh(a) == b; }
    bool operator()(XMLCh b, char a) const { return XMLCh(a) == b; }
  };

  friend bool operator==(const XMLChString& lhs, const char* rhs);

  // template functions that parse types using different parsing methods
  template <typename T>
  bool parseChar(T& c) const
  {
    if (empty())
    {
      return false;
    }

    c = static_cast<T>(*_str);
    return true;
  }

  template <typename T>
  bool parseInt(T& n) const
  {
    if (empty())
    {
      return false;
    }

    n = T(atoi(c_str()));
    return true;
  }

  template <typename T>
  bool parseFloat(T& n) const
  {
    if (empty())
    {
      return false;
    }

    n = T(atof(c_str()));
    return true;
  }

  template <typename T>
  bool parseString(T& s) const
  {
    s.resize(size());
    std::transform(_str, _end, s.begin(), convertChar);
    return true;
  }

  bool parseLongLong(int64_t& n) const;
  bool parseULongLong(uint64_t& n) const;

  // parsing functions which parse various types in the most logical way
  bool parse(char& n) const { return parseChar(n); }
  bool parse(uint8_t& n) const { return parseChar(n); }
  bool parse(int8_t& n) const { return parseChar(n); }
  bool parse(uint16_t& n) const { return parseInt(n); }
  bool parse(int16_t& n) const { return parseInt(n); }
  bool parse(uint32_t& n) const { return parseInt(n); }
  bool parse(int32_t& n) const { return parseInt(n); }
  bool parse(uint64_t& n) const { return parseULongLong(n); }
  bool parse(int64_t& n) const { return parseLongLong(n); }
  bool parse(double& n) const { return parseFloat(n); }
  bool parse(float& n) const { return parseFloat(n); }
  bool parse(std::string& n) const { return parseString(n); }

  template <size_t n, typename T>
  bool parse(Code<n, T>& s) const
  {
    const XMLCh* end = _end;
    if (size_t(end - _str) > n)
    {
      end = _str + n;
    }
    std::transform(_str, end, s.begin(), convertChar);
    return true;
  }

  template <typename T>
  T get() const
  {
    T v = T();
    parse(v);
    return v;
  }

private:
  const XMLCh* _str;
  const XMLCh* _end;

  // the way we store the ascii representation of the string.
  // If the ascii representation hasn't been initialized yet, _cstr is 0.
  // When we create the ascii representation, it is placed in _buf if
  //_buf is large enough to hold it, and _cstr points to _buf. If _buf
  // isn't large enough, _cstr is made to point to a dynamically allocated
  // buffer, and will be deallocated in the class's constructor
  mutable char* _cstr;
  mutable char _buf[15];
};

inline bool operator==(const XMLChString& lhs, const char* rhs)
{
  return (fallback::fixed::xmlEmptyStringComparison() || lhs.size() == std::strlen(rhs)) &&
         std::equal(lhs._str, lhs._end, rhs, XMLChString::charsEqual());
}

inline bool operator==(const char* lhs, const XMLChString& rhs) { return operator==(rhs, lhs); }

inline bool operator!=(const XMLChString& lhs, const char* rhs) { return !operator==(lhs, rhs); }

inline bool operator!=(const char* lhs, const XMLChString& rhs) { return !operator==(lhs, rhs); }

// a class which wraps Xerces Attributes, and allows more convenient use
class XMLAttributes
{
public:
  XMLAttributes(const xercesc::Attributes& attr) : _attr(attr) {}

  XMLChString operator[](const char* str) const;

private:
  const xercesc::Attributes& _attr;
};
}

