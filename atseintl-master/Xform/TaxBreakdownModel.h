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

#pragma once

#include "Common/MCPCarrierUtil.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Xform/AbstractTaxBreakdownModel.h"
#include "Xform/XformUtil.h"

namespace tse
{
class TaxBreakdownModel : public AbstractTaxBreakdownModel
{
  const PricingTrx& _trx;
  const TaxItem& _taxItem;

public:
  TaxBreakdownModel(const PricingTrx& trx, const TaxItem& aTaxItem) : _trx(trx), _taxItem(aTaxItem) {}
  TaxCode getTaxCode() const override { return _taxItem.taxCode(); }
  MoneyAmount getTaxAmount() const override { return _taxItem.taxAmount(); }
  CurrencyCode getTaxCurrencyCode() const override { return _taxItem.paymentCurrency(); }
  uint16_t getPaymentCurrencyPrecision() const override { return _taxItem.paymentCurrencyNoDec(); }
  Indicator getRefundableTaxTag() const override { return _taxItem.getRefundableTaxTag(); }
  CarrierCode getTaxAirlineCode() const override
  {
    return MCPCarrierUtil::swapToPseudo(&_trx, _taxItem.carrierCode());
  }
};

} // end of tse namespace
