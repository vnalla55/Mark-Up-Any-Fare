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
#include "Rules/TaxPointLoc3AsPreviousPointApplicator.h"

#include <memory>
#include <stdexcept>

#include "test/include/CppUnitHelperMacros.h"

#include "Common/LocZone.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/OptionalService.h"
#include "Rules/TaxPointLoc3AsPreviousPointRule.h"

#include "test/PaymentDetailMock.h"
#include "test/LocServiceMock.h"

namespace tax
{

class TaxPointLoc3AsPreviousPointApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxPointLoc3AsPreviousPointApplicatorTest);

  CPPUNIT_TEST(loc3DepartureWithPreviousNull);
  CPPUNIT_TEST(loc3DepartureWithPreviousMatchingUnticketed);
  CPPUNIT_TEST(loc3DepartureWithPreviousMatchingTicketed);
  CPPUNIT_TEST(loc3DepartureWithPreviousNotMatchingBecauseOfUnticketed);
  CPPUNIT_TEST(loc3DepartureWithPreviousNotMatchingBecauseOfIsInLoc);
  CPPUNIT_TEST(loc3ArrivalWithNextMatchingUnticketed);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _locZone.reset(new LocZone());
    _vendor.reset(new type::Vendor());
    _locServiceMock.reset(new LocServiceMock());

    _ticketedPointTag.reset(new type::TicketedPointTag(
      type::TicketedPointTag::MatchTicketedAndUnticketedPoints));
    _paymentDetailMock.reset(new PaymentDetailMock());
    _rule = new TaxPointLoc3AsPreviousPointRule(*_locZone, *_vendor);
  }

  void tearDown()
  {
    delete _rule;
  }

  void addOptionalService(type::OptionalServiceTag type)
  {
    _paymentDetailMock->optionalServiceItems().push_back(new OptionalService());
    _paymentDetailMock->optionalServiceItems().back().type() = type;
  }

  void loc3DepartureWithPreviousNull()
  {
    TaxPointLoc3AsPreviousPointApplicator applicator(*_rule, *_locZone, *_vendor,
                                                          *_locServiceMock);

    makePath();
    _paymentDetailMock->setTaxPointBegin(*_thisDeparture);
    _thisDeparture->setPrev(0);
    addOptionalService(type::OptionalServiceTag::FlightRelated);
    addOptionalService(type::OptionalServiceTag::BaggageCharge);

    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(*_paymentDetailMock));
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->hasLoc3());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetailMock->isFailedRule());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetailMock->areAllOptionalServicesFailed());
  }

  void loc3DepartureWithPreviousMatchingUnticketed()
  {
    TaxPointLoc3AsPreviousPointApplicator applicator(*_rule, *_locZone, *_vendor,
                                                          *_locServiceMock);

    makePath();
    _paymentDetailMock->setTaxPointBegin(*_thisDeparture);
    _previousDeparture->unticketedTransfer() = type::UnticketedTransfer::Yes;
    _paymentDetailMock->setMustBeTicketed(false);
    _locServiceMock->add(true);
    addOptionalService(type::OptionalServiceTag::FlightRelated);
    addOptionalService(type::OptionalServiceTag::TicketRelated);
    addOptionalService(type::OptionalServiceTag::BaggageCharge);

    const Geo* loc3Address = &*_previousDeparture;
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetailMock));
    CPPUNIT_ASSERT_EQUAL(loc3Address, &_paymentDetailMock->getLoc3());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->isFailedRule());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->areAllOptionalServicesFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->optionalServiceItems()[1].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->optionalServiceItems()[2].isFailed());
  }

  void loc3DepartureWithPreviousMatchingTicketed()
  {
    TaxPointLoc3AsPreviousPointApplicator applicator(*_rule, *_locZone, *_vendor,
                                                          *_locServiceMock);

    makePath();
    _paymentDetailMock->setTaxPointBegin(*_thisDeparture);
    _previousDeparture->unticketedTransfer() = type::UnticketedTransfer::No;
    _paymentDetailMock->setMustBeTicketed(true);
    _locServiceMock->add(true);
    addOptionalService(type::OptionalServiceTag::FlightRelated);
    addOptionalService(type::OptionalServiceTag::TicketRelated);
    addOptionalService(type::OptionalServiceTag::BaggageCharge);

    const Geo* loc3Address = &*_previousDeparture;
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetailMock));
    CPPUNIT_ASSERT_EQUAL(loc3Address, &_paymentDetailMock->getLoc3());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->isFailedRule());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->areAllOptionalServicesFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->optionalServiceItems()[1].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->optionalServiceItems()[2].isFailed());
  }

  void loc3DepartureWithPreviousNotMatchingBecauseOfUnticketed()
  {
    TaxPointLoc3AsPreviousPointApplicator applicator(*_rule, *_locZone, *_vendor,
                                                          *_locServiceMock);

    makePath();
    _paymentDetailMock->setTaxPointBegin(*_thisDeparture);
    _previousDeparture->unticketedTransfer() = type::UnticketedTransfer::Yes;
    _paymentDetailMock->setMustBeTicketed(true);
    _locServiceMock->add(false);
    addOptionalService(type::OptionalServiceTag::FlightRelated);
    addOptionalService(type::OptionalServiceTag::TicketRelated);
    addOptionalService(type::OptionalServiceTag::BaggageCharge);

    const Geo* loc3Address = &*_firstDeparture;
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetailMock));
    CPPUNIT_ASSERT_EQUAL(loc3Address, &_paymentDetailMock->getLoc3());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetailMock->isFailedRule());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->areAllOptionalServicesFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetailMock->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->optionalServiceItems()[1].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetailMock->optionalServiceItems()[2].isFailed());
  }

  void loc3DepartureWithPreviousNotMatchingBecauseOfIsInLoc()
  {
    TaxPointLoc3AsPreviousPointApplicator applicator(*_rule, *_locZone, *_vendor,
                                                          *_locServiceMock);

    makePath();
    _paymentDetailMock->setTaxPointBegin(*_thisDeparture);
    _previousDeparture->unticketedTransfer() = type::UnticketedTransfer::No;
    _paymentDetailMock->setMustBeTicketed(false);
    _locServiceMock->add(false);
    addOptionalService(type::OptionalServiceTag::FlightRelated);
    addOptionalService(type::OptionalServiceTag::TicketRelated);
    addOptionalService(type::OptionalServiceTag::BaggageCharge);

    const Geo* loc3Address = &*_previousDeparture;
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetailMock));
    CPPUNIT_ASSERT_EQUAL(loc3Address, &_paymentDetailMock->getLoc3());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetailMock->isFailedRule());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->areAllOptionalServicesFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetailMock->optionalServiceItems()[0].isFailed());
    CPPUNIT_ASSERT_EQUAL(false, _paymentDetailMock->optionalServiceItems()[1].isFailed());
    CPPUNIT_ASSERT_EQUAL(true, _paymentDetailMock->optionalServiceItems()[2].isFailed());
  }

  void loc3ArrivalWithNextMatchingUnticketed()
  {
    TaxPointLoc3AsPreviousPointApplicator applicator(*_rule, *_locZone, *_vendor,
                                                          *_locServiceMock);

    makePath();
    _paymentDetailMock->setTaxPointBegin(*_thisArrival);
    _nextArrival->unticketedTransfer() = type::UnticketedTransfer::No;
    _paymentDetailMock->setMustBeTicketed(false);
    _locServiceMock->add(true);

    const Geo* loc3Address = &*_nextArrival;
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(*_paymentDetailMock));
    CPPUNIT_ASSERT_EQUAL(loc3Address, &_paymentDetailMock->getLoc3());
  }

