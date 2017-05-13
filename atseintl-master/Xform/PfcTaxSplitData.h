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

#include "Common/CurrencyConversionFacade.h"
#include "Common/TaxRound.h"
#include "FareCalc/CalcTotals.h"
#include "Taxes/Pfc/PfcItem.h"
#include "Xform/AbstractTaxSplitData.h"

namespace tse
{

class PfcTaxSplitData : public AbstractTaxSplitData
{
  static const std::string PFC_DESCRIPTION;
  static const TaxCode TAX_CODE;
  static const NationCode& TAX_COUNTRY_CODE;

  CalcTotals& _calcTotals;
  const PfcItem& _pfcItem;
  PricingTrx& _pricingTrx;
  MoneyAmount _pfcValue;
  CurrencyNoDec _amountNoDec;
  CurrencyCode _currnecyCode;


  PfcTaxSplitData(CalcTotals& calcTotals, const PfcItem& pfcItem, PricingTrx& pricingTrx)
      : _calcTotals(calcTotals), _pfcItem(pfcItem), _pricingTrx(pricingTrx), _pfcValue(0),
        _amountNoDec(2), _currnecyCode(_calcTotals.taxCurrencyCode())
  {
    computeAndStoreTaxValue();
  }

  MoneyAmount
  getPfcValue();

public:

  static std::unique_ptr<AbstractTaxSplitData>
  create(PricingTrx& pricingTrx, CalcTotals& calcTotals, const PfcItem& pfcItem)
  {
    return std::unique_ptr<AbstractTaxSplitData>(
        new PfcTaxSplitData(calcTotals, pfcItem, pricingTrx));
  }

  const TaxCode&
  getTaxCode() const override
  {
    return TAX_CODE;
  }

  const MoneyAmount&
  getTaxValue() const override
  {
    return _pfcValue;
  }

  const CurrencyNoDec
  getCurrencyNoDec() const override
  {
    return _amountNoDec;
  }

  void
  setTaxValue(const MoneyAmount& taxValue) override
  {
    _pfcValue = taxValue;
  }

  void
  computeAndStoreTaxValue() override
  {
    _pfcValue = getPfcValue();
  }

  const CurrencyCode&
  getTaxCurrencyCode() const override
  {
    return _currnecyCode;
  }

  const LocCode&
  getStationCode() const override
  {
    return _pfcItem.pfcAirportCode();
  }

  const TaxDescription&
  getTaxDescription() const override
  {
    return PFC_DESCRIPTION;
  }

  std::pair<MoneyAmount, CurrencyNoDec>
  getAmountPublished() const override
  {
    return std::make_pair(_pfcItem.pfcAmount(), _pfcItem.pfcDecimals());
  }

  const CurrencyCode&
  getPublishedCurrency() const override
  {
    return _pfcItem.pfcCurrencyCode();
  }

  const NationCode&
  getTaxCountryCode() const override
  {
    return TAX_COUNTRY_CODE;
  }

  bool
  getGoodAndServicesTax() const override;

  CarrierCode
  getTaxAirlineCode() const override;

  char
  getTaxType() const override;
};

} // end of tse namespace
