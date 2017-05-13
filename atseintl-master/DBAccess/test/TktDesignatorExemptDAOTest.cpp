#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "DBAccess/TktDesignatorExemptInfo.h"
#include "DBAccess/TktDesignatorExemptDAO.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class TktDesignatorExemptTestDAO : public TktDesignatorExemptDAO
{
public:
  static TktDesignatorExemptTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return TktDesignatorExemptDAO::cache(); }

  virtual std::vector<TktDesignatorExemptInfo*>* create(TktDesignatorExemptKey key)
  {
    static SequenceNumber seqNo(17);
    std::vector<TktDesignatorExemptInfo*>* vect(new std::vector<TktDesignatorExemptInfo*>);
    TktDesignatorExemptInfo* info1(new TktDesignatorExemptInfo);
    TktDesignatorExemptInfo::dummyData(*info1);
    info1->carrier() = key._a;
    info1->sequenceNumber() = seqNo++;
    vect->push_back(info1);
    TktDesignatorExemptInfo* info2(new TktDesignatorExemptInfo);
    TktDesignatorExemptInfo::dummyData(*info2);
    info2->carrier() = key._a;
    info2->sequenceNumber() = seqNo++;
    vect->push_back(info2);
    return vect;
  }

private:
  static TktDesignatorExemptTestDAO* _instance;
  friend class DAOHelper<TktDesignatorExemptTestDAO>;
  static DAOHelper<TktDesignatorExemptTestDAO> _helper;
  TktDesignatorExemptTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : TktDesignatorExemptDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<TktDesignatorExemptTestDAO>
TktDesignatorExemptTestDAO::_helper(_name);
TktDesignatorExemptTestDAO*
TktDesignatorExemptTestDAO::_instance(0);

class TktDesignatorExemptHistoricalTestDAO : public TktDesignatorExemptHistoricalDAO
{
public:
  static TktDesignatorExemptHistoricalTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return TktDesignatorExemptHistoricalDAO::cache(); }

  virtual std::vector<TktDesignatorExemptInfo*>* create(TktDesignatorExemptHistoricalKey key)
  {
    static SequenceNumber seqNo(107);
    std::vector<TktDesignatorExemptInfo*>* vect(new std::vector<TktDesignatorExemptInfo*>);
    TktDesignatorExemptInfo* info1(new TktDesignatorExemptInfo);
    TktDesignatorExemptInfo::dummyData(*info1);
    info1->carrier() = key._a;
    info1->sequenceNumber() = seqNo++;
    vect->push_back(info1);
    TktDesignatorExemptInfo* info2(new TktDesignatorExemptInfo);
    TktDesignatorExemptInfo::dummyData(*info2);
    info2->carrier() = key._a;
    info2->sequenceNumber() = seqNo++;
    vect->push_back(info2);
    return vect;
  }

private:
  static TktDesignatorExemptHistoricalTestDAO* _instance;
  friend class DAOHelper<TktDesignatorExemptHistoricalTestDAO>;
  static DAOHelper<TktDesignatorExemptHistoricalTestDAO> _helper;
  TktDesignatorExemptHistoricalTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : TktDesignatorExemptHistoricalDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<TktDesignatorExemptHistoricalTestDAO>
TktDesignatorExemptHistoricalTestDAO::_helper(_name);
TktDesignatorExemptHistoricalTestDAO*
TktDesignatorExemptHistoricalTestDAO::_instance(0);

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

class TktDesignatorExemptDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TktDesignatorExemptDAOTest);
  CPPUNIT_TEST(testGet);
  CPPUNIT_TEST(testInvalidate);
  CPPUNIT_TEST(testHistoricalGet);
  CPPUNIT_TEST(testHistoricalInvalidate);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    MockGlobal::setStartTime();
  }

  void tearDown() { _memHandle.clear(); }

  void testGet()
  {
    TktDesignatorExemptTestDAO& dao(TktDesignatorExemptTestDAO::instance());
    DataHandle dh;
    DateTime ticketDate(time(NULL));
    CarrierCode carrier("AA");
    const std::vector<TktDesignatorExemptInfo*>& res1(
        dao.get(dh.deleteList(), carrier, ticketDate));
    CPPUNIT_ASSERT(2 == res1.size());
    // successive calls should bring the same entries
    const std::vector<TktDesignatorExemptInfo*>& res2(
        dao.get(dh.deleteList(), carrier, ticketDate));
    CPPUNIT_ASSERT(res1.size() == res2.size());
    for (size_t i = 0; i < res1.size(); ++i)
    {
      CPPUNIT_ASSERT(*res1[i] == *res2[i]);
    }
  }

  void testInvalidate()
  {
    TktDesignatorExemptTestDAO& dao(TktDesignatorExemptTestDAO::instance());
    DataHandle dh;
    CarrierCode carrier("AA");
    DateTime ticketDate(time(NULL));
    dao.get(dh.deleteList(), carrier, ticketDate); // fill the cache
    TktDesignatorExemptKey key(carrier);
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }

  void testHistoricalGet()
  {
    TktDesignatorExemptHistoricalTestDAO& dao(TktDesignatorExemptHistoricalTestDAO::instance());
    DataHandle dh;
    DateTime ticketDate(time(NULL));
    CarrierCode carrier("AA");
    const std::vector<TktDesignatorExemptInfo*>& res1(
        dao.get(dh.deleteList(), carrier, ticketDate));
    CPPUNIT_ASSERT(2 == res1.size());
    // successive calls should bring the same entries
    const std::vector<TktDesignatorExemptInfo*>& res2(
        dao.get(dh.deleteList(), carrier, ticketDate));
    CPPUNIT_ASSERT(res1.size() == res2.size());
    for (size_t i = 0; i < res1.size(); ++i)
    {
      CPPUNIT_ASSERT(*res1[i] == *res2[i]);
    }
  }

  void testHistoricalInvalidate()
  {
    TktDesignatorExemptHistoricalTestDAO& dao(TktDesignatorExemptHistoricalTestDAO::instance());
    DataHandle dh;
    CarrierCode carrier("AA");
    DateTime ticketDate(time(NULL));
    ObjectKey objectKey;
    objectKey.setValue("CARRIER", carrier);
    TktDesignatorExemptHistoricalKey key;
    CPPUNIT_ASSERT(dao.translateKey(objectKey, key, ticketDate));
    dao.get(dh.deleteList(), carrier, ticketDate); // fill the cache
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TktDesignatorExemptDAOTest);
}
