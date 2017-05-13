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

#include "DataModel/Common/CodeIO.h"
#include "DataModel/Common/Types.h"
#include "Rules/TaxMinMaxValueApplicator.h"
#include "Rules/TaxMinMaxValueRule.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "ServiceInterfaces/Services.h"
#include "TestServer/Facades/CurrencyServiceServer.h"
#include "TestServer/Facades/FallbackServiceServer.h"
#include "test/PaymentDetailMock.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tax
{

class TaxMinMaxValueTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxMinMaxValueTest);

  CPPUNIT_TEST(testDoNotApplyIfCurrencyIsDifferent);
  CPPUNIT_TEST(testTaxAmtLowerThanMinValue);
  CPPUNIT_TEST(testTaxAmtHigherThanMaxValue);
  CPPUNIT_TEST(testExchangeDetails);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _services.reset(new DefaultServices());
    _services->setFallbackService(new FallbackServiceServer());
    _services->setCurrencyService(new CurrencyServiceServer());
    _paymentDetail = new PaymentDetailMock();
    _taxValCurrency.reset(new type::CurrencyCode());
  }

  void tearDown()
  {
    delete _paymentDetail;
    _rule.reset();
    _taxValCurrency.reset();
  }

  void testDoNotApplyIfCurrencyIsDifferent()
  {
    prepareRule("USD", 10, 15, 0);
    TaxMinMaxValueApplicator applicator(_rule.get(), *_services, type::CurrencyCode());

    _paymentDetail->taxEquivalentAmount() = type::MoneyAmount(5);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(5), _paymentDetail->taxEquivalentAmount());
  }

  void testTaxAmtLowerThanMinValue()
  {
    prepareRule("NUC", 10, 15, 0);
    TaxMinMaxValueApplicator applicator(_rule.get(), *_services, *_taxValCurrency);

    _paymentDetail->taxEquivalentAmount() = type::MoneyAmount(5);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(10), _paymentDetail->taxEquivalentAmount());
  }

  void testTaxAmtHigherThanMaxValue()
  {
    prepareRule("NUC", 10, 15, 0);
    TaxMinMaxValueApplicator applicator(_rule.get(), *_services, *_taxValCurrency);

    _paymentDetail->taxEquivalentAmount() = type::MoneyAmount(20);
    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(15), _paymentDetail->taxEquivalentAmount());
  }

  void testExchangeDetails()
  {
    prepareRule("NUC", 100, 200, 2);
    TaxMinMaxValueApplicator applicator(_rule.get(), *_services, *_taxValCurrency);

    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(1), *_paymentDetail->exchangeDetails().minTaxAmount);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(2), *_paymentDetail->exchangeDetails().maxTaxAmount);
    CPPUNIT_ASSERT_EQUAL(type::CurrencyCode("NUC"), *_paymentDetail->exchangeDetails().minMaxTaxCurrency);
    CPPUNIT_ASSERT_EQUAL(type::CurDecimals(2), *_paymentDetail->exchangeDetails().minMaxTaxCurrencyDecimals);
  }

private:
  PaymentDetailMock* _paymentDetail;
  std::unique_ptr<type::CurrencyCode> _taxValCurrency;
  std::unique_ptr<TaxMinMaxValueRule> _rule;
  std::unique_ptr<tax::DefaultServices> _services;

  void prepareRule(type::CurrencyCode taxValCurrency, const type::IntMoneyAmount taxValMin,
                   const type::IntMoneyAmount taxValMax, const type::CurDecimals taxValCurrDecimals)
  {
    *_taxValCurrency = taxValCurrency;
    _rule.reset(new TaxMinMaxValueRule(*_taxValCurrency, taxValMin, taxValMax, taxValCurrDecimals));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxMinMaxValueTest);
} // namespace tax
