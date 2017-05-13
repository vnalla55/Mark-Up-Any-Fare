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

#include <deque>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <stdint.h>

namespace tse
{

namespace utils
{

// Print any vector
template <typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v)
{
  const size_t sz = v.size();
  out << "[";
  for (size_t i = 0; i < sz; ++i)
  {
    out << v[i];
    if (i != (sz - 1))
    {
      out << ", ";
    }
  }
  out << "]";
  return out;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const std::deque<T>& v)
{
  const size_t sz = v.size();
  out << "[";
  for (size_t i = 0; i < sz; ++i)
  {
    out << v[i];
    if (i != (sz - 1))
    {
      out << ", ";
    }
  }
  out << "]";
  return out;
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream& out, const std::map<K, V>& m)
{
  const size_t sz = m.size();
  size_t i = 0;
  out << "{";
  for (const auto& elem : m)
  {
    out << elem.first << ": " << elem.second;
    if (i != (sz - 1))
    {
      out << ", ";
    }
    ++i;
  }
  out << "}";
  return out;
}


template <typename T>
std::string
toStr(const T& t)
{
  std::ostringstream out;
  out << t;
  return out.str();
}

inline std::string
toStr(bool b)
{
  return b ? "true" : "false";
}

class BlockMaker
{
public:
  BlockMaker(unsigned int minWidth = 0, bool leftAlignment = true)
    : _minWidth(minWidth), _leftAlignment(leftAlignment)
  {
  }

  template <typename T>
  std::string operator()(const T& t)
  {
    // Build another intermediate stream
    // to tokenize t as a whole and then
    // insert it with left-right etc. manipulators
    // to the final stream.
    std::ostringstream token;
    token << t;

    std::ostringstream out;
    if (_leftAlignment)
    {
      out << std::left;
    }
    else
    {
      out << std::right;
    }

    if (_minWidth > 0)
    {
      out << std::setw(static_cast<int32_t>(_minWidth)) << token.str();
    }
    else
    {
      out << token.str();
    }
    return out.str();
  }

private:
  unsigned int _minWidth;
  bool _leftAlignment;
};

} // namespace utils

} // namespace tse

