#include "test/include/CppUnitHelperMacros.h"
#include "ServiceFees/CarrierStrategy.h"
#include "test/include/TestMemHandle.h"
#include "ServiceFees/OptionalFeeCollector.h"

namespace tse
{

class CarrierStrategyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CarrierStrategyTest);

  CPPUNIT_TEST(testIsIndIndWhenIndustryForMktCxr);
  CPPUNIT_TEST(testIsIndIndWhenCarrierForMktCxr);
  CPPUNIT_TEST(testIsIndIndWhenIndustryForMultipleOperCxr);
  CPPUNIT_TEST(testIsIndIndWhenCarrierForMultipleOperCxr);
  CPPUNIT_TEST(testIsIndIndWhenCarrierForOperCxr);
  CPPUNIT_TEST(testIsIndIndWhenIndustryForOperCxr);

  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  MarketingCarrierStrategy* _marketingCarrierStrategy;
  MultipleOperatingCarrierStrategy* _multipleOperatingCarrierStrategy;
  OperatingCarrierStrategy* _operatingCarrierStrategy;

public:
  void setUp()
  {
    _marketingCarrierStrategy = _memHandle.create<MarketingCarrierStrategy>();
    _multipleOperatingCarrierStrategy = _memHandle.create<MultipleOperatingCarrierStrategy>();
    _operatingCarrierStrategy = _memHandle.create<OperatingCarrierStrategy>();
  }

  void tearDown() { _memHandle.clear(); }

  void initializeSubCodes() {}

  void testIsIndIndWhenIndustryForMktCxr()
  {
    CPPUNIT_ASSERT(
        _marketingCarrierStrategy->isIndInd(OptionalFeeCollector::S5_INDCRXIND_INDUSTRY));
  }

  void testIsIndIndWhenCarrierForMktCxr()
  {
    CPPUNIT_ASSERT(
        !_marketingCarrierStrategy->isIndInd(OptionalFeeCollector::S5_INDCRXIND_CARRIER));
  }

  void testIsIndIndWhenIndustryForMultipleOperCxr()
  {
    CPPUNIT_ASSERT(
        _multipleOperatingCarrierStrategy->isIndInd(OptionalFeeCollector::S5_INDCRXIND_INDUSTRY));
  }

  void testIsIndIndWhenCarrierForMultipleOperCxr()
  {
    CPPUNIT_ASSERT(
        !_multipleOperatingCarrierStrategy->isIndInd(OptionalFeeCollector::S5_INDCRXIND_CARRIER));
  }

  void testIsIndIndWhenCarrierForOperCxr()
  {
    CPPUNIT_ASSERT(
        !_operatingCarrierStrategy->isIndInd(OptionalFeeCollector::S5_INDCRXIND_CARRIER));
  }

  void testIsIndIndWhenIndustryForOperCxr()
  {
    CPPUNIT_ASSERT(
        _operatingCarrierStrategy->isIndInd(OptionalFeeCollector::S5_INDCRXIND_INDUSTRY));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CarrierStrategyTest);
}
