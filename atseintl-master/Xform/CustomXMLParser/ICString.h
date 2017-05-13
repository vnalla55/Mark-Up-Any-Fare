#ifndef ICString_H
#define ICString_H

#include <boost/functional/hash.hpp>

#include <cstdlib>
#include <cstring>
#include <ostream>
#include "Util/BranchPrediction.h"

#include <tr1/functional>

class IValueString
{
 public:
  IValueString () throw ()
    : _pchar(nullptr)
    , _length(0)
  {
  }
  IValueString (const char *pchar,
                size_t length) throw ()
    : _pchar(pchar)
    , _length(length)
  {
  }
  IValueString (const IValueString &other) throw ()
    : _pchar(other._pchar)
    , _length(other._length)
  {
  }
  ~IValueString () throw () {}

  bool empty () const throw ()
  {
    return 0 == _length;
  }
  size_t length () const throw ()
  {
    return _length;
  }
  const char *c_str () const throw ()
  {
    return _pchar;
  }
#if 0
  operator std::string () const throw ()
  {
    return std::string(_pchar, _length);
  }
#endif// 0
  bool operator == (const IValueString &other) const throw ()
  {
    if (_length == other._length && _pchar == other._pchar)
    {
      return true;
    }
    return _length == other._length
           && 0 == strncmp(_pchar, other._pchar, _length);
  }
  bool operator == (const std::string &other) const throw ()
  {
    return _length == other.length()
           && 0 == strncmp(_pchar, other.c_str(), _length);
  }
  bool operator == (const char *other) const throw ()
  {
    return _length == strlen(other)
           && 0 == strncmp(_pchar, other, _length);
  }
  bool operator == (char c) const throw ()
  {
    return _length == 1 && _pchar[0] == c;
  }
  friend bool operator == (const char *other,
                           const IValueString &cstr) throw ()
  {
    return cstr.operator ==(other);
  }
  template <typename T> friend bool operator == (const T &other,
                                                 const IValueString &cstr) throw ()
  {
    return cstr.operator ==(other);
  }
  friend bool operator != (const char *other,
                           const IValueString &cstr) throw ()
  {
    return !cstr.operator ==(other);
  }
  template <typename T> friend bool operator != (const T &other,
                                                 const IValueString &cstr) throw ()
  {
    return !cstr.operator ==(other);
  }
  bool operator != (const std::string &other) const throw ()
  {
    return !this->operator ==(other);
  }
  friend std::ostream &operator << (std::ostream &os,
                                    const IValueString &cstr) throw ()
  {
    return os.write(cstr._pchar, static_cast<std::streamsize>(cstr._length));
  }
  void parse (std::string& value) const
  {
    value.assign(c_str(), length());
  }
  void parse (bool &value) const throw ()
  {
    value = ("true" == *this || "T" == *this);
  }
  void parse (double &value) const throw ()
  {
    if (LIKELY(_pchar != nullptr))
    {
      value = strtod(_pchar, nullptr);
    }
  }
  void parse (float &value) const throw ()
  {
    if (_pchar != nullptr)
    {
      value = static_cast<float>(strtod(_pchar, nullptr));
    }
  }
  void parse (int &value) const throw ()
  {
    if (LIKELY(_pchar != nullptr))
    {
      value = static_cast<int>(strtol(_pchar, nullptr, 10));
    }
  }
  void parse (unsigned int &value) const throw ()
  {
    if (LIKELY(_pchar != nullptr))
    {
      value = static_cast<unsigned int>(strtoul(_pchar, nullptr, 10));
    }
  }
  void parse (long &value) const throw ()
  {
    if (_pchar != nullptr)
    {
      value = strtol(_pchar, nullptr, 10);
    }
  }
  void parse (unsigned long &value) const throw ()
  {
    if (LIKELY(_pchar != nullptr))
    {
      value = strtoul(_pchar, nullptr, 10);
    }
  }
  void parse (short &value) const throw ()
  {
    if (LIKELY(_pchar != nullptr))
    {
      value = static_cast<short>(strtol(_pchar, nullptr, 10));
    }
  }
  void parse (unsigned short &value) const throw ()
  {
    if (_pchar != nullptr)
    {
      value = static_cast<unsigned short>(strtol(_pchar, nullptr, 10));
    }
  }
  void parse (char &value) const throw ()
  {
    if (LIKELY(1 == _length && _pchar != nullptr))
    {
      value = *_pchar;
    }
  }
  void parse (unsigned char &value) const throw ()
  {
    if (1 == _length && _pchar != nullptr)
    {
      value = *_pchar;
    }
  }
#if 0
  template <typename T> void parse (T &value) const throw ()
  {
    std::istringstream is(*this);
    is >> value;
  }
#endif// 0
  char operator [] (int i) const throw ()
  {
    return _pchar[i];
  }
  typedef const char * const_iterator;
  const char *begin () const throw ()
  {
    return _pchar;
  }
  const char *end () const throw ()
  {
    return _pchar + _length;
  }
  void clear () throw ()
  {
    _pchar = nullptr;
    _length = 0;
  }
  IValueString &operator = (const IValueString &other) throw ()
  {
    _pchar = other._pchar;
    _length = other._length;
    return *this;
  }
  void assign (const char *pchar,
               size_t length) throw ()
  {
    _pchar = pchar;
    _length = length;
  }
 protected:
  const char *_pchar;
  size_t _length;
};
class IKeyString : public IValueString
{
public:
  IKeyString () throw ()
  {
  }
  IKeyString (const char *pchar,
              size_t length) throw ()
    : IValueString(pchar, length)
  {
  }
  IKeyString (const IKeyString &other) throw ()
    : IValueString(other)
  {
  }
  ~IKeyString () throw () {}

  bool operator < (const IKeyString &other) const throw ()
  {
    int diff(strncmp(_pchar, other._pchar, std::min(_length, other._length)));
    return 0 == diff ? _length < other._length : diff < 0;
  }
 private:
  // not implemented
  IKeyString &operator = (const IKeyString &);
};

class IKeyStringHasher : public std::unary_function<IKeyString, size_t>
{
public:
  size_t operator () (const IKeyString &cstr) const
  {
    return boost::hash_range(cstr.begin(), cstr.end());
  }
};
#endif// ICString_H
