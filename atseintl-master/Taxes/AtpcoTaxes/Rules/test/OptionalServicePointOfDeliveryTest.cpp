// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include "test/LocServiceMock.h"

#include <stdexcept>
#include <set>

#include "test/include/CppUnitHelperMacros.h"
#include "Rules/OptionalServicePointOfDeliveryApplicator.h"
#include "Rules/OptionalServicePointOfDeliveryRule.h"
#include "test/PaymentDetailMock.h"

#include <memory>

using namespace std;

namespace tax
{

class OptionalServicePointOfDeliveryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OptionalServicePointOfDeliveryTest);

  CPPUNIT_TEST(testPointOfDeliveryVerification);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _paymentDetail = new PaymentDetailMock();

    _locServiceMock = new LocServiceMock();

    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().pointOfDeliveryLoc() = "KRK";
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().pointOfDeliveryLoc() = "DFW";
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().pointOfDeliveryLoc() = "BER";
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().pointOfDeliveryLoc() = "KRK";
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::BaggageCharge;
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().pointOfDeliveryLoc() = "BER";
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::BaggageCharge;

    _locZone = new LocZone();
    _vendor = new type::Vendor();
  }

  void tearDown()
  {
    delete _locServiceMock;
    delete _paymentDetail;
    delete _locZone;
    delete _vendor;
  }

  void testPointOfDeliveryVerification()
  {
    _locServiceMock->add(false, 2);
    _locServiceMock->add(true, 1);
    _locServiceMock->add(false, 1);
    _locServiceMock->add(true, 1);

    _rule.reset(new OptionalServicePointOfDeliveryRule(*_locZone, *_vendor));
    OptionalServicePointOfDeliveryApplicator applicator(*_rule, *_locServiceMock);

    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[1].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[2].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[3].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[4].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->areAllOptionalServicesFailed());

    _locServiceMock->add(false, 2);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[2].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[4].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->areAllOptionalServicesFailed());
  }

private:
  PaymentDetail* _paymentDetail;
  std::unique_ptr<OptionalServicePointOfDeliveryRule> _rule;
  LocServiceMock* _locServiceMock;
  LocZone* _locZone;
  type::Vendor* _vendor;
};

CPPUNIT_TEST_SUITE_REGISTRATION(OptionalServicePointOfDeliveryTest);
} // namespace tax
