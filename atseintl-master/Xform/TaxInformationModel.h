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

#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Xform/AbstractTaxInformationModel.h"

namespace tse
{
class PricingTrx;

class TaxInformationModel : public AbstractTaxInformationModel
{
  const TaxRecord& _taxRecord;
  TaxResponse* _taxResponse;
  PricingTrx& _trx;

  PfcItem* findPfc(MoneyAmount amount) const;

public:
  TaxInformationModel(const TaxRecord& taxRecord, PricingTrx& trx)
    : _taxRecord(taxRecord), _taxResponse(trx.getExcItinTaxResponse().front()), _trx(trx)
  {
  }

  TaxCode getTaxCode() const override { return _taxRecord.taxCode(); }

  MoneyAmount getTaxAmount() const override;

  CurrencyCode getTaxCurrencyCode() const override { return _taxRecord.taxCurrencyCode(); }

  LocCode getTaxPointLocSabreTaxes() const override;

  MoneyAmount getAmountPublished() const override { return _taxRecord.publishedAmount(); }

  uint16_t getPublishedCurrencyPrecision() const override;

  CurrencyCode getPublishedCurrency() const override { return _taxRecord.publishedCurrencyCode(); }

  uint16_t getPaymentCurrencyPrecision() const override { return _taxRecord.taxNoDec(); }

  NationCode getTaxCountryCode() const override { return _taxRecord.taxNation(); }

  bool getGoodAndServicesTax() const override { return _taxRecord.gstTaxInd(); }

  TaxDescription getTaxDescription() const override { return _taxRecord.taxDescription(); }
};

} // end of tse namespace
