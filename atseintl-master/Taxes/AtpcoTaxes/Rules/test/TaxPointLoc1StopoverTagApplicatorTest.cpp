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

#include "Rules/TaxPointLoc1StopoverTagApplicator.h"
#include "Rules/TaxPointLoc1StopoverTagRule.h"
#include "test/PaymentDetailMock.h"

#include <memory>

using namespace std;

namespace tax
{

class TaxPointLoc1StopoverTagApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxPointLoc1StopoverTagApplicatorTest);

  CPPUNIT_TEST(testIsConnection);
  CPPUNIT_TEST(testIsConnectionOpen);
  CPPUNIT_TEST(testIsStopover);
  CPPUNIT_TEST(testIsStopoverOpen);
  CPPUNIT_TEST(testIsNotStopover);
  CPPUNIT_TEST(testIsFareBreak);
  CPPUNIT_TEST(testIsFareBreakAndStop_TaxMatchingTag06);
  CPPUNIT_TEST(testIsFareBreakOpen);
  CPPUNIT_TEST(testIsNotFareBreak);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _geo.reset(new Geo());
    _paymentDetail.reset(new PaymentDetailMock());
    _paymentDetail->setTaxPointBegin(*_geo);
    _paymentDetail->getMutableTaxPointsProperties().resize(2);
    _stopoverTag.reset(new type::StopoverTag(type::StopoverTag::Blank));
    _ticketedPointTag.reset(
        new type::TicketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints));
  }

  void tearDown()
  {
  }

  void testIsConnection()
  {
    *_stopoverTag = type::StopoverTag::Connection;
    _paymentDetail->setLoc1Stopover(false);
    _parentRule.reset(new TaxPointLoc1StopoverTagRule(*_stopoverTag, *_ticketedPointTag, false));

    TaxPointLoc1StopoverTagApplicator applicator(*_parentRule, *_stopoverTag, *_ticketedPointTag, false);
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));
  }

  void testIsConnectionOpen()
  {
    *_stopoverTag = type::StopoverTag::Connection;
    _paymentDetail->getMutableTaxPointsProperties()[0].isOpen = true;
    _parentRule.reset(new TaxPointLoc1StopoverTagRule(*_stopoverTag, *_ticketedPointTag, false));

    TaxPointLoc1StopoverTagApplicator applicator(*_parentRule, *_stopoverTag, *_ticketedPointTag, false);
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));
  }

  void testIsStopover()
  {
    *_stopoverTag = type::StopoverTag::Stopover;
    _paymentDetail->setLoc1Stopover(true);
    _parentRule.reset(new TaxPointLoc1StopoverTagRule(*_stopoverTag, *_ticketedPointTag, false));

    TaxPointLoc1StopoverTagApplicator applicator(*_parentRule, *_stopoverTag, *_ticketedPointTag, false);
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testIsStopoverOpen()
  {
    *_stopoverTag = type::StopoverTag::Stopover;
    _paymentDetail->setLoc1Stopover(true);
    _paymentDetail->getMutableTaxPointsProperties()[0].isOpen = true;
    _parentRule.reset(new TaxPointLoc1StopoverTagRule(*_stopoverTag, *_ticketedPointTag, false));

    TaxPointLoc1StopoverTagApplicator applicator(*_parentRule, *_stopoverTag, *_ticketedPointTag, false);
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testIsNotStopover()
  {
    *_stopoverTag = type::StopoverTag::Stopover;
    _paymentDetail->setLoc1Stopover(false);
    _parentRule.reset(new TaxPointLoc1StopoverTagRule(*_stopoverTag, *_ticketedPointTag, false));

    TaxPointLoc1StopoverTagApplicator applicator(*_parentRule, *_stopoverTag, *_ticketedPointTag, false);
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));

    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));
  }

  void testIsFareBreak()
  {
    *_stopoverTag = type::StopoverTag::FareBreak;
    _paymentDetail->setLoc1FareBreak(true);
    _parentRule.reset(new TaxPointLoc1StopoverTagRule(*_stopoverTag, *_ticketedPointTag, false));

    TaxPointLoc1StopoverTagApplicator applicator(*_parentRule, *_stopoverTag, *_ticketedPointTag, false);
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testIsFareBreakAndStop_TaxMatchingTag06()
  {
    *_stopoverTag = type::StopoverTag::FareBreak;
    _parentRule.reset(new TaxPointLoc1StopoverTagRule(*_stopoverTag, *_ticketedPointTag, true));
    TaxPointLoc1StopoverTagApplicator applicator(*_parentRule, *_stopoverTag, *_ticketedPointTag, true);

    // Only fare break
    _paymentDetail->setLoc1FareBreak(true);
    _paymentDetail->setLoc1Stopover(false);

    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));

    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));

    // Fare break and stopover (not checking unticketed transfer, stopover is always ticketed)
    _paymentDetail->setLoc1Stopover(true);

    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testIsFareBreakOpen()
  {
    *_stopoverTag = type::StopoverTag::FareBreak;
    _paymentDetail->setLoc1FareBreak(true);
    _paymentDetail->getMutableTaxPointsProperties()[0].isOpen = true;
    _parentRule.reset(new TaxPointLoc1StopoverTagRule(*_stopoverTag, *_ticketedPointTag, false));

    TaxPointLoc1StopoverTagApplicator applicator(*_parentRule, *_stopoverTag, *_ticketedPointTag, false);
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testIsNotFareBreak()
  {
    *_stopoverTag = type::StopoverTag::NotFareBreak;
    _paymentDetail->setLoc1FareBreak(false);
    _parentRule.reset(new TaxPointLoc1StopoverTagRule(*_stopoverTag, *_ticketedPointTag, false));

    TaxPointLoc1StopoverTagApplicator applicator(*_parentRule, *_stopoverTag, *_ticketedPointTag, false);
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    *_ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;
    _geo->unticketedTransfer() = type::UnticketedTransfer::No;
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));

    _geo->unticketedTransfer() = type::UnticketedTransfer::Yes;
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));
  }

private:
  std::unique_ptr<Geo> _geo;
  std::unique_ptr<PaymentDetailMock> _paymentDetail;
  std::unique_ptr<TaxPointLoc1StopoverTagRule> _parentRule;
  std::unique_ptr<type::StopoverTag> _stopoverTag;
  std::unique_ptr<type::TicketedPointTag> _ticketedPointTag;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxPointLoc1StopoverTagApplicatorTest);
} // namespace tax
