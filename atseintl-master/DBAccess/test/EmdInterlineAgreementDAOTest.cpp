#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "DBAccess/EmdInterlineAgreementInfo.h"
#include "DBAccess/EmdInterlineAgreementDAO.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class EmdInterlineAgreementTestDAO : public EmdInterlineAgreementDAO
{
public:
  static EmdInterlineAgreementTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return EmdInterlineAgreementDAO::cache(); }

  virtual std::vector<EmdInterlineAgreementInfo*>* create(EmdInterlineAgreementKey key)
  {
    std::vector<EmdInterlineAgreementInfo*>* eiaList(
        new std::vector<EmdInterlineAgreementInfo*>);
    const DateTime today    = DateTime::localTime();
    const DateTime effDate  = today.subtractDays(30);
    const DateTime discDate = today.addDays(30);

    EmdInterlineAgreementInfo* eia1(new EmdInterlineAgreementInfo);
    eia1->setCountryCode(key._a);
    eia1->setGds(key._b);
    eia1->setValidatingCarrier(key._c);
    eia1->setParticipatingCarrier("XX");
    eia1->setEffDate(effDate);
    eia1->setCreateDate(effDate);
    eia1->setDiscDate(discDate);
    eia1->setExpireDate(discDate);
    eiaList->push_back(eia1);

    EmdInterlineAgreementInfo* eia2(new EmdInterlineAgreementInfo);
    eia2->setCountryCode(key._a);
    eia2->setGds(key._b);
    eia2->setValidatingCarrier(key._c);
    eia2->setParticipatingCarrier("YY");
    eia2->setEffDate(effDate);
    eia2->setCreateDate(effDate);
    eia2->setDiscDate(discDate);
    eia2->setExpireDate(discDate);
    eiaList->push_back(eia2);

    EmdInterlineAgreementInfo* eia3(new EmdInterlineAgreementInfo);
    eia3->setCountryCode(key._a);
    eia3->setGds(key._b);
    eia3->setValidatingCarrier(key._c);
    eia3->setParticipatingCarrier("ZZ");
    eia3->setEffDate(effDate);
    eia3->setCreateDate(effDate);
    eia3->setDiscDate(discDate);
    eia3->setExpireDate(discDate);
    eiaList->push_back(eia3);
    return eiaList;
  }

private:
  static EmdInterlineAgreementTestDAO* _instance;
  friend class DAOHelper<EmdInterlineAgreementTestDAO>;
  static DAOHelper<EmdInterlineAgreementTestDAO> _helper;
  EmdInterlineAgreementTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : EmdInterlineAgreementDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<EmdInterlineAgreementTestDAO>
EmdInterlineAgreementTestDAO::_helper(_name);
EmdInterlineAgreementTestDAO*
EmdInterlineAgreementTestDAO::_instance(0);

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

class EmdInterlineAgreementDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EmdInterlineAgreementDAOTest);
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
    EmdInterlineAgreementTestDAO& dao(EmdInterlineAgreementTestDAO::instance());
    DataHandle dh;
    const NationCode countryTh = "TH";
    const CrsCode gds1S = "1S";
    const CarrierCode validatingCarrierPg = "PG";
    const CarrierCode participatingCarrierXx = "XX";
    const CarrierCode participatingCarrierYy = "YY";
    const CarrierCode participatingCarrierZz = "ZZ";
    const DateTime ticketDate = DateTime::localTime();

    const std::vector<EmdInterlineAgreementInfo*>& eiaList(
        dao.get(dh.deleteList(), countryTh, gds1S, validatingCarrierPg, ticketDate));

    const size_t resultSetSize = 3;
    CPPUNIT_ASSERT_EQUAL(resultSetSize, eiaList.size());

    CPPUNIT_ASSERT_EQUAL(countryTh, eiaList[0]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(gds1S, eiaList[0]->getGds());
    CPPUNIT_ASSERT_EQUAL(validatingCarrierPg, eiaList[0]->getValidatingCarrier());
    CPPUNIT_ASSERT_EQUAL(participatingCarrierXx, eiaList[0]->getParticipatingCarrier());

    CPPUNIT_ASSERT_EQUAL(countryTh, eiaList[1]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(gds1S, eiaList[1]->getGds());
    CPPUNIT_ASSERT_EQUAL(validatingCarrierPg, eiaList[1]->getValidatingCarrier());
    CPPUNIT_ASSERT_EQUAL(participatingCarrierYy, eiaList[1]->getParticipatingCarrier());

    CPPUNIT_ASSERT_EQUAL(countryTh, eiaList[2]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(gds1S, eiaList[2]->getGds());
    CPPUNIT_ASSERT_EQUAL(validatingCarrierPg, eiaList[2]->getValidatingCarrier());
    CPPUNIT_ASSERT_EQUAL(participatingCarrierZz, eiaList[2]->getParticipatingCarrier());

    // successive calls should bring the same entries
    const std::vector<EmdInterlineAgreementInfo*>& eiaList2(
        dao.get(dh.deleteList(), countryTh, gds1S, validatingCarrierPg, ticketDate));
    CPPUNIT_ASSERT(eiaList.size() == eiaList2.size());

    for (size_t i = 0; i < eiaList.size(); ++i)
    {
      CPPUNIT_ASSERT(*eiaList[i] == *eiaList2[i]);
    }
  }

  void testInvalidate()
  {
    EmdInterlineAgreementTestDAO& dao(EmdInterlineAgreementTestDAO::instance());
    DataHandle dh;
    const NationCode countryTh = "TH";
    const CrsCode gds1S = "1S";
    const CarrierCode validatingCarrierPg = "PG";
    const DateTime ticketDate = DateTime::localTime();
    dao.get(dh.deleteList(), countryTh, gds1S, validatingCarrierPg, ticketDate);
    EmdInterlineAgreementKey key(countryTh, gds1S, validatingCarrierPg);
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(EmdInterlineAgreementDAOTest);
}
