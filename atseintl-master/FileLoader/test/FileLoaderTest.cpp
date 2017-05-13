#include "test/include/CppUnitHelperMacros.h"
#include <stdexcept>
#include "ZThreads/zthread/ThreadedExecutor.h"
#include "ZThreads/zthread/Exceptions.h"

#include "Common/Global.h"
#include "DBAccess/HashKey.h"
#include "DBAccess/SimpleCache.h"
#include "DBAccess/FareInfo.h"
#include "FileLoader/FileLoader.h"
#include "FileLoader/BFCacheUpdate.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

typedef sfc::KeyedFactory<FareKey, FareInfoVec> KeyedFactoryType;
typedef sfc::SimpleCache<FareKey, FareInfoVec> SimpleBoundFareCache;

class FileLoaderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FileLoaderTest);
  CPPUNIT_TEST(testFileLoader);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testFileLoader()
  {
    std::string loadUrl("test.load.gz");
    std::string updateUrl("test.update.gz");

    size_t capacity(0);
    class TestKeyedFactory : public KeyedFactoryType
    {
      void destroy(FareKey key, FareInfoVec* vect)
      {
        // std::cerr << "TestKeyedFactory::destroy called" << std::endl;
        size_t sz(vect->size());
        for (size_t i = 0; i < sz; ++i)
        {
          delete vect->at(i);
        }
        delete vect;
      }
    } factory;

    SimpleBoundFareCache cache(factory, "SimpleBoundFareCache", capacity, 1);

    tse::FileLoader loader(loadUrl, &cache);
    try
    {
      loader.parse();
      CPPUNIT_ASSERT(true);
    }
    catch (const std::exception& e)
    {
      CPPUNIT_ASSERT(false);
      std::cerr << "exception:" << e.what() << std::endl;
    }
    try
    {
      // start update thread
      ZThread::ThreadedExecutor executor;
      executor.execute(ZThread::Task(new tse::BFCacheUpdate(updateUrl, &cache)));
      executor.wait();
    }
    catch (ZThread::Synchronization_Exception& e)
    {
      CPPUNIT_ASSERT(false);
      std::cerr << e.what() << std::endl;
    }
    boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FileLoaderTest);
}
