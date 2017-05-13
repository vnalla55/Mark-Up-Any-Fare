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
#include "Rules/TaxPointLoc2CompareApplicator.h"
#include "Rules/TaxPointLoc2CompareRule.h"
#include "ServiceInterfaces/Services.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "test/include/CppUnitHelperMacros.h"

// TODO: remove XmlParser from unit tests
#include "test/XmlAndDiagnosticFixture.h"
#include "test/mainPath.h"

namespace tax
{

namespace
{
  const type::CurrencyCode noCurrency(UninitializedCode);
}

class TaxPointLoc2CompareTest : public XmlAndDiagnosticFixture
{
  CPPUNIT_TEST_SUITE(TaxPointLoc2CompareTest);

  CPPUNIT_TEST(testLoc1Departure_Loc2sameCityAsPreviousTicketed);
  CPPUNIT_TEST(testLoc1Departure_Loc2differentCity);
  CPPUNIT_TEST(testLoc1Departure_Loc2differentCityUnticketed);
  CPPUNIT_TEST(testLoc1Arrival_Loc2sameCity);
  CPPUNIT_TEST(testLoc1Departure_Loc2mustBeStop);
  CPPUNIT_TEST(testLoc1Departure_Loc2mustBeStopOpen);
  CPPUNIT_TEST(testLoc1Arrival_Loc2differentCity);

  CPPUNIT_TEST_SUITE_END();

  TaxName* _taxName;

public:
  void setUp()
  {
    _services.setFallbackService(new FallbackServiceServer());

    _taxName = new TaxName();
    loadXml(mainPath + "Rules/test/data/loc2compare.xml");
    RequestAnalyzer(*_request, _services).analyze();
  }

  void tearDown()
  {
    delete _taxName;
  }

  void testLoc1Departure_Loc2sameCityAsPreviousTicketed()
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
    for (unsigned i = 0; i < geos.size(); ++i)
    {
      paymentDetail.getMutableTaxPointsProperties()[i].setIsSurface(false);
    }

    TaxPointLoc2CompareRule rule(type::TaxPointLoc2Compare::Point, ticketedPointTag);
    TaxPointLoc2CompareApplicator applicator(rule);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }

  void testLoc1Departure_Loc2differentCityUnticketed()
  {
    type::TicketedPointTag ticketedPointTag(
        type::TicketedPointTag::MatchTicketedAndUnticketedPoints);

    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();

    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[6],
        geos[7],
        *_taxName);

    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    for (unsigned i = 0; i < geos.size(); ++i)
    {
      paymentDetail.getMutableTaxPointsProperties()[i].setIsSurface(false);
    }

    TaxPointLoc2CompareRule rule(type::TaxPointLoc2Compare::Point, ticketedPointTag);
    TaxPointLoc2CompareApplicator applicator(rule);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
  }

  void testLoc1Departure_Loc2differentCity()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);

    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();

    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[2],
        geos[5],
        *_taxName);

    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    for (unsigned i = 0; i < geos.size(); ++i)
    {
      paymentDetail.getMutableTaxPointsProperties()[i].setIsSurface(false);
    }

    TaxPointLoc2CompareRule rule(type::TaxPointLoc2Compare::Point, ticketedPointTag);
    TaxPointLoc2CompareApplicator applicator(rule);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
  }

  void testLoc1Arrival_Loc2sameCity()
  {
    type::TicketedPointTag ticketedPointTag(type::TicketedPointTag::MatchTicketedPointsOnly);

    std::vector<Geo>& geos = _request->geoPaths()[_request->allItins()[0].geoPathRefId()].geos();

    PaymentDetail paymentDetail(
        PaymentRuleData(0, ticketedPointTag, TaxableUnitTagSet::none(), 0, noCurrency,
            type::TaxAppliesToTagInd::Blank),
        geos[5],
        geos[2],
        *_taxName);

    paymentDetail.getMutableTaxPointsProperties().resize(geos.size());
    for (unsigned i = 0; i < geos.size(); ++i)
    {
      paymentDetail.getMutableTaxPointsProperties()[i].setIsSurface(false);
    }

    TaxPointLoc2CompareRule rule(type::TaxPointLoc2Compare::Point, ticketedPointTag);
    TaxPointLoc2CompareApplicator applicator(rule);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }

  void testLoc1Departure_Loc2mustBeStop()
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
    for (unsigned i = 0; i < geos.size(); ++i)
    {
      paymentDetail.getMutableTaxPointsProperties()[i].setIsSurface(false);
    }
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;

    TaxPointLoc2CompareRule rule(type::TaxPointLoc2Compare::Stopover, ticketedPointTag);
    TaxPointLoc2CompareApplicator applicator(rule);

    CPPUNIT_ASSERT(applicator.apply(paymentDetail));
  }

  void testLoc1Departure_Loc2mustBeStopOpen()
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
    for (unsigned i = 0; i < geos.size(); ++i)
    {
      paymentDetail.getMutableTaxPointsProperties()[i].setIsSurface(false);
    }
    paymentDetail.getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail.getMutableTaxPointsProperties()[1].isOpen = true;
    paymentDetail.getMutableTaxPointsProperties()[2].isOpen = true;
    paymentDetail.getMutableTaxPointsProperties()[3].isOpen = true;
    paymentDetail.getMutableTaxPointsProperties()[4].isOpen = true;

    TaxPointLoc2CompareRule rule(type::TaxPointLoc2Compare::Stopover, ticketedPointTag);
    TaxPointLoc2CompareApplicator applicator(rule);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }

  void testLoc1Arrival_Loc2differentCity()
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
    for (unsigned i = 0; i < geos.size(); ++i)
    {
      paymentDetail.getMutableTaxPointsProperties()[i].setIsSurface(false);
    }

    TaxPointLoc2CompareRule rule(type::TaxPointLoc2Compare::Point, ticketedPointTag);
    TaxPointLoc2CompareApplicator applicator(rule);

    CPPUNIT_ASSERT(!applicator.apply(paymentDetail));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxPointLoc2CompareTest);

} // namespace tse
