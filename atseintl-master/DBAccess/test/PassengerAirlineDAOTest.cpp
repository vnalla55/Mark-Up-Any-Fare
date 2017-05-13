#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "DBAccess/PassengerAirlineInfo.h"
#include "DBAccess/PassengerAirlineDAO.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class PassengerAirlineTestDAO : public PassengerAirlineDAO
{
public:
  static PassengerAirlineTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return PassengerAirlineDAO::cache(); }

  virtual std::vector<PassengerAirlineInfo*>*
  create(PassengerAirlineKey key)
  {
    std::vector<PassengerAirlineInfo*>* paiList(new std::vector<PassengerAirlineInfo*>);
    PassengerAirlineInfo* pai = new PassengerAirlineInfo();
    pai->setAirlineCode(key._a);
    pai->setAirlineName("American Airlines Inc.");
    pai->setEffDate(DateTime("2001-FEB-01", 0));
    pai->setDiscDate(DateTime("2001-MAR-01", 0));
    pai->setCreateDate(DateTime("2001-FEB-01", 0));
    pai->setExpireDate(DateTime("2001-MAR-01", 0));
    paiList->push_back(pai);
    return paiList;
  }

private:
  static PassengerAirlineTestDAO* _instance;
  friend class DAOHelper<PassengerAirlineTestDAO>;
  static DAOHelper<PassengerAirlineTestDAO> _helper;
  PassengerAirlineTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : PassengerAirlineDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<PassengerAirlineTestDAO>
PassengerAirlineTestDAO::_helper(_name);
PassengerAirlineTestDAO*
PassengerAirlineTestDAO::_instance(0);

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

class PassengerAirlineDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PassengerAirlineDAOTest);
  CPPUNIT_TEST(testGet);
  CPPUNIT_TEST(testInvalidate);
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
    PassengerAirlineTestDAO& dao(PassengerAirlineTestDAO::instance());
    DataHandle dh;
    const CarrierCode airlineCode = "AA";
    const std::string airlineName = "American Airlines Inc.";
    const DateTime dt("2001-FEB-15", 0);

    const PassengerAirlineInfo* pai = dao.get(dh.deleteList(), airlineCode, dt, dt);

    CPPUNIT_ASSERT(pai);

    CPPUNIT_ASSERT_EQUAL(airlineCode, pai->getAirlineCode());
    CPPUNIT_ASSERT_EQUAL(airlineName, pai->getAirlineName());

    // successive calls should bring the same entries
    const PassengerAirlineInfo* pai2 = dao.get(dh.deleteList(), airlineCode, dt, dt);
    CPPUNIT_ASSERT(pai2);
    CPPUNIT_ASSERT(*pai == *pai2);
  }

  void testInvalidate()
  {
    PassengerAirlineTestDAO& dao(PassengerAirlineTestDAO::instance());
    DataHandle dh;
    const CarrierCode airlineCode = "AA";
    const DateTime dt(DateTime::localTime());
    dao.get(dh.deleteList(), airlineCode, dt, dt);
    PassengerAirlineKey key(airlineCode);
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(PassengerAirlineDAOTest);
}
