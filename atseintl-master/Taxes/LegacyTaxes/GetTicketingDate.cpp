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
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyTaxes/GetTicketingDate.h"

namespace tse
{

GetTicketingDate::GetTicketingDate(PricingTrx& trx) : _date(trx.getRequest()->ticketingDT())
{
  ItinSelector itinSelector(trx);
  if (itinSelector.isNewItin() && itinSelector.isRefundTrx() && TrxUtil::isAutomatedRefundCat33Enabled(trx))
  {
    const BaseExchangeTrx* exchangeTrx = dynamic_cast<const BaseExchangeTrx*>(&trx);
    if (exchangeTrx && exchangeTrx->currentTicketingDT().isValid())
    {
      _date = exchangeTrx->currentTicketingDT();
    }
  }
}

const DateTime&
GetTicketingDate::get() const
{
  return _date;
}

} // end of tse namespace
