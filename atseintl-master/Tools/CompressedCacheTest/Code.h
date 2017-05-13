//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#ifndef CODE_H
#define CODE_H

#include <cstring>
#include <iosfwd>
#include <iostream>
#include <string>
#ifndef WIN32
#include <endian.h>
#endif

// extern "C"
// {
//     void handleTermiante();
// }
/** n-sized array of type T */
template<size_t n, typename T>
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

  inline bool operator== (const TArray& rhs) const throw()
  {
    return t == rhs.t && u == rhs.u;
  }

  inline bool operator== (const T* rhs) const throw()
  {
    return t == *rhs && u == (rhs + 1);
  }

  #if __BYTE_ORDER == __BIG_ENDIAN

  inline bool operator< (const T* rhs) const throw()
  {
    return t < *rhs || (t == *rhs && u < (rhs + 1));
  }

  inline bool operator> (const T* rhs) const throw()
  {
    return t > *rhs || (t == *rhs && u > (rhs + 1));
  }

  #endif // __BYTE_ORDER == __BIG_ENDIAN

};

/** array of type T with 1 element */
template<typename T>
struct TArray<0, T>
{ //lint !e407

  T t;

  public:

  /** "constructor" */
  inline void init() throw()
  {
    t = 0;
  }

  /** "assignment" */
  inline const TArray& set(const TArray& rhs) throw()
  {
    t = rhs.t;
    return *this;
  }

  inline bool operator== (const TArray& rhs) const throw()
  {
    return t == rhs.t;
  }

  inline bool operator== (const T* rhs) const throw()
  {
    return t == *rhs;
  }

  #if __BYTE_ORDER == __BIG_ENDIAN

  inline bool operator< (const T* rhs) const throw()
  {
    return t < *rhs;
  }

  inline bool operator> (const T* rhs) const throw()
  {
    return t > *rhs;
  }

  #endif // __BYTE_ORDER == __BIG_ENDIAN

};

/** replacement for short, fixed sized strings */
template<size_t n, typename T = unsigned int>
class Code
{
public:

// lifecycle - constructors/destructors

  Code()
  {
    ta.init();
  }

  Code(const Code& rhs)
  {
    ta = rhs.ta;
  }

  template<size_t m, typename U>
  Code(const Code<m, U>& rhs)
  {
    // if (rhs.length() > n)
    // throw new std::length_error("Code too long")
    ta.init();
    strncpy(buf, rhs.c_str(), n);
  }

  Code(const std::string& s)
  {
    // if (s.length() > n)
    // throw new std::length_error("Code too long")
    ta.init();
    strncpy(buf, s.c_str(), n);
  }

  Code(const char* s)
  {
    // if (strlen(s) > n)
    // throw new std::length_error("Code too long")
    ta.init();
    strncpy(buf, s, n);
  }

  Code(const char* s,size_t sz)
  {
    // if (sz > n)
    // throw new std::length_error("Code too long")
    ta.init();
    // s is not always '\0' terminated, just n as a third parameter will not do
    strncpy(buf, s, sz > n ? n : sz);
  }

  Code(const char c)
  {
    ta.init();
    *buf = c;
  }

// operators

  operator std::string() const throw()
  {
    return std::string(buf);
  }

  Code& operator= (const Code& rhs) throw()
  {
    ta.set(rhs.ta);
    return *this;
  }

  template<size_t m, typename U>
  Code& operator= (const Code<m, U>& rhs) throw()
  {
    // if (rhs.length() > n)
    // throw new std::length_error("Code too long")
    strncpy(buf, rhs.c_str(), sizeof(ta));
    return *this;
  }

  Code& operator= (const std::string& s) throw()
  {
    // if (s.size() > n)
    // throw new std::length_error("Code too long")
    strncpy(buf, s.c_str(), sizeof(ta));
    return *this;
  }

  Code& operator= (const char* s) throw()
  {
    // if (strlen(s) > n)
    // throw new std::length_error("Code too long")
    //strncpy(buf, s, sizeof(ta));
    strcpy(buf, s);
    return *this;
  }

  Code& operator= (const char c) throw()
  {
    ta.init();
    *buf = c;
    return *this;
  }

  char& operator[](int i) throw()
  {
    return *(buf + i);
  }