private:
  TaxPointLoc3AsPreviousPointRule* _rule;
  std::unique_ptr<LocZone> _locZone;
  std::unique_ptr<type::Vendor> _vendor;
  std::unique_ptr<LocServiceMock> _locServiceMock;

  std::unique_ptr<type::TicketedPointTag> _ticketedPointTag;
  std::unique_ptr<Geo> _firstDeparture, _firstArrival, _previousDeparture, _previousArrival,
      _thisDeparture, _thisArrival, _nextDeparture, _nextArrival;

  std::unique_ptr<PaymentDetailMock> _paymentDetailMock;

  void makePath()
  /* _firstDeparture - _firstArrival - _previousDeparture - _previousArrival -
   * _thisDeparture - _thisArrival - _nextDeparture - _nextArrival */
  {
    _firstDeparture.reset(new Geo());
    _firstArrival.reset(new Geo());
    _previousDeparture.reset(new Geo());
    _previousArrival.reset(new Geo());
    _thisDeparture.reset(new Geo());
    _thisArrival.reset(new Geo());
    _nextDeparture.reset(new Geo());
    _nextArrival.reset(new Geo());

    _firstArrival->setPrev(&*_firstDeparture);
    _previousDeparture->setPrev(&*_firstArrival);

    _thisDeparture->loc().tag() = type::TaxPointTag::Departure;
    _thisArrival->loc().tag() = type::TaxPointTag::Arrival;

    _thisDeparture->setPrev(&*_previousArrival);
    _previousArrival->setPrev(&*_previousDeparture);

    _thisArrival->setNext(&*_nextDeparture);
    _nextDeparture->setNext(&*_nextArrival);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxPointLoc3AsPreviousPointApplicatorTest);
} // namespace tse
