//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//               Andrzej Fediuk
//
//  Copyright Sabre 2014
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

namespace tse {

namespace skipper {

template <class T, template<class> class PointerT>
void assignValidObject(T* object, PointerT<T>& smartPointer)
{
  if (object == nullptr)
  {
    object = new T();
  }
  smartPointer.reset(object);
}

template <typename T>
class Factory
{
public:
  typedef T Type;

  T* create()
  {
    return new T();
  }

  template <typename P>
  T* create(const P& p)
  {
    return new T(p);
  }

  template <typename P, typename Q>
  T* create(const P& p, const Q& q)
  {
    return new T(p, q);
  }

  template <typename P, typename Q, typename R>
  T* create(const P& p, const Q& q, const R& r)
  {
    return new T(p, q, r);
  }
};

} // namespace skipper

} // namespace tse

