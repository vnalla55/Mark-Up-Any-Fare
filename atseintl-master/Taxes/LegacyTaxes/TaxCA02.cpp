// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2010
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
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/TaxCA02.h"

using namespace tse;


// ----------------------------------------------------------------------------
// Description:  AdjustTax
//               'Up a penny' to the MaxTax amount for the second enplanement
//                of continuous journey
// ----------------------------------------------------------------------------
void
TaxCA02::adjustTax(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  if (itinAnalyzer.correctFlag)
  {
    MoneyAmount maxTax = taxUtil::convertCurrency(trx, taxCodeReg.maxTax(), _paymentCurrency,
        taxCodeReg.taxCur(), taxCodeReg.taxCur(), CurrencyConversionRequest::TAXES, false);

    _taxAmount = maxTax - _taxAmount;
    itinAnalyzer.correctFlag = false;
  }
}
