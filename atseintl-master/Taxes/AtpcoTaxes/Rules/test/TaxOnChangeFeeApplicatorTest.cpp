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

#include "Rules/TaxOnChangeFeeApplicator.h"
#include "Rules/TaxOnChangeFeeRule.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "test/CurrencyServiceMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/MakePaymentDetail.h"

namespace tax
{
class TaxOnChangeFeeApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxOnChangeFeeApplicatorTest);
  CPPUNIT_TEST(testApplyPercentage);
  CPPUNIT_TEST(testApplyFlat);
  CPPUNIT_TEST_SUITE_END();

public:
  void testApplyPercentage()
  {
    DefaultServices services;
    services.setCurrencyService(new CurrencyServiceMock());

    const TaxOnChangeFeeRule rule(type::PercentFlatTag::Percent);
    TaxOnChangeFeeApplicator applicator(rule, services);

    const type::MoneyAmount taxAmt(12);
    const type::MoneyAmount changeFee(11);
    PaymentDetail paymentDetail =
        MakePaymentDetail::percent().taxAmt(taxAmt).changeFeeAmount(changeFee);

    applicator.apply(paymentDetail);

    CPPUNIT_ASSERT_EQUAL(changeFee, paymentDetail.changeFeeAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(changeFee * taxAmt),
                         paymentDetail.taxOnChangeFeeAmount());
  }

  void testApplyFlat()
  {
    auto currencyServiceMock = std::make_unique<CurrencyServiceMock>();
    const type::MoneyAmount converted(13);
    currencyServiceMock->setConverted(converted);
    DefaultServices services;
    services.setCurrencyService(currencyServiceMock.release());

    const TaxOnChangeFeeRule rule(type::PercentFlatTag::Flat);
    TaxOnChangeFeeApplicator applicator(rule, services);

    const type::MoneyAmount taxAmt(12);
    const type::MoneyAmount changeFee(11);
    PaymentDetail paymentDetail =
        MakePaymentDetail::percent().taxAmt(taxAmt).taxCurrency("USD").taxEquivalentCurrency("USD").changeFeeAmount(changeFee);

    applicator.apply(paymentDetail);

    CPPUNIT_ASSERT_EQUAL(changeFee, paymentDetail.changeFeeAmount());
    CPPUNIT_ASSERT_EQUAL(converted, paymentDetail.taxOnChangeFeeAmount());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxOnChangeFeeApplicatorTest);
} // namespace tax
