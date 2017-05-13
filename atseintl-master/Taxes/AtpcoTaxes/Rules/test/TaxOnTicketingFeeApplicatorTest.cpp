// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Rules/TaxOnTicketingFeeApplicator.h"
#include "Rules/TaxOnTicketingFeeRule.h"
#include "test/CurrencyServiceMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/MakePaymentDetail.h"

#include <memory>

namespace tax
{
class TaxOnTicketingFeeApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxOnTicketingFeeApplicatorTest);
  CPPUNIT_TEST(testApplyPercentage);
  CPPUNIT_TEST(testApplyFlat);
  CPPUNIT_TEST_SUITE_END();

public:
  void testApplyPercentage()
  {
    std::unique_ptr<CurrencyServiceMock> currencyServiceMock;
    currencyServiceMock.reset(new CurrencyServiceMock());

    const TaxOnTicketingFeeRule rule(type::PercentFlatTag::Percent);
    TaxOnTicketingFeeApplicator applicator(rule, *currencyServiceMock);

    const type::MoneyAmount taxAmt(1000);
    PaymentDetail paymentDetail = MakePaymentDetail::percent()
        .taxAmt(taxAmt)
        .addTicketingFee(0, type::MoneyAmount(10, 100), 0, "BG")
        .addTicketingFee(1, type::MoneyAmount(15, 100), 0, "SA");

    applicator.apply(paymentDetail);

    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(100, 1), paymentDetail.ticketingFees()[0].taxAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(150, 1), paymentDetail.ticketingFees()[1].taxAmount());
  }

  void testApplyFlat()
  {
    std::unique_ptr<CurrencyServiceMock> currencyServiceMock;
    currencyServiceMock.reset(new CurrencyServiceMock());
    const type::MoneyAmount converted(10);
    currencyServiceMock->setConverted(converted);

    const type::MoneyAmount taxAmt(1000);

    const TaxOnTicketingFeeRule rule(type::PercentFlatTag::Flat);
    TaxOnTicketingFeeApplicator applicator(rule, *currencyServiceMock);

    PaymentDetail paymentDetail = MakePaymentDetail::percent()
        .taxAmt(taxAmt)
        .taxCurrency("USD")
        .taxEquivalentCurrency("USD")
        .addTicketingFee(0, type::MoneyAmount(10, 1), 0, "BG");

    applicator.apply(paymentDetail);

    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(10, 1), paymentDetail.ticketingFees()[0].taxAmount());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxOnTicketingFeeApplicatorTest);
} // namespace tax
