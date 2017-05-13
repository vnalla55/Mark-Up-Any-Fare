#pragma once

#include "Common/Code.h"

#include <boost/container/string.hpp>

#include <string>

template <size_t n>
bool operator==(const Code<n>& lhs, const boost::container::string& rhs)
{
  return lhs == rhs.c_str();
}

inline bool operator==(const std::string& lhs, const boost::container::string& rhs)
{
  return lhs == rhs.c_str();
}

inline bool operator!=(const std::string& lhs, const boost::container::string& rhs)
{
  return lhs != rhs.c_str();
}

inline std::string operator+(const std::string& lhs, const boost::container::string& rhs)
{
  std::string str;
  str.reserve(lhs.size() + rhs.size());
  str.append(lhs);
  str.append(rhs.begin(), rhs.end());
  return str;
}

class BoostString : public boost::container::string
{
public:
  BoostString() {}

  BoostString(const char* str) : boost::container::string(str) {}

  BoostString(const char* str, size_t size) : boost::container::string(str, size) {}

  BoostString(boost::container::string str) : boost::container::string(std::move(str)) {}

  explicit BoostString(const std::string& str) : boost::container::string(str.data(), str.size()) {}

  template <size_t n>
  explicit BoostString(const Code<n>& str)
    : boost::container::string(str.data(), str.size())
  {
  }

  void swap(BoostString& str) { boost::container::string::swap(str); }

  explicit operator std::string() const { return std::string(data(), size()); }

  template <size_t n>
  explicit operator Code<n>() const
  {
    return Code<n>(data(), size());
  }

  BoostString& operator=(boost::container::string str)
  {
    assign(std::move(str));
    return *this;
  }

  BoostString& operator=(const char* str)
  {
    assign(str);
    return *this;
  }

  BoostString& operator=(const std::string& str)
  {
    assign(str.data(), str.size());
    return *this;
  }

  BoostString& operator=(char c)
  {
    assign(1, c);
    return *this;
  }

  template <size_t n>
  BoostString& operator=(const Code<n>& str)
  {
    assign(str.data(), str.size());
    return *this;
  }

  BoostString& operator+=(char ch)
  {
    append(1, ch);
    return *this;
  }

  BoostString& operator+=(const char* str)
  {
    append(str);
    return *this;
  }

  BoostString& operator+=(const std::string& str)
  {
    append(str.data(), str.size());
    return *this;
  }

  bool operator==(const char* rhs) const { return 0 == strcmp(c_str(), rhs); }

  bool operator==(const std::string& rhs) const { return c_str() == rhs; }

  bool operator!=(const char* rhs) const { return 0 != strcmp(c_str(), rhs); }

  bool operator!=(const std::string& rhs) const { return c_str() != rhs; }
};
