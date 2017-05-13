//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#pragma once

#include <fstream>
#include <boost/date_time.hpp>
#include "BaseTrx.h"
#include "CompressedDataUtils.h"
#include "SimpleCacheTest.h"
#include "CompressedCacheTest.h"

#include <thread>

const int MAXNUMBERENTRIES(100);

const size_t BASE_CACHE_OPS(500);
const int BASE_NUMGETSINTEST(BASE_CACHE_OPS * 4);
const int BASE_MAXNUMBERKEYS(BASE_CACHE_OPS * 8);
#ifdef _DEBUG
  const size_t CACHE_OPS(BASE_CACHE_OPS);
  const int NUMGETSINTEST(BASE_NUMGETSINTEST);
  const int MAXNUMBERKEYS(BASE_MAXNUMBERKEYS);
#else
  const size_t CACHE_OPS(BASE_CACHE_OPS * 10);
  const int NUMGETSINTEST(BASE_NUMGETSINTEST * 10);
  const int MAXNUMBERKEYS(BASE_MAXNUMBERKEYS * 10);
#endif

const int NUMREPEAT(7);

inline long getMemUsage()
{
  std::string pid, comm, state, ppid, pgrp, session, tty_nr,
              tpgid, flags, minflt, cminflt, majflt, cmajflt,
              utime, stime, cutime, cstime, priority, nice,
              O, itrealvalue, starttime;

  unsigned long vsize;
  long rss;
  std::ifstream stream("/proc/self/stat",std::ios_base::in);
  stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
         >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
         >> utime >> stime >> cutime >> cstime >> priority >> nice
         >> O >> itrealvalue >> starttime >> vsize >> rss;
  stream.close();
  long page_size_kb(sysconf(_SC_PAGE_SIZE) / 1024);// in case x86-64 is configured to use 2MB pages
  //double vm_usage(vsize / 1024.0);
  long resident_set(rss * page_size_kb);
  return resident_set;
}

extern uint64_t getCurrentMillis();

