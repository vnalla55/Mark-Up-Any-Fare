// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include <string>
#include <sstream>

namespace tax
{

class CopyableStream
{
public:
  CopyableStream() {}
  CopyableStream& operator=(const CopyableStream& other)
  {
    _out << other._out.str();
    return *this;
  }
  CopyableStream(const CopyableStream& other) { _out << other._out.str(); }

  template <typename T>
  CopyableStream& operator<<(T const& value)
  {
    _out << value;
    return *this;
  }

  std::string str() const { return _out.str(); }

  std::istream& getline(std::string& line) { return std::getline(this->_out, line); }

private:
  std::stringstream _out;
};
}
