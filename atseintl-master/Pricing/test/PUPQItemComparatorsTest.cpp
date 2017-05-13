#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/PricingTrx.h"
#include "Pricing/PUPQItem.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{

class PUPQItemComparatorsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PUPQItemComparatorsTest);

  CPPUNIT_TEST(testMileageLeft);
  CPPUNIT_TEST(testMileageRight);

  CPPUNIT_TEST(testTotalAmountLeft);
  CPPUNIT_TEST(testTotalAmountRight);

  CPPUNIT_TEST(testCabinLeft);
  CPPUNIT_TEST(testCabinRight);

  CPPUNIT_TEST(testFareByRuleLeft);
  CPPUNIT_TEST(testFareByRuleRight);

  CPPUNIT_TEST(testPaxTypeFarePriorityLeft);
  CPPUNIT_TEST(testPaxTypeFarePriorityRight);

  CPPUNIT_TEST(testCxrTypePriorityLeft);
  CPPUNIT_TEST(testCxrTypePriorityLeft);

  CPPUNIT_TEST(testNegotiatedFarePriorityLeft);
  CPPUNIT_TEST(testNegotiatedFarePriorityRight);

  CPPUNIT_TEST(testPuPtfRankLeft);
  CPPUNIT_TEST(testPuPtfRankRight);

  CPPUNIT_TEST(testEqual);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _pricingOptions = _memHandle.create<PricingOptions>();

    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_pricingOptions);

    _leftPricingUnit = _memHandle.create<PricingUnit>();
    _leftPUPQItem = _memHandle.create<PUPQItem>();
    _leftPUPQItem->pricingUnit() = _leftPricingUnit;

    _rightPricingUnit = _memHandle.create<PricingUnit>();
    _rightPUPQItem = _memHandle.create<PUPQItem>();
    _rightPUPQItem->pricingUnit() = _rightPricingUnit;

    _greater = *_memHandle.create<PUPQItem::GreaterFare>();
    _lower = *_memHandle.create<PUPQItem::LowerFare>();
  }

  void tearDown() { _memHandle.clear(); }

  void testMileageLeft()
  {
    _leftPricingUnit->mileage() = 100;
    _rightPricingUnit->mileage() = 50;

    CPPUNIT_ASSERT(_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(!_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testMileageRight()
  {
    _leftPricingUnit->mileage() = 20;
    _rightPricingUnit->mileage() = 50;

    CPPUNIT_ASSERT(!_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testTotalAmountLeft()
  {
    setEqualMileage();

    _leftPricingUnit->setTotalPuNucAmount(150.0);
    _rightPricingUnit->setTotalPuNucAmount(200.0);

    CPPUNIT_ASSERT(!_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testTotalAmountRight()
  {
    setEqualMileage();

    _leftPricingUnit->setTotalPuNucAmount(500.0);
    _rightPricingUnit->setTotalPuNucAmount(200.0);

    CPPUNIT_ASSERT(_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(!_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testCabinLeft()
  {
    _pricingOptions->setZeroFareLogic(true);

    setEqualMileage();
    setEqualTotalAmount();

    _leftPUPQItem->_cabinPriority.setEconomyClass();
    _rightPUPQItem->_cabinPriority.setBusinessClass();

    CPPUNIT_ASSERT(!_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testCabinRight()
  {
    _pricingOptions->setZeroFareLogic(true);

    setEqualMileage();
    setEqualTotalAmount();

    _leftPUPQItem->_cabinPriority.setBusinessClass();
    _rightPUPQItem->_cabinPriority.setEconomyClass();

    CPPUNIT_ASSERT(_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(!_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testFareByRuleLeft()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();

    setFareByRulePriority(FBR_PRIORITY_MIXED, FBR_PRIORITY_FBR_ONLY);

    CPPUNIT_ASSERT(_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testFareByRuleRight()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();

    setFareByRulePriority(FBR_PRIORITY_FBR_ONLY, FBR_PRIORITY_MIXED);

    CPPUNIT_ASSERT(!_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(!_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testPaxTypeFarePriorityLeft()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();

    setPaxTypeFarePriority(PRIORITY_LOW, DEFAULT_PRIORITY);

    CPPUNIT_ASSERT(_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testPaxTypeFarePriorityRight()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();

    setPaxTypeFarePriority(DEFAULT_PRIORITY, PRIORITY_LOW);

    CPPUNIT_ASSERT(!_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(!_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testCxrTypePriorityLeft()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();

    setFareCxrTypePriority(PRIORITY_LOW, DEFAULT_PRIORITY);

    CPPUNIT_ASSERT(_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testCxrTypePriorityRight()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();

    setFareCxrTypePriority(DEFAULT_PRIORITY, PRIORITY_LOW);

    CPPUNIT_ASSERT(!_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(!_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testNegotiatedFarePriorityLeft()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualFareCxrTypePriority();

    setNegotiatedFarePriority(PRIORITY_LOW, DEFAULT_PRIORITY);

    CPPUNIT_ASSERT(_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testNegotiatedFarePriorityRight()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualFareCxrTypePriority();

    setNegotiatedFarePriority(DEFAULT_PRIORITY, PRIORITY_LOW);

    CPPUNIT_ASSERT(!_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(!_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testPuPtfRankLeft()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualFareCxrTypePriority();
    setEqualNegotiatedFarePriority();

    setPtfRank(1, 2);

    CPPUNIT_ASSERT(_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testPuPtfRankRight()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualFareCxrTypePriority();
    setEqualNegotiatedFarePriority();

    setPtfRank(2, 1);

    CPPUNIT_ASSERT(!_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(!_lower(_leftPUPQItem, _rightPUPQItem));
  }

  void testEqual()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualFareCxrTypePriority();
    setEqualNegotiatedFarePriority();
    setEqualPtfRank();

    CPPUNIT_ASSERT(!_greater(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(!_greater(_rightPUPQItem, _leftPUPQItem));

    CPPUNIT_ASSERT(!_lower(_leftPUPQItem, _rightPUPQItem));
    CPPUNIT_ASSERT(!_lower(_rightPUPQItem, _leftPUPQItem));
  }

protected:
  void setEqualMileage()
  {
    _leftPricingUnit->mileage() = 0;
    _rightPricingUnit->mileage() = 0;
  }

  void setEqualTotalAmount()
  {
    _leftPricingUnit->setTotalPuNucAmount(200.0);
    _rightPricingUnit->setTotalPuNucAmount(200.0);
  }

  void setEqualCabin()
  {
    _leftPUPQItem->_cabinPriority.setEconomyClass();
    _rightPUPQItem->_cabinPriority.setEconomyClass();
  }

  void setFareByRulePriority(FBR_PRIORITY left, FBR_PRIORITY right)
  {
    _leftPUPQItem->mutablePriorityStatus().setFareByRulePriority(left);
    _rightPUPQItem->mutablePriorityStatus().setFareByRulePriority(right);
  }

  void setEqualFareByRulePriority()
  {
    setFareByRulePriority(FBR_PRIORITY_MIXED, FBR_PRIORITY_MIXED);
  }

  void setPaxTypeFarePriority(PRIORITY left, PRIORITY right)
  {
    _leftPUPQItem->mutablePriorityStatus().setPaxTypeFarePriority(left);
    _rightPUPQItem->mutablePriorityStatus().setPaxTypeFarePriority(right);
  }

  void setEqualPaxTypeFarePriority()
  {
    setPaxTypeFarePriority(DEFAULT_PRIORITY, DEFAULT_PRIORITY);
  }

  void setFareCxrTypePriority(PRIORITY left, PRIORITY right)
  {
    _leftPUPQItem->mutablePriorityStatus().setFareCxrTypePriority(left);
    _rightPUPQItem->mutablePriorityStatus().setFareCxrTypePriority(right);
  }

  void setEqualFareCxrTypePriority()
  {
    setFareCxrTypePriority(DEFAULT_PRIORITY, DEFAULT_PRIORITY);
  }

  void setNegotiatedFarePriority(PRIORITY left, PRIORITY right)
  {
    _leftPUPQItem->mutablePriorityStatus().setNegotiatedFarePriority(left);
    _rightPUPQItem->mutablePriorityStatus().setNegotiatedFarePriority(right);
  }

  void setEqualNegotiatedFarePriority()
  {
    setNegotiatedFarePriority(DEFAULT_PRIORITY, DEFAULT_PRIORITY);
  }

  void setPtfRank(uint16_t left, uint16_t right)
  {
    _leftPUPQItem->mutablePriorityStatus().setPtfRank(left);
    _rightPUPQItem->mutablePriorityStatus().setPtfRank(right);
  }

  void setEqualPtfRank()
  {
    setPtfRank(1, 1);
  }

private:
  TestMemHandle _memHandle;

  PricingOptions* _pricingOptions;
  PricingTrx* _trx;

  PricingUnit* _leftPricingUnit;
  PUPQItem* _leftPUPQItem;

  PricingUnit* _rightPricingUnit;
  PUPQItem* _rightPUPQItem;

  PUPQItem::GreaterFare _greater;
  PUPQItem::LowerFare _lower;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PUPQItemComparatorsTest);

} // tse
