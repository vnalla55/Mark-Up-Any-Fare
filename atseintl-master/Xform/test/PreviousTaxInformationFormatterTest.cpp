// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Common/XMLConstruct.h"
#include "Xform/PricingResponseXMLTags.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Xform/AbstractTaxBreakdownModel.h"
#include "Xform/AbstractTaxInformationModel.h"
#include "Xform/PreviousTaxInformationFormatter.h"
#include "Xform/PreviousTaxInformationModel.h"

#include "gmock/gmock.h"

#include <memory>
#include <vector>

namespace tse
{
class TaxBreakdownModelMock : public AbstractTaxBreakdownModel
{
public:
  MOCK_CONST_METHOD0(getTaxCode, TaxCode());
  MOCK_CONST_METHOD0(getTaxAmount, MoneyAmount());
  MOCK_CONST_METHOD0(getTaxCurrencyCode, CurrencyCode());
  MOCK_CONST_METHOD0(getPaymentCurrencyPrecision, uint16_t());
  MOCK_CONST_METHOD0(getTaxAirlineCode, CarrierCode());
  MOCK_CONST_METHOD0(getRefundableTaxTag, char());
};

class TaxInformationModelMock : public AbstractTaxInformationModel
{
public:
  MOCK_CONST_METHOD0(getTaxCode, TaxCode());
  MOCK_CONST_METHOD0(getTaxAmount, MoneyAmount());
  MOCK_CONST_METHOD0(getTaxCurrencyCode, CurrencyCode());
  MOCK_CONST_METHOD0(getTaxPointLocSabreTaxes, LocCode());
  MOCK_CONST_METHOD0(getAmountPublished, MoneyAmount());
  MOCK_CONST_METHOD0(getPublishedCurrencyPrecision, uint16_t());
  MOCK_CONST_METHOD0(getPublishedCurrency, CurrencyCode());
  MOCK_CONST_METHOD0(getPaymentCurrencyPrecision, uint16_t());
  MOCK_CONST_METHOD0(getTaxCountryCode, NationCode());
  MOCK_CONST_METHOD0(getGoodAndServicesTax, bool());
  MOCK_CONST_METHOD0(getTaxDescription, TaxDescription());
};

class PreviousTaxInformationModelMock : public AbstractPreviousTaxInformationModel
{
public:
  MOCK_CONST_METHOD0(getTaxBreakdown,
                     const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&());
  MOCK_CONST_METHOD0(getTaxInfoBreakdown,
                     const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&());
  MOCK_CONST_METHOD0(getTaxInformation,
                     const std::vector<std::unique_ptr<AbstractTaxInformationModel>>&());
  MOCK_CONST_METHOD0(getNetTaxBreakdown,
                     const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>>&());
  MOCK_CONST_METHOD0(getNetTaxInformation,
                     const std::vector<std::unique_ptr<AbstractTaxInformationModel>>&());
  MOCK_CONST_METHOD0(isEmpty, bool());
  MOCK_CONST_METHOD0(isNetEmpty, bool());
};

class ATaxInfoFormatter : public testing::Test
{
public:
  XMLConstruct construct;
  std::unique_ptr<PreviousTaxInformationFormatter> formatter;

