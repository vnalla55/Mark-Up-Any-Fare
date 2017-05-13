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

#include "Xform/PfcTaxSplitData.h"

namespace tse
{

const std::string PfcTaxSplitData::PFC_DESCRIPTION = "PASSENGER FACILITY CHARGES";
const TaxCode PfcTaxSplitData::TAX_CODE = "XF";
const NationCode& PfcTaxSplitData::TAX_COUNTRY_CODE = "US";

MoneyAmount
PfcTaxSplitData::getPfcValue()
{
  TaxRound taxRound;
  MoneyAmount amount = _pfcItem.pfcAmount();
  _amountNoDec = _pfcItem.pfcDecimals();

  if (_pfcItem.pfcCurrencyCode() != getTaxCurrencyCode())
  {
    Money source(_pfcItem.pfcAmount(), _pfcItem.pfcCurrencyCode());
    Money target(getTaxCurrencyCode());
    CurrencyConversionFacade ccFacade;
    ccFacade.convert(target, source, _pricingTrx, false, CurrencyConversionRequest::TAXES);
    amount = target.value();
    _amountNoDec = target.noDec(_pricingTrx.ticketingDate());
    CurrencyConverter currencyConverter;
    RoundingFactor roundingUnit = 0.1;
    RoundingRule roundingRule = NONE;
    taxRound.retrieveNationRoundingSpecifications(_pricingTrx, roundingUnit, _amountNoDec, roundingRule);
    Money targetMoney2(amount, getTaxCurrencyCode());
    _amountNoDec = targetMoney2.noDec(_pricingTrx.ticketingDate());

    if (currencyConverter.round(targetMoney2, roundingUnit, roundingRule))
      amount = targetMoney2.value();
  }

  return amount;
}

bool
PfcTaxSplitData::getGoodAndServicesTax() const
{
  return false;
}

CarrierCode
PfcTaxSplitData::getTaxAirlineCode() const
{
  return "";
}

char
PfcTaxSplitData::getTaxType() const
{
  return '\0';
}

}
