//----------------------------------------------------------------------------
//  File:        Optional.h
//  Authors:     Quan Ta
//  Created:
//
//  Description: Optional, Lazy, or Computed-Once value
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2006
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

namespace tse
{
namespace FareCalc
{

template <class T>
class Optional
{
public:
  Optional() : _initialized(false) {}
  virtual ~Optional() {}

  const T& value() const
  {
    if (!_initialized)
    {
      compute(_value);
      _initialized = true;
    }
    return _value;
  }

protected:
  virtual void compute(T& t) const = 0;

private:
  mutable T _value;
  mutable bool _initialized;
};
}
}

