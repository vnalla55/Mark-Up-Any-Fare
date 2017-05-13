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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Xform/AbstractTaxSummaryInfo.h"


namespace tse
{

class TaxSummaryInfo : public AbstractTaxSummaryInfo
{
  CalcTotals& _calcTotals;
  const TaxRecord& _taxRecord;
  PricingTrx& _pricingTrx;

public:

  TaxSummaryInfo(CalcTotals& calcTotals, const TaxRecord& taxRecord, PricingTrx& pricingTrx)
      : _calcTotals(calcTotals), _taxRecord(taxRecord), _pricingTrx(pricingTrx)
  {
  }

  const TaxCode&
  getTaxCode() const override
  {
    return _taxRecord.taxCode();
  }

  const MoneyAmount
  getTaxValue() const override
  {
    return _taxRecord.getTaxAmount();
  }

  const CurrencyNoDec
  getCurrencyNoDec() const override
  {
    return _calcTotals.taxNoDec();
  }

  const CurrencyCode&
  getTaxCurrencyCode() const override
  {
    return _taxRecord.taxCurrencyCode();
  }

  const NationCode&
  getTaxCountryCode() const override
  {
    return _taxRecord.taxNation();
  }

  bool
  getGoodAndServicesTax() const override
  {
    return _taxRecord.gstTaxInd();
  }
};

} // end of tse namespace
