#include "test/include/CppUnitHelperMacros.h"
#include <set>
#include <vector>

#include "Common/DateTime.h"
#include "Common/ItinUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseBoostStringTypes.h"

#include "Rules/RuleUtil.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

//using namespace tse;
namespace tse
{

class RuleUtilTest_FareType : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(RuleUtilTest_FareType);

  CPPUNIT_TEST(testMatchFareTypeTrueWhenEqual);
  CPPUNIT_TEST(testMatchFareTypeTrueWhenEmpty);
  CPPUNIT_TEST(testMatchFareTypeFalseWhenNotEqual);

  CPPUNIT_TEST(testMatchFareTypeTrueWhenFamilyR);
  CPPUNIT_TEST(testMatchFareTypeTrueWhenFamilyZ);
  CPPUNIT_TEST(testMatchFareTypeTrueWhenFamilyJ);
  CPPUNIT_TEST(testMatchFareTypeTrueWhenFamilyY);
  CPPUNIT_TEST(testMatchFareTypeFalseWhenFamilyZ);
  CPPUNIT_TEST(testMatchFareTypeFalseWhenFamilyY);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_request);
    _trx->getRequest()->ticketingDT() = DateTime::localTime();
  }

  void tearDown() { _memHandle.clear(); }

  void testMatchFareTypeTrueWhenEqual()
  {
    FareType ftFromRule = "RU";
    FareType ftFromFareC = "RU";
    CPPUNIT_ASSERT(RuleUtil::matchFareType(ftFromRule, ftFromFareC));
  }
  void testMatchFareTypeTrueWhenEmpty()
  {
    FareType ftFromRule;
    ftFromRule.clear();
    FareType ftFromFareC = "RU";
    CPPUNIT_ASSERT(RuleUtil::matchFareType(ftFromRule, ftFromFareC));
  }
  void testMatchFareTypeFalseWhenNotEqual()
  {
    FareType ftFromRule = "RU";
    FareType ftFromFareC = "RZ";
    CPPUNIT_ASSERT(!RuleUtil::matchFareType(ftFromRule, ftFromFareC));
  }

  void testMatchFareTypeTrueWhenFamilyR()
  {
    FareType ftFromRule = "*R";
    FareType ftFromFareC = "REX";
    CPPUNIT_ASSERT(RuleUtil::matchFareType(ftFromRule, ftFromFareC));
  }

  void testMatchFareTypeTrueWhenFamilyZ()
  {
    FareType ftFromRule = "*Z";
    FareType ftFromFareC = "ZEX";
    CPPUNIT_ASSERT(RuleUtil::matchFareType(ftFromRule, ftFromFareC));
  }

  void testMatchFareTypeTrueWhenFamilyJ()
  {
    FareType ftFromRule = "*J";
    FareType ftFromFareC = "JU";
    CPPUNIT_ASSERT(RuleUtil::matchFareType(ftFromRule, ftFromFareC));
  }

  void testMatchFareTypeTrueWhenFamilyY()
  {
    FareType ftFromRule = "*Y";
    FareType ftFromFareC = "ZEX";
    CPPUNIT_ASSERT(!RuleUtil::matchFareType(ftFromRule, ftFromFareC));
  }

  void testMatchFareTypeFalseWhenFamilyZ()
  {
    FareType ftFromRule = "*Z";
    FareType ftFromFareC = "EXP";
    CPPUNIT_ASSERT(!RuleUtil::matchFareType(ftFromRule, ftFromFareC));
  }

  void testMatchFareTypeFalseWhenFamilyY()
  {
    FareType ftFromRule = "*Y";
    FareType ftFromFareC = "JU";
    CPPUNIT_ASSERT(!RuleUtil::matchFareType(ftFromRule, ftFromFareC));
  }

private:
  PricingTrx* _trx;
  PricingRequest* _request;
  TestMemHandle _memHandle;

};

CPPUNIT_TEST_SUITE_REGISTRATION(RuleUtilTest_FareType);

}