  void SetUp() override { formatter.reset(new PreviousTaxInformationFormatter(construct)); }
};

TEST_F(ATaxInfoFormatter, IsFormattedTAXEqualToExpected)
{
  TaxInformationModelMock model;

  EXPECT_CALL(model, getTaxCode()).WillOnce(testing::Return("US1"));
  EXPECT_CALL(model, getTaxAmount()).WillOnce(testing::Return(200.0));
  EXPECT_CALL(model, getPaymentCurrencyPrecision()).WillOnce(testing::Return(2));
  EXPECT_CALL(model, getTaxCurrencyCode()).WillOnce(testing::Return("USD"));
  EXPECT_CALL(model, getPublishedCurrencyPrecision()).WillOnce(testing::Return(2));
  EXPECT_CALL(model, getPublishedCurrency()).WillOnce(testing::Return("USD"));
  EXPECT_CALL(model, getAmountPublished()).WillOnce(testing::Return(200.0));
  EXPECT_CALL(model, getTaxCountryCode()).WillOnce(testing::Return("US"));
  EXPECT_CALL(model, getGoodAndServicesTax()).WillOnce(testing::Return(false));
  EXPECT_CALL(model, getTaxDescription())
      .WillOnce(testing::Return("DOMESTIC SAFETY AND SECURITY CHARGE - DEPARTURES"));

  formatter->formatTAX(model);

  ASSERT_THAT(
      construct.getXMLData(),
      testing::Eq(
          "<TAX BC0=\"US1\" C6B=\"200.00\" C40=\"USD\" C6A=\"200.00\" C41=\"USD\" "
          "A40=\"US\" S04=\"DOMESTIC SAFETY AND SECURITY CHARGE - DEPARTURES\"/>"));
}

TEST_F(ATaxInfoFormatter, IsP2QInResponseIfGoodAndServicesTaxIsTrue)
{
  TaxInformationModelMock model;

  EXPECT_CALL(model, getTaxCode()).WillOnce(testing::Return("US1"));
  EXPECT_CALL(model, getTaxAmount()).WillOnce(testing::Return(200.0));
  EXPECT_CALL(model, getPaymentCurrencyPrecision()).WillOnce(testing::Return(2));
  EXPECT_CALL(model, getTaxCurrencyCode()).WillOnce(testing::Return("USD"));
  EXPECT_CALL(model, getPublishedCurrencyPrecision()).WillOnce(testing::Return(2));
  EXPECT_CALL(model, getPublishedCurrency()).WillOnce(testing::Return("USD"));
  EXPECT_CALL(model, getAmountPublished()).WillOnce(testing::Return(200.0));
  EXPECT_CALL(model, getTaxCountryCode()).WillOnce(testing::Return("US"));
  EXPECT_CALL(model, getGoodAndServicesTax()).WillOnce(testing::Return(true));
  EXPECT_CALL(model, getTaxDescription())
      .WillOnce(testing::Return("DOMESTIC SAFETY AND SECURITY CHARGE - DEPARTURES"));

  formatter->formatTAX(model);

  ASSERT_THAT(
      construct.getXMLData(),
      testing::Eq(
          "<TAX BC0=\"US1\" C6B=\"200.00\" C40=\"USD\" C6A=\"200.00\" C41=\"USD\" "
          "A40=\"US\" P2Q=\"T\" S04=\"DOMESTIC SAFETY AND SECURITY CHARGE - DEPARTURES\"/>"));
}

TEST_F(ATaxInfoFormatter, IsFormattedTBDEqualToExpected)
{
  TaxBreakdownModelMock model;

  EXPECT_CALL(model, getTaxCode()).WillOnce(testing::Return("US1"));
  EXPECT_CALL(model, getPaymentCurrencyPrecision()).WillOnce(testing::Return(2));
  EXPECT_CALL(model, getTaxAmount()).WillOnce(testing::Return(200.0));
  EXPECT_CALL(model, getTaxCurrencyCode()).WillOnce(testing::Return("USD"));
  EXPECT_CALL(model, getTaxAirlineCode()).WillOnce(testing::Return("LH"));
  EXPECT_CALL(model, getRefundableTaxTag()).WillOnce(testing::Return('N'));

  formatter->formatTBD(model);
  ASSERT_THAT(construct.getXMLData(),
        testing::Eq(
            "<TBD BC0=\"US1\" C6B=\"200.00\" C40=\"USD\" X21=\"N\" A04=\"LH\"/>"));
}

TEST_F(ATaxInfoFormatter, IsFormattedTaxInfoTBDEqualToExpected)
{
  TaxBreakdownModelMock model;

  EXPECT_CALL(model, getTaxCode()).WillOnce(testing::Return("US1"));
  EXPECT_CALL(model, getRefundableTaxTag()).WillOnce(testing::Return('N'));

  formatter->formatTaxInfoTBD(model);
    ASSERT_THAT(construct.getXMLData(),
          testing::Eq(
              "<TBD BC0=\"US1\" X21=\"N\"/>"));
}

TEST_F(ATaxInfoFormatter, IsEmptyCalled)
{
  PreviousTaxInformationModelMock model;

  EXPECT_CALL(model, isEmpty()).WillOnce(testing::Return(true));

  formatter->formatPTI(model);
}

TEST_F(ATaxInfoFormatter, IsGetTaxInformationAndGetTaxBreakdownCalled)
{
  PreviousTaxInformationModelMock model;

  const std::vector<std::unique_ptr<AbstractTaxInformationModel>> taxInformationVector{};
  const std::vector<std::unique_ptr<AbstractTaxBreakdownModel>> taxBreakdownVector{};

  EXPECT_CALL(model, isEmpty()).WillOnce(testing::Return(false));
  EXPECT_CALL(model, getTaxBreakdown()).WillOnce(testing::ReturnRef(taxBreakdownVector));
  EXPECT_CALL(model, getTaxInformation()).WillOnce(testing::ReturnRef(taxInformationVector));
  EXPECT_CALL(model, getTaxInfoBreakdown()).WillOnce(testing::ReturnRef(taxBreakdownVector));
  EXPECT_CALL(model, isNetEmpty()).WillOnce(testing::Return(false));
  EXPECT_CALL(model, getNetTaxInformation()).WillOnce(testing::ReturnRef(taxInformationVector));
  EXPECT_CALL(model, getNetTaxBreakdown()).WillOnce(testing::ReturnRef(taxBreakdownVector));

  formatter->formatPTI(model);
}

} // end of tse namespace