namespace tse
{
typedef HashKey<int> Key;

Key getRandomKey();

template <typename T> T& _getRef()
{
  typedef typename boost::remove_const<T>::type NONCONST;
  static NONCONST ref;
  ref.dummyData();
  return ref;
}

template <typename T> T& getRef()
{
  static T& ref(_getRef<T>());
  return ref;
}

template <typename T> bool checkEntry(Key key,
                                      const std::vector<T*>& entry)
{
  static const T& ref(getRef<T>());
  if (key._a % MAXNUMBERENTRIES != static_cast<int>(entry.size()))
  {
    std::cout << "! wrong vector size ! key=" << key
              << ",entry.size()=" << entry.size()
              << std::endl;
    return false;
  }
  for (const auto ptr : entry)
  {
    if (!(*ptr == ref))
    {
      std::cout << "! objects are different !" << std::endl;
      return false;
    }
  }
  return true;
}

template <typename T> bool checkContents(sfc::Cache<Key, T>& cache)
{
  std::shared_ptr<std::vector<Key>> keys(cache.keys());
  for (const auto& key : *keys)
  {
    BaseTrx trx;
    T* vect(cache.getIfResident(key).get());
    if (0 == vect)
    {
      std::cout << "! getIfResident() returned 0 !" << std::endl;
    }
    else
    {
      checkEntry(key, *vect);
    }
  }
  return true;
}

template <typename T> bool testInvalidate(sfc::Cache<Key, T>& cache)
{
  std::shared_ptr<std::vector<Key>> keys(cache.keys());
  for (size_t i = 0; i < 20 && i < keys->size(); ++i)
  {
    Key key((*keys)[i]);
    cache.invalidate(key);
    if (cache.getIfResident(key))
    {
      std::cout << "! entry was not invalidated,key=" << key << std::endl;
    }
  }
  return true;
}

template <typename T> struct Worker
{
  Worker(sfc::Cache<Key, std::vector<T*>>& cache,
         DataAccessObject<Key, std::vector<T*>>& dao)
    : _cache(cache)
    , _dao(dao)
    , _maxKey(CACHE_OPS * 20)
    , _currentKey(0)
  {
  }

  void runFull()
  {
    std::ostringstream os;
    try
    {
      while (_currentKey <= _maxKey)
      {
        BaseTrx trx;
        const std::vector<T*>* ptr(_cache.get(_currentKey++).get());
        if (ptr != 0 && !ptr->empty())
        {
          DateTime cd((*ptr)[0]->createDate());
          os << cd;
        }
        RemoteCache::StatusType status(RemoteCache::STATUS_NONE);
        sfc::CompressedDataPtr inUse1(_cache.getCompressed(_currentKey, status));
        sfc::CompressedDataPtr inUse2(_cache.getCompressed(_currentKey, status));
        if (inUse1)
        {
          _cache.invalidate(_currentKey);
        }
        if (0 == ptr)
        {
          std::cout << "0 == ptr" << std::endl;
        }
      }
      testInvalidate();
      while (_currentKey > 0)
      {
        BaseTrx trx;
        const std::vector<T*>* ptr(_cache.get(_currentKey--).get());
        if (ptr != 0 && !ptr->empty())
        {
          DateTime cd((*ptr)[0]->createDate());
          os << cd;
        }
        if (0 == ptr)
        {
          std::cout << "0 == ptr" << std::endl;
        }
      }
      testPut();
    }
    catch (const std::exception &e)
    {
      std::cout << e.what() << std::endl;
    }
    catch (...)
    {
      std::cout << "unexpected" << std::endl;
    }
  }

  void run()
  {
    try
    {
      while (_currentKey <= _maxKey)
      {
        const std::vector<T*>* ptr(_cache.get(_currentKey++).get());
        if (0 == ptr)
        {
          std::cout << "0 == ptr" << std::endl;
        }
      }
      while (_currentKey > 0)
      {
        const std::vector<T*>* ptr(_cache.get(_currentKey--).get());
        if (0 == ptr)
        {
          std::cout << "0 == ptr" << std::endl;
        }
      }
    }
    catch (const std::exception& e)
    {
      std::cout << e.what() << std::endl;
    }
    catch (...)
    {
      std::cout << "unexpected" << std::endl;
    }
  }
private:
  void testInvalidate()
  {
    std::shared_ptr<std::vector<Key>> keys(_cache.keys());
    size_t numToInvalidate(keys->size() / 20);
    for (size_t i = 0; i < numToInvalidate; ++i)
    {
      Key key((*keys)[i]);
      _cache.invalidate(key);
      if (_cache.getIfResident(key))
      {
        //std::cout << __FUNCTION__ << ":entry exists,key=" << key << std::endl;
      }
    }
  }
  void testPut()
  {
    size_t numToPut(_cache.getTotalCapacity() / 20);
    for (size_t i = 0; i < numToPut; ++i)
    {
      Key key(getRandomKey());
      std::vector<T *> *entry(_dao.create(key));
      _cache.put(key, entry, false);
    }
  }
  sfc::Cache<Key, std::vector<T*>>& _cache;
  DataAccessObject<Key, std::vector<T*>>& _dao;
  const int _maxKey;
  int _currentKey;
};

template <typename T> int testMultiThreadCache(size_t capacity,
                                               size_t totalCapacity,
                                               size_t numThreads,
                                               DataAccessObject<Key, std::vector<T*>>& dao)
{
  sfc::KeyedFactory<Key, std::vector<T*>> factory(dao);
  sfc::CompressedCacheTest<Key, std::vector<T*>> cache(factory, "", capacity, 3);
  cache.setTotalCapacity(totalCapacity);
  std::thread threads[1000];
  uint64_t start(getCurrentMillis());
  for (size_t i = 0; i < numThreads; ++i)
  {
    Worker<T> worker(cache, dao);
    threads[i] = std::thread(std::bind(&Worker<T>::run, worker));
  }
  for (size_t i = 0; i < numThreads; ++i)
  {
    threads[i].join();
  }
  uint64_t sec(getCurrentMillis() - start);
  std::cout << __FUNCTION__ << ':' << std::setw(3) << std::fixed << std::setprecision(3) << sec
            << "ms,memusage=" << getMemUsage() << "kb"
            << ",size=" << cache.size()
            << ",uncomprSize=" << cache.uncompressedSize() << ','
            << numThreads << " threads joined, checking..."
            << std::endl;
  checkContents(cache);
  getCacheDiagnostic(cache);
  return 0;
}

template <typename T> int testMultiThreadSimpleCache(size_t capacity,
                                                     size_t numThreads,
                                                     DataAccessObject<Key, std::vector<T*>>& dao)
{
  sfc::KeyedFactory<Key, std::vector<T*>> factory(dao);
  sfc::SimpleCacheTest<Key, std::vector<T*>> cache(factory, "", capacity, 3);
  std::thread threads[1000];
  uint64_t start(getCurrentMillis());
  for (size_t i = 0; i < numThreads; ++i)
  {
    Worker<T> worker(cache, dao);
    threads[i] = std::thread(std::bind(&Worker<T>::run, worker));
  }
  for (size_t i = 0; i < numThreads; ++i)
  {
    threads[i].join();
  }
  uint64_t sec(getCurrentMillis() - start);
  std::cout << __FUNCTION__ << ':' << std::setw(3) << std::fixed << std::setprecision(3) << sec
            << "ms,memusage=" << getMemUsage() << "kb"
            << ",size=" << cache.size()
            << ',' << numThreads << " threads joined, checking..."
            << std::endl;
  checkContents(cache);
  return 0;
}

template <typename T> int testCache(int capacity,
                                    int totalCapacity,
                                    DataAccessObject<Key, std::vector<T*>>& dao)
{
  std::ostringstream os;
  sfc::KeyedFactory<Key, std::vector<T*>> factory(dao);
  sfc::CompressedCacheTest<Key, std::vector<T*>> cache(factory, "", capacity, 3);
  cache.setTotalCapacity(totalCapacity);
  cache.setThreshold(0);
  for (int j(0); j < NUMREPEAT; ++j)
  {
    uint64_t start(getCurrentMillis());
    for (int i = 0; i < NUMGETSINTEST; ++i)
    {
      BaseTrx trx;
      const std::vector<T*> *result(cache.get(getRandomKey()).get());
      if (result != 0 && !result->empty())
      {
        DateTime cd((*result)[0]->createDate());
        os << cd;
      }
    }
    uint64_t sec(getCurrentMillis() - start);
    std::cout << j << ",memusage=" << getMemUsage() << "kb"
              << ",time=" << sec << "ms"
              << ",size=" << cache.size()
              << ",uncomprSize=" << cache.uncompressedSize()
              << ',' << NUMGETSINTEST << " cache.get"
              << std::endl;
    if (!checkContents(cache))
    {
      return 1;
    }
    if (!testInvalidate(cache))
    {
      return 1;
    }
    std::shared_ptr<std::vector<Key>> keys(cache.keys());
    for (const auto& key : *keys)
    {
      RemoteCache::StatusType status(RemoteCache::STATUS_NONE);
      sfc::CompressedDataPtr inUse1(cache.getCompressed(key, status));
      sfc::CompressedDataPtr inUse2(cache.getCompressed(key, status));
      if (inUse1)
      {
        cache.invalidate(key);
        break;
      }
    }
  }
  cache.clear();
  if (cache.size() != 0)
  {
    std::cout << "! cache.clear() failed !" << std::endl;
    return 1;
  }
  std::cout << std::endl;
  return 0;
}

template <typename T> int testTextFormat(DataAccessObject<Key, std::vector<T*>>& dao,
                                         size_t maxSize)
{
  sfc::KeyedFactory<Key, std::vector<T*>> factory(dao);
  sfc::CompressedCacheTest<Key, std::vector<T*>> cache(factory, "", 1, 3);
  cache.setTotalCapacity(0);
  cache.setThreshold(0);
  const int numgetstest(10);
  for (int i = 0; i < numgetstest; ++i)
  {
    cache.get(getRandomKey()).get();
  }
  if (!checkContents(cache))
  {
    return 1;
  }
  std::ostringstream os;
  WBuffer buffer(&os);
  buffer.printCache(cache);
  const std::string& str(os.str());
  size_t size(str.size());
  if (size > maxSize * 1000)
  {
    size = maxSize * 1000;
  }
  if (size > 0)
  {
    std::cout.write(str.c_str(), size);
  }
  std::cout << " ... ... ...\n" << std::endl;
  return 0;
}

template <typename T> int testSimpleCache(int capacity,
                                          DataAccessObject<Key, std::vector<T*>>& dao)
{
  sfc::KeyedFactory<Key, std::vector<T*>> factory(dao);
  sfc::SimpleCacheTest<Key, std::vector<T*>> cache(factory, "", capacity, 3);
  for (int j = 0; j < NUMREPEAT; ++j)
  {
    uint64_t start(getCurrentMillis());
    for (int i = 0; i < NUMGETSINTEST; ++i)
    {
      BaseTrx trx;
      cache.get(getRandomKey());
    }
    uint64_t sec(getCurrentMillis() - start);
    std::cout << j << ",memusage=" << getMemUsage() << "kb"
              << ",time=" << sec << "ms"
              << ",size=" << cache.size()
              << ',' << NUMGETSINTEST << " cache.get"
              << std::endl;
    if (!checkContents(cache))
    {
      return 1;
    }
    if (!testInvalidate(cache))
    {
      return 1;
    }
  }
  cache.clear();
  if (cache.size() != 0)
  {
    std::cout << "!!!cache.clear() failed!!!" << std::endl;
    return 1;
  }
  std::cout << std::endl;
  return 0;
}

template <typename Key, typename Type> void getCacheDiagnostic(sfc::Cache<Key, Type>& cache)
{
  sfc::CompressedCacheStats stats;
  cache.getCompressionStats(stats);
  std::cout << "#tot=" << stats._totalSize
            << " totCap=" << stats._totalCapacity
            << " #uncompr=" << stats._uncompressedSize
            << " uncomprCap=" << stats._uncompressedCapacity
            << " #compressed=" << stats._compressedSize
            << " #empty=" << stats._numberEmpty
            << " mem~" << stats._memoryEstimate
            << " avrgComprBytes=" << stats._averageCompressedBytes
            << " avrgRatio=" << stats._averageRatio
            << " thld=" << stats._threshold
            << " err:" << stats._errors
            << std::endl;
}

}// tse
