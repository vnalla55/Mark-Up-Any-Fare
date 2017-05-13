#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "DBAccess/SvcFeesFareIdInfo.h"
#include "DBAccess/SvcFeesFareIdDAO.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class SvcFeesFareIdTestDAO : public SvcFeesFareIdDAO
{
public:
  static SvcFeesFareIdTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return SvcFeesFareIdDAO::cache(); }

  virtual std::vector<SvcFeesFareIdInfo*>* create(SvcFeesFareIdKey key)
  {
    static SequenceNumber seqNo(17);
    std::vector<SvcFeesFareIdInfo*>* vect(new std::vector<SvcFeesFareIdInfo*>);
    SvcFeesFareIdInfo* info1(new SvcFeesFareIdInfo);
    SvcFeesFareIdInfo::dummyData(*info1);
    info1->vendor() = key._a;
    info1->itemNo() = key._b;
    info1->seqNo() = seqNo++;
    vect->push_back(info1);
    SvcFeesFareIdInfo* info2(new SvcFeesFareIdInfo);
    SvcFeesFareIdInfo::dummyData(*info2);
    info2->vendor() = key._a;
    info2->itemNo() = key._b;
    info2->seqNo() = seqNo++;
    vect->push_back(info2);
    return vect;
  }

private:
  static SvcFeesFareIdTestDAO* _instance;
  friend class DAOHelper<SvcFeesFareIdTestDAO>;
  static DAOHelper<SvcFeesFareIdTestDAO> _helper;
  SvcFeesFareIdTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : SvcFeesFareIdDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<SvcFeesFareIdTestDAO>
SvcFeesFareIdTestDAO::_helper(_name);
SvcFeesFareIdTestDAO*
SvcFeesFareIdTestDAO::_instance(0);

class SvcFeesFareIdHistoricalTestDAO : public SvcFeesFareIdHistoricalDAO
{
public:
  static SvcFeesFareIdHistoricalTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return SvcFeesFareIdHistoricalDAO::cache(); }

  virtual std::vector<SvcFeesFareIdInfo*>* create(SvcFeesFareIdHistoricalKey key)
  {
    static SequenceNumber seqNo(107);
    std::vector<SvcFeesFareIdInfo*>* vect(new std::vector<SvcFeesFareIdInfo*>);
    SvcFeesFareIdInfo* info1(new SvcFeesFareIdInfo);
    SvcFeesFareIdInfo::dummyData(*info1);
    info1->vendor() = key._a;
    info1->itemNo() = key._b;
    info1->seqNo() = seqNo++;
    vect->push_back(info1);
    SvcFeesFareIdInfo* info2(new SvcFeesFareIdInfo);
    SvcFeesFareIdInfo::dummyData(*info2);
    info2->vendor() = key._a;
    info2->itemNo() = key._b;
    info2->seqNo() = seqNo++;
    vect->push_back(info2);
    return vect;
  }

private:
  static SvcFeesFareIdHistoricalTestDAO* _instance;
  friend class DAOHelper<SvcFeesFareIdHistoricalTestDAO>;
  static DAOHelper<SvcFeesFareIdHistoricalTestDAO> _helper;
  SvcFeesFareIdHistoricalTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : SvcFeesFareIdHistoricalDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<SvcFeesFareIdHistoricalTestDAO>
SvcFeesFareIdHistoricalTestDAO::_helper(_name);
SvcFeesFareIdHistoricalTestDAO*
SvcFeesFareIdHistoricalTestDAO::_instance(0);

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

class SvcFeesFareIdDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SvcFeesFareIdDAOTest);
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
    SvcFeesFareIdTestDAO& dao(SvcFeesFareIdTestDAO::instance());
    DataHandle dh;
    DateTime ticketDate(time(NULL));
    VendorCode vendor("ATP");
    long long itemNo(499);
    const std::vector<SvcFeesFareIdInfo*>& res1(
        dao.get(dh.deleteList(), vendor, itemNo, ticketDate));
    CPPUNIT_ASSERT(2 == res1.size());
    // successive calls should bring the same entries
    const std::vector<SvcFeesFareIdInfo*>& res2(
        dao.get(dh.deleteList(), vendor, itemNo, ticketDate));
    CPPUNIT_ASSERT(res1.size() == res2.size());
    for (size_t i = 0; i < res1.size(); ++i)
    {
      CPPUNIT_ASSERT(*res1[i] == *res2[i]);
    }
  }

  void testInvalidate()
  {
    SvcFeesFareIdTestDAO& dao(SvcFeesFareIdTestDAO::instance());
    DataHandle dh;
    VendorCode vendor("SITA");
    long long itemNo(1999);
    DateTime ticketDate(time(NULL));
    dao.get(dh.deleteList(), vendor, itemNo, ticketDate); // fill the cache
    SvcFeesFareIdKey key(vendor, itemNo);
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }

  void testHistoricalGet()
  {
    SvcFeesFareIdHistoricalTestDAO& dao(SvcFeesFareIdHistoricalTestDAO::instance());
    DataHandle dh;
    DateTime ticketDate(time(NULL));
    VendorCode vendor("ATP");
    long long itemNo(23);
    const std::vector<SvcFeesFareIdInfo*>& res1(
        dao.get(dh.deleteList(), vendor, itemNo, ticketDate));
    CPPUNIT_ASSERT(2 == res1.size());
    // successive calls should bring the same entries
    const std::vector<SvcFeesFareIdInfo*>& res2(
        dao.get(dh.deleteList(), vendor, itemNo, ticketDate));
    CPPUNIT_ASSERT(res1.size() == res2.size());
    for (size_t i = 0; i < res1.size(); ++i)
    {
      CPPUNIT_ASSERT(*res1[i] == *res2[i]);
    }
  }

  void testHistoricalInvalidate()
  {
    SvcFeesFareIdHistoricalTestDAO& dao(SvcFeesFareIdHistoricalTestDAO::instance());
    DataHandle dh;
    VendorCode vendor("SITA");
    long long itemNo(47);
    DateTime ticketDate(time(NULL));
    ObjectKey objectKey;
    objectKey.setValue("VENDOR", vendor);
    objectKey.setValue("ITEMNO", itemNo);
    SvcFeesFareIdHistoricalKey key;
    CPPUNIT_ASSERT(dao.translateKey(objectKey, key, ticketDate));
    dao.get(dh.deleteList(), vendor, itemNo, ticketDate); // fill the cache
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(SvcFeesFareIdDAOTest);
}
