//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/Hasher.h"
#include <boost/container/string.hpp>
#include <iosfwd>
#include <string>
#include <type_traits>
#include <endian.h>

namespace
{
  inline bool cmpIntAsString(uint8_t a, uint8_t b)
  {
    return a < b;
  }
  inline bool cmpIntAsString(uint16_t a, uint16_t b)
  {
    return __builtin_bswap16(a) < __builtin_bswap16(b);
  }

  inline bool cmpIntAsString(uint32_t a, uint32_t b)
  {
    return __builtin_bswap32(a) < __builtin_bswap32(b);
  }

  inline bool cmpIntAsString(uint64_t a, uint64_t b)
  {
    return __builtin_bswap64(a) < __builtin_bswap64(b);
  }
}

namespace tse
{

}// tse

/** n-sized array of type T */
template <size_t n, typename T>
struct TArray
{
  T t;
  TArray<n - 1, T> u;

public:
  /** "constructor" */
  inline void init() throw()
  {
    t = 0;
    u.init();
  }

  /** "assignment" */
  inline const TArray& set(const TArray& rhs) throw()
  {
    t = rhs.t;
    u = rhs.u;
    return *this;
  }

  void hashCombine(size_t& hash) const throw()
  {
    boost::hash_combine(hash, t);
    u.hashCombine(hash);
  }

  inline bool operator==(const TArray& rhs) const throw() { return t == rhs.t && u == rhs.u; }

  inline bool operator==(const T* rhs) const throw() { return t == *rhs && u == (rhs + 1); }

#if __BYTE_ORDER == __BIG_ENDIAN

  inline bool operator<(const T* rhs) const throw()
  {
    return t < *rhs || (t == *rhs && u < (rhs + 1));
  }

  inline bool operator>(const T* rhs) const throw()
  {
    return t > *rhs || (t == *rhs && u > (rhs + 1));
  }

#elif __BYTE_ORDER == __LITTLE_ENDIAN
  inline bool operator<(const TArray& rhs) const noexcept
  {
    return (t != rhs.t) ?
        cmpIntAsString(t, rhs.t) :
        u < rhs.u;
  }
  inline bool operator>(const TArray& rhs) const noexcept
  {
    return (t != rhs.t) ?
        cmpIntAsString(rhs.t, t) :
        u > rhs.u;
  }
#endif
};

/** array of type T with 1 element */
template <typename T>
struct TArray<0, T>
{

  T t;

public:
  /** "constructor" */
  inline void init() throw() { t = 0; }

  /** "assignment" */
  inline const TArray& set(const TArray& rhs) throw()
  {
    t = rhs.t;
    return *this;
  }

  void hashCombine(size_t& hash) const throw()
  {
    boost::hash_combine(hash, t);
  }

  inline bool operator==(const TArray& rhs) const throw() { return t == rhs.t; }

  inline bool operator==(const T* rhs) const throw() { return t == *rhs; }

#if __BYTE_ORDER == __BIG_ENDIAN

  inline bool operator<(const T* rhs) const throw() { return t < *rhs; }

  inline bool operator>(const T* rhs) const throw() { return t > *rhs; }
#elif __BYTE_ORDER == __LITTLE_ENDIAN
  inline bool operator<(const TArray& rhs) const noexcept
  {
    return cmpIntAsString(t, rhs.t);
  }
  inline bool operator>(const TArray& rhs) const noexcept
  {
    return cmpIntAsString(rhs.t, t);
  }
#endif
};

namespace boost
{
namespace serialization
{
}
}

/** replacement for short, fixed sized strings */
template <size_t n, typename T = unsigned int>
class Code
{
static_assert(std::is_unsigned<T>::value, "In Code<n,T> T must be an unsigned integer type.");
public:
  // Value returned by various member functions when they fail.
  static const size_t npos = static_cast<size_t>(-1);

  // lifecycle - constructors/destructors

  Code() { ta.init(); }

  template <size_t m, typename U>
  Code(const Code<m, U>& rhs)
  {
    // assert(rhs.length() <= n);
    ta.init();
    *this = rhs;
  }

