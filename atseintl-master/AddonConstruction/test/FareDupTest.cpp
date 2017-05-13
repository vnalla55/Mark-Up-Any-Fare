#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "AddonConstruction/ConstructedFare.h"
#include "AddonConstruction/FareDup.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/test/ConstructionJobMock.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using boost::assign::operator+=;

class FareDupTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDupTest);
  CPPUNIT_TEST(testIsEqualPass);
  CPPUNIT_TEST(testIsEqualFailFareTariff);
  CPPUNIT_TEST(testIsEqualFailGlobalDir);
  CPPUNIT_TEST(testIsEqualFailFareClass);
  CPPUNIT_TEST(testIsEqualFailVendor);
  CPPUNIT_TEST(testIsEqualFailCarriers);
  CPPUNIT_TEST(testIsEqualFailCurrency);
  CPPUNIT_TEST(testIsEqualFailRuleNumber);
  CPPUNIT_TEST(testIsEqualFailMarkets);
  CPPUNIT_TEST(testIsEqualFailDirectionality);
  CPPUNIT_TEST(testIsEqualFailRoutings);
  CPPUNIT_TEST(testIsEqualFailFootnotes);
  CPPUNIT_TEST_SUITE_END();

  ConstructedFareInfo* _cfi1;
  ConstructedFareInfo* _cfi2;
  TestMemHandle _memH;

public:
  void setUp()
  {
    _cfi1 = _memH.create<ConstructedFareInfo>();
    _cfi2 = _memH.create<ConstructedFareInfo>();
  }

  void tearDown() { _memH.clear(); }

  void testIsEqualPass() { CPPUNIT_ASSERT_EQUAL(FareDup::EQUAL, FareDup::isEqual(*_cfi1, *_cfi2)); }

  void testIsEqualFailFareTariff()
  {
    _cfi1->fareInfo().fareTariff() = 0;
    _cfi2->fareInfo().fareTariff() = 1;
    CPPUNIT_ASSERT_EQUAL(FareDup::FARE_TARIFF_NE, FareDup::isEqual(*_cfi1, *_cfi2));
  }

  void testIsEqualFailFareClass()
  {
    _cfi1->fareInfo().fareClass() = "ABCD";
    _cfi2->fareInfo().fareClass() = "XYZQ";
    CPPUNIT_ASSERT_EQUAL(FareDup::FARE_CLASS_NE, FareDup::isEqual(*_cfi1, *_cfi2));
  }

  void testIsEqualFailGlobalDir()
  {
    _cfi1->fareInfo().globalDirection() = GlobalDirection::AF;
    _cfi2->fareInfo().globalDirection() = GlobalDirection::AP;
    CPPUNIT_ASSERT_EQUAL(FareDup::GLOBAL_DIR_NE, FareDup::isEqual(*_cfi1, *_cfi2));
  }

  void testIsEqualFailVendor()
  {
    _cfi1->fareInfo().vendor() = ATPCO_VENDOR_CODE;
    _cfi2->fareInfo().vendor() = SITA_VENDOR_CODE;
    CPPUNIT_ASSERT_EQUAL(FareDup::VENDORS_NE, FareDup::isEqual(*_cfi1, *_cfi2));
  }

  void testIsEqualFailCarriers()
  {
    _cfi1->fareInfo().carrier() = "AA";
    _cfi2->fareInfo().carrier() = "LH";
    CPPUNIT_ASSERT_EQUAL(FareDup::CARRIERS_NE, FareDup::isEqual(*_cfi1, *_cfi2));
  }

  void testIsEqualFailCurrency()
  {
    _cfi1->fareInfo().currency() = NUC;
    _cfi2->fareInfo().currency() = USD;
    CPPUNIT_ASSERT_EQUAL(FareDup::CURRENCY_NE, FareDup::isEqual(*_cfi1, *_cfi2));
  }

  void testIsEqualFailRuleNumber()
  {
    _cfi1->fareInfo().ruleNumber() = "0000";
    _cfi2->fareInfo().ruleNumber() = "1234";
    CPPUNIT_ASSERT_EQUAL(FareDup::RULE_NUMBER_NE, FareDup::isEqual(*_cfi1, *_cfi2));
  }

  void testIsEqualFailMarkets()
  {
    _cfi1->fareInfo().market1() = "DFW";
    _cfi2->fareInfo().market1() = "KRK";
    CPPUNIT_ASSERT_EQUAL(FareDup::MARKETS_NE, FareDup::isEqual(*_cfi1, *_cfi2));
  }

  void testIsEqualFailDirectionality()
  {
    _cfi1->fareInfo().directionality() = FROM;
    _cfi2->fareInfo().directionality() = TO;
    CPPUNIT_ASSERT_EQUAL(FareDup::DIRECTIONALITY_NE, FareDup::isEqual(*_cfi1, *_cfi2));
  }

  void testIsEqualFailRoutings()
  {
    _cfi1->fareInfo().routingNumber() = "059";
    _cfi2->fareInfo().routingNumber() = "134";
    CPPUNIT_ASSERT_EQUAL(FareDup::ROUTINGS_NE, FareDup::isEqual(*_cfi1, *_cfi2));
  }

  void testIsEqualFailFootnotes()
  {
    _cfi1->origAddonFootNote1() = "3B";
    _cfi2->origAddonFootNote1() = "1V";
    CPPUNIT_ASSERT_EQUAL(FareDup::FOOTNOTES_NE, FareDup::isEqual(*_cfi1, *_cfi2));
  }

  ConstructedFareInfo* getConstructedFareInfo(const MoneyAmount& constructedNucAmount,
                                              const DateTime& effDate,
                                              const DateTime& expireDate,
                                              const CarrierCode& carrier)
  {
    ConstructedFareInfo* cfi = _memH.create<ConstructedFareInfo>();
    ConstructedFareInfo::dummyData(*cfi);
    cfi->constructedNucAmount() = constructedNucAmount;
    cfi->fareInfo().effDate() = effDate;
    cfi->fareInfo().expireDate() = expireDate;
    cfi->fareInfo().carrier() = carrier;
    return cfi;
  }

  struct IsEqual
  {
    IsEqual(const ConstructedFareInfo* a) : _a(a) {}
    bool operator()(const ConstructedFareInfo* b) const
    {
      return FareDup::EQUAL == FareDup::isEqual(*_a, *b);
    }

  protected:
    const ConstructedFareInfo* _a;
  };
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareDupTest);
} // end namespace tse
