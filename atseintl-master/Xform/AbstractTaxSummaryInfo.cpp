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

#include "Xform/AbstractTaxSummaryInfo.h"
#include "Xform/ExemptTaxSummaryInfo.h"
#include "Xform/TaxSummaryInfo.h"

#include <memory>

namespace tse
{
std::shared_ptr<AbstractTaxSummaryInfo>
AbstractTaxSummaryInfo::create(CalcTotals& calcTotals, const TaxRecord& taxRecord, PricingTrx& pricingTrx)
{
  if (taxRecord.isTaxFeeExempt())
  {
    return std::shared_ptr<AbstractTaxSummaryInfo>(new ExemptTaxSummaryInfo(calcTotals, taxRecord, pricingTrx));
  }
  else
  {
    return std::shared_ptr<AbstractTaxSummaryInfo>(new TaxSummaryInfo(calcTotals, taxRecord, pricingTrx));
  }
}

} // end of tse