  Code(const std::string& s)
  {
    // assert(s.length() <= n);
    ta.init();
    *this = s;
  }

  Code(const char* s)
  {
    // assert(strlen(s) <= n);
    ta.init();
    *this = s;
  }

  Code(const char* s, size_t sz)
  {
    // assert(sz <= n);
    assign(s, sz);
  }

  explicit Code(const char c)
  {
    *this = c;
  }

  // operators

  operator std::string() const throw() { return std::string(buf); }

  Code& operator=(const Code& rhs) throw()
  {
    ta.set(rhs.ta);
    return *this;
  }

  template <size_t m, typename U>
  Code& operator=(const Code<m, U>& rhs) throw()
  {
    // assert(rhs.length() <= n);
    safeCopy(0, rhs.c_str());
    return *this;
  }

  Code& operator=(const std::string& s) throw()
  {
    // assert(s.size() <= n);
    safeCopy(0, s.c_str(), s.size());
    return *this;
  }

  Code& operator=(const boost::container::string& s) throw()
  {
    // assert(s.size() <= n);
    safeCopy(0, s.c_str(), s.size());
    return *this;
  }

  Code& operator=(const char* s) throw()
  {
    // assert(strlen(s) <= n);
    safeCopy(0, s);
    return *this;
  }

  Code& operator=(const char c) throw()
  {
    ta.init();
    *buf = c;
    return *this;
  }

  char& operator[](size_t i) throw() { return *(buf + i); }

  const char& operator[](size_t i) const throw() { return *(buf + i); }

  bool operator==(const Code& rhs) const throw() { return ta == rhs.ta; }

  template <size_t m, typename U>
  bool operator==(const Code<m, U>& rhs) const throw()
  {
    return *this == rhs.c_str();
  }

#if __BYTE_ORDER == __BIG_ENDIAN

  template <size_t m>
  bool operator<(const Code<m, T>& rhs) const throw()
  {
    return ta < (const T*)&rhs;
  }

  template <size_t m>
  bool operator>(const Code<m, T>& rhs) const throw()
  {
    return ta > (const T*)&rhs;
  }

#elif __BYTE_ORDER == __LITTLE_ENDIAN
#ifndef DISABLE_CODE_OPT
  bool operator<(const Code& rhs) const throw()
  {
    return ta < rhs.ta;
  }
  bool operator>(const Code& rhs) const throw()
  {
    return ta > rhs.ta;
  }
#endif
#endif // __BYTE_ORDER == __BYTE_ORDER or __LITTLE_ENDIAN

  template <size_t m, typename U>
  bool operator<(const Code<m, U>& rhs) const throw()
  {
    return *this < rhs.c_str();
  }

  template <size_t m, typename U>
  bool operator>(const Code<m, U>& rhs) const throw()
  {
    return *this > rhs.c_str();
  }

  bool operator!=(const Code& rhs) const throw() { return !(*this == rhs); }

  template <size_t m, typename U>
  bool operator!=(const Code<m, U>& rhs) const throw()
  {
    return !(*this == rhs);
  }

  template <size_t m, typename U>
  bool operator<=(const Code<m, U>& rhs) const throw()
  {
    return !(*this > rhs);
  }

  template <size_t m, typename U>
  bool operator>=(const Code<m, U>& rhs) const throw()
  {
    return !(*this < rhs);
  }

  bool operator==(const char* rhs) const throw()
  {
    // memcmp will not work here because of chars after the '\0'
    return strncmp(buf, rhs, n + 1) == 0;
  }

  bool operator==(const char rhs) const throw()
  {
    if (n == 1 || strlen(buf) == 1)
      return (buf[0] == rhs);

    return false;
  }

#if __BYTE_ORDER == __BIG_ENDIAN

  bool operator<(const char* rhs) const throw() { return ta < (const T*)rhs; }

  bool operator>(const char* rhs) const throw() { return ta > (const T*)rhs; }

#elif __BYTE_ORDER == __LITTLE_ENDIAN

  bool operator<(const char* rhs) const throw() { return memcmp(buf, rhs, n + 1) < 0; }

  bool operator>(const char* rhs) const throw() { return memcmp(buf, rhs, n + 1) > 0; }

#endif // __BYTE_ORDER

