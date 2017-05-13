#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "Common/ValidatingCxrConst.h"
#include "DBAccess/AirlineCountrySettlementPlanInfo.h"
#include "DBAccess/AirlineCountrySettlementPlanDAO.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class AirlineCountrySettlementPlanTestDAO : public AirlineCountrySettlementPlanDAO
{
public:
  static AirlineCountrySettlementPlanTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return AirlineCountrySettlementPlanDAO::cache(); }

  virtual std::vector<AirlineCountrySettlementPlanInfo*>*
  create(AirlineCountrySettlementPlanKey key)
  {
    std::vector<AirlineCountrySettlementPlanInfo*>* acspList(
        new std::vector<AirlineCountrySettlementPlanInfo*>);
    const DateTime today    = DateTime::localTime();
    const DateTime effDate  = today.subtractDays(30);
    const DateTime discDate = today.addDays(30);

    AirlineCountrySettlementPlanInfo* acsp1(new AirlineCountrySettlementPlanInfo);
    acsp1->setCountryCode(key._a);
    acsp1->setGds(key._b);
    acsp1->setAirline(key._c);
    acsp1->setSettlementPlanType(key._d);
    acsp1->setPreferredTicketingMethod(vcx::TM_ELECTRONIC);
    acsp1->setRequiredTicketingMethod(vcx::TM_ELECTRONIC);
    acsp1->setEffDate(effDate);
    acsp1->setCreateDate(effDate);
    acsp1->setDiscDate(discDate);
    acsp1->setExpireDate(discDate);
    acspList->push_back(acsp1);

    AirlineCountrySettlementPlanInfo* acsp2(new AirlineCountrySettlementPlanInfo);
    acsp2->setCountryCode(key._a);
    acsp2->setGds(key._b);
    acsp2->setAirline(key._c);
    acsp2->setSettlementPlanType(key._d);
    acsp2->setPreferredTicketingMethod(vcx::TM_PAPER);
    acsp2->setRequiredTicketingMethod(vcx::TM_PAPER);
    acsp2->setEffDate(effDate);
    acsp2->setCreateDate(effDate);
    acsp2->setDiscDate(discDate);
    acsp2->setExpireDate(discDate);
    acspList->push_back(acsp2);
    return acspList;
  }

private:
  static AirlineCountrySettlementPlanTestDAO* _instance;
  friend class DAOHelper<AirlineCountrySettlementPlanTestDAO>;
  static DAOHelper<AirlineCountrySettlementPlanTestDAO> _helper;
  AirlineCountrySettlementPlanTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : AirlineCountrySettlementPlanDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<AirlineCountrySettlementPlanTestDAO>
AirlineCountrySettlementPlanTestDAO::_helper(_name);
AirlineCountrySettlementPlanTestDAO*
AirlineCountrySettlementPlanTestDAO::_instance(0);

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

class AirlineCountrySettlementPlanDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AirlineCountrySettlementPlanDAOTest);
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
    AirlineCountrySettlementPlanTestDAO& dao(AirlineCountrySettlementPlanTestDAO::instance());
    DataHandle dh;
    const NationCode countryTh = "TH";
    const CrsCode gds1S = "1S";
    const CarrierCode airlinePg = "PG";
    const SettlementPlanType spTypeBsp = "BSP";
    const DateTime ticketDate = DateTime::localTime();

    const std::vector<AirlineCountrySettlementPlanInfo*>& acspList(
        dao.get(dh.deleteList(), countryTh, gds1S, airlinePg, spTypeBsp, ticketDate));

    CPPUNIT_ASSERT(2 == acspList.size());

    const char paperTicket = 'P';
    const char electronicTicket = 'E';

    CPPUNIT_ASSERT_EQUAL(countryTh, acspList[0]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(gds1S, acspList[0]->getGds());
    CPPUNIT_ASSERT_EQUAL(airlinePg, acspList[0]->getAirline());
    CPPUNIT_ASSERT_EQUAL(spTypeBsp, acspList[0]->getSettlementPlanType());
    CPPUNIT_ASSERT_EQUAL(electronicTicket, acspList[0]->getPreferredTicketingMethod());
    CPPUNIT_ASSERT_EQUAL(electronicTicket, acspList[0]->getRequiredTicketingMethod());

    CPPUNIT_ASSERT_EQUAL(countryTh, acspList[1]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(gds1S, acspList[1]->getGds());
    CPPUNIT_ASSERT_EQUAL(airlinePg, acspList[1]->getAirline());
    CPPUNIT_ASSERT_EQUAL(spTypeBsp, acspList[1]->getSettlementPlanType());
    CPPUNIT_ASSERT_EQUAL(paperTicket, acspList[1]->getPreferredTicketingMethod());
    CPPUNIT_ASSERT_EQUAL(paperTicket, acspList[1]->getRequiredTicketingMethod());

    // successive calls should bring the same entries
    const std::vector<AirlineCountrySettlementPlanInfo*>& acspList2(
        dao.get(dh.deleteList(), countryTh, gds1S, airlinePg, spTypeBsp, ticketDate));
    CPPUNIT_ASSERT(acspList.size() == acspList2.size());

    for (size_t i = 0; i < acspList.size(); ++i)
    {
      CPPUNIT_ASSERT(*acspList[i] == *acspList2[i]);
    }
  }

  void testInvalidate()
  {
    AirlineCountrySettlementPlanTestDAO& dao(AirlineCountrySettlementPlanTestDAO::instance());
    DataHandle dh;
    const NationCode countryTh = "TH";
    const CrsCode gds1S = "1S";
    const CarrierCode airlinePg = "PG";
    const SettlementPlanType spTypeBsp = "BSP";
    const DateTime ticketDate = DateTime::localTime();

    dao.get(dh.deleteList(), countryTh, gds1S, airlinePg, spTypeBsp, ticketDate);
    AirlineCountrySettlementPlanKey key(countryTh, gds1S, airlinePg, spTypeBsp);
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(AirlineCountrySettlementPlanDAOTest);
}
