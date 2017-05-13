
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

#include <iostream>

namespace tse
{

namespace utils
{

template <typename T, typename F>
class FormattingProxy
{
public:
  FormattingProxy(const T& object, const F& formatter) : _object(object), _formatter(formatter) {}

  void format(std::ostream& out) const { _formatter(out, _object); }

private:
  const T& _object;
  const F& _formatter;
};

template <typename T, typename F>
std::ostream& operator<<(std::ostream& out, const FormattingProxy<T, F>& fp)
{
  fp.format(out);
  return out;
}

template <typename T, typename F>
const FormattingProxy<T, F>
format(const T& object, const F& formatter)
{
  return FormattingProxy<T, F>(object, formatter);
}

} // namespace utils

} // namespace tse

