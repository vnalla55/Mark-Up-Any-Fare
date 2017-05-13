#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/PricingTrx.h"
#include "Pricing/PUPQItem.h"
#include "Pricing/FPPQItem.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestShoppingTrxFactory.h"

namespace tse
{

class FPPQItemComparatorsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FPPQItemComparatorsTest);

  CPPUNIT_TEST(testMileageLeft);
  CPPUNIT_TEST(testMileageRight);

  CPPUNIT_TEST(testTotalAmountLeft);
  CPPUNIT_TEST(testTotalAmountRight);

  CPPUNIT_TEST(testTotalAmountRexLeft);
  CPPUNIT_TEST(testTotalAmountRexRight);

  CPPUNIT_TEST(testTotalAmountYQYRLeft);
  CPPUNIT_TEST(testTotalAmountYQYRRight);

  CPPUNIT_TEST(testCabinLeft);
  CPPUNIT_TEST(testCabinRight);

  CPPUNIT_TEST(testNumberOfStopsPriority1);
  CPPUNIT_TEST(testNumberOfStopsPriority2);

  CPPUNIT_TEST(testFareByRuleLeft);
  CPPUNIT_TEST(testFareByRuleRight);

  CPPUNIT_TEST(testPaxTypeFarePriorityLeft);
  CPPUNIT_TEST(testPaxTypeFarePriorityRight);

  CPPUNIT_TEST(testBaseFareAmountLeft);
  CPPUNIT_TEST(testBaseFareAmountRight);

  CPPUNIT_TEST(testPUCountLeft);
  CPPUNIT_TEST(testPUCountRight);

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

    _leftFarePath = _memHandle.create<FarePath>();
    _leftFPPQItem = _memHandle.create<FPPQItem>();
    _leftFPPQItem->farePath() = _leftFarePath;

    _rightFarePath = _memHandle.create<FarePath>();
    _rightFPPQItem = _memHandle.create<FPPQItem>();
    _rightFPPQItem->farePath() = _rightFarePath;

    _greater = *_memHandle.create<FPPQItem::GreaterFare>();
    _lower = *_memHandle.create<FPPQItem::LowerFare>();
  }

  void tearDown() { _memHandle.clear(); }

  void testMileageLeft()
  {
    _leftFarePath->mileage() = 100;
    _rightFarePath->mileage() = 50;

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testMileageRight()
  {
    _leftFarePath->mileage() = 20;
    _rightFarePath->mileage() = 50;

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testTotalAmountLeft()
  {
    setEqualMileage();

    _leftFarePath->setTotalNUCAmount(150.0);
    _rightFarePath->setTotalNUCAmount(200.0);

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testTotalAmountRight()
  {
    setEqualMileage();

    _leftFarePath->setTotalNUCAmount(500.0);
    _rightFarePath->setTotalNUCAmount(200.0);

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testTotalAmountRexLeft()
  {
    setEqualMileage();

    _leftFarePath->setTotalNUCAmount(200.0);
    _leftFarePath->setRexChangeFee(15.0);

    _rightFarePath->setTotalNUCAmount(150.0);
    _rightFarePath->setRexChangeFee(75.0);

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testTotalAmountRexRight()
  {
    setEqualMileage();

    _leftFarePath->setTotalNUCAmount(150.0);
    _leftFarePath->setRexChangeFee(75.0);

    _rightFarePath->setTotalNUCAmount(200.0);
    _rightFarePath->setRexChangeFee(15.0);

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testTotalAmountYQYRLeft()
  {
    setEqualMileage();

    _leftFarePath->setTotalNUCAmount(200.0);
    _leftFarePath->setYqyrNUCAmount(15.0);

    _rightFarePath->setTotalNUCAmount(150.0);
    _rightFarePath->setYqyrNUCAmount(75.0);

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testTotalAmountYQYRRight()
  {
    setEqualMileage();

    _leftFarePath->setTotalNUCAmount(150.0);
    _leftFarePath->setYqyrNUCAmount(75.0);

    _rightFarePath->setTotalNUCAmount(200.0);
    _rightFarePath->setYqyrNUCAmount(15.0);

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testCabinLeft()
  {
    _pricingOptions->setZeroFareLogic(true);

    setEqualMileage();
    setEqualTotalAmount();

    _leftFPPQItem->_cabinPriority.setEconomyClass();
    _rightFPPQItem->_cabinPriority.setBusinessClass();

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testCabinRight()
  {
    _pricingOptions->setZeroFareLogic(true);

    setEqualMileage();
    setEqualTotalAmount();

    _leftFPPQItem->_cabinPriority.setBusinessClass();
    _rightFPPQItem->_cabinPriority.setEconomyClass();

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testNumberOfStopsPriority1()
  {
    _pricingOptions->setZeroFareLogic(true);

    setEqualMileage();
    setEqualTotalAmount();

    _leftFPPQItem->setNumberOfStopsPriority(2);
    _rightFPPQItem->setNumberOfStopsPriority(1);

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testNumberOfStopsPriority2()
  {
    _pricingOptions->setZeroFareLogic(true);

    setEqualMileage();
    setEqualTotalAmount();

    _leftFPPQItem->setNumberOfStopsPriority(1);
    _rightFPPQItem->setNumberOfStopsPriority(2);

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testFareByRuleLeft()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();

    setFareByRulePriority(FBR_PRIORITY_MIXED, FBR_PRIORITY_FBR_ONLY);

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testFareByRuleRight()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();

    setFareByRulePriority(FBR_PRIORITY_FBR_ONLY, FBR_PRIORITY_MIXED);

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testPaxTypeFarePriorityLeft()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();

    setPaxTypeFarePriority(PRIORITY_LOW, DEFAULT_PRIORITY);

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testPaxTypeFarePriorityRight()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();

    setPaxTypeFarePriority(DEFAULT_PRIORITY, PRIORITY_LOW);

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testBaseFareAmountLeft()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();

    _leftFarePath->plusUpAmount() = 25.0;
    _rightFarePath->plusUpAmount() = 50.0;

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testBaseFareAmountRight()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();

    _leftFarePath->plusUpAmount() = 50.0;
    _rightFarePath->plusUpAmount() = 25.0;

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testPUCountLeft()
  {
    using namespace boost::assign;

    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualPlusUpAmount();

    _leftFarePath->pricingUnit() += _memHandle.create<PricingUnit>(),
        _memHandle.create<PricingUnit>();

    _rightFarePath->pricingUnit() += _memHandle.create<PricingUnit>(),
        _memHandle.create<PricingUnit>(), _memHandle.create<PricingUnit>();

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testPUCountRight()
  {
    using namespace boost::assign;

    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualPlusUpAmount();

    _leftFarePath->pricingUnit() += _memHandle.create<PricingUnit>(),
        _memHandle.create<PricingUnit>(), _memHandle.create<PricingUnit>();

    _rightFarePath->pricingUnit() += _memHandle.create<PricingUnit>(),
        _memHandle.create<PricingUnit>();

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testCxrTypePriorityLeft()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualPlusUpAmount();
    setEqualPUCount();

    setFareCxrTypePriority(PRIORITY_LOW, DEFAULT_PRIORITY);

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testCxrTypePriorityRight()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualPlusUpAmount();
    setEqualPUCount();

    setFareCxrTypePriority(DEFAULT_PRIORITY, PRIORITY_LOW);

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testNegotiatedFarePriorityLeft()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualPlusUpAmount();
    setEqualPUCount();
    setEqualFareCxrTypePriority();

    setNegotiatedFarePriority(PRIORITY_LOW, DEFAULT_PRIORITY);

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testNegotiatedFarePriorityRight()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualPlusUpAmount();
    setEqualPUCount();
    setEqualFareCxrTypePriority();

    setNegotiatedFarePriority(DEFAULT_PRIORITY, PRIORITY_LOW);

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testPuPtfRankLeft()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualPlusUpAmount();
    setEqualPUCount();
    setEqualFareCxrTypePriority();
    setEqualNegotiatedFarePriority();

    setPtfRank(1, 2);

    CPPUNIT_ASSERT(_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testPuPtfRankRight()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualPlusUpAmount();
    setEqualPUCount();
    setEqualFareCxrTypePriority();
    setEqualNegotiatedFarePriority();

    setPtfRank(2, 1);

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
  }

  void testEqual()
  {
    setEqualMileage();
    setEqualTotalAmount();
    setEqualCabin();
    setEqualFareByRulePriority();
    setEqualPaxTypeFarePriority();
    setEqualPlusUpAmount();
    setEqualPUCount();
    setEqualFareCxrTypePriority();
    setEqualNegotiatedFarePriority();
    setEqualPtfRank();

    CPPUNIT_ASSERT(!_greater(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_greater(_rightFPPQItem, _leftFPPQItem));

    CPPUNIT_ASSERT(!_lower(_leftFPPQItem, _rightFPPQItem));
    CPPUNIT_ASSERT(!_lower(_rightFPPQItem, _leftFPPQItem));
  }

protected:
  void setEqualMileage()
  {
    _leftFarePath->mileage() = 0;
    _rightFarePath->mileage() = 0;
  }

  void setEqualTotalAmount()
  {
    _leftFarePath->setTotalNUCAmount(200.0);
    _leftFarePath->setYqyrNUCAmount(10.0);

    _rightFarePath->setTotalNUCAmount(200.0);
    _rightFarePath->setYqyrNUCAmount(10.0);
  }

  void setEqualCabin()
  {
    _leftFPPQItem->_cabinPriority.setEconomyClass();
    _rightFPPQItem->_cabinPriority.setEconomyClass();
  }

  void setEqualPlusUpAmount()
  {
    _leftFarePath->plusUpAmount() = 0.0;
    _rightFarePath->plusUpAmount() = 0.0;
  }

  void setEqualPUCount()
  {
    _leftFarePath->pricingUnit().clear();
    _rightFarePath->pricingUnit().clear();
  }

  void setFareByRulePriority(FBR_PRIORITY left, FBR_PRIORITY right)
  {
    _leftFPPQItem->mutablePriorityStatus().setFareByRulePriority(left);
    _rightFPPQItem->mutablePriorityStatus().setFareByRulePriority(right);
  }

  void setEqualFareByRulePriority()
  {
    setFareByRulePriority(FBR_PRIORITY_MIXED, FBR_PRIORITY_MIXED);
  }

  void setPaxTypeFarePriority(PRIORITY left, PRIORITY right)
  {
    _leftFPPQItem->mutablePriorityStatus().setPaxTypeFarePriority(left);
    _rightFPPQItem->mutablePriorityStatus().setPaxTypeFarePriority(right);
  }

  void setEqualPaxTypeFarePriority()
  {
    setPaxTypeFarePriority(DEFAULT_PRIORITY, DEFAULT_PRIORITY);
  }

  void setFareCxrTypePriority(PRIORITY left, PRIORITY right)
  {
    _leftFPPQItem->mutablePriorityStatus().setFareCxrTypePriority(left);
    _rightFPPQItem->mutablePriorityStatus().setFareCxrTypePriority(right);
  }

  void setEqualFareCxrTypePriority()
  {
    setFareCxrTypePriority(DEFAULT_PRIORITY, DEFAULT_PRIORITY);
  }

  void setNegotiatedFarePriority(PRIORITY left, PRIORITY right)
  {
    _leftFPPQItem->mutablePriorityStatus().setNegotiatedFarePriority(left);
    _rightFPPQItem->mutablePriorityStatus().setNegotiatedFarePriority(right);
  }

  void setEqualNegotiatedFarePriority()
  {
    setNegotiatedFarePriority(DEFAULT_PRIORITY, DEFAULT_PRIORITY);
  }

  void setPtfRank(uint16_t left, uint16_t right)
  {
    _leftFPPQItem->mutablePriorityStatus().setPtfRank(left);
    _rightFPPQItem->mutablePriorityStatus().setPtfRank(right);
  }

  void setEqualPtfRank()
  {
    setPtfRank(1, 1);
  }

private:
  TestMemHandle _memHandle;

  PricingOptions* _pricingOptions;
  PricingTrx* _trx;

  FarePath* _leftFarePath;
  FPPQItem* _leftFPPQItem;

  FarePath* _rightFarePath;
  FPPQItem* _rightFPPQItem;

  FPPQItem::GreaterFare _greater;
  FPPQItem::LowerFare _lower;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FPPQItemComparatorsTest);

} // tse
