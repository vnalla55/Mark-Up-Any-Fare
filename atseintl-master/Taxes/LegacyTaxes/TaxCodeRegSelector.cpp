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

#include "Common/TrxUtil.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyTaxes/TaxCodeRegSelector.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

namespace tse
{
std::vector<TaxCodeReg*>*
TaxCodeRegSelector::selectVectorNewItin(PricingTrx& trx, TaxCodeReg* taxCodeReg)
{
  if (utc::isTaxOnOC(trx, *taxCodeReg))
  {
    return &_ocTaxCodeReg;
  }
  else if (utc::isTaxOnChangeFee(trx, *taxCodeReg, trx.ticketingDate()))
  {
    return &_changeFeeTaxCodeReg;
  }
  else
  {
    return &_itinTaxCodeReg;
  }
}

std::vector<TaxCodeReg*>*
TaxCodeRegSelector::selectVectorExcItin(PricingTrx& trx, TaxCodeReg* taxCodeReg)
{
  if (!utc::isTaxOnOC(trx, *taxCodeReg) &&
      !utc::isTaxOnChangeFee(trx, *taxCodeReg, trx.ticketingDate()))
  {
    return &_itinTaxCodeReg;
  }
  else
  {
    return nullptr;
  }
}

TaxCodeRegSelector::TaxCodeRegSelector(PricingTrx& trx, std::vector<TaxCodeReg*> taxCodeRegVector)
{
  if (ItinSelector(trx).isNewItin())
  {
    for (TaxCodeReg* taxCodeReg : taxCodeRegVector)
    {
      if ( UNLIKELY( !TaxDiagnostic::isValidSeqNo(trx, taxCodeReg->seqNo()) ||
                     TaxDiagnostic::printSeqDef(trx, *taxCodeReg)) )
        continue;

      if (auto vector = selectVectorNewItin(trx, taxCodeReg))
        vector->push_back(taxCodeReg);
    }
  }
  else if (TrxUtil::isAutomatedRefundCat33Enabled(trx))
  {
    for (TaxCodeReg* taxCodeReg : taxCodeRegVector)
    {
      if ( UNLIKELY( !TaxDiagnostic::isValidSeqNo(trx, taxCodeReg->seqNo()) ||
                     TaxDiagnostic::printSeqDef(trx, *taxCodeReg)) )
        continue;

      if (auto vector = selectVectorExcItin(trx, taxCodeReg))
        vector->push_back(taxCodeReg);
    }
  }
}

} // end of tse namespace
