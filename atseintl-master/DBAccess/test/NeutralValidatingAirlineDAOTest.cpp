#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "DBAccess/NeutralValidatingAirlineInfo.h"
#include "DBAccess/NeutralValidatingAirlineDAO.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class NeutralValidatingAirlineTestDAO : public NeutralValidatingAirlineDAO
{
public:
  static NeutralValidatingAirlineTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return NeutralValidatingAirlineDAO::cache(); }

  virtual std::vector<NeutralValidatingAirlineInfo*>* create(NeutralValidatingAirlineKey key)
  {
    std::vector<NeutralValidatingAirlineInfo*>* nvaList(
        new std::vector<NeutralValidatingAirlineInfo*>);
    const DateTime today    = DateTime::localTime();
    const DateTime effDate  = today.subtractDays(30);
    const DateTime discDate = today.addDays(30);

    NeutralValidatingAirlineInfo* nva1(new NeutralValidatingAirlineInfo);
    nva1->setCountryCode(key._a);
    nva1->setGds(key._b);
    nva1->setSettlementPlanType(key._c);
    nva1->setAirline("PG");
    nva1->setEffDate(effDate);
    nva1->setCreateDate(effDate);
    nva1->setDiscDate(discDate);
    nva1->setExpireDate(discDate);
    nvaList->push_back(nva1);

    NeutralValidatingAirlineInfo* nva2(new NeutralValidatingAirlineInfo);
    nva2->setCountryCode(key._a);
    nva2->setGds(key._b);
    nva2->setSettlementPlanType(key._c);
    nva2->setAirline("IE");
    nva2->setEffDate(effDate);
    nva2->setCreateDate(effDate);
    nva2->setDiscDate(discDate);
    nva2->setExpireDate(discDate);
    nvaList->push_back(nva2);
    return nvaList;
  }

private:
  static NeutralValidatingAirlineTestDAO* _instance;
  friend class DAOHelper<NeutralValidatingAirlineTestDAO>;
  static DAOHelper<NeutralValidatingAirlineTestDAO> _helper;
  NeutralValidatingAirlineTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : NeutralValidatingAirlineDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<NeutralValidatingAirlineTestDAO>
NeutralValidatingAirlineTestDAO::_helper(_name);
NeutralValidatingAirlineTestDAO*
NeutralValidatingAirlineTestDAO::_instance(0);

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

class NeutralValidatingAirlineDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NeutralValidatingAirlineDAOTest);
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
    NeutralValidatingAirlineTestDAO& dao(NeutralValidatingAirlineTestDAO::instance());
    DataHandle dh;
    const NationCode countryTh = "TH";
    const CrsCode gds1S = "1S";
    const SettlementPlanType spTypeStu = "STU";

    const CarrierCode airlinePg = "PG";
    const CarrierCode airlineIe = "IE";
    const DateTime ticketDate = DateTime::localTime();

    const std::vector<NeutralValidatingAirlineInfo*>& nvaList(
        dao.get(dh.deleteList(), countryTh, gds1S, spTypeStu, ticketDate));

    CPPUNIT_ASSERT(2 == nvaList.size());

    CPPUNIT_ASSERT_EQUAL(countryTh, nvaList[0]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(gds1S, nvaList[0]->getGds());
    CPPUNIT_ASSERT_EQUAL(spTypeStu, nvaList[0]->getSettlementPlanType());
    CPPUNIT_ASSERT_EQUAL(airlinePg, nvaList[0]->getAirline());

    CPPUNIT_ASSERT_EQUAL(countryTh, nvaList[1]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(gds1S, nvaList[1]->getGds());
    CPPUNIT_ASSERT_EQUAL(spTypeStu, nvaList[1]->getSettlementPlanType());
    CPPUNIT_ASSERT_EQUAL(airlineIe, nvaList[1]->getAirline());

    // successive calls should bring the same entries
    const std::vector<NeutralValidatingAirlineInfo*>& nvaList2(
        dao.get(dh.deleteList(), countryTh, gds1S, spTypeStu, ticketDate));
    CPPUNIT_ASSERT(nvaList.size() == nvaList2.size());

    for (size_t i = 0; i < nvaList.size(); ++i)
    {
      CPPUNIT_ASSERT(*nvaList[i] == *nvaList2[i]);
    }
  }

  void testInvalidate()
  {
    NeutralValidatingAirlineTestDAO& dao(NeutralValidatingAirlineTestDAO::instance());
    DataHandle dh;
    const NationCode countryTh = "TH";
    const CrsCode gds1S = "1S";
    const SettlementPlanType spTypeStu = "STU";
    const DateTime ticketDate = DateTime::localTime();
    dao.get(dh.deleteList(), countryTh, gds1S, spTypeStu, ticketDate);
    NeutralValidatingAirlineKey key(countryTh, gds1S, spTypeStu);
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(NeutralValidatingAirlineDAOTest);
}
