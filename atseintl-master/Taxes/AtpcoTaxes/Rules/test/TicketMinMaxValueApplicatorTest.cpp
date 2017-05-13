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

#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FarePath.h"
#include "TestServer/Facades/CurrencyServiceServer.h"
#include "Rules/TicketMinMaxValueApplicator.h"
#include "Rules/TicketMinMaxValueRule.h"
#include "test/PaymentDetailMock.h"

#include <memory>
#include <set>
#include <stdexcept>

using namespace std;

namespace tax
{
class TicketMinMaxValueTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TicketMinMaxValueTest);
  CPPUNIT_TEST(testApplyIfEmptyFareAndMinMaxRangeNotSet);
  CPPUNIT_TEST(testApplyIfEmptyFareAndMinRangeNotSet);
  CPPUNIT_TEST(testDontApplyIfFareIsNotInMinMaxRange);
  CPPUNIT_TEST(testApplyIfFareIsInMinMaxRange);
  CPPUNIT_TEST(testFareAmountWithFeesNotWithinLimits);
  CPPUNIT_TEST(testFareAmountWithFeesWithinLimits);

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

  void testApplyIfEmptyFareAndMinMaxRangeNotSet()
  {
    FarePath farePath;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::BaseFare);
    TicketMinMaxValueRule rule(tktValApplQualifier, "USD", 0, 0, 0);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 0;
    TicketMinMaxValueApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    _paymentDetail->taxAmt() = type::MoneyAmount(5);
    applicator.apply(*_paymentDetail);

    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testApplyIfEmptyFareAndMinRangeNotSet()
  {
    FarePath farePath;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::BaseFare);
    TicketMinMaxValueRule rule(tktValApplQualifier, "USD", 0, 100, 1);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 0;
    TicketMinMaxValueApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testDontApplyIfFareIsNotInMinMaxRange()
  {
    FarePath farePath;
    Fare fare;
    fare.amount() = 100;
    farePath.fareUsages().push_back(FareUsage());
    farePath.fareUsages().back().fare() = &fare;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::BaseFare);
    TicketMinMaxValueRule rule(tktValApplQualifier, "USD", 1, 100, 1);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 0;
    TicketMinMaxValueApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));
  }

  void testApplyIfFareIsInMinMaxRange()
  {
    FarePath farePath;
    Fare fare;
    fare.amount() = 100;
    farePath.fareUsages().push_back(FareUsage());
    farePath.fareUsages().back().fare() = &fare;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::BaseFare);
    TicketMinMaxValueRule rule(tktValApplQualifier, "USD", 1, 1000, 1);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 0;
    TicketMinMaxValueApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

  void testFareAmountWithFeesNotWithinLimits()
  {
    FarePath farePath;
    Fare fare;
    fare.amount() = 100;
    farePath.fareUsages().push_back(FareUsage());
    farePath.fareUsages().back().fare() = &fare;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::FareWithFees);
    TicketMinMaxValueRule rule(tktValApplQualifier, "USD", 1, 1000, 1);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 10;
    TicketMinMaxValueApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(!applicator.apply(*_paymentDetail));
  }

  void testFareAmountWithFeesWithinLimits()
  {
    FarePath farePath;
    Fare fare;
    fare.amount() = 100;
    farePath.fareUsages().push_back(FareUsage());
    farePath.fareUsages().back().fare() = &fare;
    type::TktValApplQualifier tktValApplQualifier(type::TktValApplQualifier::FareWithFees);
    TicketMinMaxValueRule rule(tktValApplQualifier, "USD", 1, 10000, 1);

    CurrencyServiceServer currencyServiceServer;
    type::MoneyAmount totalYqYrAmount = 10;
    TicketMinMaxValueApplicator applicator(
        rule, farePath, currencyServiceServer, *_tktMinMaxCurrency, totalYqYrAmount);
    CPPUNIT_ASSERT(applicator.apply(*_paymentDetail));
  }

private:
  PaymentDetailMock* _paymentDetail;
  std::unique_ptr<type::CurrencyCode> _tktMinMaxCurrency;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TicketMinMaxValueTest);
} // namespace tax
