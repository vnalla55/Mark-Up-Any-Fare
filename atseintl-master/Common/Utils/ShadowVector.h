//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2015
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

#include "Common/Utils/CommonUtils.h"
#include "Common/Utils/DBStash.h"
#include "Common/Utils/DeepSizeof.h"
#include "Common/Utils/Pprint.h"
#include "DBAccess/Flattenizable.h"

#include <boost/noncopyable.hpp>
#include <algorithm>
#include <vector>

namespace tse
{

template <class T, class CacheAccessor = tools::GetStash<T>>
class ShadowVector: boost::noncopyable
{
public:
  typedef std::vector<T*> Storage;

  const Storage& storage() const
  {
    return _storage;
  }

  const T* insert(T* ptr)
  {
    T* cptr = CacheAccessor()().insert(ptr);
    _storage.push_back(cptr);
    return cptr;
  }

  void sync_with_cache() const
  {
    for (size_t i = 0; i < _storage.size(); ++i)
    {
      auto* p = CacheAccessor()().insert(_storage[i]);
      _storage[i] = p;
    }
  }

  Storage& mutableStorage()
  {
    return _storage;
  }

  bool operator==(const ShadowVector<T, CacheAccessor>& rhs) const
  {
    // shallow comparison suffices: pointers address unique values in SharingSet
    return _storage == rhs._storage;
  }

  ~ShadowVector()
  {
    unlink();
  }

private:


  void unlink() const
  {
    for(T* ptr: _storage)
    {
      if(!CacheAccessor()().erase(ptr))
        delete ptr;
    }
  }

  mutable Storage _storage;
};


template <typename T, typename C>
void flatten(Flattenizable::Archive& archive, const tse::ShadowVector<T, C>& v)
{
  using namespace flattenizer;
  flatten(archive, v.storage());
}

template <typename T, typename C>
void unflatten(Flattenizable::Archive& archive, tse::ShadowVector<T, C>& v)
{
  using namespace flattenizer;
  unflatten(archive, v.mutableStorage());
  v.sync_with_cache();
}

template <typename T, typename C>
void calcmem(Flattenizable::Archive& archive, const tse::ShadowVector<T, C>& v)
{
  using namespace flattenizer;
  calcmem(archive, v.storage());
}

template <typename T, typename C>
std::size_t hash_value(const tse::ShadowVector<T, C>& v)
{
  // shallow hash suffices: pointers address unique values in SharingSet
  return tse::tools::calc_hash(v.storage());
}

template <typename T, typename C>
inline void pprint_impl(std::ostream& out, const tse::ShadowVector<T, C>& v)
{
  tse::tools::pprint_collection(out, v, "ShadowV [", "]");
}

template <typename T, typename C>
inline size_t deep_sizeof_impl(const tse::ShadowVector<T, C>& v)
{
  size_t sz = sizeof(v.storage());
  for(T* ptr: v.storage())
  {
    sz += deep_sizeof(*ptr);
  }
  return sz;
}

} // namespace tse
