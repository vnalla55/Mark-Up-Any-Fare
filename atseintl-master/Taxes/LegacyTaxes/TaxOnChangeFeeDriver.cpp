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

#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxNation.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyTaxes/GetTaxCodeRegAdapter.h"
#include "Taxes/LegacyTaxes/GetTaxNation.h"
#include "Taxes/LegacyTaxes/GetTicketingDate.h"
#include "Taxes/LegacyTaxes/TaxOnChangeFeeConfig.h"
#include "Taxes/LegacyTaxes/TaxOnChangeFeeDriver.h"

namespace tse
{

TaxOnChangeFeeDriver::TaxOnChangeFeeDriver(PricingTrx& trx, AbstractGetTaxNation& getTaxNation,
    AbstractGetTaxCodeReg& getTaxCodeReg, TaxOnChangeFeeConfig& taxOnChangeFeeConfig)
{
  ItinSelector itinSelector(trx);
  if (!itinSelector.isRefundTrx() || !itinSelector.isNewItin())
    return;

  auto isTaxOnChangeFee = [&](TaxCodeReg* taxCodeReg) -> bool
  {
    return taxOnChangeFeeConfig.isTaxOnChangeFee(taxCodeReg);
  };

  for(const TaxNation* taxNation : getTaxNation.get())
  {
    for(const TaxCode& taxCode : taxNation->taxCodeOrder())
    {
      const std::vector<TaxCodeReg*>* taxCodeRegPerTaxCode = getTaxCodeReg.get(taxCode);
      std::copy_if(taxCodeRegPerTaxCode->begin(), taxCodeRegPerTaxCode->end(),
          std::back_inserter(_taxCodeReg), isTaxOnChangeFee);
    }
  }
}

const std::vector<TaxCodeReg*>&
TaxOnChangeFeeDriver::getTaxCodeReg() const
{
  return _taxCodeReg;
}

} // end of tse namespace
