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
#include "Rules/TaxPointLoc1InternationalDomesticApplicator.h"
#include "Rules/TaxPointLoc1InternationalDomesticRule.h"

#include "Common/TaxName.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"

#include "test/LocServiceMock.h"
#include "test/include/CppUnitHelperMacros.h"

#include <memory>
#include <set>

namespace tax
{

class TaxPointLoc1InternationalDomesticApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxPointLoc1InternationalDomesticApplicatorTest);
  CPPUNIT_TEST(testApplyAlways);

  CPPUNIT_TEST(testInternationalStopoverArrival);
  CPPUNIT_TEST(testInternationalStopoverArrival_US_InsideBufferZone);
  CPPUNIT_TEST(testInternationalStopoverArrival_US_OutsideBufferZone);
  CPPUNIT_TEST(testInternationalStopoverDeparture);
  CPPUNIT_TEST(testInternationalStopoverDeparture_US_InsideBufferZone);
  CPPUNIT_TEST(testInternationalStopoverDeparture_US_OutsideBufferZone);

  CPPUNIT_TEST(testInternationalStopoverArrivalOpen);
  CPPUNIT_TEST(testInternationalStopoverDepartureOpen);

  CPPUNIT_TEST(testNoDomesticStopoverArrival);
  CPPUNIT_TEST(testNoDomesticStopoverDeparture);

  CPPUNIT_TEST(testNoInternationalArrival);
  CPPUNIT_TEST(testNoInternationalDeparture);

  CPPUNIT_TEST(testDomesticArrival);
  CPPUNIT_TEST(testDomesticDeparture);

  CPPUNIT_TEST(testDomesticArrivalOpen);
  CPPUNIT_TEST(testDomesticDepartureOpen);

  CPPUNIT_TEST(testUnticketedTransfer_NoDomesticArrival);
  CPPUNIT_TEST(testUnticketedTransfer_NoDomesticDeparture);

  CPPUNIT_TEST(testUnticketedTransfer_InternationalArrival);
  CPPUNIT_TEST(testUnticketedTransfer_InternationalDeparture);

  CPPUNIT_TEST_SUITE_END();

