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

#include "Rules/BlankLimitApplicator.h"
#include "Rules/BlankLimitRule.h"
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

class BlankLimitApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BlankLimitApplicatorTest);
  CPPUNIT_TEST(applyIfRawPaymentsIsEmpty);
  CPPUNIT_TEST(applyIfPaymentDetailNotFound);
  CPPUNIT_TEST(applyIfPaymentDetailFound);
  CPPUNIT_TEST(applyIfFirstPaymentIsHigher);
  CPPUNIT_TEST(applyIfSecondPaymentIsHigher);
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

  void createApplicator(RawPayments& rawPayments)
  {
    _rule.reset(new BlankLimitRule());
    _applicator.reset(new BlankLimitApplicator(_rule.get(), rawPayments));
  }

  PaymentDetail&
  emplacePaymentDetail(RawPayments& rawPayments, const type::Index id, TaxName& taxName,
                       type::TicketedPointTag ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly)
  {
    
    rawPayments.emplace_back(
      PaymentRuleData(type::SeqNo(),
                      ticketedPointTag,
                      TaxableUnitTagSet::none(),
                      0,
                      type::CurrencyCode(UninitializedCode),
                      type::TaxAppliesToTagInd::Blank),
      _geoPath->geos()[id],
      Geo(),
      taxName
    );
    return rawPayments.back().detail;
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
    PaymentDetail& paymentDetail = emplacePaymentDetail(rawPayments, taxPointBegin, taxName);
    createApplicator(rawPayments);

    CPPUNIT_ASSERT(_applicator->apply(paymentDetail));
    CPPUNIT_ASSERT(!paymentDetail.isFailed());
  }

  void applyIfPaymentDetailNotFound()
  {
    type::Index taxPointBegin = 0;
    RawPayments rawPayments(RawPayments::WithCapacity(10));

    TaxName taxName1;
    taxName1.taxCode() = "AA";

    TaxName taxName2;
    taxName2.taxCode() = "AB";

    emplacePaymentDetail(rawPayments, taxPointBegin, taxName1);
    PaymentDetail& paymentDetail2 = emplacePaymentDetail(rawPayments, taxPointBegin, taxName2);

    createApplicator(rawPayments);

    CPPUNIT_ASSERT(_applicator->apply(paymentDetail2));
    CPPUNIT_ASSERT(!paymentDetail2.isFailed());
  }

  void applyIfPaymentDetailFound()
  {
    type::Index taxPointBegin = 0;
    RawPayments rawPayments(RawPayments::WithCapacity(10));

    TaxName taxName;
    emplacePaymentDetail(rawPayments, taxPointBegin, taxName);
    PaymentDetail& paymentDetail2 = emplacePaymentDetail(rawPayments, taxPointBegin, taxName);

    createApplicator(rawPayments);
    CPPUNIT_ASSERT(_applicator->apply(paymentDetail2));
  }

  void applyIfFirstPaymentIsHigher()
  {
    type::Index taxPointBegin1 = 0;
    type::Index taxPointBegin2 = 1;

    TaxName taxName;
    RawPayments rawPayments(RawPayments::WithCapacity(10));

    PaymentDetail& paymentDetail1 = emplacePaymentDetail(rawPayments, taxPointBegin1, taxName);
    paymentDetail1.taxAmt() = 20;
    paymentDetail1.taxApplicationLimit() = type::TaxApplicationLimit::OnceForItin;
    paymentDetail1.setCalculated();

    PaymentDetail& paymentDetail2 = emplacePaymentDetail(rawPayments, taxPointBegin2, taxName);
    paymentDetail2.taxAmt() = 10;

    createApplicator(rawPayments);

    CPPUNIT_ASSERT(_applicator->apply(paymentDetail2));
    CPPUNIT_ASSERT(!paymentDetail1.isFailed());
  }

  void applyIfSecondPaymentIsHigher()
  {
    type::Index taxPointBegin1 = 0;
    type::Index taxPointBegin2 = 1;

    TaxName taxName;
    RawPayments rawPayments(RawPayments::WithCapacity(10));

    PaymentDetail& paymentDetail1 = emplacePaymentDetail(rawPayments, taxPointBegin1, taxName);
    paymentDetail1.taxAmt() = 10;
    paymentDetail1.taxApplicationLimit() = type::TaxApplicationLimit::OnceForItin;
    paymentDetail1.setCalculated();

    PaymentDetail& paymentDetail2 = emplacePaymentDetail(rawPayments, taxPointBegin2, taxName);
    paymentDetail2.taxAmt() = 20;

    createApplicator(rawPayments);
    CPPUNIT_ASSERT(_applicator->apply(paymentDetail2));
    CPPUNIT_ASSERT(paymentDetail1.isFailed());
  }

private:
  std::unique_ptr<BlankLimitRule> _rule;
  std::unique_ptr<BlankLimitApplicator> _applicator;

  std::unique_ptr<GeoPath> _geoPath;
  boost::ptr_vector<FlightUsage> _flightUsages;
  boost::ptr_vector<Flight> _flights;
  LocServiceServer _locService;
};

CPPUNIT_TEST_SUITE_REGISTRATION(BlankLimitApplicatorTest);
} // namespace tax
