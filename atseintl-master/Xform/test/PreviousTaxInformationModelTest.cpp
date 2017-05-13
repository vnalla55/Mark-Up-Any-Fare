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

#include "DataModel/PricingTrx.h"
#include "DataModel/NetFarePath.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Xform/PreviousTaxInformationModel.h"
#include "Xform/TaxBreakdownModel.h"
#include "Xform/TaxInformationModel.h"
#include "test/include/TestConfigInitializer.h"

#include "gmock/gmock.h"

namespace tse
{
class ATaxBreakdownModel : public testing::Test
{
public:
  std::unique_ptr<AbstractTaxBreakdownModel> taxBreakdownModel;
  TaxItem taxItem;
  PricingTrx trx;

  void SetUp() override { taxBreakdownModel.reset(new TaxBreakdownModel(trx, taxItem)); }
};

TEST_F(ATaxBreakdownModel, IsTaxCodeEqualToTaxCodeInTaxItem)
{
  taxItem.taxCode() = "DU1";
  ASSERT_THAT(taxBreakdownModel->getTaxCode(), testing::Eq(taxItem.taxCode()));
}

TEST_F(ATaxBreakdownModel, IsTaxAmountInPaymentCurrencyEqualToExpected)
{
  taxItem.taxAmount() = 123.0;
  ASSERT_THAT(taxBreakdownModel->getTaxAmount(), testing::DoubleEq(123.0));
}

TEST_F(ATaxBreakdownModel, IsTaxCurrencyCodeEqualToExpected)
{
  taxItem.setPaymentCurrency(CurrencyCode("PLN"));
  ASSERT_THAT(taxBreakdownModel->getTaxCurrencyCode(), testing::Eq(CurrencyCode("PLN")));
}

TEST_F(ATaxBreakdownModel, IsPaymantCurrencyPrecisionEqualToExpected)
{
  taxItem.setPaymentCurrencyNoDec(2);
  ASSERT_THAT(taxBreakdownModel->getPaymentCurrencyPrecision(), testing::Eq(2));
}

class ATaxInformationModel : public testing::Test
{
public:
  std::unique_ptr<AbstractTaxInformationModel> taxInformationModel;
  TaxRecord taxRecord;
  PricingTrx trx;
  std::unique_ptr<TaxResponse> taxResponse;

  void SetUp() override
  {
    taxResponse.reset(new TaxResponse);
    std::vector<TaxResponse*> taxResponseVector{taxResponse.get()};
    trx.addToExcItinTaxResponse(taxResponseVector.begin(), taxResponseVector.end());
    taxInformationModel.reset(new TaxInformationModel(taxRecord, trx));
  }
};

TEST_F(ATaxInformationModel, IsTaxCodeEqualToExpected)
{
  taxRecord.setTaxCode(TaxCode("DU1"));
  ASSERT_THAT(taxInformationModel->getTaxCode(), testing::Eq(TaxCode("DU1")));
}

TEST_F(ATaxInformationModel, IsTaxAmountEqualToZeroIfExemptAllTaxes)
{
  taxRecord.setFailCode(TaxItem::EXEMPT_ALL_TAXES);
  taxRecord.setTaxAmount(MoneyAmount(123.0));
  ASSERT_THAT(taxInformationModel->getTaxAmount(), testing::DoubleEq(MoneyAmount(0.0)));
}

TEST_F(ATaxInformationModel, IsTaxAmountEqualToZeroIfExemptSpecifiedTaxes)
{
  taxRecord.setFailCode(TaxItem::EXEMPT_SPECIFIED_TAXES);
  taxRecord.setTaxAmount(MoneyAmount(123.0));
  ASSERT_THAT(taxInformationModel->getTaxAmount(), testing::DoubleEq(MoneyAmount(0.0)));
}

TEST_F(ATaxInformationModel, IsTaxAmountEqualToExpectedIfNotExempted)
{
  taxRecord.setTaxAmount(MoneyAmount(123.0));
  ASSERT_THAT(taxInformationModel->getTaxAmount(), testing::DoubleEq(MoneyAmount(123.0)));
}

TEST_F(ATaxInformationModel, IsTaxCurrencyCodeEqualToExpected)
{
  taxRecord.taxCurrencyCode() = CurrencyCode("PLN");
  ASSERT_THAT(taxInformationModel->getTaxCurrencyCode(), testing::Eq(CurrencyCode("PLN")));
}

TEST_F(ATaxInformationModel, IsTaxPointLocSabreTaxesEqualToTEIfExemptAllTaxes)
{
  taxRecord.setFailCode(TaxItem::EXEMPT_ALL_TAXES);
  ASSERT_THAT(taxInformationModel->getTaxPointLocSabreTaxes(), testing::Eq("TE"));
}

TEST_F(ATaxInformationModel, IsTaxPointLocSabreTaxesEqualToZeroIfExemptSpecifiedTaxes)
{
  taxRecord.setFailCode(TaxItem::EXEMPT_SPECIFIED_TAXES);
  ASSERT_THAT(taxInformationModel->getTaxPointLocSabreTaxes(), testing::Eq("TE"));
}

TEST_F(ATaxInformationModel, IsTaxPointLocSabreTaxesEqualToLocalBoardIfTaxCodeNotEqualToXF)
{
  taxRecord.setTaxCode(TaxCode("US1"));

  ASSERT_THAT(taxInformationModel->getTaxPointLocSabreTaxes(), testing::Eq(""));
}

TEST_F(ATaxInformationModel,
       IsTaxPointLocSabreTaxesEqualToPfcAirportCodeIfPfcAmountEqualsToTaxAmount)
{
  taxRecord.setTaxCode(TaxCode("XF"));
  taxRecord.setTaxAmount(MoneyAmount(123.0));
  std::unique_ptr<PfcItem> pfc(new PfcItem);
  pfc->setPfcAirportCode(LocCode("AA"));
  pfc->setPfcAmount(MoneyAmount(123.0));
  taxResponse->pfcItemVector().push_back(pfc.get());

  ASSERT_THAT(taxInformationModel->getTaxPointLocSabreTaxes(), testing::Eq("AA"));
}

class APreviousTaxInformationModel : public testing::Test
{
public:
  TestConfigInitializer testConfigInitializer;
  std::unique_ptr<PreviousTaxInformationModel> previousTaxInformationModel;
  PricingTrx trx;
  std::unique_ptr<TaxResponse> taxResponse;

