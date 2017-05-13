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

#include "Taxes/LegacyTaxes/TaxDU1_00.h"

#include "Common/TaxRound.h"
#include "Common/TrxUtil.h"
#include "DBAccess/TaxCodeReg.h"
#include "Diagnostic/DiagManager.h"

#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/TaxRange.h"


using namespace tse;
using namespace std;

namespace
{
const CurrencyCode targetCurrencyCode("USD");
}

class TaxRangeDU1 : public TaxRange
{
private:
  const TaxCode& _taxCode;
public:
  TaxRangeDU1(const TaxCode& taxCode)
    : TaxRange()
    , _taxCode(taxCode)
  {}

  MoneyAmount
  retrieveTotalFare(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    FarePath& farePath,
                    const CurrencyCode&) const override
  {
    DiagManager diag(trx, Diagnostic818);
    if (diag.isActive())
    {
      const std::string& strVal = trx.diagnostic().diagParamMapItem("TX");
      if (!strVal.empty() && strVal!=_taxCode)
        diag.deActivate();
    }
    diag << "***\n***DU1 START TOTAL AMOUNT CALCULATION***\n***\n";

    MoneyAmount totalFare = TaxRange::retrieveTotalFare(trx, taxResponse, farePath, targetCurrencyCode);
    diag << "FARE AMOUNT-" << totalFare << "USD\n";

    for(const TaxItem* taxItem : taxResponse.taxItemVector())
      totalFare += getYqYrAmount(trx, taxItem, diag);

    if (TrxUtil::isShoppingTaxRequest(&trx) && taxResponse.farePath() &&
        !taxResponse.farePath()->getExternalTaxes().empty())
    {
      for(const TaxItem* taxItem : taxResponse.farePath()->getExternalTaxes())
        totalFare += getYqYrAmount(trx, taxItem, diag);
    }

    diag << "TOTAL AMOUNT-" << totalFare << "USD\n";

    totalFare = TaxRound().applyTaxRound(totalFare, targetCurrencyCode, 1, NEAREST);
    diag << "ROUNDED TOTAL AMOUNT-" << totalFare << "USD\n";

    diag << "***\n***DU1 END TOTAL AMOUNT CALCULATION***\n***\n";

    return totalFare;
  };

  //calculate USD amount from YQ/YR
  MoneyAmount getYqYrAmount(PricingTrx& trx, const TaxItem* taxItem, DiagManager& diag) const
  {
    if (!taxItem || taxItem->taxCode().length()<2 || taxItem->taxCode()[0] != 'Y')
      return 0;

    if (taxItem->taxCode()[1] != 'Q' && taxItem->taxCode()[1] != 'R')
      return 0;

    if (taxItem->taxAmount() < EPSILON)
      return 0;

    MoneyAmount res = taxUtil::convertCurrency(trx, taxItem->taxAmount(), targetCurrencyCode,
        taxItem->paymentCurrency(), taxItem->paymentCurrency(), CurrencyConversionRequest::TAXES, false);

    if (diag.isActive())
    {
      diag << "TAX-" << taxItem->taxCode() << " " << taxItem->taxAmount() << taxItem->paymentCurrency() <<
          " CONVERTED TO " << res << targetCurrencyCode << "\n";
    }
    return res;
  }

};//class TaxRangeDU1


bool
TaxDU1_00::validateRange(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)

{
  return TaxRangeDU1(taxCodeReg.taxCode()).
      validateRange(trx, taxResponse, taxCodeReg, startIndex, endIndex);
}


