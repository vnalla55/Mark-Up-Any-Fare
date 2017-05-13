// ----------------------------------------------------------------
//
//   Copyright Sabre 2010
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "Common/NonFatalErrorResponseException.h"
#include "DataModel/PricingRequest.h"

namespace tse
{
class PricingRequestTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingRequestTest);
  CPPUNIT_TEST(test_checkSchemaVersion_1_1_0);
  CPPUNIT_TEST(test_checkSchemaVersion_1_0_0);
  CPPUNIT_TEST(test_checkSchemaVersion_2_0_0);
  CPPUNIT_TEST(test_checkSchemaVersion_1_2_0);
  CPPUNIT_TEST(test_checkSchemaVersion_1_1_1);
  CPPUNIT_TEST(testDiscountWithDAentryOld);
  CPPUNIT_TEST(testDiscountWithDPentryOld);
  CPPUNIT_TEST(testSetRCQValues);
  CPPUNIT_TEST(testSetRCQValues2);
  CPPUNIT_TEST(testSetRCQValuesError);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  PricingRequest* _request;

public:
  void setUp() { _request = _memHandle.create<PricingRequest>(); }

  void tearDown() { _memHandle.clear(); }

  void test_checkSchemaVersion_1_1_0()
  {
    _request->majorSchemaVersion() = 1;
    _request->minorSchemaVersion() = 1;
    _request->revisionSchemaVersion() = 0;

    CPPUNIT_ASSERT(_request->checkSchemaVersion(1, 1, 0));
  }

  void test_checkSchemaVersion_1_0_0()
  {
    _request->majorSchemaVersion() = 1;
    _request->minorSchemaVersion() = 0;
    _request->revisionSchemaVersion() = 0;

    CPPUNIT_ASSERT(!_request->checkSchemaVersion(1, 1, 0));
  }

  void test_checkSchemaVersion_2_0_0()
  {
    _request->majorSchemaVersion() = 2;
    _request->minorSchemaVersion() = 0;
    _request->revisionSchemaVersion() = 0;

    CPPUNIT_ASSERT(_request->checkSchemaVersion(1, 1, 0));
  }

  void test_checkSchemaVersion_1_2_0()
  {
    _request->majorSchemaVersion() = 1;
    _request->minorSchemaVersion() = 2;
    _request->revisionSchemaVersion() = 0;

    CPPUNIT_ASSERT(_request->checkSchemaVersion(1, 1, 0));
  }

  void test_checkSchemaVersion_1_1_1()
  {
    _request->majorSchemaVersion() = 1;
    _request->minorSchemaVersion() = 1;
    _request->revisionSchemaVersion() = 1;

    CPPUNIT_ASSERT(_request->checkSchemaVersion(1, 1, 0));
  }

  void testDiscountWithDAentryOld()
  {
    const std::vector<DiscountAmount>& previousDiscounts = _request->getDiscountAmounts();

    _request->setDiscountAmounts(std::vector<DiscountAmount>());
    CPPUNIT_ASSERT_EQUAL(_request->isDAEntry(), false);

    _request->setDiscountAmounts(std::vector<DiscountAmount>());
    _request->addDiscAmount(1, 1, 1.5, "USD");
    _request->addDiscAmount(1, 2, 1.5 , "USD");
    CPPUNIT_ASSERT_EQUAL(_request->isDAEntry(), true);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].startSegmentOrder == 1);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].endSegmentOrder == 2);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].amount == 1.5);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].currencyCode == "USD");

    _request->setDiscountAmounts(std::vector<DiscountAmount>());
    _request->addDiscAmount(1, 2, 1.5 , "USD");
    _request->addDiscAmount(1, 1, 1.5, "USD");
    CPPUNIT_ASSERT_EQUAL(_request->isDAEntry(), true);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].startSegmentOrder == 1);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].endSegmentOrder == 2);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].amount == 1.5);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].currencyCode == "USD");

    _request->setDiscountAmounts(std::vector<DiscountAmount>());
    _request->addDiscAmount(0, 1, 1.5, "USD");
    _request->addDiscAmount(0, 2, 1.5 , "USD");
    _request->addDiscAmount(0, 4, 3.14 , "USD");
    _request->addDiscAmount(0, 3, 3.14 , "USD");
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].startSegmentOrder == 1);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].endSegmentOrder == 2);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].amount == 1.5);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].currencyCode == "USD");
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[1].startSegmentOrder == 3);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[1].endSegmentOrder == 4);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[1].amount == 3.14);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[1].currencyCode == "USD");

    _request->setDiscountAmounts(std::vector<DiscountAmount>());
    _request->addDiscAmount(0, 1, 1, "USD");
    _request->addDiscAmount(0, 2, 3, "USD");
    _request->addDiscAmount(0, 3, 4 , "USD");
    _request->addDiscAmount(0, 4, 3 , "USD");
    _request->addDiscAmount(0, 5, 5 , "USD");
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].startSegmentOrder == 1);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].endSegmentOrder == 1);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[0].amount == 1);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[1].startSegmentOrder == 2);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[1].endSegmentOrder == 4);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[1].amount == 3);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[2].startSegmentOrder == 3);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[2].endSegmentOrder == 3);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[2].amount == 4);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[3].startSegmentOrder == 5);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[3].endSegmentOrder == 5);
    CPPUNIT_ASSERT(_request->getDiscountAmounts()[3].amount == 5);


    _request->setDiscountAmounts(previousDiscounts);
  }

  void testDiscountWithDPentryOld()
  {
    _request->setDiscountPercentages(std::map<int16_t, Percent>());
    CPPUNIT_ASSERT_EQUAL(_request->isDPEntry(), false);

    _request->setDiscountPercentages(std::map<int16_t, Percent>());
    _request->discPercentages().insert(std::pair<int16_t, Percent>(2, 1.5));
    _request->discPercentages().insert(std::pair<int16_t, Percent>(1, 3.14));
    CPPUNIT_ASSERT_EQUAL(_request->isDPEntry(), true);

    _request->setDiscountPercentages(std::map<int16_t, Percent>());
    _request->discPercentages().insert(std::pair<int16_t, Percent>(1, 1.5));
    _request->discPercentages().insert(std::pair<int16_t, Percent>(2, 1.5));
    _request->discPercentages().insert(std::pair<int16_t, Percent>(3, 3.14));
    _request->discPercentages().insert(std::pair<int16_t, Percent>(4, 3.14));
    CPPUNIT_ASSERT_EQUAL(_request->isDPEntry(), true);
    std::map<int16_t, Percent> discountPercentages = _request->getDiscountPercentages();
    CPPUNIT_ASSERT(discountPercentages[1] == 1.5);
    CPPUNIT_ASSERT(discountPercentages[2] == 1.5);
    CPPUNIT_ASSERT(discountPercentages[3] == 3.14);
    CPPUNIT_ASSERT(discountPercentages[4] == 3.14);
  }

  void testSetRCQValues()
  {
    PricingRequest request;
    request.setPRM(false);

    int count = request.setRCQValues("ABC123,DFE456,MMSZZA,ZDF9876");
    CPPUNIT_ASSERT(count == 4);
    CPPUNIT_ASSERT(request.rcqValues().size() == 4);
    CPPUNIT_ASSERT(request.rcqValues()[0] == "ABC123");
    CPPUNIT_ASSERT(request.rcqValues()[1] == "DFE456");
    CPPUNIT_ASSERT(request.rcqValues()[2] == "MMSZZA");
    CPPUNIT_ASSERT(request.rcqValues()[3] == "ZDF9876");
  }

  void testSetRCQValues2()
  {
    PricingRequest request;
    request.setPRM(true);

    int count = request.setRCQValues("ABC123ZZZ");
    CPPUNIT_ASSERT(count == 1);
    CPPUNIT_ASSERT(request.rcqValues().size() == 1);
    CPPUNIT_ASSERT(request.rcqValues()[0] == "ABC123ZZZ");
  }

  void testSetRCQValuesError()
  {
    PricingRequest request;
    request.setPRM(false);

    request.setPRM(true); // only one allowed

    try
    {
      request.setRCQValues("ABC123,DFE456");
    }
    catch(NonFatalErrorResponseException e)
    {
      std::cout << " Display message" << e.message() << std::endl;
      CPPUNIT_ASSERT(e.message() == "MAXIMUM 1 RETAILER RULE QUALIFIERS PERMITTED");
    }

    request.setPRM(false);

    try
    {
      request.setRCQValues("ABC123,DFE456,MMSZZA,ZDF9876###");
    }
    catch(NonFatalErrorResponseException e)
    {
      CPPUNIT_ASSERT(e.message() == "FORMAT - RETAILER RULE QUALIFIER MUST BE 2-20 ALPHANUMERIC");
    }

    try
    {
      request.setRCQValues("ABC123,DFE456,MMSZZA,Z");
    }
    catch(NonFatalErrorResponseException e)
    {
      CPPUNIT_ASSERT(e.message() == "FORMAT - RETAILER RULE QUALIFIER MUST BE 2-20 ALPHANUMERIC");
    }

    try
    {
      request.setRCQValues("ABC123,DFE456,MMSZZA,A01234567890123456789");
    }
    catch(NonFatalErrorResponseException e)
    {
      CPPUNIT_ASSERT(e.message() == "FORMAT - RETAILER RULE QUALIFIER MUST BE 2-20 ALPHANUMERIC");
    }

  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(PricingRequestTest);

class DiscountsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DiscountsTest);
  CPPUNIT_TEST(testAmountDiscountAndPlusUp);
  CPPUNIT_TEST(testPercentDiscountAndPlusUp);
  CPPUNIT_TEST(testNonConsecutiveSegmentOrderAmount);
  CPPUNIT_TEST(testNonConsecutiveSegmentOrderPercent);
  CPPUNIT_TEST_SUITE_END();

