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

#include "test/include/CppUnitHelperMacros.h"
#include "test/PaymentDetailMock.h"
#include "test/MakePaymentDetail.h"
#include "Common/RoundingUtil.h"
#include "Rules/TaxRoundingApplicator.h"
#include "ServiceInterfaces/ActivationInfoService.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"
#include "test/CurrencyServiceMock.h"
#include "Taxes/TestServer/Facades/FallbackServiceServer.h"

#include <memory>

namespace
{
class TaxRoundingRuleMock : public tax::TaxRoundingRule
{
  TaxRoundingRuleMock(const tax::type::TaxRoundingUnit& taxRoundingUnit,
                      const tax::type::TaxRoundingDir& taxRoundingDir,
                      const tax::type::TaxRoundingPrecision& taxRoundingPrecision)
    : TaxRoundingRule(taxRoundingUnit, taxRoundingDir, taxRoundingPrecision)
  {
  }

public:
  static tax::TaxRoundingRule* create(const tax::type::TaxRoundingUnit& taxRoundingUnit,
                                      const tax::type::TaxRoundingDir& taxRoundingDir,
                                      const tax::type::TaxRoundingPrecision& taxRoundingPrecision)
  {
    return new TaxRoundingRuleMock(taxRoundingUnit, taxRoundingDir, taxRoundingPrecision);
  }
};

class TaxRoundingInfoServiceMock : public tax::TaxRoundingInfoService
{
public:
  void getTrxRoundingInfo(const tax::type::Nation& nation,
                          tax::type::MoneyAmount& unit,
                          tax::type::TaxRoundingDir& dir) const
  {
    if (nation == "UK")
    {
      unit = tax::type::MoneyAmount(1, 10);
      dir = tax::type::TaxRoundingDir::NoRounding;
    }
    else if (nation == "US")
    {
      unit = tax::type::MoneyAmount(1);
      dir = tax::type::TaxRoundingDir::RoundDown;
    }
  }

  void getFareRoundingInfo(const tax::type::CurrencyCode&,
                           tax::type::MoneyAmount&,
                           tax::type::TaxRoundingDir&) const override {}

  void getNationRoundingInfo(const tax::type::Nation& /*nation*/,
                             tax::type::MoneyAmount& /*unit*/,
                             tax::type::TaxRoundingDir& /*dir*/) const override {}

  void doStandardRound(tax::type::MoneyAmount& amount,
                       tax::type::MoneyAmount& unit,
                       tax::type::TaxRoundingDir& dir,
                       tax::type::MoneyAmount /* currencyUnit */,
                       bool /* isOcFee */) const override
  {
    if (dir == tax::type::TaxRoundingDir::NoRounding)
    {
      dir = tax::type::TaxRoundingDir::RoundDown;
      unit = tax::type::MoneyAmount(1, 100);
    }

    amount = doRound(amount, unit, dir, true);
  }

  static tax::TaxRoundingInfoService* create() { return new TaxRoundingInfoServiceMock; }
};

class ActivationInfoServiceMock : public tax::ActivationInfoService
{
public:
  bool
  isAtpcoDefaultRoundingActive() const
  {
    return false;
  }

  static tax::ActivationInfoService*
  create()
  {
    return new ActivationInfoServiceMock;
  }
};

class TaxRoundingApplicatorFactory
{
  std::unique_ptr<tax::TaxRoundingRule> _taxRoundingRule;
  tax::DefaultServices _services;

public:
  TaxRoundingApplicatorFactory(const tax::type::TaxRoundingUnit& taxRoundingUnit,
                               const tax::type::TaxRoundingDir& taxRoundingDir,
                               const tax::type::TaxRoundingPrecision& taxRoundingPrecision)
    : _taxRoundingRule(TaxRoundingRuleMock::create(taxRoundingUnit, taxRoundingDir, taxRoundingPrecision))
  {
    _services.setActivationInfoService(ActivationInfoServiceMock::create());
    _services.setCurrencyService(tax::CurrencyServiceMock::create());
    _services.setTaxRoundingInfoService(TaxRoundingInfoServiceMock::create());
    _services.setFallbackService(new tax::FallbackServiceServer);
  }

  tax::TaxRoundingApplicator* create(const tax::type::Nation& posNation)
  {
    return new tax::TaxRoundingApplicator(_taxRoundingRule.get(),
                                          _services,
                                          posNation);
  }
};
}

namespace tax
{
class TaxRoundingApplicatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxRoundingApplicatorTest);
  CPPUNIT_TEST(testFixedTaxAndNoCurrencyConverion);
  CPPUNIT_TEST(testCurrenciesDontAgree_RoundingDown);
  CPPUNIT_TEST(testCurrenciesDontAgree_NoRounding);
  CPPUNIT_TEST(testCurrenciesDontAgree_Blank);
  CPPUNIT_TEST(roundingPercentWithOC);
  CPPUNIT_TEST(roundingPercentWithOC_toFive);
  CPPUNIT_TEST_SUITE_END();

