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

#include "Rules/ContinuousJourneyLimitApplicator.h"
#include "Rules/ContinuousJourneyLimitRule.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "Common/TaxName.h"
#include "TestServer/Facades/LocServiceServer.h"

#include <memory>
#include <set>

namespace tax
{

class ContinuousJourneyLimitApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ContinuousJourneyLimitApplicatorTest);
  CPPUNIT_TEST(applyIfRawPaymentsIsEmpty);
  CPPUNIT_TEST(applyIfPaymentDetailNotFound);
  CPPUNIT_TEST(applyIfFirstPaymentInContinuousJourney);
  CPPUNIT_TEST(applyIfSecondPaymentInContinuousJourney);
  CPPUNIT_TEST(applyIfThirdPaymentInContinuousJourney);

  CPPUNIT_TEST_SUITE_END();

public:
  void prepareData()
  {
    std::vector<Geo>& geos = _geoPath->geos();

    _flights.clear();

    for (int flight = 0; flight < 8; flight++)
    {
      geos.push_back(Geo());
      geos.push_back(Geo());

      const type::Index geoId = flight * 2;
      geos[geoId].id() = geoId;
      geos[geoId + 1].id() = geoId + 1;

      _flightUsages.push_back(new FlightUsage());
      _flights.push_back(new Flight());
      _flightUsages[flight].flight() = &_flights[flight];
    }

    geos[0].loc().cityCode() = "LAX";
    geos[0].loc().code() = "LAX";
    geos[0].loc().nation() = "CA";
    geos[1].loc().cityCode() = "DEN";
    geos[1].loc().code() = "DEN";
    geos[1].loc().nation() = "CA";
    geos[2].loc().cityCode() = "DEN";
    geos[2].loc().code() = "DEN";
    geos[2].loc().nation() = "CA";
    geos[3].loc().cityCode() = "CHI";
    geos[3].loc().code() = "CHI";
    geos[3].loc().nation() = "CA";
    geos[4].loc().cityCode() = "CHI";
    geos[4].loc().code() = "CHI";
    geos[4].loc().nation() = "CA";
    geos[5].loc().cityCode() = "DEN";
    geos[5].loc().code() = "DEN";
    geos[5].loc().nation() = "CA";
    geos[6].loc().cityCode() = "DEN";
    geos[6].loc().code() = "DEN";
    geos[6].loc().nation() = "CA";
    geos[7].loc().cityCode() = "PHX";
    geos[7].loc().code() = "PHX";
    geos[7].loc().nation() = "CA";
    geos[8].loc().cityCode() = "PHX";
    geos[8].loc().code() = "PHX";
    geos[8].loc().nation() = "CA";
    geos[9].loc().cityCode() = "LAX";
    geos[9].loc().code() = "LAX";
    geos[9].loc().nation() = "CA";
    geos[10].loc().cityCode() = "LAX";
    geos[10].loc().code() = "LAX";
    geos[10].loc().nation() = "CA";
    geos[11].loc().cityCode() = "CHI";
    geos[11].loc().code() = "CHI";
    geos[11].loc().nation() = "CA";
    geos[12].loc().cityCode() = "CHI";
    geos[12].loc().code() = "CHI";
    geos[12].loc().nation() = "CA";
    geos[13].loc().cityCode() = "DEN";
    geos[13].loc().code() = "DEN";
    geos[13].loc().nation() = "CA";
    geos[14].loc().cityCode() = "DEN";
    geos[14].loc().code() = "DEN";
    geos[14].loc().nation() = "CA";
    geos[15].loc().cityCode() = "MIA";
    geos[15].loc().code() = "MIA";
    geos[15].loc().nation() = "CA";

    for (unsigned i = 0; i <= 15; i++)
    {
      if (i % 2 == 0)
      {
        geos[i].loc().tag() = type::TaxPointTag::Departure;
      }
      else
        geos[i].loc().tag() = type::TaxPointTag::Arrival;
    }
  }

  void setStopovers(PaymentDetail& paymentDetail)
  {
    paymentDetail.getMutableTaxPointsProperties().at(0).isFirst = true;
    paymentDetail.getMutableTaxPointsProperties().at(15).isLast = true;

    for (unsigned index = 0; index <= 10; index++)
    {
      paymentDetail.getMutableTaxPointsProperties().at(index).isTimeStopover = true;
    }
  }

  void createApplicator(RawPayments& rawPayments)
  {
    _rule.reset(new ContinuousJourneyLimitRule());
    _applicator.reset(
      new ContinuousJourneyLimitApplicator(*_rule, *_geoPath, _locService, rawPayments));
  }

  PaymentDetail&
  emplaceRawPayment(RawPayments& payments, const type::Index id, TaxName& taxName,
                    type::TicketedPointTag ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly)
  {
    PaymentDetail& detail = payments.emplace_back(
      PaymentRuleData(type::SeqNo(),
        ticketedPointTag,
        TaxableUnitTagSet::none(),
        0,
        type::CurrencyCode(UninitializedCode),
        type::TaxAppliesToTagInd::Blank),
      _geoPath->geos()[id],
      Geo(),
      taxName);
    detail.getMutableTaxPointsProperties().resize(_geoPath->geos().size());
    return detail;
  }

  void setUp()
  {
    _geoPath.reset(new GeoPath);
    _flightUsages.clear();
    prepareData();
  }

  void tearDown()
  {
    _rule.reset();
    _applicator.reset();
  }

  void applyIfRawPaymentsIsEmpty()
  {
    type::Index taxPointBegin = 0;
    RawPayments rawPayments(RawPayments::WithCapacity(10));
    TaxName taxName;
    PaymentDetail& paymentDetail = emplaceRawPayment(rawPayments, taxPointBegin, taxName);

    createApplicator(rawPayments);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(paymentDetail));
    CPPUNIT_ASSERT_EQUAL(false, paymentDetail.isFailed());
  }

  void applyIfPaymentDetailNotFound()
  {
    type::Index taxPointBegin = 0;
    RawPayments rawPayments(RawPayments::WithCapacity(10));

    TaxName taxName1;
    taxName1.taxCode() = "AA";
    TaxName taxName2;
    taxName2.taxCode() = "AB";

    PaymentDetail& paymentDetail1 = emplaceRawPayment(rawPayments, taxPointBegin, taxName1);
    paymentDetail1.setCalculated();
    PaymentDetail& paymentDetail2 = emplaceRawPayment(rawPayments, taxPointBegin, taxName2);

    createApplicator(rawPayments);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(paymentDetail2));
    CPPUNIT_ASSERT_EQUAL(false, paymentDetail2.isFailed());
  }

  void applyIfFirstPaymentInContinuousJourney()
  {
    type::Index taxPointBegin = 0;
    TaxName taxName;

    RawPayments rawPayments(RawPayments::WithCapacity(10));
    PaymentDetail& paymentDetail = emplaceRawPayment(rawPayments, taxPointBegin, taxName);
    setStopovers(paymentDetail);

    createApplicator(rawPayments);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(paymentDetail));
  }

  void applyIfSecondPaymentInContinuousJourney()
  {
    type::Index taxPointBegin = 0;
    TaxName taxName;
    RawPayments rawPayments(RawPayments::WithCapacity(10));

    PaymentDetail& paymentDetail1 = emplaceRawPayment(rawPayments, taxPointBegin, taxName);
    setStopovers(paymentDetail1);
    paymentDetail1.setCalculated();
    PaymentDetail& paymentDetail2 = emplaceRawPayment(rawPayments, taxPointBegin, taxName);
    setStopovers(paymentDetail2);

    createApplicator(rawPayments);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(paymentDetail2));
  }

  void applyIfThirdPaymentInContinuousJourney()
  {
    type::Index taxPointBegin = 0;
    TaxName taxName;
    RawPayments rawPayments(RawPayments::WithCapacity(10));

    PaymentDetail& paymentDetail1 = emplaceRawPayment(rawPayments, taxPointBegin, taxName);
    setStopovers(paymentDetail1);
    paymentDetail1.setCalculated();
    PaymentDetail& paymentDetail2 = emplaceRawPayment(rawPayments, taxPointBegin, taxName);
    setStopovers(paymentDetail2);
    paymentDetail2.setCalculated();
    PaymentDetail& paymentDetail3 = emplaceRawPayment(rawPayments, taxPointBegin, taxName);
    setStopovers(paymentDetail3);
    paymentDetail3.setCalculated();

    createApplicator(rawPayments);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(paymentDetail3));
  }

private:
  std::unique_ptr<ContinuousJourneyLimitRule> _rule;
  std::unique_ptr<ContinuousJourneyLimitApplicator> _applicator;

  std::unique_ptr<GeoPath> _geoPath;
  boost::ptr_vector<FlightUsage> _flightUsages;
  boost::ptr_vector<Flight> _flights;
  LocServiceServer _locService;
};

CPPUNIT_TEST_SUITE_REGISTRATION(ContinuousJourneyLimitApplicatorTest);
} // namespace tax
