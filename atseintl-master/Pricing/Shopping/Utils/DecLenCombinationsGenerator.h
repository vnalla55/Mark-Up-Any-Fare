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

#include "Common/Assert.h"
#include "Pricing/Shopping/FiltersAndPipes/IGenerator.h"
#include "Pricing/Shopping/Utils/CombinationsGenerator.h"

#include <boost/utility.hpp>

#include <vector>

namespace tse
{

namespace utils
{

// Returns combinations of decreasing length
//
// E.g.
// Input sequence: abcd
// Emitted combinations:
// abcd
// abc
// abd
// acd
// bcd
// ab
// ac
// ad
// bc
// bd
// cd
// a
// b
// c
// d
// -
template <typename T>
class DecLenCombinationsGenerator : public IGenerator<std::vector<T> >, boost::noncopyable
{
public:
  void addElement(const T& element)
  {
    _combGen.addElement(element);
    reset();
  }

  void reset() { _combGen.reset(_combGen.getElementsCount()); }

  std::vector<T> next() override
  {
    if (getElementsCount() == 0)
    {
      return std::vector<T>();
    }

    while (_combGen.getLength() > 0)
    {
      const std::vector<T> out = _combGen.next();
      if (!out.empty())
      {
        return out;
      }
      // Try shorter combination
      _combGen.reset(_combGen.getLength() - 1);
    }
    return std::vector<T>();
  }

  unsigned int getElementsCount() const { return _combGen.getElementsCount(); }

private:
  CombinationsGenerator<T> _combGen;
};

} // namespace utils

} // namespace tse

