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
#include "Rules/SingleJourneyLimitApplicator.h"
#include "Rules/SingleJourneyLimitRule.h"

#include "Common/TaxName.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "TestServer/Facades/LocServiceServer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "Rules/test/RawPaymentsHelper.h"

#include <memory>
#include <set>

namespace tax
{

class SingleJourneyLimitApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SingleJourneyLimitApplicatorTest);
  CPPUNIT_TEST(applyIfRawPaymentsIsEmpty);
  CPPUNIT_TEST(applyIfPaymentDetailNotFound);
  CPPUNIT_TEST(applyIfNotSingleJourney);
  CPPUNIT_TEST(applyIfOnePerSingleJourney);
  CPPUNIT_TEST(applyIfSecondPaymentInSingleJourney);

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
    geos[0].loc().nation() = "US";
    geos[1].loc().cityCode() = "DEN";
    geos[1].loc().code() = "DEN";
    geos[1].loc().nation() = "US";
    geos[2].loc().cityCode() = "DEN";
    geos[2].loc().code() = "DEN";
    geos[2].loc().nation() = "US";
    geos[3].loc().cityCode() = "CHI";
    geos[3].loc().code() = "CHI";
    geos[3].loc().nation() = "US";
    geos[4].loc().cityCode() = "CHI";
    geos[4].loc().code() = "CHI";
    geos[4].loc().nation() = "US";
    geos[5].loc().cityCode() = "DEN";
    geos[5].loc().code() = "DEN";
    geos[5].loc().nation() = "US";
    geos[6].loc().cityCode() = "DEN";
    geos[6].loc().code() = "DEN";
    geos[6].loc().nation() = "US";
    geos[7].loc().cityCode() = "PHX";
    geos[7].loc().code() = "PHX";
    geos[7].loc().nation() = "US";
    geos[8].loc().cityCode() = "PHX";
    geos[8].loc().code() = "PHX";
    geos[8].loc().nation() = "US";
    geos[9].loc().cityCode() = "LAX";
    geos[9].loc().code() = "LAX";
    geos[9].loc().nation() = "US";
    geos[10].loc().cityCode() = "LAX";
    geos[10].loc().code() = "LAX";
    geos[10].loc().nation() = "US";
    geos[11].loc().cityCode() = "CHI";
    geos[11].loc().code() = "CHI";
    geos[11].loc().nation() = "US";
    geos[12].loc().cityCode() = "CHI";
    geos[12].loc().code() = "CHI";
    geos[12].loc().nation() = "US";
    geos[13].loc().cityCode() = "DEN";
    geos[13].loc().code() = "DEN";
    geos[13].loc().nation() = "US";
    geos[14].loc().cityCode() = "DEN";
    geos[14].loc().code() = "DEN";
    geos[14].loc().nation() = "US";
    geos[15].loc().cityCode() = "MIA";
    geos[15].loc().code() = "MIA";
    geos[15].loc().nation() = "US";

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
    _rule.reset(new SingleJourneyLimitRule());
    _applicator.reset(
      new SingleJourneyLimitApplicator(_rule.get(), *_geoPath, _locService, rawPayments));
  }

  void setUp()
  {
    _geoPath.reset(new GeoPath);
    _flightUsages.clear();
    prepareData();
    _helper.setGeoPath(*_geoPath);
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
    PaymentDetail& paymentDetail = _helper.emplace(rawPayments, taxPointBegin, taxName);

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
    PaymentDetail& paymentDetail1 = _helper.emplace(rawPayments, taxPointBegin, taxName1);
    paymentDetail1.setCalculated();

    TaxName taxName2;
    taxName2.taxCode() = "AB";
    PaymentDetail& paymentDetail2 = _helper.emplace(rawPayments, taxPointBegin, taxName2);

    createApplicator(rawPayments);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(paymentDetail2));
    CPPUNIT_ASSERT_EQUAL(false, paymentDetail2.isFailed());
  }

  void applyIfNotSingleJourney()
  {
    type::Index taxPointBegin = 0;
    RawPayments rawPayments(RawPayments::WithCapacity(10));

    TaxName taxName;
    PaymentDetail& paymentDetail1 = _helper.emplace(rawPayments, taxPointBegin, taxName);
    paymentDetail1.setCalculated();
    PaymentDetail& paymentDetail2 = _helper.emplace(rawPayments, taxPointBegin, taxName);

    createApplicator(rawPayments);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(paymentDetail2));
    CPPUNIT_ASSERT_EQUAL(false, paymentDetail2.isFailed());
  }

  void applyIfOnePerSingleJourney()
  {
    type::Index taxPointBegin = 0;
    TaxName taxName;
    RawPayments rawPayments(RawPayments::WithCapacity(10));
    PaymentDetail& paymentDetail = _helper.emplace(rawPayments, taxPointBegin, taxName);
    setStopovers(paymentDetail);

    createApplicator(rawPayments);

    CPPUNIT_ASSERT_EQUAL(true, _applicator->apply(paymentDetail));
  }

  void applyIfSecondPaymentInSingleJourney()
  {
    type::Index taxPointBegin = 0;
    TaxName taxName;
    RawPayments rawPayments(RawPayments::WithCapacity(10));

    PaymentDetail& paymentDetail1 = _helper.emplace(rawPayments, taxPointBegin, taxName);
    PaymentDetail& paymentDetail2 = _helper.emplace(rawPayments, taxPointBegin, taxName);
    setStopovers(paymentDetail1);
    setStopovers(paymentDetail2);
    paymentDetail1.setCalculated();

    createApplicator(rawPayments);

    CPPUNIT_ASSERT_EQUAL(false, _applicator->apply(paymentDetail2));
  }

private:
  std::unique_ptr<SingleJourneyLimitRule> _rule;
  std::unique_ptr<SingleJourneyLimitApplicator> _applicator;

  std::unique_ptr<GeoPath> _geoPath;
  boost::ptr_vector<FlightUsage> _flightUsages;
  boost::ptr_vector<Flight> _flights;
  LocServiceServer _locService;
  RawPaymentsHelper _helper;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SingleJourneyLimitApplicatorTest);
} // namespace tax
