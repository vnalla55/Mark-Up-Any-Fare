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

#include "test/include/CppUnitHelperMacros.h"
#include "Rules/OptionalServiceTagsApplicator.h"
#include "Rules/OptionalServiceTagsRule.h"
#include "test/PaymentDetailMock.h"

#include <memory>

namespace tax
{

class OptionalServiceTagsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OptionalServiceTagsTest);

  CPPUNIT_TEST(testTagsVerification);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _paymentDetail = new PaymentDetailMock();
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::FlightRelated;
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::TicketRelated;
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::Merchandise;
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::FareRelated;
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::BaggageCharge;
  }

  void tearDown()
  {
    delete _paymentDetail;
  }

  void testTagsVerification()
  {
    TaxableUnitTagSet tutSet = TaxableUnitTagSet::none();
    tutSet.setTag(type::TaxableUnit::OCFlightRelated);
    tutSet.setTag(type::TaxableUnit::OCFareRelated);
    tutSet.setTag(type::TaxableUnit::BaggageCharge);

    _rule.reset(new OptionalServiceTagsRule(tutSet));
    OptionalServiceTagsApplicator applicator(*_rule);

    _paymentDetail->optionalServiceItems()[2].setFailedRule(_rule.get());

    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[1].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->optionalServiceItems()[2].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[3].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->optionalServiceItems()[4].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->areAllOptionalServicesFailed());
  }

private:
  PaymentDetail* _paymentDetail;
  std::unique_ptr<OptionalServiceTagsRule> _rule;
};

CPPUNIT_TEST_SUITE_REGISTRATION(OptionalServiceTagsTest);
} // namespace tax
