//-------------------------------------------------------------------
//
//  File:        PerfectHash.h
//  Created:     October 15, 2006
//  Authors:     Kavya Katam
//
//  Description: Perfect Hashing
//
//  Copyright Sabre 2004
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

#include "Common/Assert.h"
#include "Common/KeyedFactory.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/SimpleCache.h"

#include <memory>

#include <pthread.h>
#include <sys/types.h>

namespace tse
{
template <typename Key, typename Type>
class PerfectHash : public sfc::Cache<Key, Type>
{
public:
  typedef typename sfc::Cache<Key, Type>::pointer_type TypePtr;

  PerfectHash(sfc::KeyedFactory<Key, Type>& factory,
              const std::string& name,
              size_t capacity,
              size_t version)
    : sfc::Cache<Key, Type>(factory, "PerfectCache", name, version)
  {
    TypePtr tmpPtr(new Type);
    _hashTable = new Data[HashSize];
  }

  size_t size() override { return HashSize; }

  size_t clear() override
  {
    this->queueDiskClear();
    for (size_t n = 0; n != HashSize; ++n)
    {
      Lock l(_hashTable[n].mutex);
      _hashTable[n].data = TypePtr();
    }
    return HashSize;
  }

  TypePtr get(const Key& key) override
  {
    TypePtr retval;

    const int hashKey = GetHashKey(key);
    Lock l(_hashTable[hashKey].mutex);
    if (_hashTable[hashKey].data == 0)
    {
      const TypePtr ret(sfc::Cache<Key, Type>::_factory.create(key));
      _hashTable[hashKey].data = ret;
      retval = ret;
      this->queueDiskPut(key, true);
    }
    else
    {
      retval = _hashTable[hashKey].data;
    }

    return retval;
  }

  TypePtr getIfResident(const Key& key) override
  {
    const int hashKey = GetHashKey(key);
    Lock l(_hashTable[hashKey].mutex);
    return _hashTable[hashKey].data;
  }

  void put(const Key& key, Type* object, bool updateLDC = true) override
  {
    put(key, TypePtr(object), updateLDC);
  }

  virtual void put(const Key& key, TypePtr object, bool updateLDC = true)
  {
    const int hashKey = GetHashKey(key);
    Lock l(_hashTable[hashKey].mutex);
    _hashTable[hashKey].data = object;
    if (updateLDC)
    {
      this->queueDiskPut(key, true);
    }
  }

  size_t invalidate(const Key& key) override
  {
    this->queueDiskInvalidate(key, true, true);
    const int hashKey = GetHashKey(key);
    Lock l(_hashTable[hashKey].mutex);
    _hashTable[hashKey].data = TypePtr();
    return 1;
  }

  void emptyTrash() override {}

  std::shared_ptr<std::vector<Key>> keys() override
  {
    const std::shared_ptr<std::vector<Key>> allKeys(new std::vector<Key>);
    return allKeys;
  }

  virtual ~PerfectHash() { delete[] _hashTable; }

private:
  struct Data
  {
    Data()
    {
      const pthread_mutex_t DefaultMutex = PTHREAD_MUTEX_INITIALIZER;
      mutex = DefaultMutex;
    }
    TypePtr data;
    pthread_mutex_t mutex;
  };
  static const size_t HashSize = 26 * 26 * 26;
  Data* _hashTable;
  PerfectHash(const PerfectHash& ph);
  PerfectHash& operator=(const PerfectHash& ph);

  struct deleter
  {
  public:
    deleter(sfc::KeyedFactory<Key, Type>& factory, const Key& key) : _factory(factory), _key(key) {}

    void operator()(void* p) const { _factory.destroy(_key, reinterpret_cast<Type*>(p)); }

  private:
    sfc::KeyedFactory<Key, Type>& _factory;
    Key _key;
  };

public:
  struct Lock
  {
  public:
    Lock(pthread_mutex_t& mutex) : _myMutex(mutex) { pthread_mutex_lock(&_myMutex); }

    ~Lock() { pthread_mutex_unlock(&_myMutex); }

  private:
    pthread_mutex_t& _myMutex;
  };

  const int GetHashKey(Key key)
  {
    const unsigned int hashKey =
        ((26 * 26) * (key[0] - 'A') + 26 * (key[1] - 'A') + (key[2] - 'A'));
    TSE_ASSERT(hashKey < HashSize);
    return hashKey;
  }
};

template <typename Key, typename Value>
struct PerfectHashGenerator
{
  typedef sfc::SimpleCache<Key, Value> CacheType;
};

template <typename Value>
struct PerfectHashGenerator<LocCode, Value>
{
  typedef PerfectHash<LocCode, Value> CacheType;
};
}

