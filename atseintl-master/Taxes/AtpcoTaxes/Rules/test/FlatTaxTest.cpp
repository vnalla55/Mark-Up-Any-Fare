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
#include <gmock/gmock.h>

#include "test/include/CppUnitHelperMacros.h"
#include "Rules/FlatTaxApplicator.h"
#include "Rules/FlatTaxRule.h"
#include "ServiceInterfaces/CurrencyService.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "test/PaymentDetailMock.h"
#include "TestServer/Facades/FallbackServiceServer.h"

namespace tax
{
namespace
{
class CurrencyServiceMock : public CurrencyService
{
public:
  MOCK_CONST_METHOD3(convertTo,
                     tax::type::MoneyAmount(const tax::type::CurrencyCode&,
                                            const tax::type::Money&,
                                            CalcDetails*));

  MOCK_CONST_METHOD2(getBSR, type::BSRValue(const type::CurrencyCode&, const type::CurrencyCode&));

  MOCK_CONST_METHOD1(getCurrencyDecimals, type::CurDecimals(const type::CurrencyCode&));

  MOCK_CONST_METHOD2(getNationCurrency, tax::type::CurrencyCode(const tax::type::CurrencyCode&,
                                                                const tax::type::Nation&));
};

static const int EquivalentCurrencyRate = 7;

tax::type::MoneyAmount fakeConvertToMethod(const tax::type::CurrencyCode&, const tax::type::Money& source,
                                           CalcDetails*)
{ return source._amount * EquivalentCurrencyRate; }

}

using testing::_;
using testing::Return;

class FlatTaxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FlatTaxTest);

  CPPUNIT_TEST(testFlatTax);
  CPPUNIT_TEST(testFlatTaxWithYqYrAndTaxOnTax);
  CPPUNIT_TEST(testFlatTaxWithOptionalServices);
  CPPUNIT_TEST(testFlatTaxWithOptionalServices_whenQuantityDifferentFromDefault);
  CPPUNIT_TEST(testFlatTaxWithBaggage);
  CPPUNIT_TEST(testFlatTaxWithBaggage_whenQuantityDifferentFromDefault);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    const type::MoneyAmount taxAmount(1001,100);
    const type::CurrencyCode currencyCode("USD");
    _paymentDetail = new PaymentDetailMock();
    _paymentDetail->taxAmt() = taxAmount;
    _paymentDetail->taxCurrency() = currencyCode;
    _services.setFallbackService(new FallbackServiceServer());
  }

  void tearDown()
  {
    delete _paymentDetail;
  }

  void testFlatTax()
  {
    TaxableUnitTagSet taxableUnitSet = TaxableUnitTagSet::none();
    taxableUnitSet.setTag(type::TaxableUnit::Itinerary);

    const type::MoneyAmount taxAmount(1001,100);
    const type::BSRValue bsrValue(1, 1);

    auto currencyServiceMock = std::make_unique<CurrencyServiceMock>();
    EXPECT_CALL(*currencyServiceMock, convertTo(_,_,_)).WillOnce(Return(taxAmount));

    _services.setCurrencyService(currencyServiceMock.release());

    const type::CurrencyCode paymentCurrency;
    FlatTaxApplicator applicator(_rule, _services, taxableUnitSet, paymentCurrency);

    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(1001,100), _paymentDetail->taxEquivalentAmount());
  }

  void testFlatTaxWithYqYrAndTaxOnTax()
  {
    TaxableUnitTagSet taxableUnitSet = TaxableUnitTagSet::none();
    taxableUnitSet.setTag(type::TaxableUnit::Itinerary);
    taxableUnitSet.setTag(type::TaxableUnit::YqYr);
    taxableUnitSet.setTag(type::TaxableUnit::TaxOnTax);

    const type::MoneyAmount taxAmount(3003,100);
    const type::BSRValue bsrValue(1, 1);

    auto currencyServiceMock = std::make_unique<CurrencyServiceMock>();
    EXPECT_CALL(*currencyServiceMock, convertTo(_,_,_)).WillOnce(Return(taxAmount));

    _services.setCurrencyService(currencyServiceMock.release());

    const type::CurrencyCode paymentCurrency;
    FlatTaxApplicator applicator(_rule, _services, taxableUnitSet, paymentCurrency);

    PaymentDetailMock taxOnTaxPaymentDetail;
    _paymentDetail->taxOnTaxItems().push_back(&taxOnTaxPaymentDetail);
    std::vector<TaxableYqYr> yqyrs(1);
    std::vector<type::Index> ids(1, 0);
    _paymentDetail->getMutableYqYrDetails().init(yqyrs, ids);

    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(3003,100), _paymentDetail->taxEquivalentAmount());
  }

  void testFlatTaxWithOptionalServices()
  {
    TaxableUnitTagSet taxableUnitSet = TaxableUnitTagSet::none();
    taxableUnitSet.setTag(type::TaxableUnit::Itinerary);
    taxableUnitSet.setTag(type::TaxableUnit::OCFlightRelated);

    const type::MoneyAmount otherCurrencyAmount(701,100);
    const type::BSRValue bsrValue(1, 1);

    auto currencyServiceMock = std::make_unique<CurrencyServiceMock>();
    EXPECT_CALL(*currencyServiceMock, convertTo(_,_,_))
        .Times(2)
        .WillRepeatedly(Return(otherCurrencyAmount));

    _services.setCurrencyService(currencyServiceMock.release());

    const type::CurrencyCode paymentCurrency;
    FlatTaxApplicator applicator(_rule, _services, taxableUnitSet, paymentCurrency);
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().amount() = type::MoneyAmount(50,1);

    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(otherCurrencyAmount, _paymentDetail->taxEquivalentAmount());
    CPPUNIT_ASSERT_EQUAL(otherCurrencyAmount,
        _paymentDetail->optionalServiceItems().back().getTaxEquivalentAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(1001,100),
        _paymentDetail->optionalServiceItems().back().taxAmount());
  }

  void testFlatTaxWithOptionalServices_whenQuantityDifferentFromDefault()
  {
    TaxableUnitTagSet taxableUnitSet = TaxableUnitTagSet::none();
    taxableUnitSet.setTag(type::TaxableUnit::Itinerary);
    taxableUnitSet.setTag(type::TaxableUnit::OCFlightRelated);

    const type::BSRValue bsrValue(1, 1);

    auto currencyServiceMock = std::make_unique<CurrencyServiceMock>();
    EXPECT_CALL(*currencyServiceMock, convertTo(_,_,_))
        .Times(2)
        .WillRepeatedly(testing::Invoke(fakeConvertToMethod));

    _services.setCurrencyService(currencyServiceMock.release());

    const type::CurrencyCode paymentCurrency;
    FlatTaxApplicator applicator(_rule, _services, taxableUnitSet, paymentCurrency);
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().amount() = type::MoneyAmount(50,1);
    _paymentDetail->optionalServiceItems().back().setQuantity(3);

    applicator.apply(*_paymentDetail);

    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(_paymentDetail->taxAmt() * EquivalentCurrencyRate), _paymentDetail->taxEquivalentAmount());

    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(_paymentDetail->taxAmt() * _paymentDetail->optionalServiceItems().back().getQuantity()),
                         _paymentDetail->optionalServiceItems().back().taxAmount());

    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(_paymentDetail->optionalServiceItems().back().taxAmount() * EquivalentCurrencyRate),
                         _paymentDetail->optionalServiceItems().back().getTaxEquivalentAmount());
  }

  void testFlatTaxWithBaggage()
  {
    TaxableUnitTagSet taxableUnitSet = TaxableUnitTagSet::none();
    taxableUnitSet.setTag(type::TaxableUnit::Itinerary);
    taxableUnitSet.setTag(type::TaxableUnit::BaggageCharge);

    const type::MoneyAmount taxAmount(1001,100);
    const type::BSRValue bsrValue(1, 1);

    auto currencyServiceMock = std::make_unique<CurrencyServiceMock>();
    EXPECT_CALL(*currencyServiceMock, convertTo(_,_,_))
        .Times(2)
        .WillRepeatedly(Return(taxAmount));

    _services.setCurrencyService(currencyServiceMock.release());

    const type::CurrencyCode paymentCurrency;
    FlatTaxApplicator applicator(_rule, _services, taxableUnitSet, paymentCurrency);
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().amount() = type::MoneyAmount(50,1);
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::BaggageCharge;

    applicator.apply(*_paymentDetail);
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(1001,100), _paymentDetail->taxEquivalentAmount());
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(1001,100),
        _paymentDetail->optionalServiceItems().back().taxAmount());
  }

  void testFlatTaxWithBaggage_whenQuantityDifferentFromDefault()
  {
    TaxableUnitTagSet taxableUnitSet = TaxableUnitTagSet::none();
    taxableUnitSet.setTag(type::TaxableUnit::Itinerary);
    taxableUnitSet.setTag(type::TaxableUnit::BaggageCharge);

    const type::BSRValue bsrValue(1, 1);

    auto currencyServiceMock = std::make_unique<CurrencyServiceMock>();
    EXPECT_CALL(*currencyServiceMock, convertTo(_,_,_))
        .Times(2)
        .WillRepeatedly(testing::Invoke(fakeConvertToMethod));

    _services.setCurrencyService(currencyServiceMock.release());

    const type::CurrencyCode paymentCurrency;
    FlatTaxApplicator applicator(_rule, _services, taxableUnitSet, paymentCurrency);
    _paymentDetail->optionalServiceItems().push_back(new OptionalService());
    _paymentDetail->optionalServiceItems().back().amount() = type::MoneyAmount(50,1);
    _paymentDetail->optionalServiceItems().back().type() = type::OptionalServiceTag::BaggageCharge;
    _paymentDetail->optionalServiceItems().back().setQuantity(3);

    applicator.apply(*_paymentDetail);

    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(_paymentDetail->taxAmt() * EquivalentCurrencyRate), _paymentDetail->taxEquivalentAmount());

    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(_paymentDetail->taxAmt() * _paymentDetail->optionalServiceItems().back().getQuantity()),
                         _paymentDetail->optionalServiceItems().back().taxAmount());

    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(_paymentDetail->optionalServiceItems().back().taxAmount() * EquivalentCurrencyRate),
                         _paymentDetail->optionalServiceItems().back().getTaxEquivalentAmount());
  }

private:
  PaymentDetail* _paymentDetail;
  FlatTaxRule* _rule;
  DefaultServices _services;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FlatTaxTest);
} // namespace tax
