// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include <sstream>

namespace tax
{
namespace display
{

template<class T>
class FixedWidth
{
public:
  FixedWidth(const T& val, unsigned int width, char lineFill = ' ') :
    _val(val), _width(width), _lineFill(lineFill) {}

  friend std::ostream& operator<<(std::ostream& stream, const FixedWidth& rhs)
  {
    std::ostringstream tempStream;
    tempStream.flags(std::ios::left);
    tempStream.width(rhs._width);
    tempStream.fill(rhs._lineFill);
    tempStream << rhs._val;
    stream << tempStream.str();
    return stream;
  }

private:
  const T& _val;
  unsigned int _width;
  char _lineFill;
};

template <class T>
inline FixedWidth<T> fixedWidth(const T& val, unsigned int width, char lineFill = ' ')
{
  return FixedWidth<T>(val, width, lineFill);
}

} /* namespace display */
} /* namespace tax */
