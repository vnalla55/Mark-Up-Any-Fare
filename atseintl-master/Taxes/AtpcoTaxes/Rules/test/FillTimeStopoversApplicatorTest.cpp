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
#include "Rules/FillTimeStopoversApplicator.h"
#include "Rules/FillTimeStopoversRule.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Itin.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "test/include/CppUnitHelperMacros.h"
#include "Common/TaxName.h"

#include <memory>

namespace tax
{

class FillTimeStopoversApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FillTimeStopoversApplicatorTest);

  CPPUNIT_TEST(testApplyAlways);
  CPPUNIT_TEST(testFirstLastGeoIsConnection);
  CPPUNIT_TEST(testLessThan24hConnection);
  CPPUNIT_TEST(testMoreThan24hStopover);

  CPPUNIT_TEST_SUITE_END();

  TaxName* _taxName;

public:
  void prepareFlights()
  {
    std::vector<Geo> geos;

    for (int flight = 0; flight < 3; flight++)
    {
      geos.push_back(Geo());
      geos.push_back(Geo());

      const type::Index geoId = flight * 2;
      geos[geoId].id() = geoId;
      geos[geoId + 1].id() = geoId + 1;

      _flightUsages.push_back(FlightUsage());
      _flights.push_back(new Flight());

      _flightUsages[flight].flight() = &_flights[flight];
    }

    _flights[0].departureTime() = type::Time(3, 00);
    _flights[0].arrivalTime() = type::Time(5, 00);
    _flights[0].arrivalDateShift() = 0;
    _flightUsages[0].markDepartureDate(type::Date(2013, 6, 4));
    _flightUsages[0].forcedConnection() = type::ForcedConnection::Blank;

    _flights[1].departureTime() = type::Time(3, 00);
    _flights[1].arrivalTime() = type::Time(5, 00);
    _flights[1].arrivalDateShift() = 0;
    _flightUsages[1].markDepartureDate(type::Date(2013, 6, 5));
    _flightUsages[1].forcedConnection() = type::ForcedConnection::Blank;

    _flights[2].departureTime() = type::Time(3, 00);
    _flights[2].arrivalTime() = type::Time(5, 00);
    _flights[2].arrivalDateShift() = 0;
    _flightUsages[2].markDepartureDate(type::Date(2013, 6, 6));
    _flightUsages[2].forcedConnection() = type::ForcedConnection::Blank;

    _geoPath.reset(new GeoPath);
    _geoPath->geos() = geos;
  }

  void prepareApplicator()
  {
    _rule.reset(new FillTimeStopoversRule("24", type::StopoverTimeUnit::Hours));
    _itin.reset(new Itin());
    _itin->flightUsages() = _flightUsages;
    _itin->geoPath() = _geoPath.get();
    _applicator.reset(new FillTimeStopoversApplicator(_rule.get(), *_itin.get()));
  }

  const std::shared_ptr<PaymentDetail> createPaymentDetail()
  {
    Geo taxPoint;
    type::TicketedPointTag ticketedPointTag =
      type::TicketedPointTag::MatchTicketedAndUnticketedPoints;

    std::shared_ptr<PaymentDetail> paymentDetail(
        new PaymentDetail(PaymentRuleData(type::SeqNo(),
                                          ticketedPointTag,
                                          TaxableUnitTagSet::none(),
                                          0,
                                          type::CurrencyCode(UninitializedCode),
                                          type::TaxAppliesToTagInd::Blank),
                          taxPoint,
                          taxPoint,
                          *_taxName));
    paymentDetail->getMutableTaxPointsProperties().resize(_geoPath->geos().size());
    return paymentDetail;
  }

  void setUp()
  {
    _taxName = new TaxName();
  }

  void tearDown()
  {
    _rule.reset();
    _applicator.reset();
    _geoPath.reset();

    _flightUsages.clear();
    _flights.clear();
    delete _taxName;
  }

  void testApplyAlways()
  {
    _geoPath.reset(new GeoPath);
    prepareApplicator();

    CPPUNIT_ASSERT(_applicator->apply(*createPaymentDetail()));
  }

  void testFirstLastGeoIsConnection()
  {
    prepareFlights();
    prepareApplicator();

    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail();
    _applicator->apply(*paymentDetail);

    CPPUNIT_ASSERT(paymentDetail->taxPointsProperties().front().isTimeStopover == boost::none);
    CPPUNIT_ASSERT(paymentDetail->taxPointsProperties().back().isTimeStopover == boost::none);
  }

  void testLessThan24hConnection()
  {
    prepareFlights();
    prepareApplicator();

    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail();
    _applicator->apply(*paymentDetail);

    CPPUNIT_ASSERT(paymentDetail->taxPointsProperties().at(1).isTimeStopover);
    CPPUNIT_ASSERT(paymentDetail->taxPointsProperties().at(2).isTimeStopover);
    CPPUNIT_ASSERT(paymentDetail->taxPointsProperties().at(3).isTimeStopover);
    CPPUNIT_ASSERT(paymentDetail->taxPointsProperties().at(4).isTimeStopover);

    CPPUNIT_ASSERT(!paymentDetail->taxPointsProperties().at(1).isTimeStopover.get());
    CPPUNIT_ASSERT(!paymentDetail->taxPointsProperties().at(2).isTimeStopover.get());
    CPPUNIT_ASSERT(!paymentDetail->taxPointsProperties().at(3).isTimeStopover.get());
    CPPUNIT_ASSERT(!paymentDetail->taxPointsProperties().at(4).isTimeStopover.get());
  }

  void testMoreThan24hStopover()
  {
    prepareFlights();
    _flightUsages[1].markDepartureDate(type::Date(2013, 6, 5));
    _flightUsages[2].markDepartureDate(type::Date(2013, 6, 7));

    prepareApplicator();

    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail();
    _applicator->apply(*paymentDetail);

    CPPUNIT_ASSERT(paymentDetail->taxPointsProperties().at(1).isTimeStopover);
    CPPUNIT_ASSERT(paymentDetail->taxPointsProperties().at(2).isTimeStopover);
    CPPUNIT_ASSERT(paymentDetail->taxPointsProperties().at(3).isTimeStopover);
    CPPUNIT_ASSERT(paymentDetail->taxPointsProperties().at(4).isTimeStopover);

    CPPUNIT_ASSERT(!paymentDetail->taxPointsProperties().at(1).isTimeStopover.get());
    CPPUNIT_ASSERT(!paymentDetail->taxPointsProperties().at(2).isTimeStopover.get());
    CPPUNIT_ASSERT(paymentDetail->taxPointsProperties().at(3).isTimeStopover.get());
    CPPUNIT_ASSERT(paymentDetail->taxPointsProperties().at(4).isTimeStopover.get());
  }

private:
  std::unique_ptr<FillTimeStopoversRule> _rule;
  std::unique_ptr<FillTimeStopoversApplicator> _applicator;
  std::unique_ptr<GeoPath> _geoPath;

  std::vector<FlightUsage> _flightUsages;
  boost::ptr_vector<Flight> _flights;
  std::unique_ptr<Itin> _itin;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FillTimeStopoversApplicatorTest);
} // namespace tax
