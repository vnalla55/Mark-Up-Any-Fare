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

#include "Common/TaxName.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/RulesRecord.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "Rules/PaymentDetail.h"
#include "Rules/PaymentRuleData.h"
#include "Rules/TaxPointLoc3AsNextStopoverApplicator.h"
#include "Rules/TaxPointLoc3AsNextStopoverRule.h"

#include "test/LocServiceMock.h"

#include <memory>
#include <set>

namespace tax
{
class TaxPointLoc3AsNextStopoverApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxPointLoc3AsNextStopoverApplicatorTest);
  CPPUNIT_TEST(testLoc3DepartureTillEnd);
  CPPUNIT_TEST(testLoc3DepartureWithOptional);
  CPPUNIT_TEST(testLoc3DepartureWithOptionalOpen_match);
  CPPUNIT_TEST(testLoc3ArrivalTillBegin);
  CPPUNIT_TEST(testLoc3ArrivalWithOptional);
  CPPUNIT_TEST(testLoc3ArrivalWithDomesticOpen_match);
  CPPUNIT_TEST(testUnticketedTransfer_match);
  CPPUNIT_TEST(testUnticketedTransfer_noMatch);
  CPPUNIT_TEST(testIsInLoc_noMatch);

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
    geos[3].loc().cityCode() = "WAW";
    geos[3].loc().code() = "WAW";
    geos[3].loc().nation() = "PL";
    geos[4].loc().cityCode() = "WAW";
    geos[4].loc().code() = "WAW";
    geos[4].loc().nation() = "PL";
    geos[5].loc().cityCode() = "KRK";
    geos[5].loc().code() = "KRK";
    geos[5].loc().nation() = "PL";

    geos[0].loc().tag() = type::TaxPointTag::Departure;
    geos[1].loc().tag() = type::TaxPointTag::Arrival;
    geos[2].loc().tag() = type::TaxPointTag::Departure;
    geos[3].loc().tag() = type::TaxPointTag::Arrival;
    geos[4].loc().tag() = type::TaxPointTag::Departure;
    geos[5].loc().tag() = type::TaxPointTag::Arrival;
  }

  void createApplicator()
  {
    _rule.reset(new TaxPointLoc3AsNextStopoverRule(*_locZone3, *_vendor));
    _applicator.reset(new TaxPointLoc3AsNextStopoverApplicator(
        *(_rule.get()), *_geoPath, *_locServiceMock));
  }

  const std::shared_ptr<PaymentDetail>
  createPaymentDetail(const type::Index id,
                      type::TicketedPointTag ticketedPointTag =
                          type::TicketedPointTag::MatchTicketedAndUnticketedPoints)
  {
    Geo& taxPoint1 = _geoPath->geos()[id];

    int dir = _taxName->taxPointTag() == type::TaxPointTag::Departure ? 1 : -1;
    Geo taxPoint2 = _geoPath->geos()[id + dir];

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
    paymentDetail->getMutableTaxPointsProperties()[0].isFirst = true;
    paymentDetail->getMutableTaxPointsProperties()[5].isLast = true;
    return paymentDetail;
  }

  void setStopover(PaymentDetail& paymentDetail, type::Index index)
  {
    paymentDetail.getMutableTaxPointsProperties().at(index).isTimeStopover = true;
  }

  void setUnticketedTransfer(type::Index arrivalId, type::Index departureId)
  {
    std::vector<Geo>& geos = _geoPath->geos();
    geos[arrivalId].unticketedTransfer() = type::UnticketedTransfer::Yes;
    geos[departureId].unticketedTransfer() = type::UnticketedTransfer::Yes;
  }

  void addOptionalService(PaymentDetail& paymentDetail,
                          tax::type::OptionalServiceTag type,
                          type::Index beginId,
                          type::Index endId)
  {
    paymentDetail.optionalServiceItems().push_back(new tax::OptionalService());
    paymentDetail.optionalServiceItems().back().type() = type;
    paymentDetail.optionalServiceItems().back().setTaxPointBegin(_geoPath->geos().at(beginId));
    paymentDetail.optionalServiceItems().back().setTaxPointLoc2(_geoPath->geos().at(endId));
    paymentDetail.optionalServiceItems().back().setTaxPointEnd(_geoPath->geos().at(endId));
  }

  void setUp()
  {
    _taxName = new TaxName();
    _geoPath.reset(new GeoPath);
    _locServiceMock = new LocServiceMock();
    _flightUsages.clear();
    _locZone2 = new LocZone();
    _locZone3 = new LocZone();
    _vendor = new type::Vendor();
    prepareFlights();
  }

  void tearDown()
  {
    delete _locServiceMock;
    delete _locZone2;
    delete _locZone3;
    delete _vendor;
    _rule.reset();
    _applicator.reset();
    delete _taxName;
  }

  void testLoc3DepartureTillEnd()
  {
    type::Index taxPointLoc1Id = 0;
    _taxName->taxPointTag() = type::TaxPointTag::Departure;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);
    createApplicator();

    _locServiceMock->add(true, 2);
    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(1), paymentDetail->getTaxPointLoc2().id());
    CPPUNIT_ASSERT_EQUAL(type::Index(5), paymentDetail->getLoc3().id());
    CPPUNIT_ASSERT_EQUAL(type::Index(5), paymentDetail->getTaxPointEnd().id());
    CPPUNIT_ASSERT_EQUAL(type::Index(2), _locServiceMock->counter());
    CPPUNIT_ASSERT(_locServiceMock->empty());
  }

  void testLoc3DepartureWithOptional()
  {
    type::Index taxPointLoc1Id = 0;
    _taxName->taxPointTag() = type::TaxPointTag::Departure;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);
    createApplicator();

    type::Index stopoverId = 3;
    setStopover(*paymentDetail, stopoverId);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 0, 3);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::TicketRelated, 0, 1);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 0, 1);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 0, 5);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 0, 3);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 0, 1);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 0, 5);

    _locServiceMock->add(true, 2);
    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(1), paymentDetail->getTaxPointLoc2().id());
    CPPUNIT_ASSERT_EQUAL(stopoverId, paymentDetail->getLoc3().id());
    CPPUNIT_ASSERT_EQUAL(stopoverId, paymentDetail->getTaxPointEnd().id());

    CPPUNIT_ASSERT(!paymentDetail->areAllOptionalServicesFailed());
    CPPUNIT_ASSERT(!paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT(!paymentDetail->optionalServiceItems()[1].isFailed());
    CPPUNIT_ASSERT(paymentDetail->optionalServiceItems()[2].isFailed());
    CPPUNIT_ASSERT(paymentDetail->optionalServiceItems()[3].isFailed());
    CPPUNIT_ASSERT(!paymentDetail->optionalServiceItems()[4].isFailed());
    CPPUNIT_ASSERT(paymentDetail->optionalServiceItems()[5].isFailed());
    CPPUNIT_ASSERT(paymentDetail->optionalServiceItems()[6].isFailed());
    CPPUNIT_ASSERT_EQUAL(type::Index(2), _locServiceMock->counter());
    CPPUNIT_ASSERT(_locServiceMock->empty());
  }

  void testLoc3DepartureWithOptionalOpen_match()
  {
    type::Index taxPointLoc1Id = 0;
    _taxName->taxPointTag() = type::TaxPointTag::Departure;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);
    createApplicator();

    paymentDetail->getMutableTaxPointsProperties()[3].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[4].isOpen = true;
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 0, 3);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::TicketRelated, 0, 1);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 0, 1);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 0, 5);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 0, 3);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 0, 1);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 0, 5);

    _locServiceMock->add(true, 5);
    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(3), paymentDetail->getLoc3().id());;
    CPPUNIT_ASSERT_EQUAL(type::Index(1), paymentDetail->getTaxPointLoc2().id());
    CPPUNIT_ASSERT_EQUAL(type::Index(2), _locServiceMock->counter());
    CPPUNIT_ASSERT(!_locServiceMock->empty());
  }

  void testLoc3ArrivalTillBegin()
  {
    type::Index taxPointLoc1Id = 5;
    _taxName->taxPointTag() = type::TaxPointTag::Arrival;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);
    createApplicator();

    _locServiceMock->add(true, 2);
    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(4), paymentDetail->getTaxPointLoc2().id());
    CPPUNIT_ASSERT_EQUAL(type::Index(0), paymentDetail->getLoc3().id());
    CPPUNIT_ASSERT_EQUAL(type::Index(0), paymentDetail->getTaxPointEnd().id());
    CPPUNIT_ASSERT_EQUAL(type::Index(2), _locServiceMock->counter());
    CPPUNIT_ASSERT(_locServiceMock->empty());
  }

  void testLoc3ArrivalWithOptional()
  {
    type::Index taxPointLoc1Id = 5;
    _taxName->taxPointTag() = type::TaxPointTag::Arrival;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);
    createApplicator();

    type::Index stopoverId = 2;
    setStopover(*paymentDetail, stopoverId);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 5, 2);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 5, 4);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 5, 0);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 5, 2);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 5, 4);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 5, 0);

    _locServiceMock->add(true, 2);
    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(4), paymentDetail->getTaxPointLoc2().id());
    CPPUNIT_ASSERT_EQUAL(stopoverId, paymentDetail->getLoc3().id());
    CPPUNIT_ASSERT_EQUAL(stopoverId, paymentDetail->getTaxPointEnd().id());
    CPPUNIT_ASSERT_EQUAL(type::Index(2), _locServiceMock->counter());
    CPPUNIT_ASSERT(_locServiceMock->empty());
    CPPUNIT_ASSERT(!paymentDetail->isFailedRule());
    CPPUNIT_ASSERT(!paymentDetail->areAllOptionalServicesFailed());
    CPPUNIT_ASSERT(!paymentDetail->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT(paymentDetail->optionalServiceItems()[1].isFailed());
    CPPUNIT_ASSERT(paymentDetail->optionalServiceItems()[2].isFailed());
    CPPUNIT_ASSERT(!paymentDetail->optionalServiceItems()[3].isFailed());
    CPPUNIT_ASSERT(paymentDetail->optionalServiceItems()[4].isFailed());
    CPPUNIT_ASSERT(paymentDetail->optionalServiceItems()[5].isFailed());
  }

  void testLoc3ArrivalWithDomesticOpen_match()
  {
    type::Index taxPointLoc1Id = 5;
    _taxName->taxPointTag() = type::TaxPointTag::Arrival;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);
    createApplicator();

    type::Index stopoverId = 2;
    setStopover(*paymentDetail, stopoverId);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 5, 2);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 5, 4);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 5, 0);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 5, 2);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 5, 4);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 5, 0);

    paymentDetail->getMutableTaxPointsProperties()[1].isOpen = true;
    paymentDetail->getMutableTaxPointsProperties()[2].isOpen = true;

    _locServiceMock->add(true, 5);
    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(2), paymentDetail->getLoc3().id());
    CPPUNIT_ASSERT_EQUAL(type::Index(4), paymentDetail->getTaxPointLoc2().id());
    CPPUNIT_ASSERT_EQUAL(type::Index(2), _locServiceMock->counter());
    CPPUNIT_ASSERT(!_locServiceMock->empty());
  }

  void testUnticketedTransfer_match()
  {
    type::Index taxPointLoc1Id = 5;
    _taxName->taxPointTag() = type::TaxPointTag::Arrival;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(
        taxPointLoc1Id, type::TicketedPointTag::MatchTicketedAndUnticketedPoints);
    setUnticketedTransfer(3, 4);
    createApplicator();

    type::Index stopoverId = 2;
    setStopover(*paymentDetail, stopoverId);

    _locServiceMock->add(true, 2);
    CPPUNIT_ASSERT(_applicator->apply(*paymentDetail));
    CPPUNIT_ASSERT_EQUAL(stopoverId, paymentDetail->getLoc3().id());
    CPPUNIT_ASSERT_EQUAL(type::Index(2), _locServiceMock->counter());
    CPPUNIT_ASSERT(_locServiceMock->empty());
  }

  void testUnticketedTransfer_noMatch()
  {
    type::Index taxPointLoc1Id = 5;
    _taxName->taxPointTag() = type::TaxPointTag::Arrival;
    const std::shared_ptr<PaymentDetail> paymentDetail =
        createPaymentDetail(taxPointLoc1Id, type::TicketedPointTag::MatchTicketedPointsOnly);
    setUnticketedTransfer(3, 4);
    createApplicator();

    type::Index stopoverId = 2;
    setStopover(*paymentDetail, stopoverId);

    _locServiceMock->add(true, 2);
    CPPUNIT_ASSERT(!_applicator->apply(*paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(2), _locServiceMock->counter());
    CPPUNIT_ASSERT(_locServiceMock->empty());
  }

  void testIsInLoc_noMatch()
  {
    type::Index taxPointLoc1Id = 5;
    _taxName->taxPointTag() = type::TaxPointTag::Arrival;
    const std::shared_ptr<PaymentDetail> paymentDetail = createPaymentDetail(taxPointLoc1Id);
    createApplicator();

    type::Index stopoverId = 2;
    setStopover(*paymentDetail, stopoverId);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::FlightRelated, 5, 2);
    addOptionalService(*paymentDetail, type::OptionalServiceTag::BaggageCharge, 5, 2);

    _locServiceMock->add(false, 2);
    CPPUNIT_ASSERT(!_applicator->apply(*paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::Index(2), _locServiceMock->counter());
    CPPUNIT_ASSERT(_locServiceMock->empty());
    CPPUNIT_ASSERT(paymentDetail->isFailedRule());
    CPPUNIT_ASSERT(paymentDetail->areAllOptionalServicesFailed());
  }

private:
  std::unique_ptr<TaxPointLoc3AsNextStopoverRule> _rule;
  std::unique_ptr<TaxPointLoc3AsNextStopoverApplicator> _applicator;

  std::unique_ptr<GeoPath> _geoPath;
  boost::ptr_vector<FlightUsage> _flightUsages;
  boost::ptr_vector<Flight> _flights;
  LocServiceMock* _locServiceMock;
  LocZone* _locZone2;
  LocZone* _locZone3;
  type::Vendor* _vendor;
  TaxName* _taxName;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxPointLoc3AsNextStopoverApplicatorTest);
} // namespace tax
