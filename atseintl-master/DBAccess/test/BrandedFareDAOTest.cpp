#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "DBAccess/BrandedFare.h"
#include "DBAccess/BrandedFareDAO.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class BrandedFareTestDAO : public BrandedFareDAO
{
public:
  static BrandedFareTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return BrandedFareDAO::cache(); }

  virtual std::vector<BrandedFare*>* create(BrandedFareKey key)
  {
    static SequenceNumber seqNo(17);
    std::vector<BrandedFare*>* vect(new std::vector<BrandedFare*>);
    BrandedFare* bf1(new BrandedFare);
    BrandedFare::dummyData(*bf1);
    bf1->vendor() = key._a;
    bf1->carrier() = key._b;
    bf1->seqNo() = seqNo++;
    vect->push_back(bf1);
    BrandedFare* bf2(new BrandedFare);
    BrandedFare::dummyData(*bf2);
    bf2->vendor() = key._a;
    bf2->carrier() = key._b;
    bf2->seqNo() = seqNo++;
    vect->push_back(bf2);
    return vect;
  }

private:
  static BrandedFareTestDAO* _instance;
  friend class DAOHelper<BrandedFareTestDAO>;
  static DAOHelper<BrandedFareTestDAO> _helper;
  BrandedFareTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : BrandedFareDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<BrandedFareTestDAO>
BrandedFareTestDAO::_helper(_name);
BrandedFareTestDAO*
BrandedFareTestDAO::_instance(0);

class BrandedFareHistoricalTestDAO : public BrandedFareHistoricalDAO
{
public:
  static BrandedFareHistoricalTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return BrandedFareHistoricalDAO::cache(); }

  virtual std::vector<BrandedFare*>* create(BrandedFareHistoricalKey key)
  {
    static SequenceNumber seqNo(107);
    std::vector<BrandedFare*>* vect(new std::vector<BrandedFare*>);
    BrandedFare* bf1(new BrandedFare);
    BrandedFare::dummyData(*bf1);
    bf1->vendor() = key._a;
    bf1->carrier() = key._b;
    bf1->seqNo() = seqNo++;
    vect->push_back(bf1);
    BrandedFare* bf2(new BrandedFare);
    BrandedFare::dummyData(*bf2);
    bf2->vendor() = key._a;
    bf2->carrier() = key._b;
    bf2->seqNo() = seqNo++;
    vect->push_back(bf2);
    return vect;
  }

private:
  static BrandedFareHistoricalTestDAO* _instance;
  friend class DAOHelper<BrandedFareHistoricalTestDAO>;
  static DAOHelper<BrandedFareHistoricalTestDAO> _helper;
  BrandedFareHistoricalTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : BrandedFareHistoricalDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<BrandedFareHistoricalTestDAO>
BrandedFareHistoricalTestDAO::_helper(_name);
BrandedFareHistoricalTestDAO*
BrandedFareHistoricalTestDAO::_instance(0);

class BrandedFareDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrandedFareDAOTest);
  CPPUNIT_TEST(testGet);
  CPPUNIT_TEST(testInvalidate);
  CPPUNIT_TEST(testHistoricalGet);
  CPPUNIT_TEST(testHistoricalInvalidate);
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
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    MockGlobal::setStartTime();
  }

  void tearDown() { _memHandle.clear(); }

  void testGet()
  {
    BrandedFareTestDAO& dao(BrandedFareTestDAO::instance());
    DataHandle dh;
    DateTime ticketDate(time(NULL));
    VendorCode vendor("ATP");
    CarrierCode carrier("VA");
    const std::vector<BrandedFare*>& res1(dao.get(dh.deleteList(), vendor, carrier, ticketDate));
    CPPUNIT_ASSERT(2 == res1.size());
    // successive calls should bring the same entries
    const std::vector<BrandedFare*>& res2(dao.get(dh.deleteList(), vendor, carrier, ticketDate));
    CPPUNIT_ASSERT(res1.size() == res2.size());
    for (size_t i = 0; i < res1.size(); ++i)
    {
      CPPUNIT_ASSERT(*res1[i] == *res2[i]);
    }
  }

  void testInvalidate()
  {
    BrandedFareTestDAO& dao(BrandedFareTestDAO::instance());
    DataHandle dh;
    VendorCode vendor("SITA");
    CarrierCode carrier("AA");
    DateTime ticketDate(time(NULL));
    dao.get(dh.deleteList(), vendor, carrier, ticketDate); // fill the cache
    BrandedFareKey key(vendor, carrier);
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }

  void testHistoricalGet()
  {
    BrandedFareHistoricalTestDAO& dao(BrandedFareHistoricalTestDAO::instance());
    DataHandle dh;
    DateTime ticketDate(time(NULL));
    VendorCode vendor("ATP");
    CarrierCode carrier("VA");
    const std::vector<BrandedFare*>& res1(dao.get(dh.deleteList(), vendor, carrier, ticketDate));
    CPPUNIT_ASSERT(2 == res1.size());
    // successive calls should bring the same entries
    const std::vector<BrandedFare*>& res2(dao.get(dh.deleteList(), vendor, carrier, ticketDate));
    CPPUNIT_ASSERT(res1.size() == res2.size());
    for (size_t i = 0; i < res1.size(); ++i)
    {
      CPPUNIT_ASSERT(*res1[i] == *res2[i]);
    }
  }

  void testHistoricalInvalidate()
  {
    BrandedFareHistoricalTestDAO& dao(BrandedFareHistoricalTestDAO::instance());
    DataHandle dh;
    VendorCode vendor("SITA");
    CarrierCode carrier("AA");
    DateTime ticketDate(time(NULL));
    ObjectKey objectKey;
    objectKey.setValue("VENDOR", vendor);
    objectKey.setValue("CARRIER", carrier);
    BrandedFareHistoricalKey key;
    CPPUNIT_ASSERT(dao.translateKey(objectKey, key, ticketDate));
    dao.get(dh.deleteList(), vendor, carrier, ticketDate); // fill the cache
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BrandedFareDAOTest);
}