public:
  void testAmountDiscountAndPlusUp()
  {
    Discounts discounts;
    CPPUNIT_ASSERT_EQUAL(false, discounts.isDAEntry());
    CPPUNIT_ASSERT_EQUAL(false, discounts.isPAEntry());

    discounts.clearAmountDiscounts();
    discounts.addAmount(2, 2, 1.5, USD);
    discounts.addAmount(1, 1, 3.14, USD);
    CPPUNIT_ASSERT_EQUAL(true, discounts.isDAEntry());
    CPPUNIT_ASSERT_EQUAL(false, discounts.isPAEntry());

    discounts.clearAmountDiscounts();
    discounts.addAmount(1, 1, -3.14, USD);
    discounts.addAmount(2, 2, -1.5, USD);
    CPPUNIT_ASSERT_EQUAL(false, discounts.isDAEntry());
    CPPUNIT_ASSERT_EQUAL(true, discounts.isPAEntry());

    discounts.clearAmountDiscounts();
    discounts.addAmount(1, 1, 3.14, USD);
    discounts.addAmount(2, 2, -1.5, USD);
    CPPUNIT_ASSERT_EQUAL(true, discounts.isDAEntry());
    CPPUNIT_ASSERT_EQUAL(true, discounts.isPAEntry());

    discounts.clearAmountDiscounts();
    discounts.addAmount(1, 1, -3.14, USD);
    discounts.addAmount(2, 2, 1.5, USD);
    CPPUNIT_ASSERT_EQUAL(true, discounts.isDAEntry());
    CPPUNIT_ASSERT_EQUAL(true, discounts.isPAEntry());

    discounts.clearAmountDiscounts();
    discounts.addAmount(1, 1, 1.5, USD);
    discounts.addAmount(1, 2, 1.5, USD);
    CPPUNIT_ASSERT_EQUAL(1, (int)discounts.getAmounts().at(0).startSegmentOrder);
    CPPUNIT_ASSERT_EQUAL(2, (int)discounts.getAmounts().at(0).endSegmentOrder);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, discounts.getAmounts().at(0).amount, EPSILON);
    CPPUNIT_ASSERT_EQUAL(USD, discounts.getAmounts().at(0).currencyCode);

    discounts.clearAmountDiscounts();
    discounts.addAmount(1, 2, 1.5, USD);
    discounts.addAmount(1, 1, 1.5, USD);
    CPPUNIT_ASSERT_EQUAL(1, (int)discounts.getAmounts().at(0).startSegmentOrder);
    CPPUNIT_ASSERT_EQUAL(2, (int)discounts.getAmounts().at(0).endSegmentOrder);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, discounts.getAmounts().at(0).amount, EPSILON);
    CPPUNIT_ASSERT_EQUAL(USD, discounts.getAmounts().at(0).currencyCode);

    discounts.clearAmountDiscounts();
    discounts.addAmount(0, 1, 1.5, USD);
    discounts.addAmount(0, 2, 1.5, USD);
    discounts.addAmount(0, 4, -3.14, USD);
    discounts.addAmount(0, 3, -3.14, USD);
    CPPUNIT_ASSERT_EQUAL(1, (int)discounts.getAmounts().at(0).startSegmentOrder);
    CPPUNIT_ASSERT_EQUAL(2, (int)discounts.getAmounts().at(0).endSegmentOrder);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, discounts.getAmounts().at(0).amount, EPSILON);
    CPPUNIT_ASSERT_EQUAL(USD, discounts.getAmounts().at(0).currencyCode);
    CPPUNIT_ASSERT_EQUAL(3, (int)discounts.getAmounts().at(1).startSegmentOrder);
    CPPUNIT_ASSERT_EQUAL(4, (int)discounts.getAmounts().at(1).endSegmentOrder);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-3.14, discounts.getAmounts().at(1).amount, EPSILON);
    CPPUNIT_ASSERT_EQUAL(USD, discounts.getAmounts().at(1).currencyCode);

    discounts.clearAmountDiscounts();
    discounts.addAmount(0, 1, 1, USD);
    discounts.addAmount(0, 2, 3, USD);
    discounts.addAmount(0, 3, 4, USD);
    discounts.addAmount(0, 4, 3, USD);
    discounts.addAmount(0, 5, 5, USD);
    CPPUNIT_ASSERT_EQUAL(1, (int)discounts.getAmounts().at(0).startSegmentOrder);
    CPPUNIT_ASSERT_EQUAL(1, (int)discounts.getAmounts().at(0).endSegmentOrder);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1, discounts.getAmounts().at(0).amount, EPSILON);
    CPPUNIT_ASSERT_EQUAL(2, (int)discounts.getAmounts().at(1).startSegmentOrder);
    CPPUNIT_ASSERT_EQUAL(4, (int)discounts.getAmounts().at(1).endSegmentOrder);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3, discounts.getAmounts().at(1).amount, EPSILON);
    CPPUNIT_ASSERT_EQUAL(3, (int)discounts.getAmounts().at(2).startSegmentOrder);
    CPPUNIT_ASSERT_EQUAL(3, (int)discounts.getAmounts().at(2).endSegmentOrder);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4, discounts.getAmounts().at(2).amount, EPSILON);
    CPPUNIT_ASSERT_EQUAL(5, (int)discounts.getAmounts().at(3).startSegmentOrder);
    CPPUNIT_ASSERT_EQUAL(5, (int)discounts.getAmounts().at(3).endSegmentOrder);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5, discounts.getAmounts().at(3).amount, EPSILON);
  }

  void testPercentDiscountAndPlusUp()
  {
    Discounts discounts;
    CPPUNIT_ASSERT_EQUAL(false, discounts.isDPEntry());
    CPPUNIT_ASSERT_EQUAL(false, discounts.isPPEntry());

    discounts.clearPercentageDiscounts();
    discounts.addPercentage(2, 1.5);
    discounts.addPercentage(1, 3.14);
    CPPUNIT_ASSERT_EQUAL(true, discounts.isDPEntry());
    CPPUNIT_ASSERT_EQUAL(false, discounts.isPPEntry());

    discounts.clearPercentageDiscounts();
    discounts.addPercentage(1, -3.14);
    discounts.addPercentage(2, -1.5);
    CPPUNIT_ASSERT_EQUAL(false, discounts.isDPEntry());
    CPPUNIT_ASSERT_EQUAL(true, discounts.isPPEntry());

    discounts.clearPercentageDiscounts();
    discounts.addPercentage(1, 3.14);
    discounts.addPercentage(2, -1.5);
    CPPUNIT_ASSERT_EQUAL(true, discounts.isDPEntry());
    CPPUNIT_ASSERT_EQUAL(true, discounts.isPPEntry());

    discounts.clearPercentageDiscounts();
    discounts.addPercentage(1, -3.14);
    discounts.addPercentage(2, 1.5);
    CPPUNIT_ASSERT_EQUAL(true, discounts.isDPEntry());
    CPPUNIT_ASSERT_EQUAL(true, discounts.isPPEntry());

    discounts.clearPercentageDiscounts();
    discounts.addPercentage(1, 1.5);
    discounts.addPercentage(2, 1.5);
    discounts.addPercentage(4, -3.14);
    discounts.addPercentage(3, -3.14);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, discounts.getPercentages().at(1), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.5, discounts.getPercentages().at(2), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-3.14, discounts.getPercentages().at(3), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-3.14, discounts.getPercentages().at(4), EPSILON);
  }

  void testNonConsecutiveSegmentOrderAmount()
  {
    Discounts discounts;
    discounts.addAmount(1, 2, 11, USD);
    discounts.addAmount(2, 4, 12, USD);
    discounts.addAmount(3, 5, 13, USD);
    discounts.addAmount(4, 6, 14, USD);
    discounts.addAmount(1, 1, 11, USD);
    discounts.addAmount(4, 3, 14, USD);

    CPPUNIT_ASSERT_EQUAL(4, (int)discounts.getAmounts().size());
    CPPUNIT_ASSERT_EQUAL(1, (int)discounts.getAmounts().at(0).startSegmentOrder);
    CPPUNIT_ASSERT_EQUAL(2, (int)discounts.getAmounts().at(0).endSegmentOrder);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, discounts.getAmounts().at(0).amount, EPSILON);
    CPPUNIT_ASSERT_EQUAL(4, (int)discounts.getAmounts().at(1).startSegmentOrder);
    CPPUNIT_ASSERT_EQUAL(4, (int)discounts.getAmounts().at(1).endSegmentOrder);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0, discounts.getAmounts().at(1).amount, EPSILON);
    CPPUNIT_ASSERT_EQUAL(5, (int)discounts.getAmounts().at(2).startSegmentOrder);
    CPPUNIT_ASSERT_EQUAL(5, (int)discounts.getAmounts().at(2).endSegmentOrder);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0, discounts.getAmounts().at(2).amount, EPSILON);
    // Below lines show that current implementation doesn't cover all cases.
    // As it is visible below this group should contain only segments 3 and 6
    // (for example in side-trips) but instead stores a range of segments 3-6.
    // To fix that DiscountAmount should be refactored to store vector of elements
    // instead of range.
    CPPUNIT_ASSERT_EQUAL(3, (int)discounts.getAmounts().at(3).startSegmentOrder);
    CPPUNIT_ASSERT_EQUAL(6, (int)discounts.getAmounts().at(3).endSegmentOrder);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.0, discounts.getAmounts().at(3).amount, EPSILON);
  }

  void testNonConsecutiveSegmentOrderPercent()
  {
    Discounts discounts;
    discounts.addPercentage(2, 12);
    discounts.addPercentage(4, 14);
    discounts.addPercentage(5, 15);
    discounts.addPercentage(6, 16);
    discounts.addPercentage(1, 11);
    discounts.addPercentage(3, 13);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, discounts.getPercentages().at(1), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0, discounts.getPercentages().at(2), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13.0, discounts.getPercentages().at(3), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14.0, discounts.getPercentages().at(4), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15.0, discounts.getPercentages().at(5), EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(16.0, discounts.getPercentages().at(6), EPSILON);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(DiscountsTest);
}
