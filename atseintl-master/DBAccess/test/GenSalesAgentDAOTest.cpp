#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "DBAccess/GenSalesAgentInfo.h"
#include "DBAccess/GenSalesAgentDAO.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class GenSalesAgentDAOMock : public GenSalesAgentDAO
{
public:
  static GenSalesAgentDAOMock& instance()
  {
    if (!_instance)
      _helper.init();
    return *_instance;
  }

  DAOCache& cache() { return GenSalesAgentDAO::cache(); }

  std::vector<GenSalesAgentInfo*>* create(GenSalesAgentKey key)
  {
    std::vector<GenSalesAgentInfo*>* ptr = new std::vector<GenSalesAgentInfo*>;

    const DateTime today    = DateTime::localTime();
    const DateTime effDate  = today.subtractDays(30);
    const DateTime discDate = today.addDays(30);

    GenSalesAgentInfo* obj1 = new GenSalesAgentInfo;
    obj1->setCountryCode("US");
    obj1->setGDSCode("1S");
    obj1->setSettlementPlanCode("ARC");
    obj1->setCxrCode("AA");
    obj1->setEffDate(effDate);
    obj1->setDiscDate(discDate);
    obj1->setCreateDate(effDate);
    obj1->setExpireDate(discDate);
    ptr->push_back(obj1);

    GenSalesAgentInfo* obj2 = new GenSalesAgentInfo;
    obj2->setCountryCode("FR");
    obj2->setGDSCode("1S");
    obj2->setSettlementPlanCode("BSP");
    obj2->setCxrCode("LH");
    obj2->setEffDate(effDate);
    obj2->setDiscDate(discDate);
    obj2->setCreateDate(effDate);
    obj2->setExpireDate(discDate);
    ptr->push_back(obj2);
    return ptr;
  }

private:
  static GenSalesAgentDAOMock* _instance;
  friend class DAOHelper<GenSalesAgentDAOMock>;
  static DAOHelper<GenSalesAgentDAOMock> _helper;

  GenSalesAgentDAOMock(int cacheSize = 0, const std::string& cacheType = "")
    : GenSalesAgentDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<GenSalesAgentDAOMock>
GenSalesAgentDAOMock::_helper(GenSalesAgentDAO::_name);
GenSalesAgentDAOMock*
GenSalesAgentDAOMock::_instance(0);

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

class GenSalesAgentDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(GenSalesAgentDAOTest);
  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    MockGlobal::setStartTime();
  }
  void tearDown() { _memHandle.clear(); }
  void testCreate()
  {
    GenSalesAgentDAOMock& dao(GenSalesAgentDAOMock::instance());
    DataHandle dh;

    const CrsCode gds = "1S";
    const NationCode cntry = "US";
    const SettlementPlanType sp = "ARC";
    const CarrierCode nonParticipatingCxr = "AA";
    const DateTime ticketDate = DateTime::localTime();
    const std::vector<GenSalesAgentInfo*>& gsaList =
        dao.get(dh.deleteList(), gds, cntry, sp, nonParticipatingCxr, ticketDate);
    CPPUNIT_ASSERT(gsaList.size() == 2);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(tse::GenSalesAgentDAOTest);
}
