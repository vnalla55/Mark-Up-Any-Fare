// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FarePath.h"
#include "TestServer/Facades/CurrencyServiceServer.h"
#include "Rules/TicketMinMaxValueOCApplicator.h"
#include "Rules/TicketMinMaxValueOCRule.h"
#include "test/PaymentDetailMock.h"

#include <memory>

using namespace std;

namespace tax
{
class TicketMinMaxValueOCTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TicketMinMaxValueOCTest);
  CPPUNIT_TEST(testNoOC);
  CPPUNIT_TEST(testAllOCFailed);
  CPPUNIT_TEST(testOneOCWithinLimits);
  CPPUNIT_TEST(testAllOCWithinLimits);
  CPPUNIT_TEST(testFareRelatedOC_BaseFareRule);
  CPPUNIT_TEST(testFareRelatedOCOutOfLimits_BaseFareRule);
  CPPUNIT_TEST(testFareWithFees_FeesWithinLimits);
  CPPUNIT_TEST(testFareWithFees_FeesOutOfLimits);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _paymentDetail = new PaymentDetailMock();
    _tktMinMaxCurrency.reset(new type::CurrencyCode("USD"));
  }

  void tearDown()
  {
    delete _paymentDetail;
    _tktMinMaxCurrency.reset();
  }

  void testNoOC()
  {
    FarePath farePath;
    Fare fare;
    fare.amount() = 2000; // fails
    farePath.fareUsages().push_back(FareUsage());
    farePath.fareUsages().back().fare() = &fare;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::BaseFare);
    TicketMinMaxValueOCRule rule(tktValApplQualifier, "USD", 50, 1000, 0);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 0;
    TicketMinMaxValueOCApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testAllOCFailed()
  {
    FarePath farePath;
    Fare fare;
    fare.amount() = 2000; // fails
    farePath.fareUsages().push_back(FareUsage());
    farePath.fareUsages().back().fare() = &fare;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::BaseFare);
    TicketMinMaxValueOCRule rule(tktValApplQualifier, "USD", 50, 1000, 0);

    addOc(&rule, type::OptionalServiceTag::FlightRelated, 100);
    addOc(&rule, type::OptionalServiceTag::FlightRelated, 1100);
    addOc(&rule, type::OptionalServiceTag::FlightRelated, 10);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 0;
    TicketMinMaxValueOCApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testOneOCWithinLimits()
  {
    FarePath farePath;
    Fare fare;
    fare.amount() = 200;
    farePath.fareUsages().push_back(FareUsage());
    farePath.fareUsages().back().fare() = &fare;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::BaseFare);
    TicketMinMaxValueOCRule rule(tktValApplQualifier, "USD", 50, 1000, 0);

    addOc(0, type::OptionalServiceTag::Merchandise, 10);
    addOc(0, type::OptionalServiceTag::Merchandise, 100);
    addOc(0, type::OptionalServiceTag::Merchandise, 1000);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 0;
    TicketMinMaxValueOCApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testAllOCWithinLimits()
  {
    FarePath farePath;
    Fare fare;
    fare.amount() = 200;
    farePath.fareUsages().push_back(FareUsage());
    farePath.fareUsages().back().fare() = &fare;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::BaseFare);
    TicketMinMaxValueOCRule rule(tktValApplQualifier, "USD", 50, 1000, 0);

    addOc(0, type::OptionalServiceTag::Merchandise, 50);
    addOc(0, type::OptionalServiceTag::Merchandise, 200);
    addOc(0, type::OptionalServiceTag::Merchandise, 1000);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 0;
    TicketMinMaxValueOCApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testFareRelatedOC_BaseFareRule()
  {
    FarePath farePath;
    Fare fare;
    fare.amount() = 200;
    farePath.fareUsages().push_back(FareUsage());
    farePath.fareUsages().back().fare() = &fare;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::BaseFare);
    TicketMinMaxValueOCRule rule(tktValApplQualifier, "USD", 50, 1000, 0);

    addOc(&rule, type::OptionalServiceTag::FareRelated, 10);
    addOc(0, type::OptionalServiceTag::FareRelated, 20000);
    addOc(&rule, type::OptionalServiceTag::FareRelated, 2000);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 0;
    TicketMinMaxValueOCApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testFareRelatedOCOutOfLimits_BaseFareRule()
  {
    FarePath farePath;
    Fare fare;
    fare.amount() = 20000;
    farePath.fareUsages().push_back(FareUsage());
    farePath.fareUsages().back().fare() = &fare;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::BaseFare);
    TicketMinMaxValueOCRule rule(tktValApplQualifier, "USD", 50, 1000, 0);

    addOc(&rule, type::OptionalServiceTag::FareRelated, 10);
    addOc(0, type::OptionalServiceTag::FareRelated, 200);
    addOc(&rule, type::OptionalServiceTag::FareRelated, 2000);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 0;
    TicketMinMaxValueOCApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));
  }

  void testFareWithFees_FeesWithinLimits()
  {
    FarePath farePath;
    Fare fare;
    fare.amount() = 200;
    farePath.fareUsages().push_back(FareUsage());
    farePath.fareUsages().back().fare() = &fare;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::FareWithFees);
    TicketMinMaxValueOCRule rule(tktValApplQualifier, "USD", 50, 1000, 0);

    addOc(&rule, type::OptionalServiceTag::FareRelated, 10);
    addOc(0, type::OptionalServiceTag::FareRelated, 200);
    addOc(&rule, type::OptionalServiceTag::FareRelated, 2000);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 10;
    _paymentDetail->changeFeeAmount() = type::MoneyAmount(20);
    TicketMinMaxValueOCApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testFareWithFees_FeesOutOfLimits()
  {
    FarePath farePath;
    Fare fare;
    fare.amount() = 200;
    farePath.fareUsages().push_back(FareUsage());
    farePath.fareUsages().back().fare() = &fare;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::FareWithFees);
    TicketMinMaxValueOCRule rule(tktValApplQualifier, "USD", 50, 1000, 0);

    addOc(&rule, type::OptionalServiceTag::FareRelated, 10);
    addOc(0, type::OptionalServiceTag::FareRelated, 200);
    addOc(&rule, type::OptionalServiceTag::FareRelated, 2000);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 1000;
    _paymentDetail->changeFeeAmount() = type::MoneyAmount(1000);
    TicketMinMaxValueOCApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));
  }

private:
  PaymentDetailMock* _paymentDetail;
  std::unique_ptr<type::CurrencyCode> _tktMinMaxCurrency;

  OptionalService* addOc(TicketMinMaxValueOCRule* rule,
                         const type::OptionalServiceTag& ocType,
                         const type::MoneyAmount& amount)
  {
    OptionalService* oc = new OptionalService;
    oc->setFailedRule(rule);
    oc->type() = ocType;
    oc->amount() = amount;
    _paymentDetail->optionalServiceItems().push_back(oc);

    return oc;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TicketMinMaxValueOCTest);
} // namespace tax
