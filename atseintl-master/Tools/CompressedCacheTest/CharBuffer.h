//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#pragma once

#include <cstring>
#include <algorithm>
#include <boost/noncopyable.hpp>

namespace tse
{

class CharBuffer : boost::noncopyable
{
public:
  explicit CharBuffer(size_t size = 0,
                      char* start = 0)
    : _size(size)
    , _buffer(size > 0 ? new char[size] : 0)
  {
    if (start > 0 && _buffer)
    {
      std::memcpy(_buffer, start, size);
    }
  }

  ~CharBuffer()
  {
    delete [] _buffer;
  }

  void resize(size_t size)
  {
    char* buffer(0);
    if (size)
    {
      buffer = new char[size];
      if (_buffer && _size)
      {
        std::memcpy(buffer, _buffer, std::min(_size, size));
      }
    }
    delete [] _buffer;
    _buffer = buffer;
    _size = size;
  }

  const char& operator [] (size_t i) const
  {
    return _buffer[i];
  }

  char& operator [] (size_t i)
  {
    return _buffer[i];
  }

  bool empty() const
  {
    return 0 == _buffer;
  }

  size_t size() const
  {
    return _size;
  }

  char* buffer()
  {
    return _buffer;
  }

  void swap(CharBuffer& other)
  {
    char* buffer(_buffer);
    _buffer = other._buffer;
    other._buffer = buffer;
    size_t size(_size);
    _size = other._size;
    other._size = size;
  }
private:
  size_t _size;
  char* _buffer;
};

}// tse
