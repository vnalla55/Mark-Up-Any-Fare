//-------------------------------------------------------------------
//
//  File:        Swapper.h
//  Created:     October 19, 2005
//  Authors:     David White
//
//
//  Copyright Sabre 2005
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

#include <algorithm>

namespace tse
{

// class which is used to swap two variables and then guarantee the variables
// will be swapped back when the Swapper object is destroyed.
template <typename T>
class Swapper
{
public:
  Swapper(T& a, T& b) : _a(nullptr), _b(nullptr)
  {
    _a = &a;
    _b = &b;
    swap();
  }

  ~Swapper() { swap(); }

  void cancel()
  {
    _a = 0;
    _b = 0;
  }

  // copy semantics transfer control to the target
  Swapper(const Swapper& o) : _a(o._a), _b(o._b)
  {
    o._a = nullptr;
    o._b = nullptr;
  }

  Swapper& operator=(const Swapper& o)
  {
    _a = o._a;
    _b = o._b;
    o._a = 0;
    o._b = 0;
    return *this;
  }

private:
  void swap()
  {
    if (_a != nullptr && _b != nullptr)
    {
      std::swap(*_a, *_b);
    }
  }

  mutable T* _a;
  mutable T* _b;
};
}