public:
  void prepareFlights()
  {
    std::vector<Geo>& geos = _geoPath->geos();

    _flights.clear();
    for (int flight = 0; flight < 3; flight++)
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

    geos[0].loc().cityCode() = "KRK";
    geos[0].loc().code() = "KRK";
    geos[0].loc().nation() = "PL";
    geos[1].loc().cityCode() = "MUC";
    geos[1].loc().code() = "MUC";
    geos[1].loc().nation() = "DE";
    geos[2].loc().cityCode() = "MUC";
    geos[2].loc().code() = "MUC";
    geos[2].loc().nation() = "DE";
    geos[3].loc().cityCode() = "FRA";
    geos[3].loc().code() = "FRA";
    geos[3].loc().nation() = "DE";
    geos[4].loc().cityCode() = "FRA";
    geos[4].loc().code() = "FRA";
    geos[4].loc().nation() = "DE";
    geos[5].loc().cityCode() = "DFW";
    geos[5].loc().code() = "DFW";
    geos[5].loc().nation() = "US";

    geos[0].loc().tag() = type::TaxPointTag::Departure;
    geos[1].loc().tag() = type::TaxPointTag::Arrival;
    geos[2].loc().tag() = type::TaxPointTag::Departure;
    geos[3].loc().tag() = type::TaxPointTag::Arrival;
    geos[4].loc().tag() = type::TaxPointTag::Departure;
    geos[5].loc().tag() = type::TaxPointTag::Arrival;
  }

  void createApplicator(const type::AdjacentIntlDomInd& adjacentIntlDomInd,
                        const type::TicketedPointTag& type)
  {
    _rule.reset(new TaxPointLoc1InternationalDomesticRule(adjacentIntlDomInd, type));
    _applicator.reset(
      new TaxPointLoc1InternationalDomesticApplicator(*_rule, *_geoPath, *_locServiceMock));
  }

  std::shared_ptr<PaymentDetail> createPaymentDetail(const type::Index id)
  {
    Geo& taxPoint1 = _geoPath->geos()[id];

    Geo taxPoint2;
    type::TicketedPointTag ticketedPointTag = type::TicketedPointTag::MatchTicketedPointsOnly;

    std::shared_ptr<PaymentDetail> paymentDetail(
        new PaymentDetail(PaymentRuleData(type::SeqNo(),
                                          ticketedPointTag,
                                          TaxableUnitTagSet::none(),
                                          0,
                                          type::CurrencyCode(UninitializedCode),
                                          type::TaxAppliesToTagInd::Blank),
                          taxPoint1,
                          taxPoint2,
                          *_taxName));

    paymentDetail->getMutableTaxPointsProperties().resize(_geoPath->geos().size());
    paymentDetail->getMutableTaxPointsProperties().at(0).isFirst = true;
    paymentDetail->getMutableTaxPointsProperties().at(5).isLast = true;
    return paymentDetail;
  }

  void setUnticketedTransfer(type::Index arrivalId, type::Index departureId)
  {
    std::vector<Geo>& geos = _geoPath->geos();
    geos[arrivalId].unticketedTransfer() = type::UnticketedTransfer::Yes;
    geos[departureId].unticketedTransfer() = type::UnticketedTransfer::Yes;
  }

  void setUp()
  {
    _geoPath.reset(new GeoPath);
    _locServiceMock = new LocServiceMock();
    _flightUsages.clear();
    prepareFlights();
    _taxName = new TaxName();
  }

  void tearDown()
  {
    delete _locServiceMock;
    _rule.reset();
    _applicator.reset();
    delete _taxName;
  }

  void testApplyAlways()
  {
    type::Index taxPointLoc1Id = 1;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::Blank,
                     type::TicketedPointTag::MatchTicketedAndUnticketedPoints);

    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
  }

  void testInternationalStopoverArrival()
  {
    type::Index taxPointLoc1Id = 1;

    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::AdjacentStopoverInternational,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
  }

  void testInternationalStopoverArrival_US_InsideBufferZone()
  {
    type::Index taxPointLoc1Id = 1;
    _geoPath->geos()[1].loc().nation() = "US";
    _geoPath->geos()[2].loc().nation() = "US";
    _geoPath->geos()[5].loc().nation() = "CA";
    _geoPath->geos()[5].loc().inBufferZone() = true;

    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::AdjacentStopoverInternational,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(!_applicator->apply(*paymentDetail));
  }

  void testInternationalStopoverArrival_US_OutsideBufferZone()
  {
    type::Index taxPointLoc1Id = 1;
    _geoPath->geos()[1].loc().nation() = "US";
    _geoPath->geos()[2].loc().nation() = "US";
    _geoPath->geos()[5].loc().nation() = "CA";
    _geoPath->geos()[5].loc().inBufferZone() = false;

    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::AdjacentStopoverInternational,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
  }

  void testInternationalStopoverDeparture()
  {
    type::Index taxPointLoc1Id = 4;

    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::AdjacentStopoverInternational,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
  }

  void testInternationalStopoverDeparture_US_InsideBufferZone()
  {
    type::Index taxPointLoc1Id = 4;
    _geoPath->geos()[0].loc().nation() = "MX";
    _geoPath->geos()[0].loc().inBufferZone() = true;
    _geoPath->geos()[3].loc().nation() = "US";
    _geoPath->geos()[4].loc().nation() = "US";

    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::AdjacentStopoverInternational,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(!_applicator->apply(*paymentDetail));
  }

  void testInternationalStopoverDeparture_US_OutsideBufferZone()
  {
    type::Index taxPointLoc1Id = 4;
    _geoPath->geos()[0].loc().nation() = "MX";
    _geoPath->geos()[0].loc().inBufferZone() = false;
    _geoPath->geos()[3].loc().nation() = "US";
    _geoPath->geos()[4].loc().nation() = "US";

    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::AdjacentStopoverInternational,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
  }

  void testInternationalStopoverArrivalOpen()
  {
    type::Index taxPointLoc1Id = 1;

    std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    paymentDetail->getMutableTaxPointsProperties()[1].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[2].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[3].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[4].isOpen = true;

    createApplicator(type::AdjacentIntlDomInd::AdjacentStopoverInternational,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(!_applicator->apply(*paymentDetail));
  }

  void testInternationalStopoverDepartureOpen()
  {
    type::Index taxPointLoc1Id = 4;

    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    paymentDetail->getMutableTaxPointsProperties()[1].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[2].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[3].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[4].isOpen = true;

    createApplicator(type::AdjacentIntlDomInd::AdjacentStopoverInternational,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(!_applicator->apply(*paymentDetail));
  }

  void testNoDomesticStopoverArrival()
  {
    type::Index taxPointLoc1Id = 1;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::AdjacentStopoverDomestic,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(!_applicator->apply(*paymentDetail));
  }

  void testNoDomesticStopoverDeparture()
  {
    type::Index taxPointLoc1Id = 2;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::AdjacentStopoverDomestic,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(!_applicator->apply(*paymentDetail));
  }

  void testNoInternationalArrival()
  {
    type::Index taxPointLoc1Id = 1;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::AdjacentInternational,
                     type::TicketedPointTag::MatchTicketedAndUnticketedPoints);

    CPPUNIT_ASSERT(!_applicator->apply(*paymentDetail));
  }

  void testNoInternationalDeparture()
  {
    type::Index taxPointLoc1Id = 4;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::AdjacentInternational,
                     type::TicketedPointTag::MatchTicketedAndUnticketedPoints);

    CPPUNIT_ASSERT(!_applicator->apply(*paymentDetail));
  }

  void testDomesticArrival()
  {
    type::Index taxPointLoc1Id = 1;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::AdjacentDomestic,
                     type::TicketedPointTag::MatchTicketedAndUnticketedPoints);
    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
  }

  void testDomesticDeparture()
  {
    type::Index taxPointLoc1Id = 4;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    createApplicator(type::AdjacentIntlDomInd::AdjacentDomestic,
                     type::TicketedPointTag::MatchTicketedAndUnticketedPoints);
    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
  }

  void testDomesticArrivalOpen()
  {
    type::Index taxPointLoc1Id = 1;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    paymentDetail->getMutableTaxPointsProperties()[1].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[2].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[3].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[4].isOpen = true;

    createApplicator(type::AdjacentIntlDomInd::AdjacentDomestic,
                     type::TicketedPointTag::MatchTicketedAndUnticketedPoints);
    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
  }

  void testDomesticDepartureOpen()
  {
    type::Index taxPointLoc1Id = 4;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    paymentDetail->getMutableTaxPointsProperties()[1].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[2].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[3].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[4].isOpen = true;

    createApplicator(type::AdjacentIntlDomInd::AdjacentDomestic,
                     type::TicketedPointTag::MatchTicketedAndUnticketedPoints);
    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
  }

  void testUnticketedTransfer_NoDomesticArrival()
  {
    type::Index taxPointLoc1Id = 1;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    setUnticketedTransfer(3, 4);

    createApplicator(type::AdjacentIntlDomInd::AdjacentDomestic,
                     type::TicketedPointTag::MatchTicketedPointsOnly);
    CPPUNIT_ASSERT(!_applicator->apply(*paymentDetail));
  }

  void testUnticketedTransfer_NoDomesticDeparture()
  {
    type::Index taxPointLoc1Id = 4;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    setUnticketedTransfer(1, 2);

    createApplicator(type::AdjacentIntlDomInd::AdjacentDomestic,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(!_applicator->apply(*paymentDetail));
  }

  void testUnticketedTransfer_InternationalArrival()
  {
    type::Index taxPointLoc1Id = 1;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    setUnticketedTransfer(3, 4);

    createApplicator(type::AdjacentIntlDomInd::AdjacentInternational,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
  }

  void testUnticketedTransfer_InternationalDeparture()
  {
    type::Index taxPointLoc1Id = 4;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);

    setUnticketedTransfer(1, 2);

    createApplicator(type::AdjacentIntlDomInd::AdjacentInternational,
                     type::TicketedPointTag::MatchTicketedPointsOnly);

    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
  }

private:
  std::unique_ptr<TaxPointLoc1InternationalDomesticRule> _rule;
  std::unique_ptr<TaxPointLoc1InternationalDomesticApplicator> _applicator;

  std::unique_ptr<GeoPath> _geoPath;
  boost::ptr_vector<FlightUsage> _flightUsages;
  boost::ptr_vector<Flight> _flights;
  LocServiceMock* _locServiceMock;
  type::TicketedPointTag* _ticketedPointTag;
  TaxName* _taxName;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxPointLoc1InternationalDomesticApplicatorTest);
} // namespace tax