  const char& operator[](int i) const throw()
  {
    return *(buf + i);
  }

  bool operator== (const Code& rhs) const throw()
  {
    return ta == (const T*) &rhs;
  }

  template<size_t m, typename U>
  bool operator== (const Code<m, U>& rhs) const throw()
  {
    return *this == rhs.c_str();
  }

  #if __BYTE_ORDER == __BIG_ENDIAN

  template<size_t m>
  bool operator< (const Code<m, T>& rhs) const throw()
  {
    return ta < (const T*) &rhs;
  }

  template<size_t m>
  bool operator> (const Code<m, T>& rhs) const throw()
  {
    return ta > (const T*) &rhs;
  }

  #endif // __BYTE_ORDER

  template<size_t m, typename U>
  bool operator< (const Code<m, U>& rhs) const throw()
  {
    return *this < rhs.c_str();
  }

  template<size_t m, typename U>
  bool operator> (const Code<m, U>& rhs) const throw()
  {
    return *this > rhs.c_str();
  }


  bool operator!= (const Code& rhs) const throw()
  {
    return !(*this == rhs);
  }

  template<size_t m, typename U>
  bool operator!= (const Code<m, U>& rhs) const throw()
  {
    return !(*this == rhs);
  }

  template<size_t m, typename U>
  bool operator<= (const Code<m, U>& rhs) const throw()
  {
    return !(*this > rhs);
  }

  template<size_t m, typename U>
  bool operator>= (const Code<m, U>& rhs) const throw()
  {
    return !(*this < rhs);
  }

  bool operator== (const char* rhs) const throw()
  {
    // memcmp will not work here because of chars after the '\0'
    return strncmp(buf, rhs, n+1) == 0;
  }

  #if __BYTE_ORDER == __BIG_ENDIAN

  bool operator< (const char* rhs) const throw()
  {
    return ta < (const T*) rhs;
  }

  bool operator> (const char* rhs) const throw()
  {
    return ta > (const T*) rhs;
  }

  #elif __BYTE_ORDER == __LITTLE_ENDIAN

  bool operator< (const char* rhs) const throw()
  {
    return memcmp(buf, rhs, n+1) < 0;
  }

  bool operator> (const char* rhs) const throw()
  {
    return memcmp(buf, rhs, n+1) > 0;
  }

  #endif // __BYTE_ORDER

  bool operator!= (const char* rhs) const throw()
  {
    return !(*this == rhs);
  }

  bool operator<= (const char* rhs) const throw()
  {
    return !(*this > rhs);
  }

  bool operator>= (const char* rhs) const throw()
  {
    return !(*this < rhs);
  }

  bool operator== (const std::string& rhs) const throw()
  {
    return *this == rhs.c_str();
  }

  #if __BYTE_ORDER == __BIG_ENDIAN

  bool operator< (const std::string& rhs) const throw()
  {
    return ta < (const T*) rhs.c_str();
  }

  bool operator> (const std::string& rhs) const throw()
  {
    return ta > (const T*) rhs.c_str();
  }

  #elif __BYTE_ORDER == __LITTLE_ENDIAN

  bool operator< (const std::string& rhs) const throw()
  {
    return *this < rhs.c_str();
  }

  bool operator> (const std::string& rhs) const throw()
  {
    return *this > rhs.c_str();
  }

  #endif // __BYTE_ORDER

  bool operator!= (const std::string& rhs) const throw()
  {
    return !(*this == rhs.c_str());
  }

  bool operator<= (const std::string& rhs) const throw()
  {
    return !(*this > rhs.c_str());
  }

  bool operator>= (const std::string& rhs) const throw()
  {
    return !(*this < rhs.c_str());
  }

  Code& operator+= (const std::string &s) throw()
  {
    return *this += s.c_str();
  }

  Code& operator+= (const char* s) throw()
  {
    size_t len = strlen(buf);
    while( *s != 0 && len < n )
      *(buf + len++) = *s++;
    // if (*s != 0)
    // throw new std::length_error("Code too long")
    return *this;
  }

  Code& operator+= (char c) throw()
  {
    size_t len = strlen(buf);
    if( len < n )
      *(buf + len) = c;
    // else
    // throw new std::length_error("Code too long")
    return *this;
  }

// stl-style methods / definitions

