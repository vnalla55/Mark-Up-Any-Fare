#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DataModel/PaxType.h"
#include "RexPricing/EnhancedRefundDiscountApplier.h"
#include "test/include/TestMemHandle.h"
#include "test/include/PrintCollection.h"

namespace tse
{

using namespace boost::assign;

class EnhancedRefundDiscountApplierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EnhancedRefundDiscountApplierTest);

  CPPUNIT_TEST(testApplyDiscount1ws);
  CPPUNIT_TEST(testApplyDiscount1wos);
  CPPUNIT_TEST(testApplyDiscount2ws);
  CPPUNIT_TEST(testApplyDiscount2wos);
  CPPUNIT_TEST(testApplyDiscount8);
  CPPUNIT_TEST(testApplyDiscount9);
  CPPUNIT_TEST(testApplyDiscount0);
  CPPUNIT_TEST(testAdjustDiscountTags);

  CPPUNIT_TEST_SUITE_END();

public:
  TestMemHandle _memH;
  PaxType* _paxType;
  PaxTypeInfo* _paxTypeInfo;
  EnhancedRefundDiscountApplier* _applier;
  DiscountInfo* _discount;
  Money* _fee;

  void setUp()
  {
    _paxType = _memH(new PaxType);
    _paxTypeInfo = _memH(new PaxTypeInfo);
    _paxType->paxTypeInfo() = _paxTypeInfo;

    _applier = _memH(new EnhancedRefundDiscountApplier(*_paxType));

    _discount = _memH(new DiscountInfo);
    _discount->discPercent() = 10.0;
    _discount->category() = 19;
    _fee = _memH(new Money(10.0, "PLN"));
  }

  void tearDown() { _memH.clear(); }

  void testApplyDiscount1ws()
  {
    std::vector<Indicator> discountTags(1, RefundDiscountApplier::INFANT_WITHOUT_SEAT);
    _applier->_infantWithSeat = true;

    CPPUNIT_ASSERT(_applier->applyDiscount(*_fee, discountTags, _discount));
  }
  void testApplyDiscount1wos()
  {
    std::vector<Indicator> discountTags(1, RefundDiscountApplier::INFANT_WITHOUT_SEAT);
    _applier->_infantWithoutSeat = true;

    CPPUNIT_ASSERT(_applier->applyDiscount(*_fee, discountTags, _discount));
  }
  void testApplyDiscount2ws()
  {
    std::vector<Indicator> discountTags(1, RefundDiscountApplier::INFANT_WITH_SEAT);
    _applier->_infantWithSeat = true;

    CPPUNIT_ASSERT(_applier->applyDiscount(*_fee, discountTags, _discount));
  }
  void testApplyDiscount2wos()
  {
    std::vector<Indicator> discountTags(1, RefundDiscountApplier::INFANT_WITH_SEAT);
    _applier->_infantWithoutSeat = true;

    CPPUNIT_ASSERT(_applier->applyDiscount(*_fee, discountTags, _discount));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, _fee->value(), EPSILON);
  }

  void testApplyDiscount8()
  {
    std::vector<Indicator> discountTags(1, RefundDiscountApplier::INFANT_WITHOUT_SEAT_CAT19);
    _applier->_infantWithoutSeat = true;

    CPPUNIT_ASSERT(_applier->applyDiscount(*_fee, discountTags, _discount));
  }
  void testApplyDiscount9()
  {
    std::vector<Indicator> discountTags(1, RefundDiscountApplier::INFANT_WITHOUT_SEAT_NOFEE);
    _applier->_infantWithoutSeat = true;

    CPPUNIT_ASSERT(_applier->applyDiscount(*_fee, discountTags, _discount));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, _fee->value(), EPSILON);
  }
  void testApplyDiscount0()
  {
    std::vector<Indicator> discountTags(1, RefundDiscountApplier::INFANT_WITH_SEAT_CAT19);
    _applier->_infantWithSeat = true;

    CPPUNIT_ASSERT(_applier->applyDiscount(*_fee, discountTags, _discount));
  }
  void testAdjustDiscountTags()
  {
    std::vector<Indicator> discountTags;
    discountTags += RefundDiscountApplier::INFANT_WITH_SEAT_CAT19, // 0
        RefundDiscountApplier::INFANT_WITHOUT_SEAT_CAT19, // 8
        RefundDiscountApplier::INFANT_WITHOUT_SEAT, // 1
        RefundDiscountApplier::INFANT_WITHOUT_SEAT_NOFEE, // 9
        RefundDiscountApplier::INFANT_WITH_SEAT, // 2
        RefundDiscountApplier::SENIOR; // 5

    std::vector<Indicator> expectedDiscountTags;
    expectedDiscountTags += RefundDiscountApplier::INFANT_WITHOUT_SEAT_CAT19, // 8
        RefundDiscountApplier::INFANT_WITHOUT_SEAT_NOFEE, // 9
        RefundDiscountApplier::SENIOR; // 5

    std::vector<Indicator> adoptedDiscountTags = _applier->adjustDiscountTags(discountTags);

    CPPUNIT_ASSERT(expectedDiscountTags == adoptedDiscountTags);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(EnhancedRefundDiscountApplierTest);
}
