#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "Common/TseEnums.h"
#include "Common/ValidatingCxrConst.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/CountrySettlementPlanDAO.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class CountrySettlementPlanTestDAO : public CountrySettlementPlanDAO
{
public:
  CountrySettlementPlanTestDAO() {}
  virtual ~CountrySettlementPlanTestDAO() {}

  static CountrySettlementPlanTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return CountrySettlementPlanDAO::cache(); }

  virtual std::vector<CountrySettlementPlanInfo*>* create(CountrySettlementPlanKey key)
  {
    std::vector<CountrySettlementPlanInfo*>* cspVec(new std::vector<CountrySettlementPlanInfo*>);

    const DateTime today    = DateTime::localTime();
    const DateTime effDate  = today.subtractDays(30);
    const DateTime discDate = today.addDays(30);

    CountrySettlementPlanInfo* info1(new CountrySettlementPlanInfo);
    info1->setCountryCode(key._a);
    info1->setSettlementPlanTypeCode("BSP");
    info1->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    info1->setRequiredTicketingMethod(vcx::TM_EMPTY);
    info1->setEffDate(effDate);
    info1->setCreateDate(effDate);
    info1->setDiscDate(discDate);
    info1->setExpireDate(discDate);
    cspVec->push_back(info1);

    CountrySettlementPlanInfo* info2(new CountrySettlementPlanInfo);
    info2->setCountryCode(key._a);
    info2->setSettlementPlanTypeCode("STU");
    info2->setPreferredTicketingMethod(vcx::TM_EMPTY);
    info2->setRequiredTicketingMethod(vcx::TM_PAPER);
    info2->setEffDate(effDate);
    info2->setCreateDate(effDate);
    info2->setDiscDate(discDate);
    info2->setExpireDate(discDate);
    cspVec->push_back(info2);
    return cspVec;
  }

private:
  static CountrySettlementPlanTestDAO* _instance;
  friend class DAOHelper<CountrySettlementPlanTestDAO>;
  static DAOHelper<CountrySettlementPlanTestDAO> _helper;
  CountrySettlementPlanTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : CountrySettlementPlanDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<CountrySettlementPlanTestDAO>
CountrySettlementPlanTestDAO::_helper(_name);
CountrySettlementPlanTestDAO*
CountrySettlementPlanTestDAO::_instance(0);

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

class CountrySettlementPlanDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CountrySettlementPlanDAOTest);
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
    CountrySettlementPlanTestDAO& dao(CountrySettlementPlanTestDAO::instance());
    DataHandle dh;
    const NationCode country("TH");
    const DateTime ticketDate    = DateTime::localTime();
    const std::vector<CountrySettlementPlanInfo*>& csp1(
        dao.get(dh.deleteList(), country, ticketDate));
    CPPUNIT_ASSERT(2 == csp1.size());

    const SettlementPlanType bsp("BSP");
    const SettlementPlanType stu("STU");
    const Indicator emptyTicketMethod = vcx::TM_EMPTY;
    const Indicator electronicTicketMethod = vcx::TM_ELECTRONIC;
    const Indicator paperTicketMethod = vcx::TM_PAPER;

    CPPUNIT_ASSERT_EQUAL(country, csp1[0]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(bsp, csp1[0]->getSettlementPlanTypeCode());
    CPPUNIT_ASSERT_EQUAL(electronicTicketMethod, csp1[0]->getPreferredTicketingMethod());
    CPPUNIT_ASSERT_EQUAL(emptyTicketMethod, csp1[0]->getRequiredTicketingMethod());

    CPPUNIT_ASSERT_EQUAL(country, csp1[1]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(stu, csp1[1]->getSettlementPlanTypeCode());
    CPPUNIT_ASSERT_EQUAL(emptyTicketMethod, csp1[1]->getPreferredTicketingMethod());
    CPPUNIT_ASSERT_EQUAL(paperTicketMethod, csp1[1]->getRequiredTicketingMethod());

    // successive calls should bring the same entries
    const std::vector<CountrySettlementPlanInfo*>& csp2(
        dao.get(dh.deleteList(), country, ticketDate));
    CPPUNIT_ASSERT(csp1.size() == csp2.size());

    for (size_t i = 0; i < csp1.size(); ++i)
    {
      CPPUNIT_ASSERT(*csp1[i] == *csp2[i]);
    }
  }

  void testInvalidate()
  {
    CountrySettlementPlanTestDAO& dao(CountrySettlementPlanTestDAO::instance());
    DataHandle dh;
    const NationCode country("TH");
    const DateTime ticketDate    = DateTime::localTime();
    dao.get(dh.deleteList(), country, ticketDate);
    CountrySettlementPlanKey key(country);
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CountrySettlementPlanDAOTest);
}