  void SetUp() override { taxResponse.reset(new TaxResponse); }
};

TEST_F(APreviousTaxInformationModel, IsNumberOfTaxBreakdownEqualToExpected)
{
  std::unique_ptr<TaxItem> first(new TaxItem);
  std::unique_ptr<TaxItem> second(new TaxItem);
  std::unique_ptr<TaxItem> third(new TaxItem);

  taxResponse->taxItemVector().push_back(first.get());
  taxResponse->taxItemVector().push_back(second.get());
  taxResponse->taxItemVector().push_back(third.get());

  std::vector<TaxResponse*> taxResponseVector{taxResponse.get()};

  trx.addToExcItinTaxResponse(taxResponseVector.begin(), taxResponseVector.end());
  previousTaxInformationModel.reset(new PreviousTaxInformationModel(trx));

  ASSERT_THAT(previousTaxInformationModel->getTaxBreakdown().size(), testing::Eq(3));
}

TEST_F(APreviousTaxInformationModel, IsNumberOfTaxInformationEqualToExpected)
{
  std::unique_ptr<TaxRecord> first(new TaxRecord);
  std::unique_ptr<TaxRecord> second(new TaxRecord);
  std::unique_ptr<TaxRecord> third(new TaxRecord);

  taxResponse->taxRecordVector().push_back(first.get());
  taxResponse->taxRecordVector().push_back(second.get());
  taxResponse->taxRecordVector().push_back(third.get());

  std::vector<TaxResponse*> taxResponseVector{taxResponse.get()};
  trx.addToExcItinTaxResponse(taxResponseVector.begin(), taxResponseVector.end());

  previousTaxInformationModel.reset(new PreviousTaxInformationModel(trx));

  ASSERT_THAT(previousTaxInformationModel->getTaxInformation().size(), testing::Eq(3));
}

TEST_F(APreviousTaxInformationModel, IsNumberOfTaxInfoBreakdownEqualToExpected)
{
  std::unique_ptr<TaxItem> first(new TaxItem);
  std::unique_ptr<TaxItem> second(new TaxItem);
  std::unique_ptr<TaxItem> third(new TaxItem);

  taxResponse->taxItemVector().push_back(first.get());
  taxResponse->taxItemVector().push_back(second.get());
  taxResponse->taxItemVector().push_back(third.get());

  trx.addToTaxInfoResponse(taxResponse.get());
  previousTaxInformationModel.reset(new PreviousTaxInformationModel(trx));

  ASSERT_THAT(previousTaxInformationModel->getTaxInfoBreakdown().size(), testing::Eq(3));
}

TEST_F(APreviousTaxInformationModel, IsEmpty)
{
  previousTaxInformationModel.reset(new PreviousTaxInformationModel(trx));
  ASSERT_THAT(previousTaxInformationModel->isEmpty(), true);
}

TEST_F(APreviousTaxInformationModel, IsNetEmpty)
{
  previousTaxInformationModel.reset(new PreviousTaxInformationModel(trx));
  ASSERT_THAT(previousTaxInformationModel->isNetEmpty(), true);
}

TEST_F(APreviousTaxInformationModel, IsNotEmpty)
{
  TaxItem taxItem;
  taxResponse->taxItemVector().push_back(&taxItem);

  std::vector<TaxResponse*> taxResponseVector{taxResponse.get()};

  trx.addToExcItinTaxResponse(taxResponseVector.begin(), taxResponseVector.end());
  previousTaxInformationModel.reset(new PreviousTaxInformationModel(trx));

  ASSERT_THAT(previousTaxInformationModel->isEmpty(), false);
}

TEST_F(APreviousTaxInformationModel, IsNetNotEmpty)
{
  NetFarePath netFarePath;
  taxResponse->farePath() = &netFarePath;

  TaxItem taxItem;
  taxResponse->taxItemVector().push_back(&taxItem);

  std::vector<TaxResponse*> taxResponseVector{taxResponse.get()};

  trx.addToExcItinTaxResponse(taxResponseVector.begin(), taxResponseVector.end());
  previousTaxInformationModel.reset(new PreviousTaxInformationModel(trx));

  ASSERT_THAT(previousTaxInformationModel->isNetEmpty(), false);
}

} // end of tse namespace