  typedef char* iterator;
  typedef const char* const_iterator;

  iterator begin() throw()
  {
    return buf;
  }

  const_iterator begin() const throw()
  {
    return buf;
  }

  iterator end() throw()
  {
    size_t len = strlen(buf);
    return buf + len;
  }

  const_iterator end() const throw()
  {
    size_t len = strlen(buf);
    return buf + len;
  }

  size_t size() const throw()
  {
    return length();
  }

  void push_back(char c) throw()
  {
    // if (strlen(buf) >= n)
    // throw new std::length_error("Code too long")
    *this += c;
  }

// replacements for string methods

  const char* c_str() const throw()
  {
    return buf;
  }

  void assign(const char* s,
              size_t sz)
  {
    // if (sz > n)
    // throw new std::length_error("Code too long")
    ta.init();
    // s is not always '\0' terminated, just n as a third parameter will not do
    strncpy(buf, s, sz > n ? n : sz);
  }


  const char* data() const throw()
  {
    return buf;
  }

  bool empty() const throw()
  {
    return ta.t == 0;
  }

  void clear() throw()
  {
    ta.init();
  }

  size_t length() const throw()
  {
    return strlen(buf);
  }

  std::string substr(size_t pos, size_t len) const throw()
  {
    static const std::string empty;
    size_t sz(size());
    if (pos >= sz)
    {
      return empty;
    }
    size_t length(std::min(sz - pos, len));
    return std::string(buf + pos, length);
  }

  template<size_t m, typename U>
  size_t find(const Code<m, U>& s, size_t pos = 0) const throw()
  {
    return find(s.c_str(), pos);
  }

  size_t find(const std::string& s, size_t pos = 0) const throw()
  {
    return find(s.c_str(), pos);
  }

  size_t find(const char* s, size_t pos = 0) const throw()
  {
    const char* p = strstr(buf + pos, s);
    if( p == 0 )
      return -1;
    else
      return p - buf;
  }

  size_t find(const char c, size_t pos = 0) const throw()
  {
    const char* p = strchr(buf + pos, c);
    if( p == 0 )
      return -1;
    else
      return p - buf;
  }

  int compare(const Code& s) const throw()
  {
    return memcmp(buf, s.buf, n);
  }

//prepends elements to an existing array
Code& prepend(const char* s) throw ()
{
	int len_buf = strlen(buf);
	int len_s = strlen(s);
	int t = len_buf + len_s - 1;
	int count = len_buf - 1;
	if (t < n)
	{
		while(count >= 0)
		{
			*(buf + t--) = *(buf + count--);
		}
		count = 0;
		while(count < len_s)
		{
			*(buf + count++) = *s++;
		}
	}
	return *this;
}

private:

  static const size_t tmax = (n + sizeof(T)) / sizeof(T);

  union
  {
    char buf[n+1];
    TArray<tmax - 1, T> ta;
  };

protected:
};

template<size_t n, typename T>
std::ostream& operator<< (std::ostream& os, const Code<n, T>& code) throw()
{
  return(os << code.c_str());
}

template<size_t m, size_t n, typename T, typename U>
std::string operator+ (const Code<m, T>& lhs, const Code<n, U>& rhs) throw()
{
  return std::string(lhs.c_str()).append(rhs.c_str());
}

template<size_t n, typename T>
std::string operator+ (const char lhs, const Code<n, T>& rhs) throw()
{
  return lhs + std::string(rhs.c_str());
}

template<size_t n, typename T>
std::string operator+ (const Code<n, T>& lhs, const char rhs) throw()
{
  return std::string(lhs.c_str()) + rhs;
}

template<size_t n, typename T>
std::string operator+ (const std::string& lhs, const Code<n, T>& rhs) throw()
{
  return lhs + rhs.c_str();
}

template<size_t n, typename T>
std::string operator+ (const Code<n, T>& lhs, const std::string& rhs) throw()
{
  return lhs.c_str() + rhs;
}

template<size_t n, typename T>
bool operator== (const std::string& lhs, const Code<n, T>& rhs) throw()
{
  return lhs == rhs.c_str();
}

template<size_t n, typename T>
bool operator!= (const std::string& lhs, const Code<n, T>& rhs) throw()
{
  return !(lhs == rhs.c_str());
}
#endif // CODE_H
