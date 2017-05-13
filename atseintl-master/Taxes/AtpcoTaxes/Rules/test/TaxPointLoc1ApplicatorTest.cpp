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

#include "Rules/TaxPointLoc1Applicator.h"
#include "Rules/TaxPointLoc1Rule.h"

#include "Common/TaxName.h"
#include "Common/LocZone.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Geo.h"
#include "Rules/BusinessRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"

#include "test/LocServiceMock.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tax
{

class TaxPointLoc1ApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxPointLoc1ApplicatorTest);

  CPPUNIT_TEST(testApplyTaxPointLoc1Applicator);
  CPPUNIT_TEST(testApplyWithOCTaxPointLoc1Applicator);
  CPPUNIT_TEST(testApplyWithBaggageTaxPointLoc1Applicator);
  CPPUNIT_TEST(testApplyWithIgnoredOCTaxPointLoc1Applicator);
  CPPUNIT_TEST(testDontApplyTaxPointLoc1Applicator);
  CPPUNIT_TEST(testDontApplyWithOCTaxPointLoc1Applicator);
  CPPUNIT_TEST(testDontApplyWithBaggageTaxPointLoc1Applicator);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _locZone = new LocZone();
    _vendor = new type::Vendor();
    _parent = new TaxPointLoc1Rule(*_locZone, *_vendor);
    _locServiceMock = new LocServiceMock();
    _seqNo = new type::SeqNo();
    _geo = new Geo();
    _ticketedPointTag =
      new type::TicketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints);
    _taxName = new TaxName();
    _paymentDetail = new PaymentDetail(
        PaymentRuleData(*_seqNo, *_ticketedPointTag, TaxableUnitTagSet::none(), 0,
            type::CurrencyCode(UninitializedCode),
            type::TaxAppliesToTagInd::Blank),
        *_geo,
        *_geo,
        *_taxName);
  }

  void tearDown()
  {
    delete _locZone;
    delete _vendor;
    delete _parent;
    delete _locServiceMock;
    delete _seqNo;
    delete _geo;
    delete _ticketedPointTag;
    delete _paymentDetail;
    delete _taxName;
  }

  void testApplyTaxPointLoc1Applicator()
  {
    _locServiceMock->add(true);

    TaxPointLoc1Applicator applicator(*_parent, *_locServiceMock);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->isFailedRule());
  }

  void testApplyWithOCTaxPointLoc1Applicator()
  {
    _locServiceMock->add(true);
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::FlightRelated;

    TaxPointLoc1Applicator applicator(*_parent, *_locServiceMock);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->isFailedRule());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->areAllOptionalServicesFailed());
  }

  void testApplyWithBaggageTaxPointLoc1Applicator()
  {
    _locServiceMock->add(true);
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::BaggageCharge;

    TaxPointLoc1Applicator applicator(*_parent, *_locServiceMock);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->isFailedRule());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->areAllOptionalServicesFailed());
  }

  void testApplyWithIgnoredOCTaxPointLoc1Applicator()
  {
    _locServiceMock->add(false);
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::TicketRelated;

    TaxPointLoc1Applicator applicator(*_parent, *_locServiceMock);

    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->isFailedRule());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetail->areAllOptionalServicesFailed());
  }

  void testDontApplyTaxPointLoc1Applicator()
  {
    _locServiceMock->add(false);

    TaxPointLoc1Applicator applicator(*_parent, *_locServiceMock);

    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->isFailedRule());
  }

  void testDontApplyWithOCTaxPointLoc1Applicator()
  {
    _locServiceMock->add(false);
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::FlightRelated;

    TaxPointLoc1Applicator applicator(*_parent, *_locServiceMock);

    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->isFailedRule());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->areAllOptionalServicesFailed());
  }

  void testDontApplyWithBaggageTaxPointLoc1Applicator()
  {
    _locServiceMock->add(false);
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::BaggageCharge;

    TaxPointLoc1Applicator applicator(*_parent, *_locServiceMock);

    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->isFailedRule());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetail->areAllOptionalServicesFailed());
  }

private:
  TaxPointLoc1Rule* _parent;
  LocZone* _locZone;
  type::Vendor* _vendor;
  LocServiceMock* _locServiceMock;
  type::SeqNo* _seqNo;
  Geo* _geo;
  PaymentDetail* _paymentDetail;
  type::TicketedPointTag* _ticketedPointTag;
  TaxName* _taxName;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxPointLoc1ApplicatorTest);
} // namespace tax
