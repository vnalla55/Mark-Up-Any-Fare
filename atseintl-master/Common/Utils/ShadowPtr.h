//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2016
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
#include "Common/Utils/DBStash.h"
#include "DBAccess/Flattenizable.h"

#include <boost/noncopyable.hpp>

namespace tse
{

template <class T, class CacheAccessor = tools::GetStash<T>>
class ShadowPtr: boost::noncopyable
{
public:

  T& operator*() const
  {
    TSE_ASSERT(_x != nullptr);
    return *_x;
  }

  T* operator->() const
  {
    TSE_ASSERT(_x != nullptr);
    return _x;
  }

  T* get() const
  {
    return _x;
  }

  void reset(T* ptr)
  {
    TSE_ASSERT(ptr != nullptr);
    dealloc();
    _x = ptr;
  }

  void sync_with_cache() const
  {
    _x = CacheAccessor()().insert(_x);
  }

  ~ShadowPtr()
  {
    dealloc();
  }

private:

  void dealloc()
  {
    if (_x != nullptr)
    {
      if(!CacheAccessor()().erase(_x))
        delete _x;
      _x = nullptr;
    }
  }

  mutable T* _x = nullptr;
};


template <typename T, typename C>
void flatten(Flattenizable::Archive& archive, const tse::ShadowPtr<T, C>& p)
{
  using namespace flattenizer;
  TSE_ASSERT(p.get() != nullptr);
  flatten(archive, *p);
}

template <typename T, typename C>
void unflatten(Flattenizable::Archive& archive, tse::ShadowPtr<T, C>& p)
{
  using namespace flattenizer;
  std::unique_ptr<T> ptr(new T);
  unflatten(archive, *ptr);
  p.reset(ptr.release());
  p.sync_with_cache();
}

template <typename T, typename C>
void calcmem(Flattenizable::Archive& archive, const tse::ShadowPtr<T, C>& p)
{
  using namespace flattenizer;
  TSE_ASSERT(p.get() != nullptr);
  calcmem(archive, *p);
}

} // namespace tse