  bool operator!=(const char* rhs) const throw() { return !(*this == rhs); }

  bool operator<=(const char* rhs) const throw() { return !(*this > rhs); }

  bool operator>=(const char* rhs) const throw() { return !(*this < rhs); }

  bool operator==(const std::string& rhs) const throw() { return *this == rhs.c_str(); }

  bool operator==(const boost::container::string& rhs) const throw()
  {
    return *this == rhs.c_str();
  }

#if __BYTE_ORDER == __BIG_ENDIAN

  bool operator<(const std::string& rhs) const throw() { return ta < (const T*)rhs.c_str(); }

  bool operator>(const std::string& rhs) const throw() { return ta > (const T*)rhs.c_str(); }

  bool operator<(const boost::container::string& rhs) const throw()
  {
    return ta < (const T*)rhs.c_str();
  }

  bool operator>(const boost::container::string& rhs) const throw()
  {
    return ta > (const T*)rhs.c_str();
  }

#elif __BYTE_ORDER == __LITTLE_ENDIAN

  bool operator<(const std::string& rhs) const throw() { return *this < rhs.c_str(); }

  bool operator>(const std::string& rhs) const throw() { return *this > rhs.c_str(); }

  bool operator<(const boost::container::string& rhs) const throw() { return *this < rhs.c_str(); }

  bool operator>(const boost::container::string& rhs) const throw() { return *this > rhs.c_str(); }

#endif // __BYTE_ORDER

  bool operator!=(const std::string& rhs) const throw() { return !(*this == rhs.c_str()); }

  bool operator<=(const std::string& rhs) const throw() { return !(*this > rhs.c_str()); }

  bool operator>=(const std::string& rhs) const throw() { return !(*this < rhs.c_str()); }

  bool operator!=(const boost::container::string& rhs) const throw()
  {
    return !(*this == rhs.c_str());
  }

  bool operator<=(const boost::container::string& rhs) const throw()
  {
    return !(*this > rhs.c_str());
  }

  bool operator>=(const boost::container::string& rhs) const throw()
  {
    return !(*this < rhs.c_str());
  }

  Code& operator+=(const std::string& s) throw()
  {
    safeCopy(size(), s.c_str(), s.size());
    return *this;
  }

  Code& operator+=(const char* s) throw()
  {
    safeCopy(size(), s);
    return *this;
  }

  Code& operator+=(char c) throw()
  {
    safeCopy(size(), &c, 1);
    return *this;
  }

  // stl-style methods / definitions

  typedef char* iterator;
  typedef const char* const_iterator;

  iterator begin() throw() { return buf; }

  const_iterator begin() const throw() { return buf; }

  iterator end() throw()
  {
    return buf + size();
  }

  const_iterator end() const throw()
  {
    return buf + size();
  }

  void push_back(char c) throw()
  {
    // assert(strlen(buf) < n);
    *this += c;
  }

  // replacements for string methods

  const char* c_str() const throw() { return buf; }

  void assign(const char* s, size_t sz)
  {
    // assert(sz <= n);
    ta.init();
    safeCopy(0, s, sz);
  }

  const char* data() const throw() { return buf; }

  size_t size() const throw() { return strlen(buf); }

  size_t length() const throw() { return size(); }

  bool empty() const throw() { return ta.t == 0; }

  void clear() throw() { ta.init(); }

  std::string substr(size_t pos, size_t len) const throw()
  {
    static const std::string empty;

    const size_t sz = size();
    if (UNLIKELY(pos >= sz))
      return empty;

    return std::string(buf + pos, std::min(sz - pos, len));
  }

  template <size_t m, typename U>
  size_t find(const Code<m, U>& s, size_t pos = 0) const throw()
  {
    return find(s.c_str(), pos);
  }

  size_t find(const std::string& s, size_t pos = 0) const throw() { return find(s.c_str(), pos); }

  size_t find(const char* s, size_t pos = 0) const throw()
  {
    const char* p = strstr(buf + pos, s);
    if (p == nullptr)
      return npos;
    else
      return p - buf;
  }

