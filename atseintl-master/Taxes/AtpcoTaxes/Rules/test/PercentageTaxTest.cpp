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
#include <memory>
#include <stdexcept>

#include "test/include/CppUnitHelperMacros.h"

#include "Rules/PercentageTaxApplicator.h"
#include "Rules/PercentageTaxRule.h"
#include "Rules/PaymentDetail.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "ServiceInterfaces/Services.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "test/PaymentDetailMock.h"
#include "DataModel/Common/Types.h"

using namespace std;

namespace tax
{

namespace
{
  const type::CurrencyCode USD = "USD";
  const type::CurrencyCode PLN = "PLN";
  const type::CurrencyCode noCurrency(UninitializedCode);
}

class PercentageTaxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PercentageTaxTest);

  CPPUNIT_TEST(testZero);
  CPPUNIT_TEST(test10PercentFare);
  CPPUNIT_TEST(test10PercentFareAndYqYr);
  CPPUNIT_TEST(test10PercentFareAndYqYrAndTaxOnTax);
  CPPUNIT_TEST(test10PercentFareAndOptionalService);
  CPPUNIT_TEST(test10PercentFareAndBaggage);
  CPPUNIT_TEST(noOc);

  CPPUNIT_TEST(testMixedTax_Set);
  CPPUNIT_TEST(testMixedTax_NotSet);

  CPPUNIT_TEST(test_FailedRule_ItinTag_NoFareAmount);
  CPPUNIT_TEST(test_NoFailedRule_ChangeFeeTag);

  CPPUNIT_TEST_SUITE_END();

