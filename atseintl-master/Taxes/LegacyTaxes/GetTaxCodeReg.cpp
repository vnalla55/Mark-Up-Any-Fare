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

#include "Common/FallbackUtil.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyTaxes/GetTaxCodeReg.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

namespace tse
{

FALLBACK_DECL(cat33FixTaxesOnChangeFeeAF);

std::unique_ptr<GetTaxCodeReg>
GetTaxCodeReg::create(PricingTrx& trx, const TaxCode& taxCode, std::list<TaxCode>* pChangeFeeTaxes)
{

  if (fallback::cat33FixTaxesOnChangeFeeAF(&trx))
  {
    ItinSelector itinSelector(trx);
    if (itinSelector.isRefundTrx() && itinSelector.isNewItin()  && itinSelector.isCat33FullRefund())
    {
      return std::make_unique<Cat33GetTaxCodeReg>(trx, taxCode);
    }
  }

  if (pChangeFeeTaxes && trx.isExchangeTrx())
  {
    auto it = std::find(pChangeFeeTaxes->begin(), pChangeFeeTaxes->end(), taxCode);
    if (it != pChangeFeeTaxes->end())
    {
      pChangeFeeTaxes->erase(it);
      return std::make_unique<CanadaGetTaxCodeReg>(trx, taxCode);
    }
  }

  return std::make_unique<DefaultGetTaxCodeReg>(trx, taxCode);
}

DefaultGetTaxCodeReg::DefaultGetTaxCodeReg(PricingTrx& trx, const TaxCode& taxCode)
{
  _taxCodeReg =
      &trx.dataHandle().getTaxCode(taxCode, trx.getRequest()->ticketingDT());
  _ticketDate = trx.dataHandle().ticketDate();
}

CanadaGetTaxCodeReg::CanadaGetTaxCodeReg(PricingTrx& trx, const TaxCode& taxCode)
{
  const BaseExchangeTrx* exchangeTrx = dynamic_cast<const BaseExchangeTrx*>(&trx);
  if (exchangeTrx && exchangeTrx->currentTicketingDT().isValid())
  {
    DateTime originalTicketDate = trx.dataHandle().ticketDate();
    trx.dataHandle().setTicketDate(exchangeTrx->currentTicketingDT());
    const std::vector<TaxCodeReg*>* taxCodeRegNew =
      &trx.dataHandle().getTaxCode(taxCode, exchangeTrx->currentTicketingDT());
    trx.dataHandle().setTicketDate(originalTicketDate);

    if (taxCodeRegNew && !taxCodeRegNew->empty() &&
      utc::isMatchOrigTicket(trx, *taxCodeRegNew->front(), exchangeTrx->currentTicketingDT()))
    {
      _taxCodeReg = taxCodeRegNew;
      _ticketDate = exchangeTrx->currentTicketingDT();
    }
  }

  if (!_taxCodeReg)
  {
    DefaultGetTaxCodeReg getTaxCodeReg(trx, taxCode);
    _taxCodeReg = getTaxCodeReg.taxCodeReg();
    _ticketDate = getTaxCodeReg.ticketingDate();
  }
}

Cat33GetTaxCodeReg::Cat33GetTaxCodeReg(PricingTrx& trx, const TaxCode& taxCode)
{
  ItinSelector itinSelector(trx);
  if (itinSelector.isRefundTrx() && itinSelector.isNewItin()  && itinSelector.isCat33FullRefund())
  {
    DateTime originalTicketDate = trx.dataHandle().ticketDate();
    const BaseExchangeTrx* exchangeTrx = dynamic_cast<const BaseExchangeTrx*>(&trx);
    _ticketDate = exchangeTrx->currentTicketingDT();
    trx.dataHandle().setTicketDate(_ticketDate);
    _taxCodeReg = &trx.dataHandle().getTaxCode(taxCode, _ticketDate);
    trx.dataHandle().setTicketDate(originalTicketDate);
  }
  else
  {
    DefaultGetTaxCodeReg getTaxCodeReg(trx, taxCode);
    _taxCodeReg = getTaxCodeReg.taxCodeReg();
    _ticketDate = getTaxCodeReg.ticketingDate();
  }
}

} // end of tse namespace