  size_t find(const char c, size_t pos = 0) const throw()
  {
    const char* p = strchr(buf + pos, c);
    if (p == nullptr)
      return npos;
    else
      return p - buf;
  }

  int compare(const Code& s) const throw() { return memcmp(buf, s.buf, n); }

  bool equalToConst(const char *c)const noexcept
  {
#ifndef DISABLE_CODE_OPT
    static_assert(n <= sizeof(T),"Code::equalToConst(const char *c) can't be used when Code<n,T> contains more than one word of type T.");
    return ta.t == toInt(c);
#else
    return *this == c;
#endif
  }

  // prepends elements to an existing array
  Code& prepend(const char* s) throw()
  {
    const size_t len_buf = size();
    const size_t len_s = strlen(s);

    if (len_buf + len_s <= n)
    {
      memmove(buf + len_s, buf, len_buf);
      memcpy(buf, s, len_s);
    }
    return *this;
  }

  void hash_combine(size_t& hash) const
  {
    ta.hashCombine(hash);
  }

  bool operator==(T t) const noexcept
  {
    static_assert(n <= sizeof(T),"Code::operator==(T) can't be used when Code<n,T> contains more than one word of type T.");
    return ta.t == t;
  }

  bool operator!=(T t) const noexcept
  {
    static_assert(n <= sizeof(T),"Code::operator!=(T) can't be used when Code<n,T> contains more than one word of type T.");
    return ! (*this == t);
  }
  static constexpr T toInt(const char *c) noexcept
  {
    static_assert(n <= sizeof(T),"Don't convert to T when Code<n,T> contains more than one word of type T.");
    return *c ? *c + (toInt(c + 1) << 8) : 0;
  }

private:
  static const size_t tmax = (n + sizeof(T)) / sizeof(T);

  union
  {
    char buf[n + 1];
    TArray<tmax - 1, T> ta;
  };

  void safeCopy(const size_t offset, const char* string)
  {
    safeCopy(offset, string, strlen(string));
  }

  void safeCopy(const size_t offset, const char* string, const size_t size)
  {
    const size_t limit = std::min(offset + size, n);
    memcpy(buf + offset, string, limit - offset);
    memset(buf + limit, 0, sizeof *this - limit);
  }

protected:
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version);
};

template <size_t n, typename T>
size_t hash_value(const Code<n, T> &code)
{
  boost::hash<std::string> hasher;
  return hasher(code);
}

template <size_t n, typename T>
std::ostream& operator<<(std::ostream& os, const Code<n, T>& code) throw()
{
  return (os << code.c_str());
}

template <size_t m, size_t n, typename T, typename U>
std::string operator+(const Code<m, T>& lhs, const Code<n, U>& rhs) throw()
{
  return std::string(lhs.c_str()).append(rhs.c_str());
}

template <size_t n, typename T>
std::string operator+(const char lhs, const Code<n, T>& rhs) throw()
{
  return lhs + std::string(rhs.c_str());
}

template <size_t n, typename T>
std::string operator+(const Code<n, T>& lhs, const char rhs) throw()
{
  return std::string(lhs.c_str()) + rhs;
}

template <size_t n, typename T>
std::string operator+(const std::string& lhs, const Code<n, T>& rhs) throw()
{
  return lhs + rhs.c_str();
}

template <size_t n, typename T>
std::string operator+(const Code<n, T>& lhs, const std::string& rhs) throw()
{
  return lhs.c_str() + rhs;
}

template <size_t n, typename T>
bool operator==(const std::string& lhs, const Code<n, T>& rhs) throw()
{
  return lhs == rhs.c_str();
}

template <size_t n, typename T>
bool operator!=(const std::string& lhs, const Code<n, T>& rhs) throw()
{
  return !(lhs == rhs.c_str());
}

template <size_t n, typename T>
tse::Hasher& operator<<(tse::Hasher& hasher, const Code<n, T>& s)
{
  return hasher << s.c_str();
}

namespace std
{
template <size_t n, typename T>
struct hash<Code<n, T>>
{
  size_t operator()(const Code<n, T>& code) const { return std::hash<std::string>()(code); }
};
}
