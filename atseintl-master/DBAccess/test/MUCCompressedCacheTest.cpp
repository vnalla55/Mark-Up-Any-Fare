#include <boost/tr1/random.hpp>
#include <boost/thread.hpp>
#include <snappy.h>
#include "test/include/CppUnitHelperMacros.h"
#include "DBAccess/CacheManager.h"
#include "DBAccess/MarkupControl.h"
#include "DBAccess/test/TestMarkupControlDAO.h"
#include "DBAccess/test/CompressedCacheTest.h"
#include "DBAccess/CompressedDataUtils.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

const size_t
CACHE_SIZE(100);

const int
MAXNUMBERKEYS(CACHE_SIZE * 4);

const int
NUMREPEAT(2);

const int
NUMGETSINTEST(CACHE_SIZE * 2);

namespace
{
std::tr1::mt19937 _eng;
std::tr1::uniform_int<int>
_unif(0, MAXNUMBERKEYS);
}
namespace tse
{
class MUCCompressedCacheTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MUCCompressedCacheTest);
  CPPUNIT_TEST(testMUCCompressedCache_1);
  CPPUNIT_TEST(testMUCCompressedCache_2);
  CPPUNIT_TEST(testMUCCompressedCache_3);
  CPPUNIT_TEST(testMUCCompressedCache_4);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:
  void setUp() { _memHandle.create<SpecificTestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  bool checkEntry(Key key, const std::vector<MarkupControl*>& entry)
  {
    CPPUNIT_ASSERT(key % MAXNUMBERMUCENTRIES == static_cast<int>(entry.size()));
    MarkupControl ref;
    TestMarkupControlDAO::markupControlDummyData(ref);
    for (std::vector<MarkupControl*>::const_iterator it(entry.begin()), itend(entry.end());
         it != itend;
         ++it)
    {
      const MarkupControl* data(*it);
      if (!(*data == ref))
      {
        return false;
      }
    }
    return true;
  }

  bool checkContents(sfc::Cache<Key, std::vector<MarkupControl*> >& cache)
  {
    std::shared_ptr<std::vector<Key>> keys(cache.keys());
    for (const Key& key : *keys)
    {
      std::vector<MarkupControl*>* vect(cache.getIfResident(key).get());
      if (0 == vect)
      {
        return false;
      }
      if (!checkEntry(key, *vect))
      {
        return false;
      }
    }
    return true;
  }

  void testInvalidate(sfc::Cache<Key, std::vector<MarkupControl*> >& cache)
  {
    for (size_t i = 0; i < 5 && cache.size() > 0; ++i)
    {
      Key key(static_cast<int>(_unif(_eng) % cache.size()));
      cache.get(key);
      cache.invalidate(key);
      CPPUNIT_ASSERT(!cache.getIfResident(key));
    }
  }

  RBuffer& readMarkupControlVector(RBuffer& is, std::vector<MarkupControl*>*& vect)
  {
    int vsz;
    is.read(vsz);
    vect = new std::vector<MarkupControl*>;
    vect->resize(vsz);
    for (int i = 0; i < vsz; ++i)
    {
      MarkupControl* entry(new MarkupControl);
      entry->read(is);
      (*vect)[i] = entry;
    }
    return is;
  }

  void testMUCCompression(int cacheSize)
  {
    TestMarkupControlDAO dao;
    sfc::TestKeyedFactory<Key, std::vector<MarkupControl*> > factory(dao);
    sfc::CompressedCacheTest<Key, std::vector<MarkupControl*> > cache(
        factory, "MarkupControl", cacheSize, 3);
    boost::int64_t compressedTotal(0);
    boost::int64_t uncompressedTotal(0);
    for (size_t i = 0; i < CACHE_SIZE; ++i)
    {
      Key key(_unif(_eng));
      std::vector<MarkupControl*>* entryin(cache.get(key).get());
      WBuffer os;
      os.write(*entryin);
      size_t inputSz(os.size());
      uncompressedTotal += inputSz;
      std::string deflated;
      snappy::Compress(os.buffer(), inputSz, &deflated);
      compressedTotal += deflated.length();
      std::vector<char> inflated(inputSz);
      bool uncompressed(snappy::RawUncompress(deflated.c_str(), deflated.size(), &inflated[0]));
      CPPUNIT_ASSERT(uncompressed);
      CPPUNIT_ASSERT(0 == strncmp(&inflated[0], &os.buffer()[0], inputSz));
      std::vector<MarkupControl*>* entryout(0);
      RBuffer source(inflated);
      readMarkupControlVector(source, entryout);
      std::string msg;
      CPPUNIT_ASSERT(equalPtrContainer(*entryin, *entryout, msg));
      dao.destroy(key, entryout);
    }
  }
  struct Worker
  {
    Worker(sfc::Cache<Key, std::vector<MarkupControl*> >& cache, int maxKey, int& numErrors)
      : _cache(cache), _maxKey(maxKey), _currentKey(0), _numErrors(numErrors)
    {
    }
    void run()
    {
      while (_currentKey <= _maxKey)
      {
        const std::vector<MarkupControl*>* ptr(_cache.get(_currentKey++).get());
        if (0 == ptr)
        {
          ++_numErrors;
        }
      }
    }

  private:
    sfc::Cache<Key, std::vector<MarkupControl*> >& _cache;
    const int _maxKey;
    int _currentKey;
    int& _numErrors;
  };

  void testMUCMultiThreadCache(int cacheSize)
  {
    TestMarkupControlDAO dao;
    sfc::TestKeyedFactory<Key, std::vector<MarkupControl*> > factory(dao);
    sfc::CompressedCacheTest<Key, std::vector<MarkupControl*> > cache(
        factory, "MarkupControl", cacheSize, 3);
    cache.setTotalCapacity(cacheSize * 3);
    const int NUMTHREADS(10);
    boost::thread threads[NUMTHREADS];
    int numErrors[NUMTHREADS] = {};
    for (int i = 0; i < NUMTHREADS; ++i)
    {
      Worker worker(cache, cacheSize * 10, numErrors[i]);
      threads[i] = boost::thread(boost::bind(&Worker::run, worker));
    }
    for (int i = 0; i < NUMTHREADS; ++i)
    {
      threads[i].join();
    }
    for (int i = 0; i < NUMTHREADS; ++i)
    {
      CPPUNIT_ASSERT(0 == numErrors[i]);
    }
    checkContents(cache);
  }

  int testMUCCache(int cacheSize)
  {
    TestMarkupControlDAO mucdao;
    sfc::TestKeyedFactory<Key, std::vector<MarkupControl*> > mucfactory(mucdao);
    sfc::CompressedCacheTest<Key, std::vector<MarkupControl*> > cache(
        mucfactory, "MarkupControl", cacheSize, 3);
    cache.setTotalCapacity(cacheSize * 3);
    cache.setThreshold(0);
    for (int j = 0; j < NUMREPEAT; ++j)
    {
      for (int i = 0; i < NUMGETSINTEST; ++i)
      {
        Key key(_unif(_eng));
        CPPUNIT_ASSERT(cache.get(key).get() != 0);
      }
      CPPUNIT_ASSERT(checkContents(cache));
    }
    cache.clear();
    CPPUNIT_ASSERT(0 == cache.size());
    return 0;
  }

  void testMUCCompressedCache_1() { testMUCCache(CACHE_SIZE); }

  void testMUCCompressedCache_2() { testMUCCache(MAXNUMBERKEYS); }

  void testMUCCompressedCache_3() { testMUCCompression(CACHE_SIZE); }

  void testMUCCompressedCache_4() { testMUCMultiThreadCache(CACHE_SIZE); }
};
CPPUNIT_TEST_SUITE_REGISTRATION(MUCCompressedCacheTest);
} // tse
