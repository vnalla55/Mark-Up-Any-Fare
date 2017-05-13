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

#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Request.h"
#include "Processor/RequestAnalyzer.h"
#include "Rules/ContinuousJourneyApplicator.h"
#include "Rules/ContinuousJourneyRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "ServiceInterfaces/RulesRecordsService.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "test/XmlAndDiagnosticFixture.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/mainPath.h"

namespace tax
{

namespace
{
  const type::CurrencyCode noCurrency(UninitializedCode);
}

class ContinuousJourneyTest : public XmlAndDiagnosticFixture
{
  CPPUNIT_TEST_SUITE(ContinuousJourneyTest);

  CPPUNIT_TEST(test1stInItin);
  CPPUNIT_TEST(test2ndInItin);
  CPPUNIT_TEST(test2ndInItin_ticketedAndUnticketed_match);
  CPPUNIT_TEST(test2ndInItin_ticketedOnly_fail);
  CPPUNIT_TEST(test1stInItin_invalid);
  CPPUNIT_TEST(test2ndInItin_invalid);
  CPPUNIT_TEST(test1stCJ_3rd);
  CPPUNIT_TEST(test2ndCJ_3rd);
  CPPUNIT_TEST(test2ndCJ_3rd_ticketedAndUnticketed_fail);
  CPPUNIT_TEST(test2ndCJ_3rd_ticketedOnly_match);
  CPPUNIT_TEST(test1stCJ_NotStopover);
  CPPUNIT_TEST(test2ndCJ_NotStopover);
  CPPUNIT_TEST(test1stCJ);
  CPPUNIT_TEST(test2ndCJ);

  CPPUNIT_TEST_SUITE_END();

  TaxName* _taxName;

public:
  void setUp()
  {
    _services.setFallbackService(new FallbackServiceServer());

    loadXml(mainPath + "Rules/test/data/continuous.xml");
    RequestAnalyzer(*_request, _services).analyze();
    _taxName = new TaxName();
  }

  void tearDown()
  {
    delete (&_services.rulesRecordsService());
    delete _taxName;
  }

  void test1stInItin()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[0],
        geos[1],
        *_taxName);
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney1stPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
  }

  void test2ndInItin()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[2],
        geos[3],
        *_taxName);
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[1].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[2].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[3].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[4].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney2ndPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
  }

  void test2ndInItin_ticketedAndUnticketed_match()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[2],
        geos[3],
        *_taxName);
    geos[1].unticketedTransfer() = type::UnticketedTransfer::Yes;
    geos[2].unticketedTransfer() = type::UnticketedTransfer::Yes;
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[1].isTimeStopover = false;
    paymentDetail.getMutableTaxPointsProperties()[2].isTimeStopover = false;
    paymentDetail.getMutableTaxPointsProperties()[3].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[4].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney2ndPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
  }

  void test2ndInItin_ticketedOnly_fail()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[2],
        geos[3],
        *_taxName);
    geos[1].unticketedTransfer() = type::UnticketedTransfer::Yes;
    geos[2].unticketedTransfer() = type::UnticketedTransfer::Yes;
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[1].isTimeStopover = false;
    paymentDetail.getMutableTaxPointsProperties()[2].isTimeStopover = false;
    paymentDetail.getMutableTaxPointsProperties()[3].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[4].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney2ndPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }

  void test1stInItin_invalid()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[2],
        geos[3],
        *_taxName);
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney1stPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }

  void test2ndInItin_invalid()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[0],
        geos[1],
        *_taxName);
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney2ndPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }

  void test1stCJ_3rd()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[4],
        geos[5],
        *_taxName);
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[1].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[2].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[3].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[4].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[5].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[6].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney1stPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }

  void test2ndCJ_3rd()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[4],
        geos[5],
        *_taxName);
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[1].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[2].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[3].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[4].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[5].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[6].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney2ndPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }

  void test2ndCJ_3rd_ticketedAndUnticketed_fail()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedAndUnticketedPoints);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[4],
        geos[5],
        *_taxName);
    geos[1].unticketedTransfer() = type::UnticketedTransfer::Yes;
    geos[2].unticketedTransfer() = type::UnticketedTransfer::Yes;
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[1].isTimeStopover = false;
    paymentDetail.getMutableTaxPointsProperties()[2].isTimeStopover = false;
    paymentDetail.getMutableTaxPointsProperties()[3].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[4].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[5].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[6].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney2ndPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }

  void test2ndCJ_3rd_ticketedOnly_match()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[4],
        geos[5],
        *_taxName);
    geos[1].unticketedTransfer() = type::UnticketedTransfer::Yes;
    geos[2].unticketedTransfer() = type::UnticketedTransfer::Yes;
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[1].isTimeStopover = false;
    paymentDetail.getMutableTaxPointsProperties()[2].isTimeStopover = false;
    paymentDetail.getMutableTaxPointsProperties()[3].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[4].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[5].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[6].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney2ndPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
  }

  void test1stCJ_NotStopover()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[6],
        geos[7],
        *_taxName);
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[3].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[4].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney1stPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }

  void test2ndCJ_NotStopover()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[8],
        geos[9],
        *_taxName);
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[3].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[4].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney1stPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }

  void test1stCJ()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[6],
        geos[7],
        *_taxName);
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[1].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[2].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[5].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[6].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[7].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[8].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[11].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[12].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney1stPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
  }

  void test2ndCJ()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);
    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();
    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[8],
        geos[9],
        *_taxName);
    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[1].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[2].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[5].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[6].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[7].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[8].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[11].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[12].isTimeStopover = true;
    paymentDetail.getMutableTaxPointsProperties()[13].isLast = true;

    ContinuousJourneyRule rule(type::RtnToOrig::ContinuousJourney2ndPoint);
    ContinuousJourneyApplicator applicator(
        rule, _request->geoPaths()[_request->allItins()[0].geoPathRefId()]);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ContinuousJourneyTest);
}
