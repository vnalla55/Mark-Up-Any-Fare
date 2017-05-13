// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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
#include "test/include/CppUnitHelperMacros.h"

#include "test/LocServiceMock.h"
#include "Rules/PointOfTicketingApplicator.h"
#include "Rules/PointOfTicketingRule.h"
#include "DataModel/Common/Types.h"
#include "test/PaymentDetailMock.h"

#include <memory>

namespace tax
{

class PointOfTicketingTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PointOfTicketingTest);
  CPPUNIT_TEST(testAlwaysApplayIfPointOfTicketingIsBlank);
  CPPUNIT_TEST(testApplayPointOfTicketing);
  CPPUNIT_TEST(testDontApplayPointOfTicketing);

  CPPUNIT_TEST_SUITE_END();

public:
  void createApplicator(const type::AirportCode& ticketingPoint)
  {
    _rule.reset(new PointOfTicketingRule(*_locZone, *_vendor));
    _applicator.reset(new PointOfTicketingApplicator(*_rule, ticketingPoint, *_locServiceMock));
  }

  void setUp()
  {
    _locServiceMock = new LocServiceMock();
    _locZone = new LocZone();
    _vendor = new type::Vendor();
    _paymentDetail = new PaymentDetailMock();
  }

  void tearDown()
  {
    delete _locServiceMock;
    delete _locZone;
    delete _vendor;
    delete _paymentDetail;
    _rule.reset();
    _applicator.reset();
  }

  void testApplayPointOfTicketing()
  {
    const type::AirportCode ticketingPoint = "KRK";
    createApplicator(ticketingPoint);

    _locServiceMock->add(true);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

  void testDontApplayPointOfTicketing()
  {
    const type::AirportCode ticketingPoint = "WAW";
    createApplicator(ticketingPoint);

    _locServiceMock->add(false);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(*_paymentDetail));
  }

  void testAlwaysApplayIfPointOfTicketingIsBlank()
  {
    const type::AirportCode ticketingPoint(UninitializedCode);
    createApplicator(ticketingPoint);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(*_paymentDetail));
  }

private:
  std::unique_ptr<PointOfTicketingRule> _rule;
  std::unique_ptr<PointOfTicketingApplicator> _applicator;

  LocServiceMock* _locServiceMock;
  LocZone* _locZone;
  type::Vendor* _vendor;
  PaymentDetailMock* _paymentDetail;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PointOfTicketingTest);
} // namespace tax