public:
  PercentageTaxTest() : _paymentDetail(0), _applicableTaxableUnits(TaxableUnitTagSet::none()) {}

  void setUp()
  {
    _services.reset(new DefaultServices());
    _services->setFallbackService(new FallbackServiceServer());
    _paymentDetail = new PaymentDetailMock();
    _paymentDetail->totalYqYrAmount() = type::MoneyAmount(100);
    _paymentDetail->totalTaxOnTaxAmount() = type::MoneyAmount(10);

    _applicableTaxableUnits = TaxableUnitTagSet::none();
    _serviceBaggageApplTag.reset(new type::ServiceBaggageApplTag(type::ServiceBaggageApplTag::E));

  }

  void tearDown()
  {
    delete _paymentDetail;
  }

  void testZero()
  {
    _rule.reset(
      new PercentageTaxRule(100000, USD, _applicableTaxableUnits, 0, "ATP", *_serviceBaggageApplTag));
    PercentageTaxApplicator applicator(*_rule, *_services, USD);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), _paymentDetail->taxEquivalentAmount());
  }

  void noOc()
  {
    _applicableTaxableUnits.setTag(type::TaxableUnit::Itinerary);
    _paymentDetail->setTotalFareAmount(type::MoneyAmount(1000));

    _rule.reset(
      new PercentageTaxRule(100000, USD, _applicableTaxableUnits, 0, "ATP", *_serviceBaggageApplTag));
    PercentageTaxApplicator applicator(*_rule, *_services, USD);
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().amount() = type::MoneyAmount(50);
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::FlightRelated;
    _paymentDetail->optionalServiceItems().back().setFailedRule(_rule.get());

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(100), _paymentDetail->taxEquivalentAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), _paymentDetail->optionalServiceItems().back().taxAmount());
  }

  void test10PercentFare()
  {
    _applicableTaxableUnits.setTag(type::TaxableUnit::Itinerary);
    _paymentDetail->setTotalFareAmount(type::MoneyAmount(1000));;

    _rule.reset(
      new PercentageTaxRule(100000, USD, _applicableTaxableUnits, 0, "ATP", *_serviceBaggageApplTag));
    PercentageTaxApplicator applicator(*_rule, *_services, USD);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(100), _paymentDetail->taxEquivalentAmount());
  }

  void test10PercentFareAndYqYr()
  {
    _applicableTaxableUnits.setTag(type::TaxableUnit::YqYr);
    _applicableTaxableUnits.setTag(type::TaxableUnit::Itinerary);
    _paymentDetail->setTotalFareAmount(type::MoneyAmount(1000));

    _rule.reset(
      new PercentageTaxRule(100000, USD, _applicableTaxableUnits, 0, "ATP", *_serviceBaggageApplTag));
    PercentageTaxApplicator applicator(*_rule, *_services, USD);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(110), _paymentDetail->taxEquivalentAmount());
  }

  void test10PercentFareAndYqYrAndTaxOnTax()
  {
    _applicableTaxableUnits.setTag(type::TaxableUnit::YqYr);
    _applicableTaxableUnits.setTag(type::TaxableUnit::TaxOnTax);
    _applicableTaxableUnits.setTag(type::TaxableUnit::Itinerary);
    _paymentDetail->setTotalFareAmount(type::MoneyAmount(1000));

    _rule.reset(
      new PercentageTaxRule(100000, USD, _applicableTaxableUnits, 0, "ATP", *_serviceBaggageApplTag));
    PercentageTaxApplicator applicator(*_rule, *_services, USD);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(111), _paymentDetail->taxEquivalentAmount());
  }

  void test10PercentFareAndOptionalService()
  {
    _applicableTaxableUnits.setTag(type::TaxableUnit::OCFlightRelated);
    _applicableTaxableUnits.setTag(type::TaxableUnit::Itinerary);
    _paymentDetail->setTotalFareAmount(type::MoneyAmount(1000));

    _rule.reset(
      new PercentageTaxRule(100000, USD, _applicableTaxableUnits, 0, "ATP", *_serviceBaggageApplTag));
    PercentageTaxApplicator applicator(*_rule, *_services, USD);
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().amount() = type::MoneyAmount(50);
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::FlightRelated;

    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(100), _paymentDetail->taxEquivalentAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(5), _paymentDetail->optionalServiceItems().back().taxAmount());
  }

  void test10PercentFareAndBaggage()
  {
    _applicableTaxableUnits.setTag(type::TaxableUnit::BaggageCharge);
    _applicableTaxableUnits.setTag(type::TaxableUnit::Itinerary);
    _paymentDetail->setTotalFareAmount(type::MoneyAmount(1000));

    _rule.reset(
      new PercentageTaxRule(100000, USD, _applicableTaxableUnits, 0, "ATP", *_serviceBaggageApplTag));
    PercentageTaxApplicator applicator(*_rule, *_services, USD);
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().amount() = type::MoneyAmount(50);
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::BaggageCharge;

    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(100), _paymentDetail->taxEquivalentAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(5), _paymentDetail->optionalServiceItems().back().taxAmount());
  }


  void testMixedTax_Set()
  {
    _applicableTaxableUnits.setTag(type::TaxableUnit::Itinerary);
    _paymentDetail->totalTaxOnTaxAmount() = type::MoneyAmount(10);

    _rule.reset(
        new PercentageTaxRule(1, noCurrency, _applicableTaxableUnits, 0, "ATP", *_serviceBaggageApplTag));

    PercentageTaxApplicator applicator(*_rule, *_services, PLN);
    applicator.apply(*_paymentDetail);

    CPPUNIT_ASSERT(_paymentDetail->exchangeDetails().isMixedTax);
  }

  void testMixedTax_NotSet()
  {
    _applicableTaxableUnits.setTag(type::TaxableUnit::Itinerary);
    _paymentDetail->totalTaxOnTaxAmount() = type::MoneyAmount(0);

    _rule.reset(
        new PercentageTaxRule(1, noCurrency, _applicableTaxableUnits, 0, "ATP", *_serviceBaggageApplTag));

    PercentageTaxApplicator applicator(*_rule, *_services, PLN);
    applicator.apply(*_paymentDetail);

    CPPUNIT_ASSERT(!_paymentDetail->exchangeDetails().isMixedTax);
  }

  void test_FailedRule_ItinTag_NoFareAmount()
  {
    _applicableTaxableUnits.setTag(type::TaxableUnit::Itinerary);

    _rule.reset(
          new PercentageTaxRule(100000, USD, _applicableTaxableUnits, 0, "ATP", *_serviceBaggageApplTag));

    PercentageTaxApplicator applicator(*_rule, *_services, PLN);
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(_paymentDetail->getMutableItineraryDetail().isFailedRule());
  }

  void test_NoFailedRule_ChangeFeeTag()
  {
    _applicableTaxableUnits.setTag(type::TaxableUnit::ChangeFee);

    _rule.reset(
          new PercentageTaxRule(100000, USD, _applicableTaxableUnits, 0, "ATP", *_serviceBaggageApplTag));

    PercentageTaxApplicator applicator(*_rule, *_services, PLN);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
    CPPUNIT_ASSERT(!_paymentDetail->getMutableItineraryDetail().isFailedRule());
  }

private:
  PaymentDetail* _paymentDetail;
  TaxableUnitTagSet _applicableTaxableUnits;
  std::unique_ptr<PercentageTaxRule> _rule;
  std::unique_ptr<type::ServiceBaggageApplTag> _serviceBaggageApplTag;
  std::unique_ptr<tax::DefaultServices> _services;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PercentageTaxTest);
} // namespace tax

