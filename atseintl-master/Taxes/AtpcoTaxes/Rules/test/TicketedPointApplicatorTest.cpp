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

#include "Common/TaxName.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Request.h"
#include "Processor/RequestAnalyzer.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "Rules/TicketedPointApplicator.h"
#include "Rules/TicketedPointRule.h"
#include "ServiceInterfaces/Services.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "TestServer/Facades/RulesRecordsServiceServer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/XmlAndDiagnosticFixture.h"
#include "test/mainPath.h"

namespace tax
{

namespace
{
const type::CurrencyCode noCurrency(UninitializedCode);
}

class TicketedPointApplicatorTest : public XmlAndDiagnosticFixture
{
  CPPUNIT_TEST_SUITE(TicketedPointApplicatorTest);

  CPPUNIT_TEST(testLoc1Unticketed);
  CPPUNIT_TEST(testForwardTicketed);
  CPPUNIT_TEST(testForwardSkipUnticketed);
  CPPUNIT_TEST(testBackwardTicketed);
  CPPUNIT_TEST(testBackwardSkipUnticketed);

  CPPUNIT_TEST_SUITE_END();

  TaxName* _taxName;
  RulesRecordsServiceServer _rulesRecordsServiceServer;

public:
  void setUp()
  {
    _services.setRulesRecordsService(&_rulesRecordsServiceServer);
    _services.setFallbackService(new FallbackServiceServer());

    loadXml(mainPath + "Rules/test/data/loc2IntlDom.xml");
    RequestAnalyzer(*_request, _services).analyze();
    _taxName = new TaxName();
  }

  void tearDown()
  {
    delete (&_services.rulesRecordsService());
    delete _taxName;
  }

  void testLoc1Unticketed()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);

    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    PaymentDetail paymentDetail(PaymentRuleData(0, ticketedPointTag, noUnits, 0, noCurrency,
                                  type::TaxAppliesToTagInd::Blank),
                                _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos()[3],
                                _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos()[4],
                                *_taxName);
    TicketedPointRule rule;
    TicketedPointApplicator applicator(&rule,
                                       _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }

  void testForwardTicketed()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);

    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    PaymentDetail paymentDetail(PaymentRuleData(0, ticketedPointTag, noUnits, 0, noCurrency,
                                  type::TaxAppliesToTagInd::Blank),
                                _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos()[0],
                                _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos()[1],
                                *_taxName);
    TicketedPointRule rule;
    TicketedPointApplicator applicator(&rule,
                                       _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(1), paymentDetail.getTaxPointEnd().id());
  }

  void testForwardSkipUnticketed()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);

    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    PaymentDetail paymentDetail(PaymentRuleData(0, ticketedPointTag, noUnits, 0, noCurrency,
                                  type::TaxAppliesToTagInd::Blank),
                                _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos()[2],
                                _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos()[3],
                                *_taxName);
    TicketedPointRule rule;
    TicketedPointApplicator applicator(&rule,
                                       _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(5), paymentDetail.getTaxPointEnd().id());
  }

  void testBackwardTicketed()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);

    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    PaymentDetail paymentDetail(PaymentRuleData(0, ticketedPointTag, noUnits, 0, noCurrency,
                                  type::TaxAppliesToTagInd::Blank),
                                _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos()[7],
                                _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos()[6],
                                *_taxName);
    TicketedPointRule rule;
    TicketedPointApplicator applicator(&rule,
                                       _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(6), paymentDetail.getTaxPointEnd().id());
  }

  void testBackwardSkipUnticketed()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);

    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    PaymentDetail paymentDetail(PaymentRuleData(0, ticketedPointTag, noUnits, 0, noCurrency,
                                  type::TaxAppliesToTagInd::Blank),
                                _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos()[5],
                                _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos()[4],
                                *_taxName);
    TicketedPointRule rule;
    TicketedPointApplicator applicator(&rule,
                                       _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(2), paymentDetail.getTaxPointEnd().id());
  }

  void testSaleTaxPoint()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);

    TaxableUnitTagSet noUnits = TaxableUnitTagSet::none();
    PaymentDetail paymentDetail(PaymentRuleData(0, ticketedPointTag, noUnits, 0, noCurrency,
                                  type::TaxAppliesToTagInd::Blank),
                                _request->posTaxPoints()[0],
                                _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos()[7],
                                *_taxName);
    TicketedPointRule rule;
    TicketedPointApplicator applicator(&rule,
                                       _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(7), paymentDetail.getTaxPointEnd().id());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TicketedPointApplicatorTest);
}
