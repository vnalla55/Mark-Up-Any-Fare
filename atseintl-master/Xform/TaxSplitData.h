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

#pragma once

#include "FareCalc/CalcTotals.h"
#include "FareCalc/FcTaxInfo.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Xform/AbstractTaxSplitData.h"
#include "Xform/XformUtil.h"


namespace tse
{

class TaxSplitData : public AbstractTaxSplitData
{
  CalcTotals& _calcTotals;
  const FareCalc::SplitTaxInfo& _splitTaxInfo;
  const TaxItem& _taxItem;
  PricingTrx& _pricingTrx;
  const FareUsage* _fareUsage;
  MoneyAmount _taxValue {0};

public:

  static std::shared_ptr<AbstractTaxSplitData>
      create(PricingTrx& pricingTrx, CalcTotals& calcTotals,
      const FareCalc::SplitTaxInfo& splitTaxInfo, const TaxItem& taxItem,
      const FareUsage* fareUsage)
  {
    return std::shared_ptr<AbstractTaxSplitData>(
        new TaxSplitData(calcTotals, splitTaxInfo, taxItem, pricingTrx, fareUsage));
  }

  const TaxCode&
  getTaxCode() const override
  {
    return _taxItem.taxCode();
  }

  const MoneyAmount&
  getTaxValue() const override;

  const CurrencyNoDec
  getCurrencyNoDec() const override;

  void
  setTaxValue(const MoneyAmount& taxValue) override;

  void
  computeAndStoreTaxValue() override;

  const CurrencyCode&
  getTaxCurrencyCode() const override
  {
    return _taxItem.paymentCurrency();
  }

  const LocCode&
  getStationCode() const override
  {
    return _taxItem.taxLocalBoard();
  }

  const TaxDescription&
  getTaxDescription() const override
  {
    return _taxItem.taxDescription();
  }

  std::pair<MoneyAmount, CurrencyNoDec>
  getAmountPublished() const override;

  const CurrencyCode&
  getPublishedCurrency() const override
  {
    return _taxItem.taxCur();
  }

  const NationCode&
  getTaxCountryCode() const override
  {
    return _taxItem.nation();
  }

  bool
  getGoodAndServicesTax() const override
  {
    return _taxItem.gstTax();
  }

  CarrierCode
  getTaxAirlineCode() const override;

  char
  getTaxType() const override
  {
    return _taxItem.taxType();
  }

private:

  TaxSplitData(CalcTotals& calcTotals, const FareCalc::SplitTaxInfo& splitTaxInfo,
      const TaxItem& taxItem, PricingTrx& pricingTrx, const FareUsage* fareUsage)
      : _calcTotals(calcTotals), _splitTaxInfo(splitTaxInfo),
        _taxItem(taxItem), _pricingTrx(pricingTrx), _fareUsage(fareUsage), _taxValue(0)
  {
    computeAndStoreTaxValue();
  }
};

}
