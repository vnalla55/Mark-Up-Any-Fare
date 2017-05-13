#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "DBAccess/AirlineInterlineAgreementInfo.h"
#include "DBAccess/AirlineInterlineAgreementDAO.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"

namespace tse
{
class AirlineInterlineAgreementTestDAO : public AirlineInterlineAgreementDAO
{
public:
  static AirlineInterlineAgreementTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return AirlineInterlineAgreementDAO::cache(); }

  virtual std::vector<AirlineInterlineAgreementInfo*>* create(AirlineInterlineAgreementKey key)
  {
    std::vector<AirlineInterlineAgreementInfo*>* aiaList(
        new std::vector<AirlineInterlineAgreementInfo*>);
    const DateTime today    = DateTime::localTime();
    const DateTime effDate  = today.subtractDays(30);
    const DateTime discDate = today.addDays(30);

    AirlineInterlineAgreementInfo* aia1(new AirlineInterlineAgreementInfo);
    aia1->setCountryCode(key._a);
    aia1->setGds(key._b);
    aia1->setValidatingCarrier(key._c);
    aia1->setParticipatingCarrier("XX");
    aia1->setAgreementTypeCode(vcx::AGR_STANDARD);
    aia1->setEffDate(effDate);
    aia1->setCreateDate(effDate);
    aia1->setDiscDate(discDate);
    aia1->setExpireDate(discDate);
    aiaList->push_back(aia1);

    AirlineInterlineAgreementInfo* aia2(new AirlineInterlineAgreementInfo);
    aia2->setCountryCode(key._a);
    aia2->setGds(key._b);
    aia2->setValidatingCarrier(key._c);
    aia2->setParticipatingCarrier("YY");
    aia2->setAgreementTypeCode(vcx::AGR_THIRD_PARTY);
    aia2->setEffDate(effDate);
    aia2->setCreateDate(effDate);
    aia2->setDiscDate(discDate);
    aia2->setExpireDate(discDate);
    aiaList->push_back(aia2);

    AirlineInterlineAgreementInfo* aia3(new AirlineInterlineAgreementInfo);
    aia3->setCountryCode(key._a);
    aia3->setGds(key._b);
    aia3->setValidatingCarrier(key._c);
    aia3->setParticipatingCarrier("ZZ");
    aia3->setAgreementTypeCode(vcx::AGR_PAPER);
    aia3->setEffDate(effDate);
    aia3->setCreateDate(effDate);
    aia3->setDiscDate(discDate);
    aia3->setExpireDate(discDate);
    aiaList->push_back(aia3);
    return aiaList;
  }

private:
  static AirlineInterlineAgreementTestDAO* _instance;
  friend class DAOHelper<AirlineInterlineAgreementTestDAO>;
  static DAOHelper<AirlineInterlineAgreementTestDAO> _helper;
  AirlineInterlineAgreementTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : AirlineInterlineAgreementDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<AirlineInterlineAgreementTestDAO>
AirlineInterlineAgreementTestDAO::_helper(_name);
AirlineInterlineAgreementTestDAO*
AirlineInterlineAgreementTestDAO::_instance(0);

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

class AirlineInterlineAgreementDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AirlineInterlineAgreementDAOTest);
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
    AirlineInterlineAgreementTestDAO& dao(AirlineInterlineAgreementTestDAO::instance());
    DataHandle dh;
    const NationCode countryTh = "TH";
    const CrsCode gds1S = "1S";
    const CarrierCode validatingCarrierPg = "PG";
    const CarrierCode participatingCarrierXx = "XX";
    const CarrierCode participatingCarrierYy = "YY";
    const CarrierCode participatingCarrierZz = "ZZ";
    const DateTime ticketDate = DateTime::localTime();

    const std::vector<AirlineInterlineAgreementInfo*>& aiaList(
        dao.get(dh.deleteList(), countryTh, gds1S, validatingCarrierPg, ticketDate));

    //const size_t resultSetSize = 3;
    //CPPUNIT_ASSERT_EQUAL(resultSetSize, aiaList.size());

    CPPUNIT_ASSERT_EQUAL(countryTh, aiaList[0]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(gds1S, aiaList[0]->getGds());
    CPPUNIT_ASSERT_EQUAL(validatingCarrierPg, aiaList[0]->getValidatingCarrier());
    CPPUNIT_ASSERT_EQUAL(participatingCarrierXx, aiaList[0]->getParticipatingCarrier());
    CPPUNIT_ASSERT_EQUAL(vcx::AGR_STANDARD, aiaList[0]->getAgreementTypeCode());

    CPPUNIT_ASSERT(aiaList[0]->isStandardAgreement());
    CPPUNIT_ASSERT(!aiaList[0]->isThirdPartyAgreement());
    CPPUNIT_ASSERT(!aiaList[0]->isPaperOnlyAgreement());

    CPPUNIT_ASSERT_EQUAL(countryTh, aiaList[1]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(gds1S, aiaList[1]->getGds());
    CPPUNIT_ASSERT_EQUAL(validatingCarrierPg, aiaList[1]->getValidatingCarrier());
    CPPUNIT_ASSERT_EQUAL(participatingCarrierYy, aiaList[1]->getParticipatingCarrier());
    CPPUNIT_ASSERT_EQUAL(vcx::AGR_THIRD_PARTY, aiaList[1]->getAgreementTypeCode());

    CPPUNIT_ASSERT(aiaList[1]->isThirdPartyAgreement());
    CPPUNIT_ASSERT(!aiaList[1]->isStandardAgreement());
    CPPUNIT_ASSERT(!aiaList[1]->isPaperOnlyAgreement());

    CPPUNIT_ASSERT_EQUAL(countryTh, aiaList[2]->getCountryCode());
    CPPUNIT_ASSERT_EQUAL(gds1S, aiaList[2]->getGds());
    CPPUNIT_ASSERT_EQUAL(validatingCarrierPg, aiaList[2]->getValidatingCarrier());
    CPPUNIT_ASSERT_EQUAL(participatingCarrierZz, aiaList[2]->getParticipatingCarrier());
    CPPUNIT_ASSERT_EQUAL(vcx::AGR_PAPER, aiaList[2]->getAgreementTypeCode());

    CPPUNIT_ASSERT(aiaList[2]->isPaperOnlyAgreement());
    CPPUNIT_ASSERT(!aiaList[2]->isThirdPartyAgreement());
    CPPUNIT_ASSERT(!aiaList[2]->isStandardAgreement());

    // successive calls should bring the same entries
    const std::vector<AirlineInterlineAgreementInfo*>& aiaList2(
        dao.get(dh.deleteList(), countryTh, gds1S, validatingCarrierPg, ticketDate));
    CPPUNIT_ASSERT(aiaList.size() == aiaList2.size());

    for (size_t i = 0; i < aiaList.size(); ++i)
    {
      CPPUNIT_ASSERT(*aiaList[i] == *aiaList2[i]);
    }
  }

  void testInvalidate()
  {
    AirlineInterlineAgreementTestDAO& dao(AirlineInterlineAgreementTestDAO::instance());
    DataHandle dh;
    const NationCode countryTh = "TH";
    const CrsCode gds1S = "1S";
    const CarrierCode validatingCarrierPg = "PG";
    const DateTime ticketDate = DateTime::localTime();
    dao.get(dh.deleteList(), countryTh, gds1S, validatingCarrierPg, ticketDate);
    AirlineInterlineAgreementKey key(countryTh, gds1S, validatingCarrierPg);
    CPPUNIT_ASSERT(dao.cache().getIfResident(key) != 0);
    dao.cache().invalidate(key);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(AirlineInterlineAgreementDAOTest);
}