public:
  void testFixedTaxAndNoCurrencyConverion()
  {
    PaymentDetail payDetail = MakePaymentDetail::flat().currency("USD").taxCurrency("USD").rounding(
        type::MoneyAmount(1, 100), type::TaxRoundingDir::Blank);

    TaxRoundingApplicatorFactory applicatorFactory(type::TaxRoundingUnit::TwoDigits,
                                                   type::TaxRoundingDir::Blank,
                                                   type::TaxRoundingPrecision::ToUnits);
    std::unique_ptr<TaxRoundingApplicator> applicator(applicatorFactory.create("UK"));

    CPPUNIT_ASSERT(applicator);
    CPPUNIT_ASSERT(applicator->apply(payDetail));
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(1, 100), payDetail.calcDetails().roundingUnit);
    CPPUNIT_ASSERT(payDetail.calcDetails().roundingDir == type::TaxRoundingDir::Blank);
  }

  void testCurrenciesDontAgree_RoundingDown()
  {
    PaymentDetail payDetail =
        MakePaymentDetail::flat("PL", "104", "PL")
            .amount(2351, 1000)
            .currency("PLN")
            .taxCurrency("USD")
            .rounding(type::MoneyAmount(1, 100), type::TaxRoundingDir::RoundDown);

    TaxRoundingApplicatorFactory applicatorFactory(type::TaxRoundingUnit::TwoDigits,
                                                   type::TaxRoundingDir::RoundDown,
                                                   type::TaxRoundingPrecision::ToUnits);
    std::unique_ptr<TaxRoundingApplicator> applicator(applicatorFactory.create("PL"));

    CPPUNIT_ASSERT(applicator);
    CPPUNIT_ASSERT(applicator->apply(payDetail));
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(47, 20), payDetail.taxEquivalentAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(1, 100), payDetail.calcDetails().roundingUnit);
    CPPUNIT_ASSERT(payDetail.calcDetails().roundingDir == type::TaxRoundingDir::RoundDown);
  }

  void testCurrenciesDontAgree_NoRounding()
  {
    PaymentDetail payDetail =
        MakePaymentDetail::flat("PL", "104", "PL")
            .amount(2351, 1000)
            .currency("PLN")
            .taxCurrency("USD")
            .rounding(type::MoneyAmount(1, 100), type::TaxRoundingDir::NoRounding);

    TaxRoundingApplicatorFactory applicatorFactory(type::TaxRoundingUnit::TwoDigits,
                                                   type::TaxRoundingDir::NoRounding,
                                                   type::TaxRoundingPrecision::ToUnits);
    std::unique_ptr<TaxRoundingApplicator> applicator(applicatorFactory.create("UK"));

    CPPUNIT_ASSERT(applicator);
    CPPUNIT_ASSERT(applicator->apply(payDetail));
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(47, 20), payDetail.taxEquivalentAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(1, 100), payDetail.calcDetails().roundingUnit);
    CPPUNIT_ASSERT(payDetail.calcDetails().roundingDir == type::TaxRoundingDir::NoRounding);
  }

  void testCurrenciesDontAgree_Blank()
  {
    PaymentDetail payDetail = MakePaymentDetail::flat("PL", "104", "PL")
                                  .amount(2351, 1000)
                                  .currency("PLN")
                                  .taxCurrency("USD")
                                  .rounding(-1, type::TaxRoundingDir::Blank);

    TaxRoundingApplicatorFactory applicatorFactory(type::TaxRoundingUnit::TwoDigits,
                                                   type::TaxRoundingDir::RoundDown,
                                                   type::TaxRoundingPrecision::ToUnits);
    std::unique_ptr<TaxRoundingApplicator> applicator(applicatorFactory.create("US"));

    CPPUNIT_ASSERT(applicator);
    CPPUNIT_ASSERT(applicator->apply(payDetail));
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(47, 20), payDetail.taxEquivalentAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(1, 100), payDetail.calcDetails().roundingUnit);
    CPPUNIT_ASSERT(payDetail.calcDetails().roundingDir == type::TaxRoundingDir::RoundDown);
  }

  void roundingPercentWithOC()
  {
    PaymentDetail payDetail = MakePaymentDetail::percent().amount(12345, 10000).currency("USD");
    AddOptionalService(payDetail).taxAmount(92345, 10000);

    TaxRoundingApplicatorFactory applicatorFactory(type::TaxRoundingUnit::TwoDigits,
                                                   type::TaxRoundingDir::Nearest,
                                                   type::TaxRoundingPrecision::ToUnits);
    std::unique_ptr<TaxRoundingApplicator> applicator(applicatorFactory.create("PL"));

    CPPUNIT_ASSERT(applicator->apply(payDetail));
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(123, 100), payDetail.taxEquivalentAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(923, 100),
                         payDetail.optionalServiceItems()[0].getTaxEquivalentAmount());
  }

  void roundingPercentWithOC_toFive()
  {
    PaymentDetail payDetail = MakePaymentDetail::percent().amount(12345, 10000).currency("USD");
    AddOptionalService(payDetail).taxAmount(92345, 10000);

    TaxRoundingApplicatorFactory applicatorFactory(type::TaxRoundingUnit::TwoDigits,
                                                   type::TaxRoundingDir::Nearest,
                                                   type::TaxRoundingPrecision::ToFives);
    std::unique_ptr<TaxRoundingApplicator> applicator(applicatorFactory.create("PL"));

    CPPUNIT_ASSERT(applicator->apply(payDetail));
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(5, 4), payDetail.taxEquivalentAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(37, 4),
                         payDetail.optionalServiceItems()[0].getTaxEquivalentAmount());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxRoundingApplicatorTest);
} // namespace tax
