#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "Common/TseConsts.h"
#include "DBAccess/SvcFeesFeatureInfo.h"
#include "DBAccess/SvcFeesFeatureDAO.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class SvcFeesFeatureTestDAO : public SvcFeesFeatureDAO
{
public:
  static SvcFeesFeatureTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return SvcFeesFeatureDAO::cache(); }

  virtual std::vector<SvcFeesFeatureInfo*>* create(SvcFeesFeatureKey key)
  {
    static SequenceNumber seqNo(17);
    std::vector<SvcFeesFeatureInfo*>* vect(new std::vector<SvcFeesFeatureInfo*>);
    SvcFeesFeatureInfo* info1(new SvcFeesFeatureInfo);
    SvcFeesFeatureInfo::dummyData(*info1);
    info1->vendor() = key._a;
    info1->itemNo() = key._b;
    info1->seqNo() = seqNo++;
    vect->push_back(info1);
    SvcFeesFeatureInfo* info2(new SvcFeesFeatureInfo);
    SvcFeesFeatureInfo::dummyData(*info2);
    info2->vendor() = key._a;
    info2->itemNo() = key._b;
    info2->seqNo() = seqNo++;
    vect->push_back(info2);
    return vect;
  }

private:
  static SvcFeesFeatureTestDAO* _instance;
  friend class DAOHelper<SvcFeesFeatureTestDAO>;
  static DAOHelper<SvcFeesFeatureTestDAO> _helper;
  SvcFeesFeatureTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : SvcFeesFeatureDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<SvcFeesFeatureTestDAO>
SvcFeesFeatureTestDAO::_helper(_name);
SvcFeesFeatureTestDAO*
SvcFeesFeatureTestDAO::_instance(0);

class SvcFeesFeatureHistoricalTestDAO : public SvcFeesFeatureHistoricalDAO
{
public:
  static SvcFeesFeatureHistoricalTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return SvcFeesFeatureHistoricalDAO::cache(); }

  virtual std::vector<SvcFeesFeatureInfo*>* create(SvcFeesFeatureHistoricalKey key)
  {
    static SequenceNumber seqNo(107);
    std::vector<SvcFeesFeatureInfo*>* vect(new std::vector<SvcFeesFeatureInfo*>);
    SvcFeesFeatureInfo* info1(new SvcFeesFeatureInfo);
    SvcFeesFeatureInfo::dummyData(*info1);
    info1->vendor() = key._a;
    info1->itemNo() = key._b;
    info1->seqNo() = seqNo++;
    vect->push_back(info1);
    SvcFeesFeatureInfo* info2(new SvcFeesFeatureInfo);
    SvcFeesFeatureInfo::dummyData(*info2);
    info2->vendor() = key._a;
    info2->itemNo() = key._b;
    info2->seqNo() = seqNo++;
    vect->push_back(info2);
    return vect;
  }

private:
  static SvcFeesFeatureHistoricalTestDAO* _instance;
  friend class DAOHelper<SvcFeesFeatureHistoricalTestDAO>;
  static DAOHelper<SvcFeesFeatureHistoricalTestDAO> _helper;
  SvcFeesFeatureHistoricalTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : SvcFeesFeatureHistoricalDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<SvcFeesFeatureHistoricalTestDAO>
SvcFeesFeatureHistoricalTestDAO::_helper(_name);
SvcFeesFeatureHistoricalTestDAO*
SvcFeesFeatureHistoricalTestDAO::_instance(0);

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

class SvcFeesFeatureDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SvcFeesFeatureDAOTest);
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
    SvcFeesFeatureTestDAO& dao(SvcFeesFeatureTestDAO::instance());
    DataHandle dh;
    DateTime ticketDate(time(NULL));
    VendorCode vendor("ATP");
    long long itemNo(499);
    const std::vector<SvcFeesFeatureInfo*>& res1(
        dao.get(dh.deleteList(), vendor, itemNo, ticketDate));
    CPPUNIT_ASSERT(2 == res1.size());
    // successive calls should bring the same entries
    const std::vector<SvcFeesFeatureInfo*>& res2(
        dao.get(dh.deleteList(), vendor, itemNo, ticketDate));
    CPPUNIT_ASSERT(res1.size() == res2.size());
    for (size_t i = 0; i < res1.size(); ++i)
    {
      CPPUNIT_ASSERT(*res1[i] == *res2[i]);
    }
  }

  void testInvalidate()
  {
    SvcFeesFeatureTestDAO& dao(SvcFeesFeatureTestDAO::instance());
    DataHandle dh;
    VendorCode vendor("SITA");
    long long itemNo(1999);
    DateTime ticketDate(time(NULL));
    dao.get(dh.deleteList(), vendor, itemNo, ticketDate); // fill the cache
    SvcFeesFeatureKey key(vendor, itemNo);
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }

  void testHistoricalGet()
  {
    SvcFeesFeatureHistoricalTestDAO& dao(SvcFeesFeatureHistoricalTestDAO::instance());
    DataHandle dh;
    DateTime ticketDate(time(NULL));
    VendorCode vendor("ATP");
    long long itemNo(23);
    const std::vector<SvcFeesFeatureInfo*>& res1(
        dao.get(dh.deleteList(), vendor, itemNo, ticketDate));
    CPPUNIT_ASSERT(2 == res1.size());
    // successive calls should bring the same entries
    const std::vector<SvcFeesFeatureInfo*>& res2(
        dao.get(dh.deleteList(), vendor, itemNo, ticketDate));
    CPPUNIT_ASSERT(res1.size() == res2.size());
    for (size_t i = 0; i < res1.size(); ++i)
    {
      CPPUNIT_ASSERT(*res1[i] == *res2[i]);
    }
  }

  void testHistoricalInvalidate()
  {
    SvcFeesFeatureHistoricalTestDAO& dao(SvcFeesFeatureHistoricalTestDAO::instance());
    DataHandle dh;
    VendorCode vendor("SITA");
    long long itemNo(47);
    DateTime ticketDate(time(NULL));
    ObjectKey objectKey;
    objectKey.setValue("VENDOR", vendor);
    objectKey.setValue("ITEMNO", itemNo);
    SvcFeesFeatureHistoricalKey key;
    CPPUNIT_ASSERT(dao.translateKey(objectKey, key, ticketDate));
    dao.get(dh.deleteList(), vendor, itemNo, ticketDate); // fill the cache
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(SvcFeesFeatureDAOTest);
}
