#pragma once

#include "DBAccess/CompressedCache.h"
#include "DBAccess/test/TestKeyedFactory.h"

namespace sfc
{
template <typename Key, typename Type>
class CompressedCacheTest : public CompressedCache<Key, Type>
{
public:
  CompressedCacheTest(TestKeyedFactory<Key, Type>& factory,
                      const std::string& name,
                      size_t capacity,
                      size_t version)
    : CompressedCache<Key, Type>(factory, name, capacity, version)
  {
  }

  virtual ~CompressedCacheTest()
  {
    this->clear();
    delete this->_uninitializedCacheEntry.get();
  }
}; // class CompressedCacheTest
} // namespace sfc
