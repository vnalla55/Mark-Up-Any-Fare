#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "DBAccess/MarkupControl.h"
#include "DBAccess/MarkupControlDAO.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockDataManager.h"
#include "test/include/MockGlobal.h"

#include <algorithm>

namespace tse
{
class MarkupControlWildcardTestDAO : public MarkupControlDAO
{
public:
  static MarkupControlWildcardTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return MarkupControlDAO::cache(); }

  virtual std::vector<MarkupControl*>* create(MarkupControlKey key)
  {
    std::vector<MarkupControl*>* vect(new std::vector<MarkupControl*>);
    MarkupControl* info1(new MarkupControl);
    MarkupControl::dummyData(*info1);
    info1->rule() = key._d;
    info1->ruleTariff() = key._c;
    info1->seqNo() = key._e;
    info1->ownerPseudoCity() = key._f;
    info1->markupType() = 'U';
    info1->ownerPseudoCityType() = 'U';
    vect->push_back(info1);
    return vect;
  }

  size_t invalidate(const ObjectKey& objectKey)
  {
    size_t result(0);
    std::string vendor, carrier, ruleTariff, rule, seqNo, pcc;
    objectKey.getValue("VENDOR", vendor);
    objectKey.getValue("CARRIER", carrier);
    objectKey.getValue("RULETARIFF", ruleTariff);
    objectKey.getValue("RULE", rule);
    objectKey.getValue("SEQNO", seqNo);
    objectKey.getValue("OWNERPSEUDOCITY", pcc);
    if (ruleTariff == "?" || rule == "?" || seqNo == "?" || pcc == "?")
    {
      std::shared_ptr<std::vector<MarkupControlKey>> cacheKeys = cache().keys();
      int ruleTariffValue = -1;
      if (ruleTariff != "?")
        ruleTariffValue = atoi(ruleTariff.c_str());
      int seqNoValue = -1;
      if (seqNo != "?")
        seqNoValue = atoi(seqNo.c_str());
      for (size_t i = 0; i < cacheKeys->size(); ++i)
      {
        const MarkupControlKey& cacheKey = (*cacheKeys)[i];
        if ((vendor == cacheKey._a) && (carrier == cacheKey._b) &&
            ((ruleTariffValue == cacheKey._c) || (ruleTariff == "?")) &&
            ((rule == cacheKey._d) || (rule == "?")) &&
            ((seqNoValue == cacheKey._e) || (seqNo == "?")) &&
            ((pcc == cacheKey._f) || (pcc == "?")))
        {
          if (_loadOnUpdate)
          {
            cache().put(cacheKey, create(cacheKey));
            result += 1;
          }
          else
          {
            result += cache().invalidate(cacheKey);
          }
        }
      }
    }
    else
    {
      DataAccessObjectBase<MarkupControlKey, std::vector<MarkupControl*> >::invalidate(objectKey);
    }
    return result;
  }

private:
  static MarkupControlWildcardTestDAO* _instance;
  friend class DAOHelper<MarkupControlWildcardTestDAO>;
  static DAOHelper<MarkupControlWildcardTestDAO> _helper;
  MarkupControlWildcardTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : MarkupControlDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<MarkupControlWildcardTestDAO>
MarkupControlWildcardTestDAO::_helper(_name);
MarkupControlWildcardTestDAO*
MarkupControlWildcardTestDAO::_instance(0);

class MarkupControlWildcardHistoricalTestDAO : public MarkupControlHistoricalDAO
{
public:
  static MarkupControlWildcardHistoricalTestDAO& instance()
  {
    if (0 == _instance)
    {
      _helper.init();
    }
    return *_instance;
  }

  DAOCache& cache() { return MarkupControlHistoricalDAO::cache(); }

  virtual std::vector<MarkupControl*>* create(MarkupControlHistoricalKey key)
  {
    std::vector<MarkupControl*>* vect(new std::vector<MarkupControl*>);
    MarkupControl* info1(new MarkupControl);
    MarkupControl::dummyData(*info1);
    info1->rule() = key._d;
    info1->ruleTariff() = key._c;
    info1->seqNo() = key._e;
    info1->ownerPseudoCity() = key._f;
    info1->markupType() = 'U';
    info1->ownerPseudoCityType() = 'U';
    vect->push_back(info1);
    return vect;
  }

