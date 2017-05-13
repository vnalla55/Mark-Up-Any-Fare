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

#include "TaxDisplay/Response/LineParams.h"

#include <sstream>
#include <string>

namespace tax
{
namespace display
{

class Line
{
public:
  Line();
  Line(const Line&) = default;
  Line(Line&&) = default;
  Line(const std::string& str, const LineParams& params);
  explicit Line(const std::string& str);
  explicit Line(std::string&& str);
  explicit Line(const LineParams& params);

  bool operator==(const Line& rhs) const
  {
    return _params == rhs._params && _str == rhs._str;
  }

  template<class Val>
  Line& operator<<(const Val& val) { return add(val); }

  template<class Val>
  Line& add(const Val& val)
  {
    std::ostringstream stream;
    stream << val;
    return add(stream.str());
  }

  Line& add(const std::string& str)
  {
    _str += str;
    return *this;
  }

  void clear() { _str.clear(); }

  LineParams& params() { return _params; }
  const LineParams& params() const { return _params;}

  std::string& str() { return _str; }
  const std::string& str() const { return _str; }

private:
  LineParams _params;
  std::string _str;
};

} /* namespace display */
} /* namespace tax */
