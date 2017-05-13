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

#include "Rules/CurrencyOfSaleApplicator.h"
#include "Rules/CurrencyOfSaleRule.h"
#include "DataModel/Common/Types.h"
#include "test/PaymentDetailMock.h"

namespace tax
{

class CurrencyOfSaleApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CurrencyOfSaleApplicatorTest);

  CPPUNIT_TEST(testConstructor);

  CPPUNIT_TEST(testCurrencyOfSaleZZZFormMiles);
  CPPUNIT_TEST(testCurrencyOfSaleZZZFormNotMiles);

  CPPUNIT_TEST(testDifferentCurrency);
  CPPUNIT_TEST(testSameCurrency);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _currencyOfSale = type::CurrencyCode("PLN");
    _paymentCurrency = type::CurrencyCode("PLN");
    _formOfPayment.reset(new type::FormOfPayment(type::FormOfPayment::Miles));
  }

  void tearDown()
  {
    _formOfPayment.reset();
  }

  void testConstructor()
  {
    CPPUNIT_ASSERT_NO_THROW(
      CurrencyOfSaleApplicator(*_rule, _currencyOfSale, _paymentCurrency, *_formOfPayment));
  }

  void testCurrencyOfSaleZZZFormMiles()
  {
    _currencyOfSale = type::CurrencyCode("ZZZ");
    *_formOfPayment = type::FormOfPayment::Miles;

    CurrencyOfSaleApplicator applicator(*_rule, _currencyOfSale, _paymentCurrency, *_formOfPayment);
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(_paymentDetailMock));
  }

  void testCurrencyOfSaleZZZFormNotMiles()
  {
    _currencyOfSale = type::CurrencyCode("ZZZ");
    *_formOfPayment = type::FormOfPayment::Blank;

    CurrencyOfSaleApplicator applicator(*_rule, _currencyOfSale, _paymentCurrency, *_formOfPayment);
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(_paymentDetailMock));
  }

  void testDifferentCurrency()
  {
    _currencyOfSale = type::CurrencyCode("USD");
    _paymentCurrency = type::CurrencyCode("PLN");

    CurrencyOfSaleApplicator applicator(*_rule, _currencyOfSale, _paymentCurrency, *_formOfPayment);
    CPPUNIT_ASSERT_EQUAL(false, applicator.apply(_paymentDetailMock));
  }

  void testSameCurrency()
  {
    _currencyOfSale = type::CurrencyCode("PLN");
    _paymentCurrency = type::CurrencyCode("PLN");

    CurrencyOfSaleApplicator applicator(*_rule, _currencyOfSale, _paymentCurrency, *_formOfPayment);
    CPPUNIT_ASSERT_EQUAL(true, applicator.apply(_paymentDetailMock));
  }

private:
  CurrencyOfSaleRule* _rule;
  PaymentDetailMock _paymentDetailMock;

  type::CurrencyCode _currencyOfSale;
  type::CurrencyCode _paymentCurrency;
  std::unique_ptr<type::FormOfPayment> _formOfPayment;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CurrencyOfSaleApplicatorTest);
} // namespace tax