  virtual size_t invalidate(const ObjectKey& objectKey)
  {
    std::string vendor, carrier, ruleTariff, rule, seqNo, pcc;
    DateTime startdate, enddate;
    objectKey.getValue("VENDOR", vendor);
    objectKey.getValue("CARRIER", carrier);
    objectKey.getValue("RULETARIFF", ruleTariff);
    objectKey.getValue("RULE", rule);
    objectKey.getValue("SEQNO", seqNo);
    objectKey.getValue("OWNERPSEUDOCITY", pcc);
    size_t result(0);
    if (ruleTariff == "?" || rule == "?" || seqNo == "?" || pcc == "?")
    {
      std::shared_ptr<std::vector<MarkupControlHistoricalKey>> cacheKeys = cache().keys();
      int ruleTariffValue = -1;
      if (ruleTariff != "?")
        ruleTariffValue = atoi(ruleTariff.c_str());
      int seqNoValue = -1;
      if (seqNo != "?")
        seqNoValue = atoi(seqNo.c_str());
      for (size_t i = 0; i < cacheKeys->size(); ++i)
      {
        const MarkupControlHistoricalKey& cacheKey = (*cacheKeys)[i];
        if ((vendor == cacheKey._a) && (carrier == cacheKey._b) &&
            ((ruleTariffValue == cacheKey._c) || (ruleTariff == "?")) &&
            ((rule == cacheKey._d) || (rule == "?")) &&
            ((seqNoValue == cacheKey._e) || (seqNo == "?")) &&
            ((pcc == cacheKey._f) || (pcc == "?")))
        {
          if (_loadOnUpdate)
          {
            cache().put(cacheKey, create(cacheKey));
            result += 1;
          }
          else
          {
            result += cache().invalidate(cacheKey);
          }
        }
      }
    }
    else
    {
      result += HistoricalDataAccessObject<MarkupControlHistoricalKey,
                                 std::vector<MarkupControl*> >::invalidate(objectKey);
    }
    return result;
  }

private:
  static MarkupControlWildcardHistoricalTestDAO* _instance;
  friend class DAOHelper<MarkupControlWildcardHistoricalTestDAO>;
  static DAOHelper<MarkupControlWildcardHistoricalTestDAO> _helper;
  MarkupControlWildcardHistoricalTestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : MarkupControlHistoricalDAO(cacheSize, cacheType)
  {
  }
};

DAOHelper<MarkupControlWildcardHistoricalTestDAO>
MarkupControlWildcardHistoricalTestDAO::_helper(_name);
MarkupControlWildcardHistoricalTestDAO*
MarkupControlWildcardHistoricalTestDAO::_instance(0);

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

class MarkupControlWildcardDAOTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MarkupControlWildcardDAOTest);
  CPPUNIT_TEST(testGet);
  CPPUNIT_TEST(testInvalidate1);
  CPPUNIT_TEST(testInvalidate2);
  CPPUNIT_TEST(testInvalidate3);

  CPPUNIT_TEST(testHistoricalGet);
  CPPUNIT_TEST(testHistoricalInvalidate1);
  CPPUNIT_TEST(testHistoricalInvalidate2);
  CPPUNIT_TEST(testHistoricalInvalidate3);

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
    MarkupControlWildcardTestDAO& dao(MarkupControlWildcardTestDAO::instance());
    DataHandle dh;
    DateTime ticketDate(time(NULL));
    PseudoCityCode pcc("LMNOP");
    PseudoCityCode homePCC("STUVW");
    VendorCode vendor("ABCD");
    CarrierCode carrier("EFG");
    TariffNumber ruleTariff(2);
    RuleNumber rule("LMNO");
    int seqNo(2);
    long secondarySellerId(3);
    const std::vector<MarkupControl*>& res1(dao.get(dh.deleteList(),
                                                    pcc,
                                                    homePCC,
                                                    vendor,
                                                    carrier,
                                                    ruleTariff,
                                                    rule,
                                                    seqNo,
                                                    secondarySellerId,
                                                    ticketDate));
    CPPUNIT_ASSERT(2 == res1.size());
    // successive calls should bring the same entries
    const std::vector<MarkupControl*>& res2(dao.get(dh.deleteList(),
                                                    pcc,
                                                    homePCC,
                                                    vendor,
                                                    carrier,
                                                    ruleTariff,
                                                    rule,
                                                    seqNo,
                                                    secondarySellerId,
                                                    ticketDate));
    CPPUNIT_ASSERT(res1.size() == res2.size());
    for (size_t i = 0; i < res1.size(); ++i)
    {
      CPPUNIT_ASSERT(*res1[i] == *res2[i]);
    }
  }

  void testInvalidate1() // uses 3 wildcards
  {
    MarkupControlWildcardTestDAO& dao(MarkupControlWildcardTestDAO::instance());
    DataHandle dh;
    PseudoCityCode pcc("LMNOP");
    PseudoCityCode homePCC("STUVW");
    VendorCode vendor("ABCD");
    CarrierCode carrier("EFG");
    DateTime ticketDate(time(NULL));
    TariffNumber ruleTariff(1);
    TariffNumber ruleTariff2(2);
    RuleNumber rule("HIJK");
    RuleNumber rule2("LMNO");
    int seqNo(2);
    int seqNo2(3);
    long secondarySellerId(3);
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule2,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    std::shared_ptr<std::vector<MarkupControlKey>> cacheKeys = dao.cache().keys();
    MarkupControlKey key(vendor, carrier, ruleTariff, rule, seqNo, pcc);
    ObjectKey ok;
    dao.translateKey(key, ok);
    ok.setValue("RULETARIFF", "?");
    ok.setValue("SEQNO", "?");
    ok.setValue("OWNERPSEUDOCITY", "?");
    dao.invalidate(ok);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
    key._c = ruleTariff2;
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
    key._e = seqNo2;
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
    key._c = ruleTariff;
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
    cacheKeys = dao.cache().keys();
  }

  void testInvalidate2() // uses 1 wildcard
  {
    MarkupControlWildcardTestDAO& dao(MarkupControlWildcardTestDAO::instance());
    DataHandle dh;
    PseudoCityCode pcc("LMNOP");
    PseudoCityCode homePCC("STUVW");
    VendorCode vendor("ABCD");
    CarrierCode carrier("EFG");
    DateTime ticketDate(time(NULL));
    TariffNumber ruleTariff(1);
    TariffNumber ruleTariff2(2);
    RuleNumber rule("HIJK");
    RuleNumber rule2("LMNO");
    int seqNo(2);
    int seqNo2(3);
    long secondarySellerId(3);
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule2,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    std::shared_ptr<std::vector<MarkupControlKey>> cacheKeys = dao.cache().keys();
    MarkupControlKey key(vendor, carrier, ruleTariff, rule, seqNo, pcc);
    ObjectKey ok;
    dao.translateKey(key, ok);
    ok.setValue("RULE", "?");
    dao.invalidate(ok);
    cacheKeys = dao.cache().keys();
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
    key._d = rule2;
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }

  void testInvalidate3() // uses no wildcards
  {
    MarkupControlWildcardTestDAO& dao(MarkupControlWildcardTestDAO::instance());
    DataHandle dh;
    PseudoCityCode pcc("LMNOP");
    PseudoCityCode homePCC("STUVW");
    VendorCode vendor("ABCD");
    CarrierCode carrier("EFG");
    DateTime ticketDate(time(NULL));
    TariffNumber ruleTariff(1);
    TariffNumber ruleTariff2(2);
    RuleNumber rule("HIJK");
    RuleNumber rule2("LMNO");
    int seqNo(2);
    int seqNo2(3);
    long secondarySellerId(3);
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule2,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            homePCC,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    std::shared_ptr<std::vector<MarkupControlKey>> cacheKeys = dao.cache().keys();
    MarkupControlKey key(vendor, carrier, ruleTariff, rule, seqNo, pcc);
    ObjectKey ok;
    dao.translateKey(key, ok);
    dao.invalidate(ok);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
    cacheKeys = dao.cache().keys();
  }

  void testHistoricalGet()
  {
    MarkupControlWildcardHistoricalTestDAO& dao(MarkupControlWildcardHistoricalTestDAO::instance());
    DataHandle dh;
    DateTime ticketDate(time(NULL));
    PseudoCityCode pcc("LMNOP");
    PseudoCityCode homePCC("STUVW");
    VendorCode vendor("ABCD");
    CarrierCode carrier("EFG");
    TariffNumber ruleTariff(2);
    RuleNumber rule("LMNO");
    int seqNo(2);
    long secondarySellerId(3);
    const std::vector<MarkupControl*>& res1(dao.get(dh.deleteList(),
                                                    pcc,
                                                    homePCC,
                                                    vendor,
                                                    carrier,
                                                    ruleTariff,
                                                    rule,
                                                    seqNo,
                                                    secondarySellerId,
                                                    ticketDate));
    CPPUNIT_ASSERT(2 == res1.size());
    // successive calls should bring the same entries
    const std::vector<MarkupControl*>& res2(dao.get(dh.deleteList(),
                                                    pcc,
                                                    homePCC,
                                                    vendor,
                                                    carrier,
                                                    ruleTariff,
                                                    rule,
                                                    seqNo,
                                                    secondarySellerId,
                                                    ticketDate));
    CPPUNIT_ASSERT(res1.size() == res2.size());
    for (size_t i = 0; i < res1.size(); ++i)
    {
      CPPUNIT_ASSERT(*res1[i] == *res2[i]);
    }
  }

  void testHistoricalInvalidate1() // uses 3 wildcards
  {
    MarkupControlWildcardHistoricalTestDAO& dao(MarkupControlWildcardHistoricalTestDAO::instance());
    DataHandle dh;
    PseudoCityCode pcc("LMNOP");
    PseudoCityCode homePCC("STUVW");
    VendorCode vendor("ABCD");
    CarrierCode carrier("EFG");
    DateTime ticketDate(time(NULL));
    TariffNumber ruleTariff(1);
    TariffNumber ruleTariff2(2);
    RuleNumber rule("HIJK");
    RuleNumber rule2("LMNO");
    int seqNo(2);
    int seqNo2(3);
    long secondarySellerId(3);
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule2,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    std::shared_ptr<std::vector<MarkupControlHistoricalKey>> cacheKeys = dao.cache().keys();
    MarkupControlHistoricalKey key(vendor, carrier, ruleTariff, rule, seqNo, pcc);
    ObjectKey ok;
    dao.translateKey(key, ok);
    ok.setValue("RULE", "?");
    ok.setValue("SEQNO", "?");
    ok.setValue("OWNERPSEUDOCITY", "?");
    dao.invalidate(ok);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
    key._d = rule2;
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
    key._e = seqNo2;
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
    key._c = ruleTariff;
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
    cacheKeys = dao.cache().keys();
  }

  void testHistoricalInvalidate2() // uses 1 wildcard
  {
    MarkupControlWildcardHistoricalTestDAO& dao(MarkupControlWildcardHistoricalTestDAO::instance());
    DataHandle dh;
    PseudoCityCode pcc("LMNOP");
    PseudoCityCode homePCC("STUVW");
    VendorCode vendor("ABCD");
    CarrierCode carrier("EFG");
    DateTime ticketDate(time(NULL));
    TariffNumber ruleTariff(1);
    TariffNumber ruleTariff2(2);
    RuleNumber rule("HIJK");
    RuleNumber rule2("LMNO");
    int seqNo(2);
    int seqNo2(3);
    long secondarySellerId(3);
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule2,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    std::shared_ptr<std::vector<MarkupControlHistoricalKey>> cacheKeys = dao.cache().keys();
    MarkupControlHistoricalKey key(vendor, carrier, ruleTariff, rule, seqNo, pcc);
    ObjectKey ok;
    dao.translateKey(key, ok);
    ok.setValue("RULETARIFF", "?");
    dao.invalidate(ok);
    cacheKeys = dao.cache().keys();
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
    key._c = ruleTariff2;
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
  }

  void testHistoricalInvalidate3() // uses no wildcards
  {
    MarkupControlWildcardHistoricalTestDAO& dao(MarkupControlWildcardHistoricalTestDAO::instance());
    DataHandle dh;
    PseudoCityCode pcc("LMNOP");
    PseudoCityCode homePCC("STUVW");
    VendorCode vendor("ABCD");
    CarrierCode carrier("EFG");
    DateTime ticketDate(time(NULL));
    TariffNumber ruleTariff(1);
    TariffNumber ruleTariff2(2);
    RuleNumber rule("HIJK");
    RuleNumber rule2("LMNO");
    int seqNo(2);
    int seqNo2(3);
    long secondarySellerId(3);
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule2,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            pcc,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    dao.get(dh.deleteList(),
            homePCC,
            homePCC,
            vendor,
            carrier,
            ruleTariff2,
            rule2,
            seqNo2,
            secondarySellerId,
            ticketDate); // fill the cache
    std::shared_ptr<std::vector<MarkupControlHistoricalKey>> cacheKeys = dao.cache().keys();
    MarkupControlHistoricalKey key(vendor, carrier, ruleTariff, rule, seqNo, pcc);
    ObjectKey ok;
    dao.translateKey(key, ok);
    dao.invalidate(ok);
    CPPUNIT_ASSERT(0 == dao.cache().getIfResident(key));
    cacheKeys = dao.cache().keys();
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(MarkupControlWildcardDAOTest);
}
